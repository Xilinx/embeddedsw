/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_linear_example.c
*
* This file contains the example using XZDma driver to do data transfer in
* Linear mode on ZDMA device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   vns     2/27/15  First release
*       ms      04/05/17 Modified comment lines notation in functions to
*                        avoid unnecessary description to get displayed
*                        while generating doxygen.
* 1.3   mus    08/14/17  Do not perform cache operations if CCI is enabled
* 1.4   adk    11/02/17  Updated example to fix compilation errors for IAR
*			 compiler.
* 1.7   adk    21/03/19  Fix data alignment in the example for IAR compiler.
* 1.13	sk     08/02/21	 Make Done, Pause variables as volatile to fix failure
* 			 at optimization level 2.
* 1.13  asa    08/24/21  Make changes to add a missing data invalidation
*                        operation just before the destination buffer data is
*                        being read. Along with the changes were done to ensure
*                        proper sizes for linear SG buffers.
*                        Changes were done to allocate the buffers with sizes
*                        which are in sync with other examples.
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

/************************** Function Prototypes ******************************/

#ifndef SDT
int XZDma_LinearExample(u16 DeviceId);
static int SetupInterruptSystem(XScuGic *IntcInstancePtr,
				XZDma *InstancePtr, u16 IntrId);
#else
int XZDma_LinearExample(UINTPTR BaseAddress);
#endif
static void DoneHandler(void *CallBackRef);
static void ErrorHandler(void *CallBackRef, u32 Mask);

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
#define ZDMA_INTR_DEVICE_ID	XPAR_XADMAPS_0_INTR/**< ZDMA Interrupt Id */
#endif

#define TESTDATA1		0xABCD1230 /**< Test data */
#define TESTDATA2		0x00005000 /**< Test data */

#define DESCRIPTOR1_DATA_SIZE	1024 /**< Descriptor 1 data in bytes */
#define DESCRIPTOR2_DATA_SIZE	64   /**< Descriptor 2 data in bytes */

#define POLL_TIMEOUT_COUNTER    1000000U
#define NUM_OF_EVENTS		1
/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
#ifndef SDT
XScuGic Intc;		/**< XIntc Instance */
#endif
#if defined(__ICCARM__)
#pragma data_alignment = 64
u32 DstBuf[256];   /**< Destination buffer */
#pragma data_alignment = 64
u32 SrcBuf[256];   /**< Source buffer */
#pragma data_alignment = 64
u32 Dst1Buf[400];  /**< Destination buffer */
#pragma data_alignment = 64
u32 Src1Buf[400];  /**< Source buffer */
#pragma data_alignment = 64
u32 AlloMem[200];  /**< memory allocated for descriptors */
#else
u32 DstBuf[256] __attribute__ ((aligned (64)));	/**< Destination buffer */
u32 SrcBuf[256] __attribute__ ((aligned (64)));	/**< Source buffer */
u32 Dst1Buf[256] __attribute__ ((aligned (64)));/**< Destination buffer */
u32 Src1Buf[256] __attribute__ ((aligned (64)));/**< Source buffer */
u32 AlloMem[256] __attribute__ ((aligned (64)));
/**< memory allocated for descriptors */
#endif
volatile static u32 Done = 0;	/**< Variable for Done interrupt */
volatile static u32 Pause = 0;	/**< Variable for Pause interrupt */

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

	/* Run the Linear example */
#ifndef SDT
	Status = XZDma_LinearExample((u16)ZDMA_DEVICE_ID);
#else
	Status = XZDma_LinearExample(XPAR_XZDMA_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ZDMA Linear mode Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in linear mode on the ZDMA
* driver.
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
int XZDma_LinearExample(u16 DeviceId)
#else
int XZDma_LinearExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configure; /* Configuration values */
	XZDma_Transfer Data[2];
	u32 Value;
	u32 Index;

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

	/* Filling the buffers for data transfer */
	Value = TESTDATA1;
	for (Index = 0; Index < (DESCRIPTOR1_DATA_SIZE / 4); Index++) {
		SrcBuf[Index] = Value++;
	}
	Value = TESTDATA2;
	for (Index = 0; Index < (DESCRIPTOR2_DATA_SIZE / 4); Index++) {
		Src1Buf[Index] = Value++;
	}

	/* ZDMA has set in simple transfer of Normal mode */
	Status = XZDma_SetMode(&ZDma, TRUE, XZDMA_NORMAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Allocated memory starting address should be 64 bit aligned */
	XZDma_CreateBDList(&ZDma, XZDMA_LINEAR, (UINTPTR)AlloMem, 256);

	/* Interrupt call back has been set */
	XZDma_SetCallBack(&ZDma, XZDMA_HANDLER_DONE,
			  (void *)DoneHandler, &ZDma);
	XZDma_SetCallBack(&ZDma, XZDMA_HANDLER_ERROR,
			  (void *)ErrorHandler, &ZDma);
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

	/* Configuration settings */
	XZDma_GetChDataConfig(&ZDma, &Configure);
	Configure.OverFetch = 0;
	Configure.SrcIssue = 0x1F;
	Configure.SrcBurstType = XZDMA_INCR_BURST;
	Configure.SrcBurstLen = 0xF;
	Configure.DstBurstType = XZDMA_INCR_BURST;
	Configure.DstBurstLen = 0xF;
	if (Config->IsCacheCoherent) {
		Configure.SrcCache = 0xF;
		Configure.DstCache = 0xF;
	}
	XZDma_SetChDataConfig(&ZDma, &Configure);

	/* Filling the data transfer elements */
	Data[0].SrcAddr = (UINTPTR)SrcBuf;
	Data[0].Size = DESCRIPTOR1_DATA_SIZE;
	Data[0].DstAddr = (UINTPTR)DstBuf;
	if (Config->IsCacheCoherent) {
		Data[0].SrcCoherent = 1;
		Data[0].DstCoherent = 1;
	} else {
		Data[0].SrcCoherent = 0;
		Data[0].DstCoherent = 0;
	}
	Data[0].Pause = 1;

	Data[1].SrcAddr = (UINTPTR)Src1Buf;
	Data[1].Size = DESCRIPTOR2_DATA_SIZE;
	Data[1].DstAddr = (UINTPTR)Dst1Buf;
	if (Config->IsCacheCoherent) {
		Data[1].SrcCoherent = 1;
		Data[1].DstCoherent = 1;
	}
	Data[1].Pause = 0;

	/*
	 * Invalidating destination address and flushing
	 * source address in cache before the start of DMA data transfer.
	 */
	if (!Config->IsCacheCoherent) {
		Xil_DCacheFlushRange((INTPTR)Data[0].SrcAddr, Data[0].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[0].DstAddr, Data[0].Size);
		Xil_DCacheFlushRange((INTPTR)Data[1].SrcAddr, Data[1].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[1].DstAddr, Data[1].Size);
	}
	/* Enable required interrupts */
	XZDma_EnableIntr(&ZDma, (XZDMA_IXR_DMA_DONE_MASK |
				 XZDMA_IXR_DMA_PAUSE_MASK));
	XZDma_Start(&ZDma, Data, 2); /* Initiates the data transfer */

	/*
	 * wait until pause interrupt is generated and
	 * has been resumed or timeout
	 */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUM_OF_EVENTS, &Pause);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Resuming the ZDMA core */
	XZDma_Resume(&ZDma);

	/* Wait till DMA done interrupt generated or timeout */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUM_OF_EVENTS, &Done);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Disable relevant interrupts */
	XZDma_DisableIntr(&ZDma, (XZDMA_IXR_DMA_DONE_MASK |
				  XZDMA_IXR_DMA_PAUSE_MASK));

	/* Before the destination buffer data is accessed do one more invalidation
	 * to ensure that the latest data is read. This is as per ARM recommendations.
	 */
	if (!Config->IsCacheCoherent) {
		Xil_DCacheInvalidateRange((INTPTR)Data[0].DstAddr, Data[0].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[1].DstAddr, Data[1].Size);
	}
	/* Validating the data transfer */
	for (Index = 0; Index < (DESCRIPTOR1_DATA_SIZE / 4); Index++) {
		if (SrcBuf[Index] != DstBuf[Index]) {
			return XST_FAILURE;
		}
	}
	for (Index = 0; Index < (DESCRIPTOR2_DATA_SIZE / 4); Index++) {
		if (Src1Buf[Index] != Dst1Buf[Index]) {
			return XST_FAILURE;
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

/*****************************************************************************/
/**
* This static function handles ZDMA pause interrupts.
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
static void ErrorHandler(void *CallBackRef, u32 Mask)
{
	Pause = 1;
}
