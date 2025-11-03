/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_writeonlymode_example.c
*
* This file contains the example using XZDma driver to do simple data transfer
* in Write only mode on ZDMA device. In this mode data will be predefined
* and will be repetitively written into the given address and for given size.
* For ADMA only 2 words are repeated and for GDMA 4 words are repeated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
*       ms      04/05/17 Modified comment lines notation in functions to
*                        avoid unnecessary description to get displayed
*                        while generating doxygen.
* 1.3   mus    08/14/17  Do not perform cache operations if CCI is enabled
* 1.4   adk    11/02/17  Updated example to fix compilation errors for IAR
*			 compiler.
* 1.7   adk    18/03/19  Update the example data verification check to support
*			 versal adma IP.
* 1.7   adk    21/03/19  Fix alignment pragmas in the example for IAR compiler.
* 1.13	sk     08/02/21	 Make Done variable as volatile to fix failure at
* 			 optimization level 2.
* 1.13  asa    08/24/21  Make changes to add a missing data invalidation
*                        operation just before the destination buffer data is
*                        being read.
*                        Changes were done for other cleanups and also to
*                        ensure that the DMA is reset before the program exits.
* 1.16  sa     09/29/22  Fix infinite loops in the example.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"
#ifndef SDT
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif
#include "xil_util.h"
#include "xplatform_info.h"

/************************** Function Prototypes ******************************/

#ifndef SDT
int XZDma_WriteOnlyExample(u16 DeviceId);
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XZDma *InstancePtr, u16 IntrId);
#else
int XZDma_WriteOnlyExample(UINTPTR BaseAddress);
#endif
static void DoneHandler(void *CallBackRef);

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define ZDMA_DEVICE_ID		XPAR_XZDMA_0_DEVICE_ID /* ZDMA device Id */
#define ZDMA_INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
/**< SCUGIC Device ID */
#define ZDMA_INTR_DEVICE_ID	XPAR_XADMAPS_0_INTR /**< ZDMA Interrupt Id */
#endif

#define SIZE			1024 /* Size of the data to be written */

#define POLL_TIMEOUT_COUNTER    1000000U
#define NUM_OF_EVENTS           1
/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
#ifndef SDT
XScuGic Intc;		/**< XIntc Instance */
#endif
u32 SrcBuf[4];		/**< Source buffer */
#if defined(__ICCARM__)
#pragma data_alignment = 64
u32 DstBuf[300]; /**< Destination buffer */
#else
u32 DstBuf[300] __attribute__ ((aligned (64))); /**< Destination buffer */
#endif
static volatile u32 Done = 0;	/**< Done Flag for interrupt generation */

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

	/* Run the simple write only mode example */
#ifndef SDT
	Status = XZDma_WriteOnlyExample((u16)ZDMA_DEVICE_ID);
#else
	Status = XZDma_WriteOnlyExample(XPAR_XZDMA_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Write Only mode ZDMA Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in simple mode of write only
* mode on the ZDMA driver.
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
#ifndef SDT
int XZDma_WriteOnlyExample(u16 DeviceId)
#else
int XZDma_WriteOnlyExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configur; /* Configuration values */
	XZDma_Transfer Data;
	u32 *Buf = (u32 *)DstBuf;
	u32 Index;
	u32 Index1;

	/*
	 * Initialize the ZDMA driver so that it's ready to use.
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
#ifndef SDT
	Config = XZDma_LookupConfig(DeviceId);
#else
	Config = XZDma_LookupConfig(BaseAddress);
#endif
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

	/* ZDMA has set in simple transfer of Normal mode */
	Status = XZDma_SetMode(&ZDma, FALSE, XZDMA_WRONLY_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect to the interrupt controller.
	 */
#ifndef SDT
	Status = SetupInterruptSystem(&Intc, &(ZDma),
				      ZDMA_INTR_DEVICE_ID);
#else
	Status = XSetupInterruptSystem(&ZDma, &XZDma_IntrHandler,
				       Config->IntrId, Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
#endif
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	(void)XZDma_SetCallBack(&ZDma,
				XZDMA_HANDLER_DONE, (void *)(DoneHandler),
				&ZDma);
	/* Configuration settings */
	Configur.OverFetch = 0;
	Configur.SrcIssue = 0x1F;
	Configur.SrcBurstType = XZDMA_INCR_BURST;
	Configur.SrcBurstLen = 0xF;
	Configur.DstBurstType = XZDMA_INCR_BURST;
	Configur.DstBurstLen = 0xF;
	if (Config->IsCacheCoherent) {
		Configur.SrcCache = 0xF;
		Configur.DstCache = 0xF;
	}
	XZDma_SetChDataConfig(&ZDma, &Configur);
	/*
	 * Transfer elements
	 */
	Data.DstAddr = (UINTPTR)DstBuf;
	Data.DstCoherent = 0;
	Data.Pause = 0;
	Data.SrcAddr = (UINTPTR)NULL;
	Data.SrcCoherent = 0;
	Data.Size = SIZE; /* Size in bytes */
	if (Config->IsCacheCoherent) {
		Data.DstCoherent = 1;
		Data.SrcCoherent = 1;
	}

	if (ZDma.Config.DmaType == 0) { /* For GDMA */
		SrcBuf[0] = 0x1234;
		SrcBuf[1] = 0xABCD;
		SrcBuf[2] = 0x4567;
		SrcBuf[3] = 0xEF;
		XZDma_WOData(&ZDma, SrcBuf);
	} else { /* For ADMA */
		SrcBuf[0] = 0x1234;
		SrcBuf[1] = 0xABCD;
		XZDma_WOData(&ZDma, SrcBuf);
	}

	if(XIOCoherencySupported())
	{
		if (!Config->IsCacheCoherent) {
			Xil_DCacheFlushRange((INTPTR)DstBuf, SIZE);
		}
	}
	else
	{
		Xil_DCacheFlushRange((INTPTR)DstBuf, SIZE);
	}

	XZDma_EnableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);

	XZDma_Start(&ZDma, &Data, 1); /* Initiates the data transfer */

	/* Wait till DMA destination done interrupt generated or timeout */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUM_OF_EVENTS, &Done);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XZDma_DisableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);

	/* Before the destination buffer data is accessed do one more invalidation
	 * to ensure that the latest data is read. This is as per ARM recommendations.
	 */
	if(XIOCoherencySupported())
	{
		if (!Config->IsCacheCoherent) {
			Xil_DCacheInvalidateRange((INTPTR)DstBuf, SIZE);
		}
	}
	else
	{
		Xil_DCacheInvalidateRange((INTPTR)DstBuf, SIZE);
	}
	/* Validation */
	if (ZDma.Config.DmaType == 0) { /* For GDMA */
		for (Index = 0; Index < (SIZE / 4) / 4; Index++) {
			for (Index1 = 0; Index1 < 4; Index1++) {
				if (SrcBuf[Index1] != *Buf++) {
					return XST_FAILURE;
				}
			}
		}
	} else { /* For ADMA */
#ifdef versal
		for (Index = 0; Index < (SIZE / 4) / 4; Index++) {
			for (Index1 = 0; Index1 < 4; Index1++) {
#else
		for (Index = 0; Index < (SIZE / 4) / 2; Index++) {
			for (Index1 = 0; Index1 < 2; Index1++) {
#endif
				if (SrcBuf[Index1] != *Buf++) {
					return XST_FAILURE;
				}
			}
		}
	}

	/* Reset the DMA to remove all configurations done in this example  */
	XZDma_Reset(&ZDma);

	return XST_SUCCESS;

}
#ifndef SDT
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
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XZDma *InstancePtr, u16 IntrId)
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
#endif
/*****************************************************************************/
/**
* This static function handles ZDMA Done interrupts.
*
* @param	CallBackRef is the callback reference passed from the interrupt
*		handler, which in our case is a pointer to the driver instance.
* @param	Event specifies which interrupts were occurred.
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
