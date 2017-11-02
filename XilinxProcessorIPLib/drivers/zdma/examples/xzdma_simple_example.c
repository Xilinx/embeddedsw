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
* @file xzdma_simple_example.c
*
* This file contains the example using XZDma driver to do simple data transfer
* in Normal mode on ZDMA device for 1MB data transfer.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
*       vns    10/13/15  Declared static array rather than hard code memory.
*       ms     04/05/17  Modified comment lines notation in functions to
*                        avoid unnecessary description to get displayed
*                        while generating doxygen.
* 1.3   mus    08/14/17  Do not perform cache operations if CCI is enabled
* 1.4   adk    11/02/17  Updated example to fix compilation errors for IAR
*			 compiler.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xil_cache.h"

/************************** Function Prototypes ******************************/

int XZDma_SimpleExample(u16 DeviceId);
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XZDma *InstancePtr, u16 IntrId);
static void DoneHandler(void *CallBackRef);

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define ZDMA_DEVICE_ID		XPAR_XZDMA_0_DEVICE_ID /* ZDMA device Id */
#define ZDMA_INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
					/**< SCUGIC Device ID */
#define ZDMA_INTR_DEVICE_ID	XPAR_XADMAPS_0_INTR	/**< ZDMA Interrupt Id */

#define SIZE		1000000 /**< Size of the data to be transferred */
#define TESTVALUE	0x1230 /**< For writing into source buffer */

/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
XScuGic Intc;		/**< XIntc Instance */

#if defined(__ICCARM__)
    #pragma data_alignment = 64
	u8 DstBuf[SIZE]; /**< Destination buffer */
	u8 SrcBuf[SIZE]; /**< Source buffer */
	#pragma data_alignment = 4
#else
u8 DstBuf[SIZE] __attribute__ ((aligned (64)));	/**< Destination buffer */
u8 SrcBuf[SIZE] __attribute__ ((aligned (64)));	/**< Source buffer */
#endif

u8 Done = 0;				/**< Done flag */

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

	/* Run the simple example */
	Status = XZDma_SimpleExample((u16)ZDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Simple Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ZDMA Simple Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in simple mode of normal mode
* on the ZDMA driver.
*
* @param	DeviceId is the XPAR_<ZDMA Instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int XZDma_SimpleExample(u16 DeviceId)
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configure; /* Configuration values */
	XZDma_Transfer Data;
	u32 Index;
	u32 Value;

	/*
	 * Initialize the ZDMA driver so that it's ready to use.
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XZDma_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XZDma_CfgInitialize(&ZDma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XZDma_SelfTest(&ZDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Filling the buffer for data transfer */
	Value = TESTVALUE;
	for (Index = 0; Index < SIZE/4; Index++) {
		*(SrcBuf +Index) = Value++;
	}
	/*
	 * Invalidating destination address and flushing
	 * source address in cache
	 */
	if (!Config->IsCacheCoherent) {
	Xil_DCacheFlushRange((INTPTR)SrcBuf, SIZE);
	Xil_DCacheInvalidateRange((INTPTR)DstBuf, SIZE);
	}

	/* ZDMA has set in simple transfer of Normal mode */
	Status = XZDma_SetMode(&ZDma, FALSE, XZDMA_NORMAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Interrupt call back has been set */
	XZDma_SetCallBack(&ZDma, XZDMA_HANDLER_DONE,
				(void *)DoneHandler, &ZDma);
	/*
	 * Connect to the interrupt controller.
	 */
	Status = SetupInterruptSystem(&Intc, &(ZDma),
			ZDMA_INTR_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Enable required interrupts */
	XZDma_EnableIntr(&ZDma, (XZDMA_IXR_DMA_DONE_MASK));

	/* Configuration settings */
	Configure.OverFetch = 1;
	Configure.SrcIssue = 0x1F;
	Configure.SrcBurstType = XZDMA_INCR_BURST;
	Configure.SrcBurstLen = 0xF;
	Configure.DstBurstType = XZDMA_INCR_BURST;
	Configure.DstBurstLen = 0xF;
	Configure.SrcCache = 0x2;
	Configure.DstCache = 0x2;
	if (Config->IsCacheCoherent) {
		Configure.SrcCache = 0xF;
		Configure.DstCache = 0xF;
	}
	Configure.SrcQos = 0;
	Configure.DstQos = 0;

	XZDma_SetChDataConfig(&ZDma, &Configure);
	/*
	 * Transfer elements
	 */
	Data.DstAddr = (UINTPTR)DstBuf;
	Data.DstCoherent = 1;
	Data.Pause = 0;
	Data.SrcAddr = (UINTPTR)SrcBuf;
	Data.SrcCoherent = 1;
	Data.Size = SIZE; /* Size in bytes */

	XZDma_Start(&ZDma, &Data, 1); /* Initiates the data transfer */

	while (Done == 0); /* Wait till DMA done interrupt generated */

	/* Checking the data transferred */
	for (Index = 0; Index < SIZE/4; Index++) {
		if (SrcBuf[Index] != DstBuf[Index]) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function sets up the interrupt system so interrupts can occur for the
* ZDMA. This function is application-specific. The user should modify this
* function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	InstancePtr contains a pointer to the instance of the ZDMA
*		driver which is going to be connected to the interrupt
*		controller.
* @param	IntrId is the interrupt Id and is typically
*		XPAR_<ZDMA_instance>_INTR value from xparameters.h.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if failed
*
* @note		None.

*
****************************************************************************/
int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XZDma *InstancePtr,
				u16 IntrId)
{
	int Status;

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig; /* Config for interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(ZDMA_INTC_DEVICE_ID);
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
			(Xil_ExceptionHandler) XZDma_IntrHandler,
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
* This static function handles ZDMA Done interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DoneHandler(void *CallBackRef)
{
	Done = 1;

}
