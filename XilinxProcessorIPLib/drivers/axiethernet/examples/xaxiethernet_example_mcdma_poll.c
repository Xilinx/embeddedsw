/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet_example_mcdma_poll.c
*
* Implements examples that utilize the Axi Ethernet's polled mode MCDMA
* packet transfer mode to send and receive frames.
*
* These examples demonstrate:
*
* - How to perform simple send and receive
* - Checksum offload
* - How to perform multiple frames send and receive
*
* Functional guide to example:
*
* - AxiEthernetSgDmaSingleFrameExample demonstrates the simplest way to
*   send and receive frames in polling mode.
*
* - AxiEthernetSgDmaPartialChecksumOffloadExample demonstrates the partial
*   checksum offloading. The HW must be setup for partial checksum
*   offloading for this example to execute.
*
* - AxiEthernetSgDmaFullChecksumOffloadExample demonstrates the full
*   checksum offloading. The HW must be setup for full checksum offloading
*   for this example to execute.
*
* - AxiEthernetSgDmaMultiFrameExample demonstrates the way to send and recv
*   Multiple frames in polling mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   adk           Initial Release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#include "xmcdma.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "stdio.h"		/* stdio */
#include "stdlib.h"

#ifdef XPAR_XUARTNS550_NUM_INSTANCES
#include "xuartns550_l.h"
#endif

#if defined __aarch64__
#include "xil_mmu.h"
#endif
/*************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define AXIETHERNET_DEVICE_ID	XPAR_AXIETHERNET_0_DEVICE_ID
#define AXIMCDMA_DEVICE_ID	XPAR_MCDMA_0_DEVICE_ID

#define RXBD_CNT			1024	/* Number of RxBDs to use */
#define TXBD_CNT			1024	/* Number of TxBDs to use */
#define BD_ALIGNMENT			64	/* Byte alignment of BDs */
#define PAYLOAD_SIZE			1000 	/* Payload size used in examples */
#define BD_USR0_OFFSET			0 	/* AXI4-Stream Control Word offset from
						 * the start of app user words in BD. Offset
						 * 0 means, Control Word 0, used for enabling
						 * checksum offloading.
						 */
#define BD_USR1_OFFSET			1	/* AXI4-Stream Control Word offset from
						 * the start of app user words in BD. Offset
						 * 1 means, Control Word 1, used for
						 * mentioning checksum begin and checksum
						 * insert points
						 */
#define BD_USR2_OFFSET			2	/* AXI4-Stream Control Word offset from
						 * the start of app user words in BD. Offset
						 * 2 means, Control Word 2, used for
						 * mentioning checksum seed.
						 */

#define PARTIAL_CSUM_ENABLE		0x00000001 /* Option for partial csum enable */
#define FULL_CSUM_ENABLE		0x00000002 /* Option for full csum enable */
#define TX_CS_INIT_OFFSET		16	   /* Offset in the control word where
						    * byte offset in the ethernet frame
						    * to start CSUM calculation need to
						    * be inserted
						    */
#define RCV_FRM_NOT_CORRUPTED		0xFFFF	   /* If the received frame is not
						    * corrupted, for partial csum
						    * offloading case, the control
						    * word 3 would return 0xFFFF
						    */

#define IP_VERSION_4			0x800	   /* For IPV4, the Ethernet frame
						    * type/length field will have a
						    * value of 0x800
						    */
#define IP_HEADER_VERSION		0x04	   /* For IPv4, the version entry in
						    * IP header is always 4
						    */
#define IP_HEADER_LEN_IN_WORDS		0x05	   /* For our case, the header length
						    * is always 20 bytes (5 words
						    */
#define IP_HEADER_LENGTH		20	   /* IP header length in bytes. Used
						    * as offset to kigure out the start
						    * of TCP header.
						    */
#define IP_HEADER_PROTOCOL_TCP		0x6	   /* IP header protocol entry. For TCP
						    * packets, it is 6.
						    */
#define IP_HEADER_PROTOCOL_UDP		0x17 	   /* IP header protocol entry. For UDP
						    * packets, it is 17.
						    */
#define TCP_HEADER_LEN_IN_WORDS		0x5	   /* For our case, the header length
						    * is always 20 bytes (5 words)
						    */
#define TCP_HEADER_LENGTH		20	   /* IP header length in bytes. Used
						    * as offset to kigure out the start
						    * of TCP header.
						    */
#define FULL_CSUM_STATUS_MASK		0x00000038 /* Mask to extract full checksum
						    * status field from AXI4 Stream
						    * Status Word 2.
						    */

#define FULL_CSUM_VALIDATED		0x00000002 /* If bits 3-5 in AXI4 Status word
						    * have a value of 0x010, it means
						    * both IP and TCP checksums have
						    * been found to be correct.
						    */
#define IP_TCP_CSUMS_NOT_CHECKED	0x00000000
#define IP_CSUM_OK_TCP_NOT_CHECKED	0x00000001
#define TCP_CSUM_NOT_CHECKED_IP_NOT_OK	0x00000005
#define IP_CSUM_OK_TCP_NOT_OK		0x00000006

#define DUMMY_TCP_PORT_1		0x1111
#define DUMMY_TCP_PORT_2		0x1112
/*
 * Number of bytes to reserve for BD space for the number of BDs desired
 */
#define RXBD_SPACE_BYTES RXBD_CNT * 64 * 16
#define TXBD_SPACE_BYTES TXBD_CNT * 64 * 16
#define BLOCK_SIZE_2MB 0x200000U

/*************************** Variable Definitions ****************************/

static EthernetFrame TxFrame;	/* Transmit buffer */
static EthernetFrame RxFrame;	/* Receive buffer */

XAxiEthernet AxiEthernetInstance;
XMcdma DmaInstance;

/*
 * Aligned memory segments to be used for buffer descriptors
 */
char RxBdSpace[RXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char TxBdSpace[TXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));


/*
 * Counters to be incremented by callbacks
 */
volatile int FramesRx;	/* Num of frames that have been received */
volatile int FramesTx;	/* Num of frames that have been sent */
volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */
volatile int ExternalLoopback; /* Variable for External loopback */
volatile int Hascsum;	/* Tells whether h/w is capable of CSUM or not */

/*************************** Function Prototypes *****************************/
/*
 * Examples
 */
int AxiEthernetSgDmaPollExample(XAxiEthernet *AxiEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				u16 AxiEthernetDeviceId,
				u16 AxiMcDmaDeviceId);
int AxiEthernetSgDmaSingleFrameExample(XAxiEthernet *AxiEthernetInstancePtr,
					   XMcdma *DmaInstancePtr, u8 ChanId);
int AxiEthernetSgDmaMultiFrameExample(XAxiEthernet *AxiEthernetInstancePtr,
					   XMcdma *DmaInstancePtr, u8 ChanId);
int AxiEthernetSgDmaPartialChecksumOffloadExample(XAxiEthernet *AxiEthernetInstancePtr,
						  XMcdma *DmaInstancePtr, u8 ChanId);
int AxiEthernetSgDmaFullChecksumOffloadExample(XAxiEthernet *AxiEthernetInstancePtr,
					       XMcdma *DmaInstancePtr, u8 ChanId);
static int RxBdSetup(XMcdma *McDmaInstPtr, XAxiEthernet *AxiEthernetInstancePtr);
static int TxBdSetup(XMcdma *McDmaInstPtr, XAxiEthernet *AxiEthernetInstancePtr);
void AxiEthernetPHYRegistersDump(XAxiEthernet * AxiEthernetInstancePtr);

/*****************************************************************************/
/**
*
* This is the main function for the Axi Ethernet example.
*
* @param	None.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure
*
* @note		None.
*
****************************************************************************/
int main(void)
{
	int Status;

	AxiEthernetUtilErrorTrap("\r\n--- Enter main() ---");
	AxiEthernetUtilErrorTrap("This test may take several minutes to finish");

	Status = AxiEthernetSgDmaPollExample(&AxiEthernetInstance,
					     &DmaInstance,
					     AXIETHERNET_DEVICE_ID,
					     AXIMCDMA_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Polled Mode Test Failed");
		AxiEthernetUtilErrorTrap("--- Exiting main() ---");
		return XST_FAILURE;
	}

	AxiEthernetUtilErrorTrap("Polled Mode Test passed");
	AxiEthernetUtilErrorTrap("--- Exiting main() ---");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function demonstrates the usage usage of the Axi Ethernet by sending
* and receiving frames in polled MCDMA mode.
*
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	DmaInstancePtr is a pointer to the instance of the AXI MCDMA
*		component.
* @param	AxiEthernetDeviceId is Device ID of the Axi Ethernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiMcDmaDeviceId is Device ID of the Axi DMAA Device ,
*		typically XPAR_<AXIMCDMA_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		AxiMcdma hardware must be initialized before initializing
*		AxiEthernet. Since AxiMcdma reset line is connected to the
*		AxiEthernet reset line, a reset of AxiMcdma hardware during its
*		initialization would reset AxiEthernet.
*
******************************************************************************/
int AxiEthernetSgDmaPollExample(XAxiEthernet *AxiEthernetInstancePtr,
				XMcdma *DmaInstancePtr,
				u16 AxiEthernetDeviceId,
				u16 AxiMcDmaDeviceId)
{
	int Status;
	int LoopbackSpeed;
	XAxiEthernet_Config *MacCfgPtr;
	XMcdma_Config* DmaConfig;
	u8 ChanId;

	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Get the configuration of AxiEthernet hardware.
	 */
	MacCfgPtr = XAxiEthernet_LookupConfig(AxiEthernetDeviceId);

	/*
	 * Check whether MCDMA is present or not
	 */
	if(MacCfgPtr->AxiDevType != XPAR_AXI_MCDMA) {
		AxiEthernetUtilErrorTrap
			("Device HW not configured for MCDMA mode\r\n");
		return XST_FAILURE;
	}

	DmaConfig = XMcdma_LookupConfig(AxiMcDmaDeviceId);

	/*
	 * Initialize AXIMCDMA engine. AXIMCDMA engine must be initialized before
	 * AxiEthernet. During AXIMCDMA engine initialization, AXIMCDMA hardware is
	 * reset, and since AXIMCDMA reset line is connected to AxiEthernet, this
	 * would ensure a reset of AxiEthernet.
	 */
	Status = XMcDma_CfgInitialize(DmaInstancePtr, DmaConfig);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error initializing DMA\r\n");
		return XST_FAILURE;
	}

	/*
	 * Initialize AxiEthernet hardware.
	 */
	Status = XAxiEthernet_CfgInitialize(AxiEthernetInstancePtr, MacCfgPtr,
					MacCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in initialize");
		return XST_FAILURE;
	}

	if (MacCfgPtr->Enable_1588)
		Padding = 8;

#if defined(__aarch64__)
        Xil_SetTlbAttributes((UINTPTR)TxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
        Xil_SetTlbAttributes((UINTPTR)TxBdSpace + BLOCK_SIZE_2MB, NORM_NONCACHE | INNER_SHAREABLE);
        Xil_SetTlbAttributes((UINTPTR)RxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
        Xil_SetTlbAttributes((UINTPTR)RxBdSpace + BLOCK_SIZE_2MB, NORM_NONCACHE | INNER_SHAREABLE);
#endif

	/*
	 * Setup Buffer Descriptors for MCDMA
	 * Rx and Tx Channels
	 */
	RxBdSetup(DmaInstancePtr, AxiEthernetInstancePtr);
	TxBdSetup(DmaInstancePtr, AxiEthernetInstancePtr);

	/*
	 * Set PHY to loopback, speed depends on phy type.
	 * MII is 100 and all others are 1000.
	 */
	if (XAxiEthernet_GetPhysicalInterface(AxiEthernetInstancePtr) ==
	                                      XAE_PHY_TYPE_MII) {
	        LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED;
	} else if (MacCfgPtr->Speed == 2500){
	        LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED_2p5G;
	} else {
		LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED_1G;
	}

	/*
	 * Uncomment Below line of code if user would like to test
	 * External loopback
	 */
	/* ExternalLoopback = 1; */
	AxiEthernetUtilEnterLoopback(AxiEthernetInstancePtr, LoopbackSpeed);

	/*
	 * Set PHY<-->MAC data clock
	 */
	Status =  XAxiEthernet_SetOperatingSpeed(AxiEthernetInstancePtr,
	               (u16)LoopbackSpeed);
	if (Status != XST_SUCCESS) {
	        return XST_FAILURE;
	}

	/*
	 * Setting the operating speed of the MAC needs a delay.  There
	 * doesn't seem to be register to poll, so please consider this
	 * during your application design.
	 */
	 AxiEthernetUtilPhyDelay(2);


	/****************************/
	/* Run through the examples */
	/****************************/
	for (ChanId = 1 ; ChanId <= AxiEthernetInstancePtr->Config.AxiMcDmaChan_Cnt; ChanId++) {
		/*
		 * Run the AxiEthernet DMA Single Frame poll example
		 */
		Status = AxiEthernetSgDmaSingleFrameExample(AxiEthernetInstancePtr,
							  DmaInstancePtr, ChanId);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Tx/Rx Partial checksum offload will be disabled when any of
		 * Tx/Rx VLAN features are built in hardware.
		 */
		if (XAxiEthernet_IsRxPartialCsum(AxiEthernetInstancePtr) &&
			XAxiEthernet_IsTxPartialCsum(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanTran(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanStrp(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanTag(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanTran(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanStrp(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanTag(AxiEthernetInstancePtr))  {
			Hascsum = 1;
			Status = AxiEthernetSgDmaPartialChecksumOffloadExample(AxiEthernetInstancePtr,
									DmaInstancePtr, ChanId);

			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			Hascsum = 0;
		}


		/*
		 * Tx/Rx Full checksum offload will be disabled when any of
		 * Tx/Rx VLAN features are built in hardware.
		 */
		if (XAxiEthernet_IsRxFullCsum(AxiEthernetInstancePtr) &&
			XAxiEthernet_IsTxFullCsum(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanTran(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanStrp(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsTxVlanTag(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanTran(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanStrp(AxiEthernetInstancePtr) &&
			!XAxiEthernet_IsRxVlanTag(AxiEthernetInstancePtr))  {
			Hascsum = 1;
			Status = AxiEthernetSgDmaFullChecksumOffloadExample
			(AxiEthernetInstancePtr,DmaInstancePtr, ChanId);

			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
			Hascsum = 0;
		}

		/*
		 * Run the AxiEthernet DMA Multi Frame poll example
		 */
		Status = AxiEthernetSgDmaMultiFrameExample(AxiEthernetInstancePtr,
								DmaInstancePtr, ChanId);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		xil_printf("Channel id %d tests passed\n\r", ChanId);
	}

	/*
	 * Stop the device
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup the buffer descriptor chain for the Ingress mcdma Channels
*
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
static int RxBdSetup(XMcdma *McDmaInstPtr, XAxiEthernet *AxiEthernetInstancePtr)
{
	XMcdma_ChanCtrl *Rx_Chan;
	u8 ChanId;
	int BdCount = RXBD_CNT;
	int Status;
	UINTPTR RxBdSpacePtr;


	RxBdSpacePtr = (UINTPTR)&RxBdSpace;

	for (ChanId = 1; ChanId <= AxiEthernetInstancePtr->Config.AxiMcDmaChan_Cnt; ChanId++) {
		Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, ChanId);

		/* Disable all interrupts */
		XMcdma_IntrDisable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Rx_Chan, (UINTPTR)RxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		RxBdSpacePtr += (RXBD_CNT * sizeof(XMcdma_Bd));
	 }

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup the buffer descriptor chain for the Egress Channels
*
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
static int TxBdSetup(XMcdma *McDmaInstPtr, XAxiEthernet *AxiEthernetInstancePtr)
{
	XMcdma_ChanCtrl  *Tx_Chan;
	u8 ChanId;
	int BdCount = RXBD_CNT;
	int Status;
	UINTPTR TxBdSpacePtr;

	TxBdSpacePtr = (UINTPTR)&TxBdSpace;

	for (ChanId = 1; ChanId <= AxiEthernetInstancePtr->Config.AxiMcDmaChan_Cnt; ChanId++) {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, ChanId);

		XMcdma_IntrDisable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

		Status = XMcDma_ChanBdCreate(Tx_Chan, (UINTPTR)TxBdSpacePtr, BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("TX bd create failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		TxBdSpacePtr += (TXBD_CNT * sizeof(XMcdma_Bd));
	}
	xil_printf("BD seutp done\n\r");

	return XST_SUCCESS;
}

u16 Swap8(u8 Data)
{
	return (u8) (((Data & 0xF0U) >> 4U) | ((Data & 0x0FU) << 4U));
}

void AxiEnthernetSetMAC(char *MacAddr, u8 ChanId)
{
	int i;

	for (i = 0 ; i < XAE_MAC_ADDR_SIZE ; i++) {
		MacAddr[i] = AxiEthernetMAC[i];
		if (i == 0)
			MacAddr[i] = Swap8(ChanId);
	}
}

static void PollTxData(XMcdma *McDmaInstPtr, u16 Chan_id)
{
	int ProcessedBdCnt;
	XMcdma_ChanCtrl *Tx_Chan;
	XMcdma_Bd *BdPtr;

	if (Hascsum) {
		FramesTx++;
	} else {
		Tx_Chan = XMcdma_GetMcdmaTxChan(McDmaInstPtr, Chan_id);
		ProcessedBdCnt = XMcdma_BdChainFromHW(Tx_Chan, 0xFFFF, &BdPtr);
		XMcdma_BdChainFree(Tx_Chan, ProcessedBdCnt, BdPtr);
		FramesTx += ProcessedBdCnt;
	}
}

int CheckTxDmaResult(XMcdma *InstancePtr) {
        u16 Chan_id = 1;
        u32 i, Chan_SerMask;

        Chan_SerMask = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
                                                                  XMCDMA_TXCH_SERV_OFFSET);

        for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask; i <<= 1, Chan_id++)
                if (Chan_SerMask & i)
                        PollTxData(InstancePtr, Chan_id);

        return XST_SUCCESS;
}

static void PollRxData(XMcdma *McDmaInstPtr, u32 Chan_id)
{
	int ProcessedBdCnt, i;
	XMcdma_ChanCtrl *Rx_Chan;
	XMcdma_Bd *BdPtr;
	int TxFrameLength, RxFrameLength;
	UINTPTR RxBufPtr;

	if (Hascsum) {
		FramesRx++;
		return;
	}
	TxFrameLength = XAE_HDR_SIZE + PAYLOAD_SIZE + Padding;
	Rx_Chan = XMcdma_GetMcdmaRxChan(McDmaInstPtr, Chan_id);
	ProcessedBdCnt = XMcdma_BdChainFromHW(Rx_Chan, 0xFFFF, &BdPtr);

	for (i = 0 ; i < ProcessedBdCnt; i++) {
		RxFrameLength = XMcDma_BdGetActualLength(BdPtr, 0x007FFFFF);

		if (RxFrameLength != TxFrameLength) {
			AxiEthernetUtilErrorTrap("Length mismatch");
			FramesRx += ProcessedBdCnt;
			return;
		}
		RxBufPtr = XMcdma_BdRead(BdPtr, XMCDMA_BD_BUFA_OFFSET);
		BdPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Rx_Chan, BdPtr);

		if (AxiEthernetUtilFrameVerify(&TxFrame, (EthernetFrame *)RxBufPtr) != 0) {
			FramesRx += ProcessedBdCnt;
			AxiEthernetUtilErrorTrap("Data mismatch");
			return;
		}
	}

	FramesRx += ProcessedBdCnt;
}

int CheckDmaResult(XMcdma *InstancePtr) {
        u16 Chan_id = 1;
        u32 i, Chan_SerMask;

        Chan_SerMask = XMcdma_ReadReg(InstancePtr->Config.BaseAddress,
                                      XMCDMA_RX_OFFSET + XMCDMA_RXCH_SER_OFFSET);

        for (i = 1, Chan_id = 1; i != 0 && i <= Chan_SerMask; i <<= 1, Chan_id++)
                if (Chan_SerMask & i)
                        PollRxData(InstancePtr, Chan_id);

        return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the usage of the Axi Ethernet by sending and
* receiving a single frame in using MCDMA in polled mode.
* The source packet will be described by two descriptors. It will be received
* into a buffer described by a single descriptor.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
* @param	ChanId is the MCDMA Channel Id to be operate on.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSgDmaSingleFrameExample(XAxiEthernet *AxiEthernetInstancePtr,
					   XMcdma *DmaInstancePtr, u8 ChanId)
{
	int Status;
	u32 TxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XMcdma_ChanCtrl *Rx_Chan, *Tx_Chan;
	XMcdma_Bd *BdCurPtr;
	u32  CrBits, len, i;
	UINTPTR TxBufPtr;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	Rx_Chan = XMcdma_GetMcdmaRxChan(DmaInstancePtr, ChanId);
	Tx_Chan = XMcdma_GetMcdmaTxChan(DmaInstancePtr, ChanId);

	AxiEnthernetSetMAC(AxiEthernetMAC, ChanId - 1);

	/*
	 * Set the MAC address
	 */
	Status = XAxiEthernet_SetMacAddress(AxiEthernetInstancePtr,
	                                    AxiEthernetMAC);
	if (Status != XST_SUCCESS) {
	      AxiEthernetUtilErrorTrap("Error setting MAC address");
	      return XST_FAILURE;
	}

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
				     XAE_RECEIVER_ENABLE_OPTION |
				     XAE_TRANSMITTER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * Start the Axi Ethernet and enable its ERROR interrupts
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Calculate the frame length (not including FCS)
	 */
	if (Padding)
		TxFrameLength = XAE_HDR_SIZE + PayloadSize + Padding;
	else
		TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/*
	 * Setup packet to be transmitted
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Invalidate the RX frame before giving it to DMA RX channel to
	 * receive data.
	 */
	Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, TxFrameLength);

	/*
	 * Enable DMA RX interrupt.
	 *
	 * Interrupt coalescing parameters are left at their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);


	Status = XMcDma_ChanSubmit(Rx_Chan, (UINTPTR)&RxFrame,
				   TxFrameLength);
	if (Status != XST_SUCCESS) {
		xil_printf("ChanSubmit failed\n\r");
		return XST_FAILURE;
	}

	Status = XMcDma_ChanToHw(Rx_Chan);
	if (Status != XST_SUCCESS) {
			xil_printf("XMcDma_ChanToHw failed\n\r");
			return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit interrupts
	 */
	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);
	BdCurPtr = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);
	TxBufPtr = (UINTPTR)&TxFrame;
	for (i = 0 ; i < 2 ; i++) {
		CrBits = 0;

		if (i == 0)
			len = 64;
		if (i == 1)
			len = TxFrameLength - 64;

		Status = XMcDma_ChanSubmit(Tx_Chan, TxBufPtr,
								   len);
		if (Status != XST_SUCCESS) {
			xil_printf("ChanSubmit failed\n\r");
			return XST_FAILURE;
		}
		if (i == 0) {
			CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
		}
		if (i == 1) {
			CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
		}

		XMcDma_BdSetCtrl(BdCurPtr, CrBits);
		XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
		TxBufPtr += 64;
		BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
	}

	Status = XMcDma_ChanToHw(Tx_Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("XMcDma_ChanToHw failed for Tx\n\r");
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (1) {
		CheckTxDmaResult(DmaInstancePtr);
		if (FramesTx)
			break;
	}

	/*
	 * Wait for Rx indication
	 */
	while (1) {
		CheckDmaResult(DmaInstancePtr);
		if (FramesRx)
			break;
	}


	/*
	 * Finished this example. If everything worked correctly, all TxBDs and
	 * RxBDs should be free for allocation. Stop the device.
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function demonstrates the usage of the Axi Ethernet by sending and
* receiving multiple frames in using MCDMA in polled mode.
* Each source packet will be described by two descriptors. It will be received
* into a buffer described by a single descriptor.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
* @param	ChanId is the MCDMA Channel Id to be operate on.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSgDmaMultiFrameExample(XAxiEthernet *AxiEthernetInstancePtr,
			              XMcdma *DmaInstancePtr, u8 ChanId)
{

	int Status;
	u32 TxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XMcdma_ChanCtrl *Rx_Chan, *Tx_Chan;
	XMcdma_Bd *BdCurPtr;
	u32 CrBits, len, i, j;
	UINTPTR TxBufPtr, RxBufPtr;
	int FrameCnt = NUM_PACKETS;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	Rx_Chan = XMcdma_GetMcdmaRxChan(DmaInstancePtr, ChanId);
	Tx_Chan = XMcdma_GetMcdmaTxChan(DmaInstancePtr, ChanId);

	/*
	 * Calculate the frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize + Padding;

	AxiEnthernetSetMAC(AxiEthernetMAC, ChanId - 1);

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
				     XAE_RECEIVER_ENABLE_OPTION |
				     XAE_TRANSMITTER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * Start the Axi Ethernet and enable its ERROR interrupts
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Setup packet to be transmitted
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);


	/*
	 * Enable DMA RX interrupt.
	 *
	 * Interrupt coalescing parameters are left at their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	RxBufPtr = (UINTPTR)&RxFrame;
	for (j = 0 ; j < FrameCnt ; j++) {

		/*
		 * Invalidate the RX frame before giving it to DMA RX channel to
		 * receive data.
		 */
		Xil_DCacheInvalidateRange((UINTPTR)RxBufPtr, TxFrameLength);

		Status = XMcDma_ChanSubmit(Rx_Chan, RxBufPtr,
					   TxFrameLength);
		if (Status != XST_SUCCESS) {
			xil_printf("ChanSubmit failed\n\r");
			return XST_FAILURE;
		}
		RxBufPtr += TxFrameLength;
	}

	Status = XMcDma_ChanToHw(Rx_Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("XMcDma_ChanToHw failed\n\r");
		return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit interrupts
	 */
	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	BdCurPtr = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);
	TxBufPtr = (UINTPTR)&TxFrame;
	for (j = 0 ; j < FrameCnt ; j++) {
		Xil_DCacheFlushRange((UINTPTR)TxBufPtr, TxFrameLength);
		for (i = 0 ; i < 2 ; i++) {
			CrBits = 0;

			if (i == 0)
				len = 64;
			if (i == 1)
				len = TxFrameLength - 64;

			Status = XMcDma_ChanSubmit(Tx_Chan, TxBufPtr,
						len);
			if (Status != XST_SUCCESS) {
				xil_printf("ChanSubmit failed\n\r");
				return XST_FAILURE;
			}
			if (i == 0) {
				CrBits |= XMCDMA_BD_CTRL_SOF_MASK;
			}
			if (i == 1) {
				CrBits |= XMCDMA_BD_CTRL_EOF_MASK;
			}

			XMcDma_BdSetCtrl(BdCurPtr, CrBits);
			XMCDMA_CACHE_FLUSH((UINTPTR)(BdCurPtr));
			TxBufPtr += 64;
			BdCurPtr = (XMcdma_Bd *)XMcdma_BdChainNextBd(Tx_Chan, BdCurPtr);
		}
		TxBufPtr += TxFrameLength;
		AxiEthernetUtilFrameHdrFormatMAC((EthernetFrame *)TxBufPtr, AxiEthernetMAC);
		AxiEthernetUtilFrameHdrFormatType((EthernetFrame *)TxBufPtr, PayloadSize);
		AxiEthernetUtilFrameSetPayloadData((EthernetFrame *)TxBufPtr, PayloadSize);
	}

	Status = XMcDma_ChanToHw(Tx_Chan);
	if (Status != XST_SUCCESS) {
		xil_printf("XMcDma_ChanToHw failed for Tx\n\r");
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (1) {
		CheckTxDmaResult(DmaInstancePtr);
		if (FramesTx >= FrameCnt)
			break;
	}

	/*
	 * Wait for Rx indication
	 */
	while (1) {
		CheckDmaResult(DmaInstancePtr);
		if (FramesRx >= FrameCnt)
			break;
	}

	/*
	 * Finished this example. If everything worked correctly, all TxBDs and
	 * RxBDs should be free for allocation. Stop the device.
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This example sends and receives a single packet in loopback mode with
* checksum offloading support.
*
* The transmit frame will be che	cksummed over the entire Ethernet payload
* and inserted into the last 2 bytes of the frame.
*
* On receive, HW should calculate the Ethernet payload checksum and return a
* value of 0xFFFF which means the payload data was likely not corrupted.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSgDmaPartialChecksumOffloadExample(XAxiEthernet *AxiEthernetInstancePtr,
						  XMcdma *DmaInstancePtr, u8 ChanId)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XMcdma_ChanCtrl *Rx_Chan, *Tx_Chan;
	XMcdma_Bd *BdCurPtr, *Bd1Ptr;
	u32 BdSts;
	UINTPTR TxBufPtr;

	/*
	 * Cannot run this example if partial checksum offloading support is not
	 * available
	 */
	if (!(XAxiEthernet_IsRxPartialCsum(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsTxPartialCsum(AxiEthernetInstancePtr))) {
		AxiEthernetUtilErrorTrap("Partial Checksum offloading not available");
		return XST_FAILURE;
	}

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;

	Rx_Chan = XMcdma_GetMcdmaRxChan(DmaInstancePtr, ChanId);
	Tx_Chan = XMcdma_GetMcdmaTxChan(DmaInstancePtr, ChanId);


	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));


	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/*
	 * Setup the packet to be transmitted,
	 * Last 2 bytes are reserved for checksum
	 */
	AxiEthernetUtilFrameMemClear(&TxFrame);
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize - 2);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Invalidate the RX frame before giving it to DMA RX channel to
	 * receive data.
	 */
	Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, TxFrameLength);

	/*
	 * Interrupt coalescing parameters are set to their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	Status = XMcdma_SetChanCoalesceDelay(Tx_Chan, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for transmit");
		return XST_FAILURE;
	}

	Status = XMcdma_SetChanCoalesceDelay(Rx_Chan, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for recv");
		return XST_FAILURE;
	}

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
				     XAE_RECEIVER_ENABLE_OPTION |
				     XAE_TRANSMITTER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * Start the Axi Ethernet and enable its ERROR interrupts
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);
	XAxiEthernet_IntEnable(AxiEthernetInstancePtr,
						XAE_INT_RECV_ERROR_MASK);

	/*
	 * Enable DMA receive related interrupts
	 */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	Status = XMcDma_ChanSubmit(Rx_Chan, (UINTPTR)&RxFrame,
	                           TxFrameLength);
	if (Status != XST_SUCCESS) {
	      xil_printf("ChanSubmit failed\n\r");
	      return XST_FAILURE;
	}

	Status = XMcDma_ChanToHw(Rx_Chan);
	if (Status != XST_SUCCESS) {
	         xil_printf("XMcDma_ChanToHw failed\n\r");
	         return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit related interrupts
	 */
	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	/*
	 * Setup the TxBD
	 */
	BdCurPtr = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);
	TxBufPtr = (UINTPTR)&TxFrame;
	Status = XMcDma_ChanSubmit(Tx_Chan, TxBufPtr,
                               TxFrameLength);
	if (Status != XST_SUCCESS) {
		xil_printf("ChanSubmit failed\n\r");
		return XST_FAILURE;
	}

	XMcDma_BdSetCtrl(BdCurPtr, XMCDMA_BD_CTRL_SOF_MASK |
			 XMCDMA_BD_CTRL_EOF_MASK);

	/*
	 * Setup TxBd checksum offload attributes.
	 * Note that the checksum offload values can be set globally for all
	 * TxBds when XAxiDma_BdRingClone() is called to setup Tx BD space.
	 * This would eliminate the need to set them here.
	 */
	/* Enable hardware checksum computation for the buffer descriptor */
	Status = XMcDma_BdSetAppWord(BdCurPtr, BD_USR0_OFFSET, PARTIAL_CSUM_ENABLE);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling partial csum offloading");
		return XST_FAILURE;
	}

	/* Write Start Offset and Insert Offset into BD */
	Status = XMcDma_BdSetAppWord(BdCurPtr,BD_USR1_OFFSET,
		((XAE_HDR_SIZE << TX_CS_INIT_OFFSET) | (TxFrameLength - 2)));
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in setting csum insert point");
		return XST_FAILURE;
	}

	/* Write 0, as the seed value, to the BD */
	Status = XMcDma_BdSetAppWord(BdCurPtr,BD_USR2_OFFSET,0);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling partial csum offloading");
		return XST_FAILURE;
	}

	/*
	 * Enqueue to HW
	 */
	Status = XMcDma_ChanToHw(Tx_Chan);
	if (Status != XST_SUCCESS) {
	      xil_printf("XMcDma_ChanToHw failed for Tx\n\r");
	      return XST_FAILURE;
	}


	/*
	 * Wait for transmission to complete
	 */

	while (1) {
		CheckTxDmaResult(DmaInstancePtr);
		if (FramesTx)
			break;
	}

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 2 to HW, then there should be only 2 ready
	 * for post processing.
	 */
	 if (XMcdma_BdChainFromHW(Tx_Chan, 2, &Bd1Ptr) == 0) {
		 xil_printf("Bd1Ptr is %x\n\r", Bd1Ptr);
	      AxiEthernetUtilErrorTrap("TxBD was not ready for post processing");
	      return XST_FAILURE;
	 }


	/*
	 * Examine the TxBDs.
	 *
	 * There isn't much to do. The only thing to check would be DMA exception
	 * bits. But this would also be caught in the error handler. So we just
	 * return these BDs to the free list
	 */
	Status =  XMcdma_BdChainFree(Tx_Chan, 1, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

	/*
	 * Wait for Rx indication
	 */
	while (1) {
		CheckDmaResult(DmaInstancePtr);
		if (FramesRx)
			break;
	}

	/*
	 * Now that the frame has been received, post process our RxBD.
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	 */
	if (XMcdma_BdChainFromHW(Rx_Chan, 1, &Bd1Ptr)== 0) {
		AxiEthernetUtilErrorTrap("RxBD was not ready for post processing");
		return XST_FAILURE;
	}

	/*
	 * There is no device status to check. If there was a DMA error, it
	 * should have been reported to the error handler. Check the receive
	 * length against the transmitted length, then verify the data.
	 *
	 * Note in Axi Ethernet case, USR4_OFFSET word in the RX BD is used to store
	 * the real length of the received packet
	 */
	BdCurPtr = Bd1Ptr;
	BdSts = XMcDma_BdGetSts(BdCurPtr);
	if ((BdSts & XMCDMA_BD_STS_ALL_ERR_MASK) ||
		(!(BdSts & XMCDMA_BD_STS_COMPLETE_MASK))) {
			AxiEthernetUtilErrorTrap("Rx Error");
			return XST_FAILURE;
	} else {
		RxFrameLength =  XMcdma_BdRead64((BdCurPtr), XMCDMA_BD_STS_OFFSET)
		                                     & 0x007FFFFF;
	}

	if (RxFrameLength != TxFrameLength) {
		AxiEthernetUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}

	/*
	 * Verify the checksum as computed by HW. It should add up to 0xFFFF
	 * if frame was uncorrupted
	 */
	if ((u16) (XMcdma_BdRead64(Bd1Ptr, XMCDMA_BD_USR3_OFFSET))
		!= RCV_FRM_NOT_CORRUPTED) {
		AxiEthernetUtilErrorTrap("Rx checksum incorrect");
		return XST_FAILURE;
	}

	/*
	 * Return the RxBD back to the channel for later allocation. Free the
	 * exact number we just post processed.
	 */


	/*
	 * Finished this example. If everything worked correctly, all TxBDs
	 * and RxBDs should be free for allocation. Stop the device.
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This example sends and receives a single packet in loopback mode with
* full checksum offloading support.
*
* An Ethernet frame is formed with IP header, TCP header and payload. The
* hardware calculates the IP Header checksum and populates it at the
* appropriate location in the IP header. Similarly, the hardware calculates
* the TCP Pseudo header from IP header and calculates TCP checksum for TCP
* Pseudo header, TCP header and TCP payload. It then populates the checksum
* in the TCP header.
* The Ethernet frame is then looped back and received. The hardware validates
* the IP header checksum and the TCP checksum.
* The example retrieves the checksum validation information from the AXI4-
* Stream Appword2.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSgDmaFullChecksumOffloadExample(XAxiEthernet *AxiEthernetInstancePtr,
					       XMcdma *DmaInstancePtr, u8 ChanId)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = 64;
	XMcdma_ChanCtrl *Rx_Chan, *Tx_Chan;
	u8 DummyIPAddr[4];
	u32 FullCsumStatus;
	u8 *IPHdrPntr;
	u8 *TCPHdrPntr;
	u16 TempShortToCopy;
	u8 *PtrToCopyShort;
	u32 BdSts;
	XMcdma_Bd *BdCurPtr, *Bd1Ptr;
	UINTPTR TxBufPtr;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;

	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	Rx_Chan = XMcdma_GetMcdmaRxChan(DmaInstancePtr, ChanId);
	Tx_Chan = XMcdma_GetMcdmaTxChan(DmaInstancePtr, ChanId);

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + IP_HEADER_LENGTH + TCP_HEADER_LENGTH +
								PayloadSize ;

	AxiEnthernetSetMAC(AxiEthernetMAC, ChanId - 1);

	/*
	 * Setup the Ethernet frame header.
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, IP_VERSION_4);

	/*
	 * Setup the IP header.
	 */
	IPHdrPntr = (u8 *)((u8 *)(TxFrame) + XAE_HDR_SIZE);

	/*
	 * Form the version and IP header length. Version is 4 for IP
	 * and length is 5 words
	 */
	*IPHdrPntr++ = (IP_HEADER_VERSION << 4) | IP_HEADER_LEN_IN_WORDS;

	/* Type of service is not important for this example */
	*IPHdrPntr++ = 0x00;

	/*
	 * Total length in bytes. This will be our payload size in this example
	 */
	TempShortToCopy = Xil_Htons(PayloadSize + IP_HEADER_LENGTH +
							TCP_HEADER_LENGTH);
	PtrToCopyShort = (u8 *)(&TempShortToCopy);
	*IPHdrPntr++ = PtrToCopyShort[0];
	*IPHdrPntr++ = PtrToCopyShort[1];

	/*
	 * Fill the IP Identification (16 bit wide) field. Here it is made
	 * as zero.
	 */
	*IPHdrPntr++ = 0;
	*IPHdrPntr++ = 0;

	/*
	 * Fragmentation is not allowed for full checksum offloading.
	 * Hence both the fragment offset (LS 14 bits) and fragment flags
	 * (MS 2 bits) are set to 0.
	 */
	TempShortToCopy = Xil_Htons(0x4000);
	PtrToCopyShort = (u8 *)(&TempShortToCopy);
	*IPHdrPntr++ = PtrToCopyShort[0];
	*IPHdrPntr++ = PtrToCopyShort[1];

	/*
	 * IP time to live. Let us make it 0x3F. Not important in any case for
	 * this example. IP protocol field is 6 for TCP.
	 */
	*IPHdrPntr++ = 0x3F;
	*IPHdrPntr++ = IP_HEADER_PROTOCOL_TCP;

	/*
	 * IP header checksum (16 bit wide) is made 0. It will be calculated
	 * by the hardware and filled in at this place.
	 */
	*IPHdrPntr++ = 0;
	*IPHdrPntr++ = 0;

	/*
	 * Let us use some source address (172.23.16.1) and the same destination
	 * address. In any case the packet is looped back in the PHY and these
	 * addresses are meaningful only for IP header checksum calculation
	 * and Pseudo-header for TCP checksum calculation.
	 */
	DummyIPAddr[0] = 172;
	DummyIPAddr[1] = 23;
	DummyIPAddr[2] = 16;
	DummyIPAddr[3] = 1;
	memcpy(IPHdrPntr, DummyIPAddr, 4);
	IPHdrPntr = IPHdrPntr + 4;
	memcpy(IPHdrPntr, DummyIPAddr, 4);
	IPHdrPntr = IPHdrPntr + 4;

	/*
	 * Populate the TCP header fields.
	 */
	TCPHdrPntr = (u8 *)((u8 *)(TxFrame) + XAE_HDR_SIZE + IP_HEADER_LENGTH);

	/*
	 * Use some port number for source and destination for the purpose of
	 * filling up the TCP header.
	 */
	TempShortToCopy = Xil_Htons(DUMMY_TCP_PORT_1);
	PtrToCopyShort = (u8 *)(&TempShortToCopy);
	*TCPHdrPntr++ = PtrToCopyShort[0];
	*TCPHdrPntr++ = PtrToCopyShort[1];

	TempShortToCopy = Xil_Htons(DUMMY_TCP_PORT_2);
	PtrToCopyShort = (u8 *)(&TempShortToCopy);
	*TCPHdrPntr++ = PtrToCopyShort[0];
	*TCPHdrPntr++ = PtrToCopyShort[1];

	/*
	 * Sequence number and Ack number are of no significance in this
	 * example.
	 */
	/* Copy TCP Seq number */
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;

	/* Copy TCP Ack number */
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;

	/* Size of TCP header which is 20 bytes (5 words) in normal case. */
	*TCPHdrPntr++ = TCP_HEADER_LEN_IN_WORDS;
	*TCPHdrPntr++ = 0xF0; /* Some entry for flags */

	/*
	 * TCP Window and TCP Urgent pointer are irrelevant for this
	 * example. Hence they are made zero. TCP Checksum will be
	 * calculated and populated in TCP Checksum field. Hence it is
	 * left as zero.
	 */
	/* Copy TCP Window */
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;

	/* Copy TCP Checksum */
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;

	/* Copy TCP Urgent pointer */
	*TCPHdrPntr++ = 0;
	*TCPHdrPntr++ = 0;

	/* Now populate the payload data for the TCP packet */
	AxiEthernetUtilFrameSetPayloadData((EthernetFrame *)((u8 *)(TxFrame) +
			IP_HEADER_LENGTH + TCP_HEADER_LENGTH), PayloadSize);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Invalidate the RX frame before giving it to DMA RX channel to
	 * receive data.
	 */
	Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, TxFrameLength);

	/*
	 * Interrupt coalescing parameters are set to their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	Status = XMcdma_SetChanCoalesceDelay(Tx_Chan, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for transmit");
		return XST_FAILURE;
	}

	Status = XMcdma_SetChanCoalesceDelay(Rx_Chan, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for recv");
		return XST_FAILURE;
	}

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
				     XAE_RECEIVER_ENABLE_OPTION |
				     XAE_TRANSMITTER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * Start the Axi Ethernet and enable its ERROR interrupts
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);
	XAxiEthernet_IntEnable(AxiEthernetInstancePtr,
			       XAE_INT_RECV_ERROR_MASK);

	/*
	 * Enable DMA receive related interrupts
	 */
	XMcdma_IntrEnable(Rx_Chan, XMCDMA_IRQ_ALL_MASK);

	Status = XMcDma_ChanSubmit(Rx_Chan, (UINTPTR)&RxFrame,
	                           TxFrameLength);
	if (Status != XST_SUCCESS) {
	      xil_printf("ChanSubmit failed\n\r");
	      return XST_FAILURE;
	}

	Status = XMcDma_ChanToHw(Rx_Chan);
	if (Status != XST_SUCCESS) {
	         xil_printf("XMcDma_ChanToHw failed\n\r");
	         return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit related interrupts
	 */
	XMcdma_IntrEnable(Tx_Chan, XMCDMA_IRQ_ALL_MASK);

	/*
	 * Setup the TxBD
	 */
	BdCurPtr = (XMcdma_Bd *)XMcdma_GetChanCurBd(Tx_Chan);
	TxBufPtr = (UINTPTR)&TxFrame;
	Status = XMcDma_ChanSubmit(Tx_Chan, TxBufPtr,
                                   TxFrameLength);
	if (Status != XST_SUCCESS) {
		xil_printf("ChanSubmit failed\n\r");
		return XST_FAILURE;
	}

	XMcDma_BdSetCtrl(BdCurPtr, XMCDMA_BD_CTRL_SOF_MASK |
			 XMCDMA_BD_CTRL_EOF_MASK);

	/*
	 * Setup TxBd checksum offload attributes.
	 */
	/* Enable hardware checksum computation for the buffer descriptor */
	Status = XMcDma_BdSetAppWord(BdCurPtr, BD_USR0_OFFSET, FULL_CSUM_ENABLE);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling full csum offloading");
		return XST_FAILURE;
	}

	/*
	 * Enqueue to HW
	 */
	Status = XMcDma_ChanToHw(Tx_Chan);
	if (Status != XST_SUCCESS) {
	      xil_printf("XMcDma_ChanToHw failed for Tx\n\r");
	      return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */

	while (1) {
		CheckTxDmaResult(DmaInstancePtr);
		if (FramesTx)
			break;
	}

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	   */
	if (XMcdma_BdChainFromHW(Tx_Chan, 1, &Bd1Ptr) == 0) {
		xil_printf("Bd1Ptr is %x\n\r", Bd1Ptr);
		AxiEthernetUtilErrorTrap("TxBDs were not ready for post processing");
		return XST_FAILURE;
	}

	/*
	 * Examine the TxBDs.
	 *
	 * There isn't much to do. The only thing to check would be DMA exception
	 * bits. But this would also be caught in the error handler. So we just
	 * return these BDs to the free list
	 */
	 Status =  XMcdma_BdChainFree(Tx_Chan, 1, Bd1Ptr);
	 if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

	/*
	 * Wait for Rx indication
	 */
	while (1) {
		CheckDmaResult(DmaInstancePtr);
		if (FramesRx)
			break;
	}

	/*
	 * Now that the frame has been received, post process our RxBD.
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	 */
	 if (XMcdma_BdChainFromHW(Rx_Chan, 1, &Bd1Ptr)== 0) {
	          AxiEthernetUtilErrorTrap("RxBD was not ready for post processing");
	         return XST_FAILURE;
	 }


	/*
	 * There is no device status to check. If there was a DMA error, it
	 * should have been reported to the error handler. Check the receive
	 * length against the transmitted length, then verify the data.
	 *
	 * Note in Axi Ethernet case, USR4_OFFSET word in the RX BD is used to store
	 * the real length of the received packet
	 */
	BdCurPtr = Bd1Ptr;
	BdSts = XMcDma_BdGetSts(BdCurPtr);
	if ((BdSts & XMCDMA_BD_STS_ALL_ERR_MASK) ||
		(!(BdSts & XMCDMA_BD_STS_COMPLETE_MASK))) {
			AxiEthernetUtilErrorTrap("Rx Error");
			return XST_FAILURE;
	} else {
		RxFrameLength =  XMcdma_BdRead64((BdCurPtr), XMCDMA_BD_STS_OFFSET)
					   & 0x007FFFFF;
	}

	if (RxFrameLength != TxFrameLength) {
		AxiEthernetUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}

	/*
	 * Read the full checksum validation status from AXI4 Status Word.
	 */
	FullCsumStatus = (((XMcdma_BdRead(BdCurPtr, XMCDMA_BD_USR2_OFFSET)) &
						FULL_CSUM_STATUS_MASK) >> 3);
	if (FullCsumStatus != FULL_CSUM_VALIDATED) {
		if(FullCsumStatus == IP_TCP_CSUMS_NOT_CHECKED) {
			AxiEthernetUtilErrorTrap("IP and TCP checksums not checked");
			return XST_FAILURE;
		}
		else if(FullCsumStatus == IP_CSUM_OK_TCP_NOT_CHECKED) {
			AxiEthernetUtilErrorTrap("IP checksum is OK, TCP checksum not checked for");
			return XST_FAILURE;
		}
		else if(FullCsumStatus == TCP_CSUM_NOT_CHECKED_IP_NOT_OK) {
			AxiEthernetUtilErrorTrap("IP checksum is not correct, TCP checksum not checked for");
			return XST_FAILURE;
		}
		else if(FullCsumStatus == IP_CSUM_OK_TCP_NOT_OK) {
			AxiEthernetUtilErrorTrap("IP checksum is correct, TCP checksum is incorrect");
			return XST_FAILURE;
		}
		else {
			AxiEthernetUtilErrorTrap("IP and TCP checksums not validated because of other reasons");
			return XST_FAILURE;
		}
	}

	/*
	 * Return the RxBD back to the channel for later allocation. Free the
	 * exact number we just post processed.
	 */


	/*
	 * Finished this example. If everything worked correctly, all TxBDs
	 * and RxBDs should be free for allocation. Stop the device.
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;

}
