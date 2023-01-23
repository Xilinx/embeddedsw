/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_simple_poll.c
 *
 * This file demonstrates how to use xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in simple transfer mode without
 * interrupt.
 *
 * The completion of the transfer is checked through polling. Using polling
 * mode can give better performance on an idle system, where the DMA engine
 * is lowly loaded, and the application has nothing else to do. The polling
 * mode can yield better turn-around time for DMA transfers.
 *
 * Modify NUMBER_OF_TRANSFERS for a different number of simple transfer to be
 * done in this test.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  01/28/11 Modified function prototype of
 * 		       XAxiCdma_SimplePollExample to function taking only one
 * 		       arguments i.e. device id.
 * 2.01a srt  03/06/12 Modified Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 * 4.3   ms   01/22/17 Modified xil_printf statement in main function to
 *            ensure that "Successfully ran" and "Failed" strings are
 *            available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 4.6   rsp  09/13/19 Add error prints for failing scenarios.
 *                     Fix cache maintenance ops for source and dest buffer.
 * 4.7   rsp  12/06/19 For aarch64 include xil_mmu.h. Fixes gcc warning.
 * 4.11  sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 ****************************************************************************/
#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xil_util.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#ifndef TESTAPP_GEN
#define DMA_CTRL_DEVICE_ID	XPAR_AXICDMA_0_DEVICE_ID
#endif

#define BUFFER_BYTESIZE		64	/* Length of the buffers for DMA
					 * transfer
					 */

#define NUMBER_OF_TRANSFERS	4	/* Number of transfers to do in this
					 * test
					 */

#define RESET_LOOP_COUNT	10	/* Number of times to check reset is
					 * done
					 */
#define POLL_TIMEOUT_COUNTER    1000000U

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
static int DoSimplePollTransfer(XAxiCdma *InstancePtr, int Length, int Retries);

static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length);

int XAxiCdma_SimplePollExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

static XAxiCdma AxiCdmaInstance;	/* Instance of the XAxiCdma */

/* Source and Destination buffer for DMA transfer.
 */
volatile static u8 SrcBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
volatile static u8 DestBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));

/* Shared variables used to test the callbacks.
 */
volatile static u32 Done = 0;	/* Dma transfer is done */
volatile static u32 Error = 0;	/* Dma Error occurs */


/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
	Status = XAxiCdma_SimplePollExample(DMA_CTRL_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("AxiCdma_SimplePoll Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran AxiCdma_SimplePoll Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif


/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiCdma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		If the hardware build has problems with interrupt,
*			then this function hangs
*
*
******************************************************************************/
int XAxiCdma_SimplePollExample(u16 DeviceId)
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	int SubmitTries = 10;	/* try 10 times on submission */
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;

	/* Initialize the XAxiCdma device.
	 */
	CfgPtr = XAxiCdma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(&AxiCdmaInstance, CfgPtr,
		CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiCdma_IntrDisable(&AxiCdmaInstance, XAXICDMA_XR_IRQ_ALL_MASK);

	for (Index = 0; Index < Tries; Index++) {
		Status = DoSimplePollTransfer(&AxiCdmaInstance,
			BUFFER_BYTESIZE, SubmitTries);
		if (Status != XST_SUCCESS) {
				return XST_FAILURE;
		}
	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}


/*****************************************************************************/
/*
* This function does one simple transfer in polled mode
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	Length is the transfer length
* @param	Retries is how many times to retry on submission
*
* @return
*		- XST_SUCCESS if transfer is successful
*		- XST_FAILURE if either the transfer fails or the data has
*		  error
*
* @note		None
*
******************************************************************************/
static int DoSimplePollTransfer(XAxiCdma *InstancePtr, int Length, int Retries)
{
	int Index;
	u8  *SrcPtr;
	u8  *DestPtr;
	int Status;

	/* Initialize the source buffer bytes with a pattern and the
	 * the destination buffer bytes to zero
	 */
	SrcPtr = (u8 *)SrcBuffer;
	DestPtr = (u8 *)DestBuffer;
	for (Index = 0; Index < BUFFER_BYTESIZE; Index++) {
		SrcPtr[Index] = Index & 0xFF;
		DestPtr[Index] = 0;
	}

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)&SrcBuffer, Length);
	Xil_DCacheFlushRange((UINTPTR)&DestBuffer, Length);

	/* Try to start the DMA transfer
	 */
	while (Retries) {
		Retries -= 1;

		Status = XAxiCdma_SimpleTransfer(InstancePtr, (UINTPTR)SrcBuffer,
			(UINTPTR)DestBuffer, Length, NULL, NULL);
		if (Status == XST_SUCCESS) {
			break;
		}
	}

	/* Return failure if failed to submit the transfer
	 */
	if (!Retries) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			    "Failed to submit the transfer with %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or timeout
	 */
	Status = Xil_WaitForEvent(InstancePtr->BaseAddr + XAXICDMA_SR_OFFSET,
                       XAXICDMA_SR_IDLE_MASK, XAXICDMA_SR_IDLE_MASK, POLL_TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"Failed to complete the transfer with %d\r\n", Status);
		return XST_FAILURE;
	}

	/* If the hardware has errors, this example fails
	 * This is a poll example, no interrupt handler is involved.
	 * Therefore, error conditions are not cleared by the driver.
	 */
	Error = XAxiCdma_GetError(InstancePtr);
	if (Error != 0x0) {
		int TimeOut = RESET_LOOP_COUNT;

		/* Need to reset the hardware to restore to the correct state
		 */
		XAxiCdma_Reset(InstancePtr);

		while (TimeOut) {
			if (XAxiCdma_ResetIsDone(InstancePtr)) {
				break;
			}
			TimeOut -= 1;
		}

		/* Reset has failed, print a message to notify the user
		 */
		xdbg_printf(XDBG_DEBUG_ERROR, "Reset done failed\r\n");
		return XST_FAILURE;
	}

	/* Transfer completes successfully, check data
	 */
	Status = CheckData(SrcPtr, DestPtr, Length);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/*
* This function checks that two buffers have the same data
*
* @param	SrcPtr is the source buffer
* @param	DestPtr is the destination buffer
* @param	Length is the length of the buffer to check
*
* @return
*		- XST_SUCCESS if the two buffer matches
*		- XST_FAILURE otherwise
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length)
{
	int Index;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)DestPtr, Length);

	for (Index = 0; Index < Length; Index++) {
		if ( DestPtr[Index] != SrcPtr[Index]) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Data check failure %d: %x/%x\r\n",
			    Index, DestPtr[Index], SrcPtr[Index]);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
