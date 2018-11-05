/******************************************************************************
*
* Copyright (C) 2014-2017 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsudma_intr_example.c
*
* This file contains a design example using the XCsuDma driver in interrupt
* mode. It sends data and expects to receive the same data through the device
* using the local loop back mode.
*
* @note
* The example contains an infinite loop such that if interrupts are not
* working it may hang.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vnsld  22/10/14 First release
* 1.2   adk    11/22/17 Added peripheral test app support.
* 1.2	adk    11/01/18 Declared static array rather than hard code memory for
*			buffers.
*	adk    18/01/18 Remove unnecessary column in XIntc_Connect() API.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xparameters.h"
#include "xil_exception.h"
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

/************************** Function Prototypes ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CSUDMA_DEVICE_ID  XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTG_INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define INTG_CSUDMA_INTR_DEVICE_ID XPAR_INTC_0_CSUDMA_0_VEC_ID /**< ZDMA Interrupt Id */
#else
#define INTC		XScuGic
#define INTG_INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTG_CSUDMA_INTR_DEVICE_ID 	XPAR_XCSUDMA_INTR /**< Interrupt device ID
						 *  of CSU DMA device ID */
#endif

#define CSU_SSS_CONFIG_OFFSET	0x008		/**< CSU SSS_CFG Offset */
#define CSUDMA_LOOPBACK_CFG	0x00000050	/**< LOOP BACK configuration
						  *  macro */
#define SIZE		0x100		/**< Size of the data to be
					  *  transfered */

#if defined(__ICCARM__)
    #pragma data_alignment = 64
	u32 DstBuf[SIZE]; /**< Destination buffer */
	u32 SrcBuf[SIZE]; /**< Source buffer */
	#pragma data_alignment = 4
#else
u32 DstBuf[SIZE] __attribute__ ((aligned (64)));	/**< Destination buffer */
u32 SrcBuf[SIZE] __attribute__ ((aligned (64)));	/**< Source buffer */
#endif


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XCsuDma_IntrExample(INTC *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId);
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XCsuDma *CsuDmaInstance,
				u16 CsuDmaIntrId);
void IntrHandler(void *CallBackRef);

static void SrcHandler(void *CallBackRef, u32 Event);
static void DstHandler(void *CallBackRef, u32 Event);

/************************** Variable Definitions *****************************/

#ifndef TESTAPP_GEN
XCsuDma CsuDma;		/**<Instance of the Csu_Dma Device */
static INTC Intc;	/* Instance of the Interrupt Controller */
#endif
u32 DstDone = 0;

#ifndef TESTAPP_GEN
/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the selftest example */
	Status = XCsuDma_IntrExample(&Intc, &CsuDma, (u16)CSUDMA_DEVICE_ID,
				     INTG_CSUDMA_INTR_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CSU_DMA Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CSU_DMA Interrupt Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function performs data transfer in loop back mode in interrupt mode
* and verify the data.
*
* @param	DeviceId is the XPAR_<CSUDMA Instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XCsuDma_IntrExample(INTC *IntcInstancePtr, XCsuDma *CsuDmaInstance,
			u16 DeviceId, u16 IntrId)
{
	int Status;
	XCsuDma_Config *Config;
	u32 Index = 0;
	u32 *SrcPtr = SrcBuf;
	u32 *DstPtr = DstBuf;
	u32 Test_Data = 0xABCD1234;
	u32 *Ptr = SrcBuf;
	u32 EnLast = 0;
	/*
	 * Initialize the CsuDma driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCsuDma_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XCsuDma_CfgInitialize(CsuDmaInstance, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(CsuDmaInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller.
	 */
	Status = SetupInterruptSystem(IntcInstancePtr, CsuDmaInstance,
				      IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Enable interrupts */
	XCsuDma_EnableIntr(CsuDmaInstance, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	/*
	 * Setting CSU_DMA in loop back mode.
	 */

	Xil_Out32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET,
		(Xil_In32(XCSU_BASEADDRESS + CSU_SSS_CONFIG_OFFSET) |
						CSUDMA_LOOPBACK_CFG));

	/* Data writing at source address location */

	for(Index = 0; Index < SIZE; Index++)
	{
		*Ptr = Test_Data;
		Test_Data += 0x1;
		Ptr++;
	}

	/* Data transfer in loop back mode */
	XCsuDma_Transfer(CsuDmaInstance, XCSUDMA_DST_CHANNEL, (UINTPTR)DstBuf, SIZE, EnLast);
	XCsuDma_Transfer(CsuDmaInstance, XCSUDMA_SRC_CHANNEL, (UINTPTR)SrcBuf, SIZE, EnLast);

	/* Wait for generation of destination work is done */
	while(DstDone == 0);
	/* Disable interrupts */
	XCsuDma_DisableIntr(CsuDmaInstance, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(CsuDmaInstance, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(CsuDmaInstance, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	/*
	 * Verifying data of transfered by comparing data at
	 * source and address locations.
	 */

	for (Index = 0; Index < SIZE; Index++) {
		if (*SrcPtr != *DstPtr) {
			return XST_FAILURE;
		}
		else {
			SrcPtr++;
			DstPtr++;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function sets up the interrupt system so interrupts can occur for the
* CSU DMA. This function is application-specific. The user should modify this
* function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	InstancePtr contains a pointer to the instance of the CSU DMA
*		driver which is going to be connected to the interrupt
*		controller.
* @param	IntrId is the interrupt Id and is typically
*		XPAR_<CSUDMA_instance>_INTR value from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.

*
****************************************************************************/
static int SetupInterruptSystem(INTC *IntcInstancePtr,
				XCsuDma *InstancePtr,
				u16 IntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTG_INTC_DEVICE_ID);
	if (Status != XST_SUCCESS)
	{

		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}
#endif
	/*
	 * Connect the driver interrupt handler
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)IntrHandler, InstancePtr);
	if (Status != XST_SUCCESS)
	{

		xil_printf("Failed connect intc\r\n");
		return XST_FAILURE;
	}
#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupt for the CSUDMA device.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);
#ifndef TESTAPP_GEN
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				(void *)IntcInstancePtr);
#endif
#else
#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig; /* Config for interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTG_INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
				IntcInstancePtr);
#endif

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
			(Xil_ExceptionHandler) IntrHandler,
				  (void *) InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the device
	 */
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif
#ifndef TESTAPP_GEN


	/*
	 * Enable interrupts
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
#endif


	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* This function is the interrupt handler for the CSU_DMA driver.
*
* This handler reads the interrupt status from the Status register, determines
* the source of the interrupts, calls according callbacks, and finally clears
* the interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void IntrHandler(void *CallBackRef)
{
	u32 SrcPending;
	u32 DstPending;
	XCsuDma *XCsuDmaPtr = NULL;
	XCsuDmaPtr = (XCsuDma *)((void *)CallBackRef);

	/* Handling interrupt */

	/* Getting pending interrupts of source */
	SrcPending = XCsuDma_IntrGetStatus(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL);
	XCsuDma_IntrClear(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL, SrcPending);
	SrcPending &= (~XCsuDma_GetIntrMask(XCsuDmaPtr, XCSUDMA_SRC_CHANNEL));

	/* Getting pending interrupts of destination */
	DstPending = XCsuDma_IntrGetStatus(XCsuDmaPtr, XCSUDMA_DST_CHANNEL);
	XCsuDma_IntrClear(XCsuDmaPtr, XCSUDMA_DST_CHANNEL, DstPending);
	DstPending &= (~XCsuDma_GetIntrMask(XCsuDmaPtr, XCSUDMA_DST_CHANNEL));


	if (SrcPending != 0x00) {
		SrcHandler(XCsuDmaPtr, SrcPending);
	}

	if (DstPending != 0x00) {
		DstHandler(XCsuDmaPtr, DstPending);
	}
}

/*****************************************************************************/
/**
* This is static function which handlers source channel interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occured.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SrcHandler(void *CallBackRef, u32 Event)
{
	if (Event & XCSUDMA_IXR_INVALID_APB_MASK) {
		/*
		 * Code to handle Invalid APB access
		 * Interrupt should be put here.
		 */
	}

	if (Event & XCSUDMA_IXR_FIFO_THRESHHIT_MASK) {
		/*
		 * Code to handle FIFO Threshold hit
		 * Interrupt should be put here.
		 */
	}

	if (Event & (XCSUDMA_IXR_TIMEOUT_MEM_MASK |
				XCSUDMA_IXR_TIMEOUT_STRM_MASK)) {
		/*
		 * Code to handle Timeout
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_AXI_WRERR_MASK) {
		/*
		 * Code to handle AXI read error
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_DONE_MASK) {
		/*
		 * Code to handle Done
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_MEM_DONE_MASK) {
		/*
		 * Code to handle Memory done
		 * Interrupt should be put here.
		 */
	}
}

/*****************************************************************************/
/**
* This static function handles destination channel interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occured.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DstHandler(void *CallBackRef, u32 Event)
{
	if (Event & XCSUDMA_IXR_FIFO_OVERFLOW_MASK) {
		/*
		 * Code to handle FIFO overflow
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_INVALID_APB_MASK) {
		/*
		 * Code to handle Invalid APB access
		 * Interrupt should be put here.
		 */
	}

	if (Event & XCSUDMA_IXR_FIFO_THRESHHIT_MASK) {
		/*
		 * Code to handle FIFO Threshold hit
		 * Interrupt should be put here.
		 */
	}

	if (Event & (XCSUDMA_IXR_TIMEOUT_MEM_MASK |
				XCSUDMA_IXR_TIMEOUT_STRM_MASK)) {
		/*
		 * Code to handle Time out memory or stream
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_AXI_WRERR_MASK) {
		/*
		 * Code to handle AXI read error
		 * Interrupt should be put here.
		 */
	}
	if (Event & XCSUDMA_IXR_DONE_MASK) {

		DstDone = 1;
	}
}
