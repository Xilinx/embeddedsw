/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"

/************************** Function Prototypes ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CSUDMA_DEVICE_ID  XPAR_XCSUDMA_0_DEVICE_ID /* CSU DMA device Id */
#define INTG_INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTG_CSUDMA_INTR_DEVICE_ID 	XPAR_XCSUDMA_INTR /**< Interrupt device ID
						 *  of CSU DMA device ID */

#define CSU_SSS_CONFIG_OFFSET	0x008		/**< CSU SSS_CFG Offset */
#define CSUDMA_LOOPBACK_CFG	0x00000050	/**< LOOP BACK configuration
						  *  macro */
#define SRC_ADDR	0x04200000	/**< Source Address */
#define DST_ADDR	0x04300000	/**< Destination Address */
#define SIZE		0x100		/**< Size of the data to be
					  *  transfered */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int XCsuDma_IntrExample(u16 DeviceId);
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XCsuDma *CsuDmaInstance,
				u16 CsuDmaIntrId);
void IntrHandler(void *CallBackRef);

static void SrcHandler(void *CallBackRef, u32 Event);
static void DstHandler(void *CallBackRef, u32 Event);

/************************** Variable Definitions *****************************/

XCsuDma CsuDma;		/**<Instance of the Csu_Dma Device */
u32 DstDone = 0;
XScuGic Intc;	/* Instance of the Interrupt Controller */

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
	Status = XCsuDma_IntrExample((u16)CSUDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CSU_DMA Interrupt Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran CSU_DMA Interrupt Example\r\n");
	return XST_SUCCESS;
}

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
int XCsuDma_IntrExample(u16 DeviceId)
{
	int Status;
	XCsuDma_Config *Config;
	u32 Index = 0;
	u32 *SrcPtr = (u32 *)SRC_ADDR;
	u32 *DstPtr = (u32 *)DST_ADDR;
	u32 Test_Data = 0xABCD1234;
	u32 *Ptr = (u32 *)SRC_ADDR;
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

	Status = XCsuDma_CfgInitialize(&CsuDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCsuDma_SelfTest(&CsuDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller.
	 */
	Status = SetupInterruptSystem(&Intc, &CsuDma,
				INTG_CSUDMA_INTR_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Enable interrupts */
	XCsuDma_EnableIntr(&CsuDma, XCSUDMA_DST_CHANNEL,
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
	XCsuDma_Transfer(&CsuDma, XCSUDMA_DST_CHANNEL, DST_ADDR, SIZE, EnLast);
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, SRC_ADDR, SIZE, EnLast);

	/* Wait for generation of destination work is done */
	while(DstDone == 0);
	/* Disable interrupts */
	XCsuDma_DisableIntr(&CsuDma, XCSUDMA_DST_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
	/* To acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_DST_CHANNEL, XCSUDMA_IXR_DONE_MASK);

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
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XCsuDma *InstancePtr,
				u16 IntrId)
{
	int Status;

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


	/*
	 * Enable interrupts
	 */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);


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
