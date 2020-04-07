/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmcdma_polled_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in polling mode.
 *
 * This examples shows how to do multiple packets and multiple BD's
 * per packet transfers.
 *
 * H/W Requirements:
 * In order to test this example at the h/w level AXI MCDMA MM2S should
 * be connected to the S2MM channel.
 *
 * System level considerations for Zynq UltraScale+ designs:
 * On ZU+ MPSOC for PL IP's 3 different ports are available HP, HPC and ACP.
 *
 * The explanation below talks about HPC and HP port.
 *
 * HPC design considerations:
 * ZU+ MPSOC has in-built cache coherent interconnect(CCI) to take care of
 * Coherency through HPC port. CCI is only support at EL1 NS level.
 * Following needs to be done by the system components before running the
 * example-
 * 1) Snooping should be enabled in the S3 (0xFD6E4000)
 * 2) Mark the DDR memory being used for buffers as outer-shareable.
 * translation_table.S.
 * .set Memory,	0x405 | (2 << 8) | (0x0).
 *
 * It is recommended to use HPC to make use of H/W coherency feature.
 *
 * HP design considerations:
 * The example uses un-cached memory for buffer descriptors and uses
 * Normal memory for buffers..
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- --------   -------------------------------------------------------
 * 1.0	 adk  18/07/2017 Initial Version.
 * 1.2	 rsp  07/19/2018 Read channel count from IP config.
 *       rsp  08/17/2018 Fix typos and rephrase comments.
 *	 rsp  08/17/2018 Read Length register value from IP config.
 * 1.3   rsp  02/05/2019 Remove snooping enable from application.
 *       rsp  02/06/2019 Programmatically select cache maintenance ops for HPC
 *                       and non-HPC designs. In Rx remove arch64 specific dsb
 *                       instruction by performing cache invalidate operation
 *                       for all supported architectures.
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

#define NUM_MAX_CHANNELS	16

#define TEST_START_VALUE	0xC

int TxPattern[NUM_MAX_CHANNELS + 1];
int RxPattern[NUM_MAX_CHANNELS + 1];
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
int num_channels;

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

	/* Read numbers of channels from IP config */
	num_channels = Mcdma_Config->RxNumChannels;

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
        if (RxDone >= NUMBER_OF_BDS_TO_TRANSFER * num_channels)
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

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
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

				if(!McDmaInstPtr->Config.IsRxCacheCoherent)
					Xil_DCacheInvalidateRange(RxBufferPtr, MAX_PKT_LEN);

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
* @param	McDmaInstPtr is the instance pointer to the AXI MCDMA engine.
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

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
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
* @param	RxPacket is the pointer to Rx packet.
* @param	ByteCount is the length of Rx packet.
* @param	ChanId is the MCDMA channel id to be worked on.
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

	for(Index = 0; Index < ByteCount; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error : %x/%x\r\n",
						(unsigned int)RxPacket[Index],
						(unsigned int)Value);
			return XST_FAILURE;
			break;
		}
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
* @param	McDmaInstPtr is the instance pointer to the AXI MCDMA engine.
* @Chan_id	ChanId is the MCDMA channel id to be worked on.
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
        u8 *RxPacket;
        int ProcessedBdCount, i;
        int MaxTransferBytes;
        int RxPacketLength;

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
        MaxTransferBytes = MAX_TRANSFER_LEN(McDmaInstPtr->Config.MaxTransferlen - 1);

        /* Check received data */
        for (i = 0; i < ProcessedBdCount; i++) {
		RxPacket = (void *)XMcdma_BdRead64(BdPtr1, XMCDMA_BD_BUFA_OFFSET);
		RxPacketLength = XMcDma_BdGetActualLength(BdPtr1, MaxTransferBytes);
		/* Invalidate the DestBuffer before receiving the data,
		 * in case the data cache is enabled
		 */
		if (!McDmaInstPtr->Config.IsRxCacheCoherent)
			Xil_DCacheInvalidateRange((UINTPTR)RxPacket, RxPacketLength);

		if (CheckData((u8 *) RxPacket, RxPacketLength, Chan_id) != XST_SUCCESS) {
                        xil_printf("Data check failed for the Chan %x\n\r", Chan_id);
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

	for (ChanId = 1; ChanId <= num_channels; ChanId++) {
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
