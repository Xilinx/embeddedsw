/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_hybrid_poll.c
 *
 * This file demonstrates how to use xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in hybrid transfer mode without
 * interrupt. There are three transfers: a simple transfer, a sg transfer, and
 * another simple transfer.
 *
 * The completion of the transfer is checked through polling. Using polling
 * mode can give better performance on an idle system, where the DMA engine
 * is lowly loaded, and the application has nothing else to do. The polling
 * mode can yield better turn-around time for DMA transfers.
 *
 * Modify NUMBER_OF_BDS_TO_TRANSFER for a different number of BDs to be
 * transferred in the SG transfer.
 *
 * To see the debug print, you need a uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system
 * and the transfer length should be cache-line size aligned.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  02/01/11 Modified function prototype of
 * 		       XAxiCdma_HybridPollExample to a function taking only one
 *		       arguments i.e. device id.
 * 2.01a srt  03/06/12 Added V7 DDR Base Address to fix CR 649405.
 * 		       Modified Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 * 2.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 4.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 4.3   ms   01/22/17 Modified xil_printf statement in main function to
 *            ensure that "Successfully ran" and "Failed" strings are
 *            available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 4.4   rsp  02/22/18 Support data buffers above 4GB.Use UINTPTR for storing
 *  	               and typecasting buffer address(CR-995116).
 * 4.8   sk   07/10/20 Fix CheckData failure by adding the Cache operations for
 *                     receive and destination buffers.
 * 4.8	 sk   09/30/20 Modify the buffer length to make it cache-line aligned.
 * </pre>
 *
 ****************************************************************************/
#include "xaxicdma.h"
#include "xdebug.h"
#include "xil_cache.h"
#include "xenv.h"	/* memset */
#include "xparameters.h"

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */
#ifndef TESTAPP_GEN
#define DMA_CTRL_DEVICE_ID  XPAR_AXICDMA_0_DEVICE_ID
#endif

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

#define BD_SPACE_BASE       (MEMORY_BASE + 0x03000000)
#define BD_SPACE_HIGH       (MEMORY_BASE + 0x03001FFF)
#define TX_BUFFER_BASE      (MEMORY_BASE + 0x00630000)
#define RX_BUFFER_BASE      (MEMORY_BASE + 0x00660000)
#define RX_BUFFER_HIGH      (MEMORY_BASE + 0x0068FFFF)


#define BUFFER_BYTESIZE 128 	/* Length of the buffers for simple transfer */
#define MAX_PKT_LEN         1024  /* Length of BD for SG transfer */

#define MARK_UNCACHEABLE        0x701

/* Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 */
#define NUMBER_OF_BDS_TO_TRANSFER	30

#define RESET_LOOP_COUNT    10 /* Number of times to check reset is done */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if defined(XPAR_UARTNS550_0_BASEADDR)
static void Uart550_Setup(void);
#endif

static int SetupSgTransfer(XAxiCdma *InstancePtr);
static int SubmitSgTransfer(XAxiCdma *InstancePtr);
static int CheckSgCompletion(XAxiCdma *InstancePtr);
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length);
static int DoSimplePollTransfer(XAxiCdma *InstancePtr, int Length, int Retries);
static int DoSgPollTransfer(XAxiCdma *InstancePtr, int Length);
int XAxiCdma_HybridPollExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


static XAxiCdma Engine;       /* Instance of the XAxiCdma */


/* Source and Destination buffer for simple transfer.
 */
volatile static u8 SrcBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));
volatile static u8 DestBuffer[BUFFER_BYTESIZE] __attribute__ ((aligned (64)));

/* Transmit buffer for scatter gather transfer.
 */
static u32 *TransmitBufferPtr = (u32 *) TX_BUFFER_BASE;
static u32 *ReceiveBufferPtr = (u32 *) RX_BUFFER_BASE;

/* Shared variables used to test the callbacks.
 */
volatile static int Done = 0;    /* Dma transfer is done */
volatile static int Error = 0;   /* Dma Error occurs */


/*****************************************************************************/
/**
* The entry point for this example. It sets up uart16550 if one is available,
* invokes the example function, and reports the execution status.
*
* @param	None.
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main()
{

	int Status;

#if defined(XPAR_UARTNS550_0_BASEADDR)
	Uart550_Setup();
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer
	 */
	Status = XAxiCdma_HybridPollExample(DMA_CTRL_DEVICE_ID);

	if (Status != (XST_SUCCESS)) {
		xil_printf("Axicdma Hybrid polled Example Failed\r\n");
		return (XST_FAILURE);
	}

	xil_printf("Successfully ran Axicdma Hybrid polled Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}
#endif

#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
* This function setup the baudrate to 9600 and data bits to 8 in Uart16550
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Uart550_Setup(void)
{
	/* Set the baudrate to be predictable
	 */
	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);

}
#endif

/*****************************************************************************/
/*
* This function setup the driver for scatter gather transfer
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if setup successful
* 		- XST_FAILURE if setup failed with error
*
* @note		None.
*
******************************************************************************/
static int SetupSgTransfer(XAxiCdma *InstancePtr)
{
	int Status;
	XAxiCdma_Bd BdTemplate;
	int BdCount;
	u8 *SrcBufferPtr;
	int Index;

	/* Setup BD ring */
	BdCount = XAxiCdma_BdRingCntCalc(XAXICDMA_BD_MINIMUM_ALIGNMENT,
				    BD_SPACE_HIGH - BD_SPACE_BASE + 1,
				    (UINTPTR)BD_SPACE_BASE);

	Status = XAxiCdma_BdRingCreate(InstancePtr, BD_SPACE_BASE,
		BD_SPACE_BASE, XAXICDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Create BD ring failed %d\r\n",
								Status);

		return XST_FAILURE;
	}

	/*
	 * Setup a BD template to copy to every BD.
	 */
	XAxiCdma_BdClear(&BdTemplate);
	Status = XAxiCdma_BdRingClone(InstancePtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Clone BD ring failed %d\r\n",
								Status);

		return XST_FAILURE;
	}

	/* Initialize receive buffer to 0's and transmit buffer with pattern
	 */
	memset((void *)ReceiveBufferPtr, 0,
		MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	SrcBufferPtr = (u8 *)TransmitBufferPtr;
	for(Index = 0; Index < MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER; Index++) {
		SrcBufferPtr[Index] = Index & 0xFF;
	}

	/* Flush the TransmitBuffer and ReceiveBuffer before the DMA transfer,
	 * in case the Data Cache is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TransmitBufferPtr,
		MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	Xil_DCacheFlushRange((UINTPTR)ReceiveBufferPtr,
			MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function submit the scatter gather transfer to the hardware
*
* @param 	InstancePtr is a pointer to the XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if submission successful
* 		- XST_FAILURE if submission failed with error
*
* @note		None.
*
******************************************************************************/
static int SubmitSgTransfer(XAxiCdma *InstancePtr)
{
	XAxiCdma_Bd *BdPtr;
	XAxiCdma_Bd *BdCurPtr;
	int Status;
	int Index;
	UINTPTR SrcBufferAddr;
	UINTPTR DstBufferAddr;

	Status = XAxiCdma_BdRingAlloc(InstancePtr,
		    NUMBER_OF_BDS_TO_TRANSFER, &BdPtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed bd alloc\r\n");

		return XST_FAILURE;
	}

	SrcBufferAddr = (UINTPTR)TransmitBufferPtr;
	DstBufferAddr = (UINTPTR)ReceiveBufferPtr;
	BdCurPtr = BdPtr;

	/* Set up the BDs
	 */
	for(Index = 0; Index < NUMBER_OF_BDS_TO_TRANSFER; Index++) {

		Status = XAxiCdma_BdSetSrcBufAddr(BdCurPtr, SrcBufferAddr);
		if(Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Set src addr failed %d, %x/%x\r\n",
			    Status, (unsigned int)BdCurPtr,
			    (unsigned int)SrcBufferAddr);

			return XST_FAILURE;
		}

		Status = XAxiCdma_BdSetDstBufAddr(BdCurPtr, DstBufferAddr);
		if(Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Set dst addr failed %d, %x/%x\r\n",
			    Status, (unsigned int)BdCurPtr,
			    (unsigned int)DstBufferAddr);

			return XST_FAILURE;
		}

		Status = XAxiCdma_BdSetLength(BdCurPtr, MAX_PKT_LEN);
		if(Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Set BD length failed %d\r\n", Status);

			return XST_FAILURE;
		}

		SrcBufferAddr += MAX_PKT_LEN;
		DstBufferAddr += MAX_PKT_LEN;
		BdCurPtr = XAxiCdma_BdRingNext(InstancePtr, BdCurPtr);
	}

	/* Give the BDs to hardware */
	Status = XAxiCdma_BdRingToHw(InstancePtr, NUMBER_OF_BDS_TO_TRANSFER,
		BdPtr, NULL, NULL);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Failed to hw %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function checks the completion of the scatter gather transfer
*
* @param 	InstancePtr is a pointer to the XAxiCdma instance
*
* @return	Number of BDs that hardware has finished transfer
*
* @note		None.
*
******************************************************************************/
static int CheckSgCompletion(XAxiCdma *InstancePtr)
{
	int BdCount;
	XAxiCdma_Bd *BdPtr;
	XAxiCdma_Bd *BdCurPtr;
	int Status;
	int Index;

	/* Check whether the hardware has encountered any problems.
	 * In some error cases, the DMA engine may not able to update the
	 * BD that has caused the problem.
	 */
	if (XAxiCdma_GetError(InstancePtr) != 0x0) {
		Error = 1;
		return 0;
	}

	/* Get all processed BDs from hardware
	 */
	BdCount = XAxiCdma_BdRingFromHw(InstancePtr, XAXICDMA_ALL_BDS, &BdPtr);

	/* Check finished BDs then release them
	 */
	if(BdCount > 0) {
		BdCurPtr = BdPtr;

		for (Index = 0; Index < BdCount; Index++) {

			/* If the completed BD has error bit set,
			 * then the example fails
			 */
			if (XAxiCdma_BdGetSts(BdCurPtr) &
			    XAXICDMA_BD_STS_ALL_ERR_MASK)	{
				Error = 1;
				return 0;
			}

			BdCurPtr = XAxiCdma_BdRingNext(InstancePtr, BdCurPtr);
		}

		/* Release the BDs so later submission can use them
		 */
		Status = XAxiCdma_BdRingFree(InstancePtr, BdCount, BdPtr);

		if(Status != XST_SUCCESS) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Error free BD %d\r\n", Status);

			Error = 1;
			return 0;
		}

		Done += BdCount;
	}

	return Done;
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
* 		- XST_SUCCESS if the two buffer matches
* 		- XST_FAILURE otherwise
*
* @note		None.
*
******************************************************************************/
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length)
{
	int Index;

	/* Invalidate the DestBuffer before receiving the data,
	 * in case the data cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)DestPtr, Length);

	for (Index = 0; Index < Length; Index++) {
		if ( DestPtr[Index] != SrcPtr[Index]) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Data check failure %d: %x/%x\r\n",
			    Index, (unsigned int)DestPtr[Index],
			    (unsigned int)SrcPtr[Index]);

			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function does one simple transfer in polled mode
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param 	Length is the transfer length
* @param 	Retries is how many times to retry on submission
*
* @return
* 		- XST_SUCCESS if transfer is successful
* 		- XST_FAILURE if either the transfer fails or the data has error
*
* @note		None.
*
******************************************************************************/
static int DoSimplePollTransfer(XAxiCdma *InstancePtr, int Length, int Retries)
{
	u32 Index;
	u8  *SrcPtr;
	u8  *DestPtr;
	int Status = 0;

	/* Initialize the source buffer bytes with a pattern and the
	 * the destination buffer bytes to zero
	 */
	SrcPtr = (u8 *)SrcBuffer;
	DestPtr = (u8 *)DestBuffer;
	for (Index = 0; Index < BUFFER_BYTESIZE; Index++) {
		SrcPtr[Index] = Index & 0xFF;
		DestPtr[Index] = 0;
	}

	/* Flush the SrcBuffer and DestBuffer before the DMA transfer,
	 * in case the Data Cache is enabled
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
		    "Failed to submit the transfer %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done
	 */
	while (XAxiCdma_IsBusy(InstancePtr)) {
		/* Wait */
	}

	/* If the hardware has errors, this example fails
	 * This is a poll example, no interrupt handler is involved.
	 * Therefore, error conditions are not cleared by the driver.
	 */
	Error = XAxiCdma_GetError(InstancePtr);
	if (Error != 0x0) {
		int TimeOut = RESET_LOOP_COUNT;

		xdbg_printf(XDBG_DEBUG_ERROR, "Transfer has error %x\r\n",
		    (unsigned int)Error);

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
		if (!TimeOut) {
			xdbg_printf(XDBG_DEBUG_ERROR,
			    "Reset hardware failed with %d\r\n", Status);

		}

		return XST_FAILURE;
	}

	/* Transfer completes successfully, check data
	 */
	Status = CheckData(SrcPtr, DestPtr, Length);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Check data failed for simple "
		    "transfer\r\n");

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
* This function does one scatter gather transfer with 2 BDs in polled mode
*
* @param	InstancePtr is a pointer to the XAxiCdma instance
* @param	Length is the transfer length
*
* @return
* 		- XST_SUCCESS if transfer is successful
* 		- XST_FAILURE if either the transfer fails or the data has
*		  error
*
* @note		None.
*
******************************************************************************/
static int DoSgPollTransfer(XAxiCdma *InstancePtr, int Length)
{
	int Status;
	u8 *SrcPtr;
	u8 *DstPtr;

	SrcPtr = (u8 *)TransmitBufferPtr;
	DstPtr = (u8 *)ReceiveBufferPtr;

#ifdef __aarch64__
	Xil_SetTlbAttributes(BD_SPACE_BASE, MARK_UNCACHEABLE);
#endif
	/* Submit the DMA transfer
	 */
	Status = SubmitSgTransfer(InstancePtr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Do SG transfer failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or error occurs
	 */
	while ((CheckSgCompletion(InstancePtr) < NUMBER_OF_BDS_TO_TRANSFER) &&
		!Error) {
		/* Wait */
	}

	if(Error) {
		return XST_FAILURE;
	}

	/* Transfer completed successfully, check data
	 */
	Status = CheckData(SrcPtr, DstPtr,
		MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Check data failed for sg "
		    "transfer\r\n");

		return XST_FAILURE;
	}

	/* Test finishes successfully, return successfully
	 */
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The example to do hybrid transfers through polling. The transfer sequence is:
*	a simple polled transfer
*	a scatter gather polled transfer with multiple BDs
* 	a simple polled transfer
*
* @param	DeviceId is the Device Id of the XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if error occurs
*
* @note		If the hardware build has problems with interrupt, then this
*		function hangs
*
******************************************************************************/
int XAxiCdma_HybridPollExample(u16 DeviceId)
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	int SubmitTries = 10;	/* try 10 times on submission */

	/* Initialize the XAxiCdma device.
	 */
	CfgPtr = XAxiCdma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Cannot find config structure for device %d\r\n",
			DeviceId);

		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(&Engine, CfgPtr,
						CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Initialization failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiCdma_IntrDisable(&Engine, XAXICDMA_XR_IRQ_ALL_MASK);

	/* Setup scatter gather transfer
	 */
	Status = SetupSgTransfer(&Engine);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Setup SG transfer failed with error %d\r\n", Status);
		return XST_FAILURE;
	}

	Done = 0;
	Error = 0;

	/* Do one simple transfer
	 */
	Status = DoSimplePollTransfer(&Engine,
		BUFFER_BYTESIZE, SubmitTries);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "First simple transfer failed with error %d\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("First simple transfer successful\r\n");

	Done = 0;
	Error = 0;

	/* Then do a scatter gather transfer
	 */
	Status = DoSgPollTransfer(&Engine, BUFFER_BYTESIZE);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Scatter gather transfer failed with error %d\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("Scatter gather transfer successful\r\n");

	Done = 0;
	Error = 0;

	/* Finally, do another simple transfer
	 */
	Status = DoSimplePollTransfer(&Engine,
		BUFFER_BYTESIZE, SubmitTries);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Second simple transfer failed with error %d\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("Second simple transfer successful\r\n");

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}




