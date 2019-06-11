/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
 * @file xaxidma_example_poll_multi_pkts.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer multiple packets in polling mode when the
 * AXI DMA core is configured in Scatter Gather Mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system. The
 * h/w system built in Area mode has a maximum DDR memory limit of 64MB. In
 * throughput mode, it is 512MB.  These limits are need to ensured for
 * proper operation of this code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   05/17/10 First release
 * 2.00a jz   08/10/10 Second release, added in xaxidma_g.c, xaxidma_sinit.c,
 *		       updated tcl file, added xaxidma_porting_guide.h, removed
 *		       workaround for endianness
 * 4.00a rkv  02/22/11 Name of the file has been changed for naming consistency
 * 5.00a srt  03/06/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 *		       Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Added tabspace for return statements in functions
 *                     for proper documentation while generating doxygen.
 * 9.5   adk  17/10/17 Marked the BD region as Normal Non-Cacheable for A53
 *		       (CR#987026).
 * 9.6   rsp  02/14/18 Support data buffers above 4GB.Use UINTPTR for storing
 *                     and typecasting buffer address(CR-992638).
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/

/*********************** TEMPORARY ******************************************/
/*
 * Device hardware build related constants.
 */
#define DMA_BASE_ADDR		XPAR_AXIDMA_0_BASEADDR
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
		DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define TX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00000FFF)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00001000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00020000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00030000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x0003FFFF)

#define MAX_PKT_LEN		0x200
#define NUMBER_OF_PACKETS 	0x10
#define TEST_START_VALUE	0xC

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#if defined(XPAR_UARTNS550_0_BASEADDR)
static void Uart550_Setup(void);
#endif

static int RxSetup(XAxiDma * AxiDmaInstPtr);
static int TxSetup(XAxiDma * AxiDmaInstPtr);
static int SendPackets(XAxiDma * AxiDmaInstPtr);
static int CheckData(void);
static int CheckDmaResult(XAxiDma * AxiDmaInstPtr);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;

/*
 * Buffer for transmit packet.
 */
u32 *Packet = (u32 *) TX_BUFFER_BASE;

static XAxiDma_Bd *LastRxBdPtr = NULL;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA loopback
* widget.
*
* @param	None
*
* @return
*		- XST_SUCCESS if test pass,
* 		- XST_FAILURE if test fails
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	XAxiDma_Config *Config;

#if defined(XPAR_UARTNS550_0_BASEADDR)

	Uart550_Setup();

#endif

	xil_printf("\r\n--- Entering main() --- \r\n");
#ifdef __aarch64__
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, NORM_NONCACHE);
	Xil_SetTlbAttributes(RX_BD_SPACE_BASE, NORM_NONCACHE);
#endif

	Config = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);

		return XST_FAILURE;
	}

	/* Initialize DMA engine */
	Status = XAxiDma_CfgInitialize(&AxiDma, Config);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);

		return XST_FAILURE;
	}

	if(!XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as simple mode \r\n");

		return XST_FAILURE;
	}

	Status = TxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = RxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Send packets */
	Status = SendPackets(&AxiDma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check DMA transfer result */

	Status = CheckDmaResult(&AxiDma);

	if (Status != XST_SUCCESS) {
		xil_printf("AXI DMA poll multi Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran AXI DMA poll multi Example\r\n");
	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
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
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	- XST_SUCCESS if the setup is successful
*		- XST_FAILURE if setup is failure
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdCount;
	u32 FreeBdCount;
	UINTPTR RxBufferPtr;
	int i;

	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	/* Disable all RX interrupts before RxBD space setup */

	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set delay and coalescing */
	XAxiDma_BdRingSetCoalesce(RxRingPtr, Coalesce, Delay);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
				RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE,
				RX_BD_SPACE_BASE,
				XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup an all-zero BD as the template for the Rx channel.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Attach buffers to RxBD ring so we are ready to receive packets
	 */
	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	RxBufferPtr = RX_BUFFER_BASE;

	for (i = 0; i < FreeBdCount; i++) {

		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set buffer addr %x on BD %x failed %d\r\n",
			(unsigned int)RxBufferPtr, (UINTPTR)BdCurPtr,
								Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
				RxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		/* Receive BDs do not need to set anything for the control
		 * The hardware will set the SOF/EOF bits per stream status
		 */
		XAxiDma_BdSetCtrl(BdPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);
		RxBufferPtr += MAX_PKT_LEN;

		if (i == (FreeBdCount - 1)) {
			LastRxBdPtr = BdCurPtr;
		}

		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the TX channel of a DMA engine to be ready for packet
* transmission
*
* @param	AxiDmaInstPtr is the instance pointer to the DMA engine.
*
* @return	- XST_SUCCESS if the setup is successful
*		- XST_FAILURE if setup is failure
*
* @note     None.
*
******************************************************************************/
static int TxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	XAxiDma_Bd BdTemplate;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	u32 BdCount;

	TxRingPtr = XAxiDma_GetTxRing(&AxiDma);

	/* Disable all TX interrupts before Tx BD space setup */

	XAxiDma_BdRingIntDisable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set TX delay and coalesce */
	XAxiDma_BdRingSetCoalesce(TxRingPtr, Coalesce, Delay);

	/* Setup Tx BD space  */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
				TX_BD_SPACE_HIGH - TX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(TxRingPtr, TX_BD_SPACE_BASE,
				TX_BD_SPACE_BASE,
				XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("failed create BD ring in txsetup\r\n");

		return XST_FAILURE;
	}

	/*
	 * We create an all-zero BD as the template.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("failed bdring clone in txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start the TX channel */
	Status = XAxiDma_BdRingStart(TxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("failed start bdring txsetup %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function transmits packets non-blockingly through the DMA engine.
*
* @param	AxiDmaInstPtr points to the DMA engine instance
*
* @return	- XST_SUCCESS if the DMA accepts the packet successfully,
*		- XST_FAILURE if failure.
*
* @note		None.
*
******************************************************************************/
static int SendPackets(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	u8 *TxPacket;
	u8 Value;
	XAxiDma_Bd *BdPtr;
	int Status;
	int i;
	UINTPTR BufAddr;
	XAxiDma_Bd *CurBdPtr;

	/* Create pattern in the packet to transmit
	 */
	TxPacket = (u8 *) Packet;

	Value = TEST_START_VALUE;

	for(i = 0; i < MAX_PKT_LEN * NUMBER_OF_PACKETS; i ++) {
		TxPacket[i] = Value;

		Value = (Value + 1) & 0xFF;
	}

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN *
							NUMBER_OF_PACKETS);
#ifdef __aarch64__
	Xil_DCacheFlushRange((UINTPTR)RX_BUFFER_BASE, MAX_PKT_LEN *
						 NUMBER_OF_PACKETS);
#endif

	TxRingPtr = XAxiDma_GetTxRing(AxiDmaInstPtr);

	/* Allocate BDs */
	Status = XAxiDma_BdRingAlloc(TxRingPtr, NUMBER_OF_PACKETS, &BdPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set up the BDs using the information of the packet to transmit */
	BufAddr = (UINTPTR)Packet;
	CurBdPtr = BdPtr;

	for (i = 0; i < NUMBER_OF_PACKETS; i++) {
		u32 CrBits = 0;

		Status = XAxiDma_BdSetBufAddr(CurBdPtr, BufAddr);
		if (Status != XST_SUCCESS) {
			xil_printf("Tx set buffer addr %x on BD %x failed %d\r\n",
			(unsigned int)BufAddr, (UINTPTR)CurBdPtr, Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(CurBdPtr, MAX_PKT_LEN,
				TxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Tx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (UINTPTR)CurBdPtr, Status);

			return XST_FAILURE;
		}

		if (i == 0) {
			CrBits |= XAXIDMA_BD_CTRL_TXSOF_MASK;

#if (XPAR_AXIDMA_0_SG_INCLUDE_STSCNTRL_STRM == 1)
			/* The first BD has total transfer length set in
			 * the last APP word, this is for the loopback widget
			 */
			Status = XAxiDma_BdSetAppWord(CurBdPtr,
			    XAXIDMA_LAST_APPWORD,
			    MAX_PKT_LEN * NUMBER_OF_PACKETS);

			if (Status != XST_SUCCESS) {
				xil_printf("Set app word failed with %d\r\n",
								Status);
			}
#endif
		}
		if (i == (NUMBER_OF_PACKETS - 1)) {
			CrBits |= XAXIDMA_BD_CTRL_TXEOF_MASK;
			XAxiDma_BdSetCtrl(CurBdPtr,
					XAXIDMA_BD_CTRL_TXEOF_MASK);
		}
		XAxiDma_BdSetCtrl(CurBdPtr, CrBits);

		XAxiDma_BdSetId(CurBdPtr, BufAddr);

		BufAddr += MAX_PKT_LEN;
		CurBdPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(TxRingPtr, CurBdPtr);
	}

	/* Give the BD to DMA to kick off the transmission. */
	Status = XAxiDma_BdRingToHw(TxRingPtr, NUMBER_OF_PACKETS, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("to hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return	- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation is failure.
*
* @note		None.
*
******************************************************************************/
static int CheckData(void)
{
	u8 *RxPacket;
	int i = 0;
	u8 Value;

	RxPacket = (u8 *) RX_BUFFER_BASE;
	Value = TEST_START_VALUE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
#ifndef __aarch64__
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, MAX_PKT_LEN *
								NUMBER_OF_PACKETS);
#endif

	for(i = 0; i < MAX_PKT_LEN * NUMBER_OF_PACKETS; i++) {
		if (RxPacket[i] != Value) {
			xil_printf("Data error %d: %x/%x\r\n",
			    i, (unsigned int)RxPacket[i], (unsigned int)Value);

			return XST_FAILURE;
		}
		Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function delays until the DMA transaction is finished, checks data,
* and cleans up.
*
* @param	AxiDmaInstPtr points to the DMA engine instance
*
* @return	- XST_SUCCESS if DMA transfer is successful and data is correct
*		- XST_FAILURE if failure
*
* @note		None.
*
******************************************************************************/
static int CheckDmaResult(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr;
	XAxiDma_BdRing *RxRingPtr;
	XAxiDma_Bd *BdPtr;
	u32 ProcessedBdCount = 0;
	u32 FreeBdCount;
	int Status;

	TxRingPtr = XAxiDma_GetTxRing(AxiDmaInstPtr);
	RxRingPtr = XAxiDma_GetRxRing(AxiDmaInstPtr);

	/* Wait until the TX transactions are done */
	while (ProcessedBdCount < NUMBER_OF_PACKETS) {
		ProcessedBdCount += XAxiDma_BdRingFromHw(TxRingPtr,
					       XAXIDMA_ALL_BDS, &BdPtr);
	}

	/* Free all processed TX BDs for future transmission */
	Status = XAxiDma_BdRingFree(TxRingPtr, ProcessedBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Wait until the data has been received by the Rx channel */
	ProcessedBdCount = 0;

	while (ProcessedBdCount < NUMBER_OF_PACKETS) {

		ProcessedBdCount += XAxiDma_BdRingFromHw(RxRingPtr,
					       XAXIDMA_ALL_BDS, &BdPtr);
	}


	/* Check received data */
	if (CheckData() != XST_SUCCESS) {

		return XST_FAILURE;
	}

	/* Free all processed RX BDs for future transmission */
	Status = XAxiDma_BdRingFree(RxRingPtr, ProcessedBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("free bd failed\r\n");
		return XST_FAILURE;
	}

	/* Return processed BDs to RX channel so we are ready to receive new
	 * packets:
	 *    - Allocate all free RX BDs
	 *    - Pass the BDs to RX channel
	 */
	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);
	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("bd alloc failed\r\n");
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);

	return Status;
}
