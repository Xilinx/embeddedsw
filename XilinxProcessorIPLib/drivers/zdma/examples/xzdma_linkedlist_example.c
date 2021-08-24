/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_linkedlist_example.c
*
* This file contains the example using XZDma driver to do data transfer in
* Linked list mode on ZDMA device.
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
* 1.7   adk    21/03/19  Fix alignment pragmas in the example for IAR compiler.
* 1.13  asa    08/24/21  Make changes to add a missing data invalidation
*                        operation just before the destination buffer data is
*                        being read. Along with the changes were done to ensure
*                        proper sizes for linked list buffers.
*                        Changes were done to allocate Src1Buf and Dst1Buf
*                        properly.
*                        Changes were done for other cleanups and also to
*                        ensure that the DMA is reset before the program exits.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xzdma.h"
#include "xparameters.h"
#include "xscugic.h"
#include "bspconfig.h"

/************************** Function Prototypes ******************************/

int XZDma_LinkedListExample(u16 DeviceId);
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
#define ZDMA_INTR_DEVICE_ID	XPAR_XADMAPS_0_INTR /**< ZDMA Interrupt Id */
#define TESTDATA1		0xABCD1230 /**< Test data */
#define TESTDATA2		0x00005000 /**< Test data */

#define DESCRIPTOR1_DATA_SIZE	1024 /**< Descriptor 1 data in bytes */
#define DESCRIPTOR2_DATA_SIZE	64   /**< Descriptor 2 data in bytes */

/**************************** Type Definitions *******************************/


/************************** Variable Definitions *****************************/

XZDma ZDma;		/**<Instance of the ZDMA Device */
XScuGic Intc;		/**< XIntc Instance */

#if defined(__ICCARM__)
    #pragma data_alignment = 64
	u32 SrcBuf[256];
    #pragma data_alignment = 64
	u32 DstBuf[256];
	#pragma data_alignment = 64
	u32 Src1Buf[256];
    #pragma data_alignment = 64
	u32 Dst1Buf[256];
	#pragma data_alignment = 64
	u32 AlloMem[256];
#else
u32 SrcBuf[256] __attribute__ ((aligned (64)));
u32 DstBuf[256] __attribute__ ((aligned (64)));
u32 Src1Buf[256] __attribute__ ((aligned (64)));
u32 Dst1Buf[256] __attribute__ ((aligned (64)));
u32 AlloMem[256] __attribute__ ((aligned (64)));
#endif

volatile static u8 Done = 0;

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

	/* Run the Linked list example */
	Status = XZDma_LinkedListExample((u16)ZDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ZDMA Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran ZDMA Linked list Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a test of the data transfer in linked mode on the ZDMA
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
int XZDma_LinkedListExample(u16 DeviceId)
{
	int Status;
	XZDma_Config *Config;
	XZDma_DataConfig Configur; /* Configuration values */
	XZDma_Transfer Data[2];
	u32 Value;
	u32 Index;
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
	/* Filling the buffers for data transfer */
	Value = TESTDATA1;
	for (Index = 0; Index < (DESCRIPTOR1_DATA_SIZE/4); Index++) {
		SrcBuf[Index] = Value++;
	}
	Value = TESTDATA2;
	for (Index = 0; Index < (DESCRIPTOR2_DATA_SIZE/4); Index++) {
		*(Src1Buf+Index) = Value++;
	}

	/* ZDMA has set in linked list mode for normal transfer */
	Status = XZDma_SetMode(&ZDma, TRUE, XZDMA_NORMAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Allocated memory starting address should be 64 bit aligned */
	XZDma_CreateBDList(&ZDma, XZDMA_LINKEDLIST, (UINTPTR)AlloMem, 256);

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
	/*
	 * All the configurations are kept as is which was got
	 * through XZDma_GetChDataConfig and only over fetch is disabled
	 */
	/* Configuration settings */
	XZDma_GetChDataConfig(&ZDma, &Configur);
	Configur.OverFetch = 0;
	if (Config->IsCacheCoherent) {
		Configur.SrcCache = 0xF;
		Configur.DstCache = 0xF;
	}
	XZDma_SetChDataConfig(&ZDma, &Configur);

	/* Filling the data transfer elements */
	Data[0].SrcAddr = (UINTPTR)SrcBuf;
	Data[0].Size = DESCRIPTOR1_DATA_SIZE;
	Data[0].DstAddr = (UINTPTR)DstBuf;
	Data[0].SrcCoherent = 0;
	Data[0].DstCoherent = 0;
	Data[0].Pause = 0;

	Data[1].SrcAddr = (UINTPTR)Src1Buf;
	Data[1].Size = DESCRIPTOR2_DATA_SIZE;
	Data[1].DstAddr = (UINTPTR)Dst1Buf;
	Data[1].SrcCoherent = 0;
	Data[1].DstCoherent = 0;
	Data[1].Pause = 0;
	/*
	 * Invalidating destination address and flushing
	 * source address in cache before initiating the data transfer
	 */
	if (!Config->IsCacheCoherent) {
		Xil_DCacheFlushRange((INTPTR)Data[0].SrcAddr, Data[0].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[0].DstAddr, Data[0].Size);
		Xil_DCacheFlushRange((INTPTR)Data[1].SrcAddr, Data[1].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[1].DstAddr, Data[1].Size);
	}

	/* Enable required interrupts */
	XZDma_EnableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);
	XZDma_Start(&ZDma, Data, 2); /* Initiates the data transfer */

	while (Done == 0); /* Wait till DMA done interrupt generated */

	XZDma_DisableIntr(&ZDma, XZDMA_IXR_DMA_DONE_MASK);

	/* Before the destination buffer data is accessed do one more invalidation
         * to ensure that the latest data is read. This is as per ARM recommendations.
         */
	if (!Config->IsCacheCoherent) {
		Xil_DCacheInvalidateRange((INTPTR)Data[0].DstAddr, Data[0].Size);
		Xil_DCacheInvalidateRange((INTPTR)Data[1].DstAddr, Data[1].Size);
	}

	/* Validating the data transfer */
	for (Index = 0; Index < (DESCRIPTOR1_DATA_SIZE/4); Index++) {
		if (SrcBuf[Index] != DstBuf[Index]) {
			return XST_FAILURE;
		}
	}
	for (Index = 0; Index < (DESCRIPTOR2_DATA_SIZE/4); Index++) {
		if (Src1Buf[Index] != Dst1Buf[Index]) {
			return XST_FAILURE;
		}
	}

	/* Reset the DMA to remove all configurations done in this example  */
	XZDma_Reset(&ZDma);

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
