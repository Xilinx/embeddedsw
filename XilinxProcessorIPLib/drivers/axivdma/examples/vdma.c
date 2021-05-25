/******************************************************************************
* Copyright (C) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file vdma.c
 *
 * This file comprises sample application to  usage of VDMA APi's in vdma_api.c.
 *  .
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.0   adk  11/26/15 First release
 * 4.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 ****************************************************************************/

/*** Include file ***/
#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_assert.h"
#include "xaxivdma.h"
#include "xaxivdma_i.h"


#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define MEMORY_BASE		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define MEMORY_BASE	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDR
#define MEMORY_BASE	XPAR_MIG_0_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define MEMORY_BASE	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#else
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEMORY_BASE		0x01000000
#endif

/*** Global Variables ***/
unsigned int srcBuffer = (MEMORY_BASE  + 0x1000000);

/* Instance of the Interrupt Controller */
static XIntc Intc;

int run_triple_frame_buffer(XAxiVdma* InstancePtr, int DeviceId, int hsize,
		int vsize, int buf_base_addr, int number_frame_count,
		int enable_frm_cnt_intr);

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId,
				u16 WriteIntrId);

/*****************************************************************************/
/**
* Main function
*
* This is main entry point to demonstrate this example.
*
* @return	None
*
******************************************************************************/
int main(){

	int Status;
	XAxiVdma InstancePtr;
	XAxiVdma InstancePtr_1;

	xil_printf("\n--- Entering main() --- \r\n");
	xil_printf("Starting the first VDMA \n\r");

	/* Calling the API to configure and start VDMA without frame counter interrupt */
	Status = run_triple_frame_buffer(&InstancePtr, 0, 1920, 1080,
						srcBuffer, 100, 0);
	if (Status != XST_SUCCESS) {
		xil_printf("Transfer of frames failed with error = %d\r\n",Status);
		return XST_FAILURE;
	} else {
		xil_printf("Transfer of frames started \r\n");
	}

	xil_printf("Starting the second VDMA \r\n");

	/* Calling the API to configure and start second VDMA with frame counter interrupt
	 * Please note source buffer pointer is being offset a bit */
	Status = run_triple_frame_buffer(&InstancePtr_1, 1, 1920, 1080,
						srcBuffer + 0x1000000, 100, 1);
	if (Status != XST_SUCCESS){
		xil_printf("Transfer of frames failed with error = %d\r\n",Status);
		return XST_FAILURE;
	} else {
		xil_printf("Transfer of frames started \r\n");
	}

	/* Enabling the interrupt for second VDMA */
	SetupIntrSystem(&InstancePtr_1, XPAR_INTC_0_AXIVDMA_1_MM2S_INTROUT_VEC_ID,
			XPAR_INTC_0_AXIVDMA_1_S2MM_INTROUT_VEC_ID);


	/* Infinite while loop to let it run */
	while(1);
}


/*****************************************************************************/
/*
 * Call back function for read channel
 *
 * The user can put his code that should get executed when this
 * call back happens.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Read Call back function is called\r\n");
}

/*****************************************************************************/
/*
 * The user can put his code that should get executed when this
 * call back happens.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Read Call back Error function is called\r\n");

}

/*****************************************************************************/
/*The user can put his code that should get executed when this
 * call back happens.
 *
 *
 * This callback only clears the interrupts and updates the transfer status.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void WriteCallBack(void *CallbackRef, u32 Mask)
{
	/* User can add his code in this call back function */
	xil_printf("Write Call back function is called\r\n");

}

/*****************************************************************************/
/*
* The user can put his code that should get executed when this
* call back happens.
*
* @param	CallbackRef is the call back reference pointer
* @param	Mask is the interrupt mask passed in from the driver
*
* @return	None
*
******************************************************************************/
static void WriteErrorCallBack(void *CallbackRef, u32 Mask)
{

	/* User can add his code in this call back function */
	xil_printf("Write Call back Error function is called \r\n");

}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA.  This function assumes INTC component exists in the hardware system.
*
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	ReadIntrId is the read channel Interrupt ID.
* @param	WriteIntrId is the write channel Interrupt ID.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId,
				u16 WriteIntrId)
{
	int Status;
	XIntc *IntcInstancePtr =&Intc;

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf( "Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, ReadIntrId,
	         (XInterruptHandler)XAxiVdma_ReadIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed read channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, WriteIntrId,
	         (XInterruptHandler)XAxiVdma_WriteIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed write channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf( "Failed to start intc\r\n");
		return XST_FAILURE;
	}

	/* Enable interrupts from the hardware */
	XIntc_Enable(IntcInstancePtr, ReadIntrId);
	XIntc_Enable(IntcInstancePtr, WriteIntrId);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_InterruptHandler,
			(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	/* Register call-back functions
	 */
	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL, ReadCallBack,
		(void *)AxiVdmaPtr, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR,
		ReadErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL,
		WriteCallBack, (void *)AxiVdmaPtr, XAXIVDMA_WRITE);

	XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR,
		WriteErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_WRITE);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts
*
* @param	ReadIntrId is interrupt ID associated w/ DMA read channel
* @param	WriteIntrId is interrupt ID associated w/ DMA write channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(u16 ReadIntrId, u16 WriteIntrId)
{

	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(&Intc, ReadIntrId);
	XIntc_Disconnect(&Intc, WriteIntrId);

}
