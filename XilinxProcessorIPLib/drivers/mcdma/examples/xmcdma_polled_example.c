/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * @file xmcdma_polled_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in polling mode.
 *
 * This examples shows how to do multiple packets transfers,
 * as well as how to do multiple show how to do multiple packets transfers,
 * as well as how to do multiple BD's per packet transfers.
 *
 * H/W Requirments:
 * In order to test this example at the h/w level AXI MCDMA MM2S should
 * connect with the S2MM.
 *
 * System level Considerations for ZynqUltrascale+ designs:
 * On ZU+ MPSOC for PL IP's 3 differnet ports are availble HP, HPC and ACP.
 *
 * The explanation below talks about HPC and HP port.
 *
 * HPC design considerations:
 * ZU+ MPSOC has in-built cache coherent interconnect(CCI) to take care of
 * coherency through HPC port.
 * Following needs to be done by the users before running the examples.
 * 1) Snooping should be enabled in the S3 (0xFD6E4000)
 * 2) Mark the DDR memory being used for buffers as outer sharable.
 * To do that please modify baremetal bsp file translation_table.S.
 * Change
 * .set Memory,	0x405 | (3 << 8) | (0x0).
 * to
 * .set Memory,	0x405 | (2 << 8) | (0x0).
 *
 * Please uncomment below define for HPC design so that applicaiton won't do
 * Any Cache flush/invalidation.
 *
 * //#define HPC_DESIGN
 *
 * It is recomended to use HPC to make use of H/W coherency feature.
 *
 * HP design considerations:
 * The example uses un-cached memory for buffer descriptors and uses
 * Normal memory for buffers..
 *
 * A53 does not provide seperate instruction for cache invalidation.
 * It supports flush (clean + invalidation). Before a DMA starts,
 * Application is expected to do a cache flush for the relevant memory.
 * Once DMA ends, the data can simply be read from memory.
 * However, there will be occasions when A53 L1 cache system can prefetch memory locations
 * Which were earlier flushed. On such scenarios there is high probablity that CPU reads
 * Memory from cache and DMA is still not complete for this memory. This leads lost
 * Coherency between cache and memory. Subsequent data verification(after DMA is complete) thus fails.
 *
 * It is generally an unpredictable behavior. It is highly unlikely to happen for a single buffer usecase.
 * But for multiple buffers staying in adjacent cache locations,
 * There is a high probability that users can get into such failures.
 *
 *  The L1 prefetch is a feature of the L1 cache system for improving performance.
 *  The L1 cache has its own algorithm to prefetch. The prefetch stops when:
 *   -> the memory accesses cross a 4KB page boundary.
 *   -> a dsb or PRFM instruction is executed.
 *   -> the program execution does not hit the prefetched data lines.
 *
 * Accordingly the solution for the above problem is:
 *
 * 1) Use dsb
 *     The location of the dsb is crucial. The programmer needs to predict the maximum probabilty
 *     When the L1 prefetch will happen for relevant DMA addresses.
 *     It will be typically in the DMA done interrupt (where the data verification happens for a buffer).
 *     The cache line size is 64 bytes. The prefetch obviously will happen in chunks of 64 bytes.
 *     However, because of the unpredictability nature of the prefetch, it is difficult to find out the exact point of dsb.
 *     To be at a safer side, the dsb can be put for every memory location fetched.
 *
 *     There will be heavy performance penalty. Every dsb clears the store buffers.
 *     Executing dsbs very frequently will degrade the performance significantly.
 *
 *    2) Disable Prefetch of L1 Cahe
 *       This can be done by setting the CPUACTLR_EL1 register.
 *
 *    3) Put buffers from 4k apart
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- --------   -------------------------------------------------------
 * 1.0	 adk  18/07/2017 Initial Version.
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xmcdma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xmcdma_hw.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif


/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#define MCDMA_DEV_ID	XPAR_MCDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifdef XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_R5_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x10000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x10000000)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x20000000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x50000000)

#define NUMBER_OF_BDS_PER_PKT		10
#define NUMBER_OF_PKTS_TO_TRANSFER 	100
#define NUMBER_OF_BDS_TO_TRANSFER	(NUMBER_OF_PKTS_TO_TRANSFER * \
						NUMBER_OF_BDS_PER_PKT)

#define MAX_PKT_LEN		1024
#define BLOCK_SIZE_2MB 0x200000U

#define NUM_CHANNELS	16

#define TEST_START_VALUE	0xC

#ifdef __aarch64__
// #define HPC_DESIGN
#endif

int TxPattern[NUM_CHANNELS + 1];
int RxPattern[NUM_CHANNELS + 1];
int TestStartValue[] = {0xC, 0xB, 0x3, 0x55, 0x33, 0x20, 0x80, 0x66, 0x88};

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
static int RxSetup(XMcdma *McDmaInstPtr);
static int TxSetup(XMcdma *McDmaInstPtr);
static int SendPacket(XMcdma *McDmaInstPtr);
static int CheckData(u8 *RxPacket, int ByteCount, u32 ChanId);
static int CheckDmaResult(XMcdma * McDmaInstPtr, u32 Chan_id);
static void Mcdma_Poll(XMcdma * McDmaInstPtr);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XMcdma AxiMcdma;

volatile int TxDone;
volatile int RxDone;

/*
 * Buffer for transmit packet. Must be 32-bit aligned to be used by DMA.
 */
UINTPTR *Packet = (UINTPTR *) TX_BUFFER_BASE;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the tests on DMA core. It sets up
* DMA engine to be ready to receive and send packets, then a packet is
* transmitted and will be verified after it is received via the DMA.
*
* @param	None
*
* @return
*		- XST_SUCCESS if test passes
*		- XST_FAILURE if test fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status, i;

	TxDone = 0;
	RxDone = 0;

	XMcdma_Config *Mcdma_Config;

	xil_printf("\r\n--- Entering main() --- \r\n");

#ifdef HPC_DESIGN
	Xil_Out32(0xFD6E4000,0x1);
#endif

#ifdef __aarch64__
#if (TX_BD_SPACE_BASE < 0x100000000UL)
	for (i = 0; i < (RX_BD_SPACE_BASE - TX_BD_SPACE_BASE) / BLOCK_SIZE_2MB; i++) {
		Xil_SetTlbAttributes(TX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
		Xil_SetTlbAttributes(RX_BD_SPACE_BASE + (i * BLOCK_SIZE_2MB), NORM_NONCACHE);
	}
#else
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, NORM_NONCACHE);
#endif
#endif


	Mcdma_Config = XMcdma_LookupConfig(MCDMA_DEV_ID);
	if (!Mcdma_Config) {
			xil_printf("No config found for %d\r\n", MCDMA_DEV_ID);

			return XST_FAILURE;
	}


	Status = XMcDma_CfgInitialize(&AxiMcdma, Mcdma_Config);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}


	Status = TxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = RxSetup(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = SendPacket(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	/* Check DMA transfer result */
    while (1) {
        Mcdma_Poll(&AxiMcdma);
        if (RxDone >= NUMBER_OF_BDS_TO_TRANSFER * NUM_CHANNELS)
              break;
   }


	xil_printf("AXI MCDMA SG Polling Test %s\r\n",
		(Status == XST_SUCCESS)? "passed":"failed");

	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	McDmaInstPtr is the pointer to the instance of the AXI MCDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Rx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR RxBufferPtr;
	UINTPTR RxBdSpacePtr;
	int Status;
	u32 i, j;
	u32 buf_align;

	RxBufferPtr = RX_BUFFER_BASE;
	RxBdSpacePtr = RX_BD_SPACE_BASE;

	for (ChanId = 1; ChanId <= NUM_CHANNELS; ChanId++) {
		Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Rx_Chan, RxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				Status = XMcDma_ChanSubmit(Rx_Chan, RxBufferPtr,
							   MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				/* Clear the receive buffer, so we can verify data */
				memset((void *)RxBufferPtr, 0, MAX_PKT_LEN);

#ifndef HPC_DESIGN
			Xil_DCacheInvalidateRange(RxBufferPtr, MAX_PKT_LEN);
#endif
				RxBufferPtr += MAX_PKT_LEN;
				if (!Rx_Chan->Has_Rxdre) {
					buf_align = RxBufferPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
					RxBufferPtr += buf_align;
				}
			}
		}

		Status = XMcDma_ChanToHw(Rx_Chan);
		if (Status != XST_SUCCESS) {
				xil_printf("XMcDma_ChanToHw failed\n\r");
				return XST_FAILURE;
		}

		RxBufferPtr += MAX_PKT_LEN;
		if (!Rx_Chan->Has_Rxdre) {
			buf_align = RxBufferPtr % 64;
			if (buf_align > 0)
				buf_align = 64 - buf_align;
			RxBufferPtr += buf_align;
		}
		RxBdSpacePtr += BdCount * Rx_Chan->Separation;
		XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);
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
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int TxSetup(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan;
	int ChanId;
	int BdCount = NUMBER_OF_BDS_TO_TRANSFER;
	UINTPTR TxBufferPtr;
	UINTPTR TxBdSpacePtr;
	int Status;
	u32 i, j;
	u32 buf_align;

	TxBufferPtr = TX_BUFFER_BASE;
	TxBdSpacePtr = TX_BD_SPACE_BASE;

	for (ChanId = 1; ChanId <= NUM_CHANNELS; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Tx_Chan, TxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		for (j = 0 ; j < NUMBER_OF_PKTS_TO_TRANSFER; j++) {
			for (i = 0 ; i < NUMBER_OF_BDS_PER_PKT; i++) {
				Status = XMcDma_ChanSubmit(Tx_Chan, TxBufferPtr,
							  MAX_PKT_LEN);
				if (Status != XST_SUCCESS) {
					xil_printf("ChanSubmit failed\n\r");
					return XST_FAILURE;
				}

				TxBufferPtr += MAX_PKT_LEN;
				if (!Tx_Chan->Has_Txdre) {
					buf_align = TxBufferPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
				    TxBufferPtr += buf_align;
				}

				/* Clear the receive buffer, so we can verify data */
				memset((void *)TxBufferPtr, 0, MAX_PKT_LEN);

			}
		}

		TxBufferPtr += MAX_PKT_LEN;
		if (!Tx_Chan->Has_Txdre) {
			buf_align = TxBufferPtr % 64;
			if (buf_align > 0)
				buf_align = 64 - buf_align;
		    TxBufferPtr += buf_align;
		}

		TxBdSpacePtr += BdCount * Tx_Chan->Separation;
		XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);
	 }


	return XST_SUCCESS;
}

/*****************************************************************************/
/*
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
static int CheckData(u8 *RxPacket, int ByteCount, u32 ChanId)
{
	u32 Index;
	u8 Value;


	Value = TestStartValue[ChanId] + RxPattern[ChanId]++;


	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
#ifndef __aarch64__
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, ByteCount);
#endif

	for(Index = 0; Index < ByteCount; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error : %x/%x\r\n",
						(unsigned int)RxPacket[Index],
						(unsigned int)Value);
			return XST_FAILURE;
			break;
		}
#ifndef HPC_DESIGN
#ifdef __aarch64__
		dsb();
#endif
#endif
		Value = (Value + 1) & 0xFF;
	}


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits until the DMA transaction is finished, checks data,
* and cleans up.
*
* @param	None
*
* @return	- XST_SUCCESS if DMA transfer is successful and data is correct,
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
static int CheckDmaResult(XMcdma *McDmaInstPtr, u32 Chan_id)
{
        XMcdma_ChanCtrl *Rx_Chan = 0, *Tx_Chan = 0;
        XMcdma_Bd *BdPtr1;
        int ProcessedBdCount, i;

        Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Tx_Chan,
                                                0xFFFF,
                                                &BdPtr1);
        TxDone += ProcessedBdCount;

        Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Rx_Chan,
                                                        0xFFFF,
                                                        &BdPtr1);
        RxDone += ProcessedBdCount;

        /* Check received data */
        for (i = 0; i < ProcessedBdCount; i++) {
                if (CheckData((u8 *)XMcdma_BdRead64(BdPtr1, XMCDMA_BD_BUFA_OFFSET),
                                          XMcDma_BdGetActualLength(BdPtr1, 0x00FFFFFF), Chan_id) != XST_SUCCESS) {
                        xil_printf("Data check failied for the Chan %x\n\r", Chan_id);
                        return XST_FAILURE;
                }
                BdPtr1 = (XMcdma_Bd *) XMcdma_BdRead64(BdPtr1, XMCDMA_BD_NDESC_OFFSET);
        }

        return XST_SUCCESS;
}


static void Mcdma_Poll(XMcdma * McDmaInstPtr) {
        u16 Chan_id = 1;
        u32 i;
        u32 Chan_SerMask;

        Chan_SerMask = XMcdma_ReadReg(McDmaInstPtr->Config.BaseAddress,
                                      XMCDMA_RX_OFFSET + XMCDMA_RXCH_SER_OFFSET);

        for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask; i <<= 1, Chan_id++)
             if (Chan_SerMask & i)
                 CheckDmaResult(&AxiMcdma, Chan_id);
}


static int SendPacket(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan = NULL;
	u32 Index, Pkts, Index1;
	XMcdma_Bd *BdCurPtr;
	u32 Status;
	u8 *TxPacket;
	u8 Value;
	u32 ChanId;

	for (ChanId = 1; ChanId <= NUM_CHANNELS; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		BdCurPtr = XMcdma_GetChanCurBd(Tx_Chan);
		for(Index = 0; Index < NUMBER_OF_PKTS_TO_TRANSFER; Index++) {
			for(Pkts = 0; Pkts < NUMBER_OF_BDS_PER_PKT; Pkts++) {
				u32 CrBits = 0;

				Value = TestStartValue[ChanId] + TxPattern[ChanId]++;
				TxPacket = (u8 *)XMcdma_BdRead64(BdCurPtr, XMCDMA_BD_BUFA_OFFSET);
				for(Index1 = 0; Index1 < MAX_PKT_LEN; Index1++) {
					TxPacket[Index1] = Value;

					Value = (Value + 1) & 0xFF;
				}
				Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN);

				if (Pkts == 0) {
					CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
				}

				if (Pkts == (NUMBER_OF_BDS_PER_PKT - 1)) {
					CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
				}
				XMcDma_BdSetCtrl(BdCurPtr, CrBits);
				XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
				BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
			}
		}

		Status = XMcDma_ChanToHw(Tx_Chan);
		if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed for Channel %d\n\r", ChanId);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
