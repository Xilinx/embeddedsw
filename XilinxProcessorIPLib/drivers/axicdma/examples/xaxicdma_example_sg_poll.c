/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxicdma_example_sg_poll.c
 *
 * This file demonstrates how to use the xaxicdma driver on the Xilinx AXI
 * CDMA core (AXICDMA) to transfer packets in scatter gather transfer mode
 * without interrupt.
 *
 * The completion of the transfer is checked through polling. Using polling
 * mode can give better performance on an idle system, where the DMA engine
 * is lowly loaded, and the application has nothing else to do. The polling
 * mode can yield better turn-around time for DMA transfers.
 *
 * To see the debug print, you need a uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options for the example, also
 * comment out the "#undef DEBUG" in xdebug.h. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/30/10 First release
 * 2.01a rkv  01/28/11 Modified function prototype of XAxiCdma_SgPollExample to
 * 		       a function taking only one arguments i.e. device id.
 * 2.01a srt  03/05/12 Added V7 DDR Base Address to fix CR 649405.
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
 * 4.4   rsp  02/22/18 Support data buffers above 4GB.Use UINTPTR for
 *                     typecasting buffer address(CR-995116).
 * 4.6   rsp  09/13/19 Fix cache maintenance ops for source and dest buffer.
 * 4.7   rsp  12/06/19 For aarch64 include xil_mmu.h. Fixes gcc warning.
 * 4.10  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * </pre>
 *
 ****************************************************************************/
#include "xaxicdma.h"
#include "xdebug.h"
#include "xenv.h"	/* memset */
#include "xil_cache.h"
#include "xparameters.h"
#ifdef __aarch64__
#include "xil_mmu.h"
#endif

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
#define DMA_CTRL_DEVICE_ID	XPAR_AXICDMA_0_DEVICE_ID
#endif


#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define MEMORY_BASE		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define MEMORY_BASE	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define MEMORY_BASE	XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define MEMORY_BASE	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#else
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEMORY_BASE		0x01000000
#endif

#define BD_SPACE_BASE		(MEMORY_BASE + 0x03000000)
#define BD_SPACE_HIGH		(MEMORY_BASE + 0x03001FFF)
#define TX_BUFFER_BASE		(MEMORY_BASE + 0x00630000)
#define RX_BUFFER_BASE		(MEMORY_BASE + 0x00660000)
#define RX_BUFFER_HIGH		(MEMORY_BASE + 0x0068FFFF)

#define MAX_PKT_LEN		128
#define MARK_UNCACHEABLE	0x701

/* Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 */
#define NUMBER_OF_BDS_TO_TRANSFER	1

#define RESET_LOOP_COUNT	10 /* Number of times to check reset is done */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if defined(XPAR_UARTNS550_0_BASEADDR)
static void Uart550_Setup(void);
#endif

static int CheckCompletion(XAxiCdma *InstancePtr);
static int SetupTransfer(XAxiCdma * InstancePtr);
static int DoTransfer(XAxiCdma * InstancePtr);
static int CheckData(u8 *SrcPtr, u8 *DestPtr, int Length);
int XAxiCdma_SgPollExample(u16 DeviceId);

/************************** Variable Definitions *****************************/


static XAxiCdma AxiCdmaInstance;	/* Instance of the XAxiCdma */

/* Transmit buffer for DMA transfer.
 */
static u32 *TransmitBufferPtr = (u32 *) TX_BUFFER_BASE;
static u32 *ReceiveBufferPtr = (u32 *) RX_BUFFER_BASE;

/* Shared variables used to test the callbacks.
 */
volatile static int Done = 0;	/* Dma transfer is done */
volatile static int Error = 0;	/* Dma Bus Error occurs */

/*****************************************************************************/
/**
* The entry point for this example. It sets up uart16550 if one is available,
* invokes the example function, and reports the execution status.
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

#ifdef XPAR_UARTNS550_0_BASEADDR
	Uart550_Setup();
#endif

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the interrupt example for simple transfer
	 */
	Status = XAxiCdma_SgPollExample(DMA_CTRL_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiCdma_SgPoll Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiCdma_SgPoll Example\r\n");
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
* @note		None
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
* Check for transfer completion.
*
* If the DMA engine has errors or any of the finished BDs has error bit set,
* then the example should fail.
*
* @param	InstancePtr is pointer to the XAxiCdma instance.
*
* @return	Number of Bds that have been completed by hardware.
*
* @note		None
*
******************************************************************************/
static int CheckCompletion(XAxiCdma *InstancePtr)
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
		xdbg_printf(XDBG_DEBUG_ERROR, "Transfer error %x\r\n",
		    (unsigned int)XAxiCdma_GetError(InstancePtr));

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
			    "Error free BD %x\r\n", Status);

			Error = 1;
			return 0;
		}

		Done += BdCount;
	}

	return Done;
}

/*****************************************************************************/
/**
*
* This function sets up the DMA engine to be ready for scatter gather transfer
*
* @param	InstancePtr is pointer to the XAxiCdma instance.
*
* @return
*		- XST_SUCCESS if the setup is successful
*		- XST_FAILURE if error occurs
*
* @note		None
*
******************************************************************************/
static int SetupTransfer(XAxiCdma * InstancePtr)
{
	int Status;
	XAxiCdma_Bd BdTemplate;
	int BdCount;
	u8 *SrcBufferPtr;
	int Index;

	/* Disable all interrupts
	 */
	XAxiCdma_IntrDisable(InstancePtr, XAXICDMA_XR_IRQ_ALL_MASK);

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

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TransmitBufferPtr,
		MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);
	Xil_DCacheFlushRange((UINTPTR)ReceiveBufferPtr,
		MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER);

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function non-blockingly transmits all packets through the DMA engine.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return
*		- XST_SUCCESS if the DMA accepts all the packets successfully,
*		- XST_FAILURE if error occurs
*
* @note		None
*
******************************************************************************/
static int DoTransfer(XAxiCdma * InstancePtr)
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
* @note		None
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

/*****************************************************************************/
/**
* The example to do the scatter gather transfer through polling.
*
* @param	DeviceId is the Device Id of the XAxiCdma instance
*
* @return
* 		- XST_SUCCESS if example finishes successfully
* 		- XST_FAILURE if error occurs
*
* @note		None
*
******************************************************************************/
int XAxiCdma_SgPollExample(u16 DeviceId)
{
	XAxiCdma_Config *CfgPtr;
	int Status;
	u8 *SrcPtr;
	u8 *DstPtr;

	SrcPtr = (u8 *)TransmitBufferPtr;
	DstPtr = (u8 *)ReceiveBufferPtr;

#ifdef __aarch64__
	Xil_SetTlbAttributes(BD_SPACE_BASE, MARK_UNCACHEABLE);
#endif

	/* Initialize the XAxiCdma device.
	 */
	CfgPtr = XAxiCdma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Cannot find config structure for device %d\r\n",
			XPAR_AXICDMA_0_DEVICE_ID);

		return XST_FAILURE;
	}

	Status = XAxiCdma_CfgInitialize(&AxiCdmaInstance, CfgPtr,
		CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Initialization failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Setup the BD ring
	 */
	Status = SetupTransfer(&AxiCdmaInstance);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Setup BD ring failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	Done = 0;
	Error = 0;

	/* Start the DMA transfer
	 */
	Status = DoTransfer(&AxiCdmaInstance);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		    "Do transfer failed with %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Wait until the DMA transfer is done or error occurs
	 */
	while ((CheckCompletion(&AxiCdmaInstance) < NUMBER_OF_BDS_TO_TRANSFER)
		&& !Error) {
		/* Wait */
	}

	if(Error) {
		int TimeOut = RESET_LOOP_COUNT;

		xdbg_printf(XDBG_DEBUG_ERROR, "Transfer has error %x\r\n",
				Error);

		/* Need to reset the hardware to restore to the correct state
		 */
		XAxiCdma_Reset(&AxiCdmaInstance);

		while (TimeOut) {
			if (XAxiCdma_ResetIsDone(&AxiCdmaInstance)) {
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



