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
 * @file xmcdma_interrupt_example.c
 *
 * This file demonstrates how to use the mcdma driver on the Xilinx AXI
 * MCDMA core (AXI MCDMA) to transfer packets in interrupt mode.
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
 * Please refer xmcdma_polled_example.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date      Changes
 * ----- ---- --------  -------------------------------------------------------
 * 1.0	 adk  18/07/17	Initial Version.
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

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#define MCDMA_DEV_ID	XPAR_AXI_MCDMA_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_MCDMA_0_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_MCDMA_0_VEC_ID
#else
#define TX_INTR_ID(ChanId) XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH##ChanId##_INTROUT_INTR
#define RX_INTR_ID(ChanId) XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH##ChanId##_INTROUT_INTR
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

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

#define PACKETS_PER_IRQ 50
#define MAX_PKT_LEN		1024
#define BLOCK_SIZE_2MB 0x200000U

#define NUM_CHANNELS	8

#define TEST_START_VALUE	0xC
#ifdef __aarch64__
// #define HPC_DESIGN
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
static INTC Intc;	/* Instance of the Interrupt Controller */


/************************** Function Prototypes ******************************/
static int RxSetup(XMcdma *McDmaInstPtr);
static int TxSetup(XMcdma *McDmaInstPtr);
static int SendPacket(XMcdma *McDmaInstPtr);
static int CheckData(u8 *RxPacket, int ByteCount);
static int SetupIntrSystem(INTC * IntcInstancePtr, XMcdma *McDmaInstPtr, u16 IntrId, u8 Direction);
static void TxDoneHandler(void *CallBackRef, u32 Chan_id);
static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static void DoneHandler(void *CallBackRef, u32 Chan_id);
static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask);
static int ChanIntr_Id(XMcdma_ChanCtrl *Chan, int ChanId);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XMcdma AxiMcdma;

volatile int RxChanDone;
volatile int TxChanDone;
volatile int RxDone;
volatile int TxDone;
volatile int Error;


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

	/* Initalize flgas */
	RxChanDone = 0;
	TxChanDone  = 0;
	RxDone = 0;
	TxDone = 0;
	Error = 0;

	Status = SendPacket(&AxiMcdma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	 while (1) {
	        if ((RxDone >= NUMBER_OF_BDS_TO_TRANSFER * NUM_CHANNELS) && !Error)
	              break;
	 }

	xil_printf("AXI MCDMA SG Interrupt Test %s\r\n",
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
				Xil_DCacheInvalidateRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN);
#endif
				RxBufferPtr += MAX_PKT_LEN;
				if (!Rx_Chan->Has_Rxdre) {
					buf_align = RxBufferPtr % 64;
					if (buf_align > 0)
						buf_align = 64 - buf_align;
					RxBufferPtr += buf_align;
				}
			}

			RxBufferPtr += MAX_PKT_LEN;
			if (!Rx_Chan->Has_Rxdre) {
				buf_align = RxBufferPtr % 64;
				if (buf_align > 0)
					buf_align = 64 - buf_align;
				RxBufferPtr += buf_align;
			}
			XMcdma_SetChanCoalesceDelay(Rx_Chan, PACKETS_PER_IRQ, 255);
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

		/* Setup Interrupt System and register callbacks */
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_DONE,
		                          (void *)DoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_HANDLER_ERROR,
		                          (void *)ErrorHandler, McDmaInstPtr);

		Status = SetupIntrSystem(&Intc, McDmaInstPtr, ChanIntr_Id(Rx_Chan, ChanId),
					 XMCDMA_MEM_TO_DEV);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed RX interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}
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

			TxBufferPtr += MAX_PKT_LEN;
			if (!Tx_Chan->Has_Txdre) {
				buf_align = TxBufferPtr % 64;
				if (buf_align > 0)
					buf_align = 64 - buf_align;
			    TxBufferPtr += buf_align;
			}
		}

		TxBdSpacePtr += BdCount * Tx_Chan->Separation;
		XMcdma_SetChanCoalesceDelay(Tx_Chan, PACKETS_PER_IRQ, 255);

		/* Setup Interrupt System and register callbacks */
		 XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_DONE,
                            (void *)TxDoneHandler, McDmaInstPtr);
		XMcdma_SetCallBack(McDmaInstPtr, XMCDMA_TX_HANDLER_ERROR,
                             (void *)TxErrorHandler, McDmaInstPtr);

		Status = SetupIntrSystem(&Intc, McDmaInstPtr, ChanIntr_Id(Tx_Chan, ChanId),
					XMCDMA_DEV_TO_MEM);
		if (Status != XST_SUCCESS) {
		      xil_printf("Failed Tx interrupt setup %d\r\n", ChanId);
		      return XST_FAILURE;
		}

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
static int CheckData(u8 *RxPacket, int ByteCount)
{
	u32 Index;
	u8 Value;


	Value = TEST_START_VALUE;

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

static int SendPacket(XMcdma *McDmaInstPtr)
{
	XMcdma_ChanCtrl *Tx_Chan = NULL;
	u32 Index, Pkts, Index1;
	XMcdma_Bd *BdCurPtr;
	u32 Status;
	u8 *TxPacket;
	u8 Value;
	u32 ChanId;

	BdCurPtr = (XMcdma_Bd *)TX_BD_SPACE_BASE;
	for (ChanId = 1; ChanId <= NUM_CHANNELS; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		for(Index = 0; Index < NUMBER_OF_PKTS_TO_TRANSFER; Index++) {
			for(Pkts = 0; Pkts < NUMBER_OF_BDS_PER_PKT; Pkts++) {
				u32 CrBits = 0;

				Value = TEST_START_VALUE;
				TxPacket = (u8 *)XMcdma_BdRead64(BdCurPtr, XMCDMA_BD_BUFA_OFFSET);
				for(Index1 = 0; Index1 < MAX_PKT_LEN; Index1++) {
					TxPacket[Index1] = Value;

					Value = (Value + 1) & 0xFF;
				}

#ifndef HPC_DESIGN
				Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN);
#endif
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

		BdCurPtr = (XMcdma_Bd *)(TX_BD_SPACE_BASE +
				   (sizeof(XMcdma_Bd) * NUMBER_OF_BDS_TO_TRANSFER * ChanId));
		Status = XMcDma_ChanToHw(Tx_Chan);
		if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed for Channel %d\n\r", ChanId);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

static void DoneHandler(void *CallBackRef, u32 Chan_id)
{
        XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
        XMcdma_ChanCtrl *Rx_Chan = 0;
        XMcdma_Bd *BdPtr1, *FreeBdPtr;
        int ProcessedBdCount, i;

        Rx_Chan = XMcdma_GetMcdmaRxChan(InstancePtr, Chan_id);
        ProcessedBdCount = XMcdma_BdChainFromHW(Rx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);
        RxDone += ProcessedBdCount;

        FreeBdPtr = BdPtr1;

        for (i = 0; i < ProcessedBdCount; i++) {
                if (CheckData(XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_BUFA_OFFSET),
			      XMcDma_BdGetActualLength(FreeBdPtr, 0x00FFFFFF)) != XST_SUCCESS) {
                        xil_printf("Data check failied for the Chan %x\n\r", Chan_id);
                }
                FreeBdPtr = (XMcdma_Bd *) XMcdma_BdRead64(FreeBdPtr, XMCDMA_BD_NDESC_OFFSET);
        }

        RxChanDone += 1;
}

static void ErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside error Handler Chan_id is %d\n\r", Chan_id);
	Error = 1;
}

static void TxDoneHandler(void *CallBackRef, u32 Chan_id)
{
	XMcdma *InstancePtr = (XMcdma *)((void *)CallBackRef);
	XMcdma_ChanCtrl *Tx_Chan = 0;
	XMcdma_Bd *BdPtr1;
	int ProcessedBdCount;

	Tx_Chan = XMcdma_GetMcdmaTxChan(InstancePtr, Chan_id);
	ProcessedBdCount = XMcdma_BdChainFromHW(Tx_Chan, NUMBER_OF_BDS_TO_TRANSFER, &BdPtr1);

	TxDone += ProcessedBdCount;
	TxChanDone += 1;
}

static void TxErrorHandler(void *CallBackRef, u32 Chan_id, u32 Mask)
{
	xil_printf("Inside Tx error Handler Chan_id is %d and Mask %x\n\r", Chan_id, Mask);
	Error = 1;
}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	Chan is the MCDMA Channel instance to be worked on.
* @param	InrId is the MCDMA Channel Interrupt Id.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE.if not succesful
*
* @note		None.
*
******************************************************************************/

static int SetupIntrSystem(INTC * IntcInstancePtr, XMcdma *McDmaInstPtr, u16 IntrId, u8 Direction)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

	if (Direction == XMCDMA_DEV_TO_MEM)
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) XMcdma_TxIntrHandler, McDmaInstPtr);
	else
		Status = XIntc_Connect(IntcInstancePtr, IntrId,
				       (XInterruptHandler) XMcdma_IntrHandler, McDmaInstPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed tx connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, IntrId);
#else

	XScuGic_Config *IntcConfig;
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	if (Direction == XMCDMA_DEV_TO_MEM)
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)XMcdma_TxIntrHandler, McDmaInstPtr);
	else
		Status = XScuGic_Connect(IntcInstancePtr, IntrId,
					 (Xil_InterruptHandler)XMcdma_IntrHandler, McDmaInstPtr);


	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, IntrId);

#endif
	/* Enable interrupts from the hardware */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER,
				(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
 * This function maps the mcdma channel with the Corresponding interrupt id
 * Generated in the xparameters.h file.
 *
 * @param	Chan is the MCDMA Channel instance to be worked on.
 * @param:	ChanId is the MCDMA channel id to be worked on
 *
 * @return:
 *		- Corresponding interrupt ID on success.
 *		- XST_FAILURE if unable to find a valid interrupt id
 * 		  For a given MCDMA channel.
 *
 * @note:	Make sure the XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH1_INTROUT_INTR
 *		Checks in the API below are properly mapped for your design.
 */
/*****************************************************************************/
static int ChanIntr_Id(XMcdma_ChanCtrl *Chan, int ChanId)
{


	switch(ChanId) {
	case 1:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH1_INTROUT_INTR
			return TX_INTR_ID(1);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH1_INTROUT_INTR
			return RX_INTR_ID(1);
#endif
		}
	case 2:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH2_INTROUT_INTR
			return TX_INTR_ID(2);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH2_INTROUT_INTR
			return RX_INTR_ID(2);
#endif
		}
	case 3:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH3_INTROUT_INTR
			return TX_INTR_ID(3);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH3_INTROUT_INTR
			return RX_INTR_ID(3);
#endif
		}
	case 4:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH4_INTROUT_INTR
			return TX_INTR_ID(4);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH4_INTROUT_INTR
			return RX_INTR_ID(4);
#endif
		}
	case 5:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH5_INTROUT_INTR
			return TX_INTR_ID(5);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH5_INTROUT_INTR
			return RX_INTR_ID(5);
#endif
		}
	case 6:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH6_INTROUT_INTR
			return TX_INTR_ID(6);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH6_INTROUT_INTR
			return RX_INTR_ID(6);
#endif
		}
	case 7:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH7_INTROUT_INTR
			return TX_INTR_ID(7);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH7_INTROUT_INTR
			return RX_INTR_ID(7);
#endif
		}
	case 8:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH8_INTROUT_INTR
			return TX_INTR_ID(8);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH8_INTROUT_INTR
			return RX_INTR_ID(8);
#endif
		}
	case 9:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH9_INTROUT_INTR
			return TX_INTR_ID(9);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH9_INTROUT_INTR
			return RX_INTR_ID(9);
#endif
		}
	case 10:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH10_INTROUT_INTR
			return TX_INTR_ID(10);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH10_INTROUT_INTR
			return RX_INTR_ID(10);
#endif
		}
	case 11:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH11_INTROUT_INTR
			return TX_INTR_ID(11);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH11_INTROUT_INTR
			return RX_INTR_ID(11);
#endif
		}
	case 12:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH12_INTROUT_INTR
			return TX_INTR_ID(12);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH12_INTROUT_INTR
			return RX_INTR_ID(12);
#endif
		}
	case 13:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH13_INTROUT_INTR
			return TX_INTR_ID(13);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH13_INTROUT_INTR
			return RX_INTR_ID(13);
#endif
		}
	case 14:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH14_INTROUT_INTR
			return TX_INTR_ID(14);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH14_INTROUT_INTR
			return RX_INTR_ID(14);
#endif
		}
	case 15:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH15_INTROUT_INTR
			return TX_INTR_ID(15);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH15_INTROUT_INTR
			return RX_INTR_ID(15);
#endif
		}
	case 16:
		if(!(Chan->IsRxChan)) {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_MM2S_CH16_INTROUT_INTR
			return TX_INTR_ID(16);
#endif
		} else {
#ifdef XPAR_FABRIC_AXI_MCDMA_0_S2MM_CH16_INTROUT_INTR
			return RX_INTR_ID(16);
#endif
		}
	default:
		break;
	}

	return XST_FAILURE;
}
