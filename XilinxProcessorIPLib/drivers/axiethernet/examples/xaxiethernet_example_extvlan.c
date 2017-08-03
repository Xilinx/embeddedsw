/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/**
*
* @file xaxiethernet_example_extvlan.c
*
* Implements examples that utilize the Axi Ethernet's interrupt driven SGDMA
* packet transfer mode to send and receive frames.
*
* This example demonstrates:
*
* - How to setup VLAN strip/translate/tag in TX and RX direction
* - How the VLAN tags are handled
* - What is the expected VLAN tag(s) after VLAN functions are invoked
*
* Functional guide to example:
*
* - AxiEthernetSgDmaIntrExtVlanExample demonstrates the extended VLAN
*   capability. The HW must be setup for extended VLAN for this example
*   to execute.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  4/30/10  First release based on the ll temac driver
* 1.01a asa  12/10/10 Changes made to enable the AXIDMA Tx/Rx ring interrupts
*		      		  before allocation of Tx/Rx BDs.
* 3.00a asa  6/25/12  Modified XAxiDma_BdSetLength API call to support new
*		      		  AXI DMA driver version 7.00a.
* 3.00a bss  10/22/12 Added support for Fast Interrupt Handlers.
* 3.01a srt  02/14/13 Added support for Zynq (CR 681136)
* 5.4   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     for proper documentation while generating doxygen.
* 5.7   rsp  19/12/17 Defined Padding and ExternalLoopback variables.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#include "xaxidma.h"
#include "xil_cache.h"
#include "xil_exception.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

#ifdef XPAR_XUARTNS550_NUM_INSTANCES
#include "xuartns550_l.h"
#endif
/*************************** Constant Definitions ****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define AXIETHERNET_DEVICE_ID	XPAR_AXIETHERNET_0_DEVICE_ID
#define AXIDMA_DEVICE_ID	XPAR_AXIDMA_0_DEVICE_ID
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define AXIETHERNET_IRPT_INTR	XPAR_INTC_0_AXIETHERNET_0_VEC_ID
#define DMA_RX_IRPT_INTR	XPAR_AXIETHERNET_0_CONNECTED_DMARX_INTR
#define DMA_TX_IRPT_INTR	XPAR_AXIETHERNET_0_CONNECTED_DMATX_INTR
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define AXIETHERNET_IRPT_INTR	XPAR_FABRIC_AXIETHERNET_0_VEC_ID
#define DMA_RX_IRPT_INTR	XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define DMA_TX_IRPT_INTR	XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID
#endif
#endif

#define RXBD_CNT		128	/* Number of RxBDs to use */
#define TXBD_CNT		128	/* Number of TxBDs to use */
#define BD_ALIGNMENT		XAXIDMA_BD_MINIMUM_ALIGNMENT/*
							     * Byte alignment
							     * of BDs
							     */
#define PAYLOAD_SIZE		1000	/* Payload size used in examples */
#define BD_VLAN_TPID_OFFSET	3	/* AXI4-Stream status Word offset from
					 * the start of app user words in BD.
					 * TPID is at status word 3, bits
					 * 31-16
					 */
#define BD_VLAN_VID_OFFSET	4	/* AXI4-Stream status word offset from
					 * the start of app user words in BD.
					 * VID is at status word 4, bits
					 * 31-16
					 */
#define TPID_MASK		0xFFFF0000	/* Mask to extract TPID from
						 * the AXI4 STream status
						 * word
						 */

/*
 * Number of bytes to reserve for BD space for the number of BDs desired
 */
#define RXBD_SPACE_BYTES XAxiDma_BdRingMemCalc(BD_ALIGNMENT, RXBD_CNT)
#define TXBD_SPACE_BYTES XAxiDma_BdRingMemCalc(BD_ALIGNMENT, TXBD_CNT)


/*************************** Variable Definitions ****************************/

static EthernetFrame TxFrame;	/* Transmit buffer */
static EthernetFrame RxFrame;	/* Receive buffer */

XAxiEthernet AxiEthernetInstance;
XAxiDma DmaInstance;

#if XPAR_INTC_0_HAS_FAST == 1

/* Variables for Fast Interrupt Handlers */
XAxiEthernet *AxiEthernetInstancePtr_Fast;
XAxiDma_BdRing *TxRingPtr_Fast;
XAxiDma_BdRing *RxRingPtr_Fast;

/******  Fast Interrupt Handlers prototypes  ******/

static void AxiEthernetErrorFastHandler(void) __attribute__ ((fast_interrupt));

static void RxIntrFastHandler(void) __attribute__ ((fast_interrupt));

static void TxIntrFastHandler(void) __attribute__ ((fast_interrupt));

#else

static void AxiEthernetErrorHandler(XAxiEthernet *AxiEthernet);
static void RxIntrHandler(XAxiDma_BdRing *RxRingPtr);
static void TxIntrHandler(XAxiDma_BdRing *TxRingPtr);

#endif

/*
 * Aligned memory segments to be used for buffer descriptors
 */
char RxBdSpace[RXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));
char TxBdSpace[TXBD_SPACE_BYTES] __attribute__ ((aligned(BD_ALIGNMENT)));

/*
 * Counters to be incremented by callbacks
 */
static volatile int FramesRx;	/* Num of frames that have been received */
static volatile int FramesTx;	/* Num of frames that have been sent */
static volatile int DeviceErrors;	/* Num of errors detected in the device */

volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */
volatile int ExternalLoopback; /* Variable for External loopback */

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif

#ifndef TESTAPP_GEN
static INTC IntcInstance;
#endif

/*************************** Function Prototypes *****************************/

/*
 * Examples
 */
int AxiEthernetExtVlanExample(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetDeviceId,
				u16 AxiDmaDeviceId,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId,
				u16 DmaTxIntrId);
int AxiEthernetSgDmaIntrExtVlanExample(XAxiEthernet *AxiEthernetInstancePtr,
				     XAxiDma *DmaInstancePtr);

/*
 * Interrupt setup and Callbacks for examples
 */

static int AxiEthernetSetupIntrSystem(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId, u16 DmaTxIntrId);

static void AxiEthernetDisableIntrSystem(INTC *IntcInstancePtr,
				   u16 AxiEthernetIntrId,
				   u16 DmaRxIntrId, u16 DmaTxIntrId);


/*****************************************************************************/
/**
*
* This is the main function for the Axi Ethernet example. This function is not
* included if the example is generated from the TestAppGen test  tool.
*
* @param	None.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE to indicate failure
*
* @note		None.
*
****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

#ifdef XPAR_XUARTNS550_NUM_INSTANCES
	XUartNs550_SetBaud(STDIN_BASEADDRESS, XPAR_XUARTNS550_CLOCK_HZ, 9600);
	XUartNs550_SetLineControlReg(STDIN_BASEADDRESS, XUN_LCR_8_DATA_BITS);
#endif

#if XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
#endif

#if XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
#endif

	AxiEthernetUtilErrorTrap("\r\n--- Enter main() ---");
	AxiEthernetUtilErrorTrap("This test may take several minutes to finish");

	/*
	 * Call the Axi Ethernet vlan example main function
	 */
	Status = AxiEthernetExtVlanExample(&IntcInstance,
						&AxiEthernetInstance,
						&DmaInstance,
						AXIETHERNET_DEVICE_ID,
						AXIDMA_DEVICE_ID,
						AXIETHERNET_IRPT_INTR,
						DMA_RX_IRPT_INTR,
						DMA_TX_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Axiethernet extvlan Example Failed");
		AxiEthernetUtilErrorTrap("--- Exiting main() ---");
		return XST_FAILURE;
	}

	AxiEthernetUtilErrorTrap("Successfully ran Axiethernet extvlan Example\r\n");
	AxiEthernetUtilErrorTrap("--- Exiting main() ---");

	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function demonstrates the usage usage of the Axi Ethernet by sending
* and receiving frames in interrupt driven SGDMA mode.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	DmaInstancePtr is a pointer to the instance of the AXIDMA
*		component.
* @param	AxiEthernetDeviceId is Device ID of the Axi Ethernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiDmaDeviceId is Device ID of the Axi DMAA Device ,
*		typically XPAR_<AXIDMA_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiEthernetIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	DmaRxIntrId is the interrupt id for DMA Rx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMARX_INTR
* @param	DmaTxIntrId is the interrupt id for DMA Tx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMATX_INTR
*
* @return
*		-XST_SUCCESS to indicate success.
*		-XST_FAILURE to indicate failure.
*
* @note		AxiDma hardware must be initialized before initializing
*		AxiEthernet. Since AxiDma reset line is connected to the
*		AxiEthernet reset line, a reset of AxiDma hardware during its
*		initialization would reset AxiEthernet.
*
******************************************************************************/
int AxiEthernetExtVlanExample(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetDeviceId,
				u16 AxiDmaDeviceId,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId,
				u16 DmaTxIntrId)
{
	int Status;
	int LoopbackSpeed;
	XAxiEthernet_Config *MacCfgPtr;
	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd BdTemplate;
	XAxiDma_Config* DmaConfig;

	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Get the configuration of AxiEthernet hardware.
	 */
	MacCfgPtr = XAxiEthernet_LookupConfig(AxiEthernetDeviceId);

	/*
	 * Check if DMA is present or not.
	 */
	if(MacCfgPtr->AxiDevType != XPAR_AXI_DMA) {
		AxiEthernetUtilErrorTrap
			("Device HW not configured for SGDMA mode\r\n");
		return XST_FAILURE;
	}

	DmaConfig = XAxiDma_LookupConfig(AxiDmaDeviceId);

	/*
	 * Initialize AXIDMA engine. AXIDMA engine must be initialized before
	 * AxiEthernet. During AXIDMA engine initialization, AXIDMA hardware is
	 * reset, and since AXIDMA reset line is connected to AxiEthernet, this
	 * would ensure a reset of AxiEthernet.
	 */
	Status = XAxiDma_CfgInitialize(DmaInstancePtr, DmaConfig);
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
	 * Setup RxBD space.
	 *
	 * We have already defined a properly aligned area of memory to store
	 * RxBDs at the beginning of this source code file so just pass its
	 * address into the function. No MMU is being used so the physical and
	 * virtual addresses are the same.
	 *
	 * Setup a BD template for the Rx channel. This template will be copied
	 * to every RxBD. We will not have to explicitly set these again.
	 */

	/*
	 * Create the RxBD ring
	 */
	Status = XAxiDma_BdRingCreate(RxRingPtr, (u32) &RxBdSpace,
				     (u32) &RxBdSpace, BD_ALIGNMENT, RXBD_CNT);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting up RxBD space");
		return XST_FAILURE;
	}
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error initializing RxBD space");
		return XST_FAILURE;
	}

	/*
	 * Setup TxBD space.
	 *
	 * Like RxBD space, we have already defined a properly aligned area of
	 * memory to use.
	 */

	/*
	 * Create the TxBD ring
	 */
	Status = XAxiDma_BdRingCreate(TxRingPtr, (u32) &TxBdSpace,
				     (u32) &TxBdSpace, BD_ALIGNMENT, TXBD_CNT);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting up TxBD space");
		return XST_FAILURE;
	}

	/*
	 * We reuse the bd template, as the same one will work for both rx and
	 * tx.
	 */
	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error initializing TxBD space");
		return XST_FAILURE;
	}

	/*
	 * Set PHY to loopback, speed depends on phy type.
	 * MII is 100 and all others are 1000.
	 */
	if (XAxiEthernet_GetPhysicalInterface(AxiEthernetInstancePtr)
						== XAE_PHY_TYPE_MII){
		LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED;
	} else {
		LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED_1G;
	}
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

	/*
	 * Connect to the interrupt controller and enable interrupts
	 */
	Status = AxiEthernetSetupIntrSystem(IntcInstancePtr,
					AxiEthernetInstancePtr,
					DmaInstancePtr,
					AxiEthernetIntrId, DmaRxIntrId,
					DmaTxIntrId);

	/****************************/
	/* Run the example */
	/****************************/

	/* Run the new VLAN feature. Make sure HW has the capability */
	if (XAxiEthernet_IsTxVlanTran(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsTxVlanStrp(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsTxVlanTag(AxiEthernetInstancePtr)  &&
		XAxiEthernet_IsRxVlanTran(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsRxVlanStrp(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsRxVlanTag(AxiEthernetInstancePtr)) {
		Status = AxiEthernetSgDmaIntrExtVlanExample
		(AxiEthernetInstancePtr,DmaInstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/*
	 * Disable the interrupts for the AxiEthernet device
	 */
	AxiEthernetDisableIntrSystem(IntcInstancePtr, AxiEthernetIntrId,
						DmaRxIntrId, DmaTxIntrId);

	/*
	 * Stop the device
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This example sends and receives a single packet in loopback mode with
* extended VLAN support.
*
* The transmit frame will have VLAN field populated.
*
* On receive, HW should pass the VLAN field to receive BDs.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return
*		-XST_SUCCESS to indicate success.
*		-XST_FAILURE to indicate failure.
*
* @note		Summary of VLAN tags handling in this example
*
* Frame setup with Tpid1+Cfi1+TxPid => 0x88A83111
* Frame translated to TxTransVid => 0x88A83222
* Frame tagged to Tpid2+Cfi2+TxTagVid => 0x9100C333 + 0x88A83222
* Frame sent and loopbacked.
*
* Frame stripped with RxStrpVid(0x333) => 0x88A83222
* Frame translated (key:RxVid:0x222) RxTransVid => 0x88A83444
*
******************************************************************************/
int AxiEthernetSgDmaIntrExtVlanExample(XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	u16 Tpid1 = 0x88A8;
	u16 Tpid2 = 0x9100;
	u8  Cfi1  = 0x03;
	u8  Cfi2  = 0x0C;
	u16 TxVid      = 0x0111;
	u16 TxTransVid = 0x0222;
	u16 TxTagVid   = 0x0333;
	u16 RxVid      = 0x0222;
	u16 RxTransVid = 0x0444;
	u16 RxStrpVid  = 0x0333;
	u32 VTagCfiVid;
	u16 RxCfiVid;
	u16 RxTpid;
	u32 RxStatusControlWord;
	int Valid;


	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdSts;

	/*
	 * Cannot run this example if extended features support is not enabled
	 */
	if (!(XAxiEthernet_IsTxVlanTran(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsTxVlanStrp(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsTxVlanTag(AxiEthernetInstancePtr)  &&
		XAxiEthernet_IsRxVlanTran(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsRxVlanStrp(AxiEthernetInstancePtr) &&
		XAxiEthernet_IsRxVlanTag(AxiEthernetInstancePtr))) {
		AxiEthernetUtilErrorTrap("Extended VLAN not available");
		return XST_FAILURE;
	}

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;
	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	/*
	 * Calculate frame length (not including FCS) plus one VLAN tag
	 */
	TxFrameLength = XAE_HDR_VLAN_SIZE + PayloadSize;

	/*
	 * Setup the packet with one VALN tag = VtagCfiVid to be transmitted
	 * initially.
	 */
	VTagCfiVid = (((u32)Tpid1 << 16) | ((u32)Cfi1 << 12) | TxVid);
	AxiEthernetUtilFrameMemClear(&TxFrame);
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrVlanFormatVid(&TxFrame, 0, VTagCfiVid);
	AxiEthernetUtilFrameHdrVlanFormatType(&TxFrame, PayloadSize, 1);
	AxiEthernetUtilFrameSetVlanPayloadData(&TxFrame, PayloadSize, 1);

	/* Intended VLAN setup:
	 * TX translation and tagging. RX stripping and translation.
	 *
	 * Frame setup with Tpid1+Cfi1+TxPid => 0x88A83111
	 * Frame translated to TxTransVid => 0x88A83222
	 * Frame tagged to Tpid2+Cfi2+TxTagVid => 0x9100C333 + 0x88A83222
	 * Frame sent and loopbacked.
	 *
	 * Frame stripped with RxStrpVid(0x333) => 0x88A83222
	 * Frame translated (key:RxVid:0x222) RxTransVid => 0x88A83444
	 */

	/* Extended VLAN transmit side. Stripping->Translation->Tagging */
	/*
	 * Configure VLAN TX tag mode, set to XAE_VTAG_SELECT.
	 */
	Status  = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
						XAE_EXT_TXVLAN_TAG_OPTION);
	Status |= XAxiEthernet_SetVTagMode(AxiEthernetInstancePtr,
						XAE_VTAG_SELECT, XAE_TX);

	/*
	 * TX VLAN translation from TxVid to TxTransVid and enable tagging.
	 */
	Status |= XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
						XAE_EXT_TXVLAN_TRAN_OPTION);
	Status |= XAxiEthernet_SetVidTable(AxiEthernetInstancePtr, TxVid,
						TxTransVid,0, 1, XAE_TX);

	/*
	 * TX VLAN tagging is keyed on TxVid to add one additional tag based
	 * on register XAE_TTAG_OFFSET value.
	 */
	VTagCfiVid = (((u32)Tpid2 << 16) | ((u32)Cfi2 << 12) | TxTagVid);
	Status |= XAxiEthernet_SetVTagValue(AxiEthernetInstancePtr,
							VTagCfiVid, XAE_TX);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting TX VLAN");
		return XST_FAILURE;
	}

	/* Extended VLAN receive side. Stripping->Translation->Tagging */
	/*
	 * Configure VLAN RX strip mode, set to XAE_VSTRP_SELECT.
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
						XAE_EXT_RXVLAN_STRP_OPTION);
	Status |= XAxiEthernet_SetVStripMode(AxiEthernetInstancePtr,
						XAE_VSTRP_SELECT, XAE_RX);

	/*
	 * RX VLAN strips based on RxStrpVid and enable stripping.
	 */
	Status |= XAxiEthernet_SetVidTable(AxiEthernetInstancePtr, RxStrpVid,
						RxStrpVid, 1, 0, XAE_RX);

	/*
	 * RX VLAN translation from RxVid to RxTransVid only.
	 */
	Status |= XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
						XAE_EXT_RXVLAN_TRAN_OPTION);
	Status |= XAxiEthernet_SetVidTable(AxiEthernetInstancePtr, RxVid,
						RxTransVid, 0, 0, XAE_RX);

	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting RX VLAN");
		return XST_FAILURE;
	}

	/* Configure VLAN TPIDs for HW to recognize. */
	Status  = XAxiEthernet_SetTpid(AxiEthernetInstancePtr, Tpid1, 0);
	Status |= XAxiEthernet_SetTpid(AxiEthernetInstancePtr, Tpid2, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting TPIDs");
		return XST_FAILURE;
	}

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((u32)&TxFrame, TxFrameLength);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Invalidate the RX frame before giving it to DMA RX channel to
	 * receive data.
	 */
	Xil_DCacheInvalidateRange((u32)&RxFrame, TxFrameLength + 4);

	/*
	 * Interrupt coalescing parameters are set to their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */

	Status = XAxiDma_BdRingSetCoalesce(TxRingPtr, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for transmit");
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingSetCoalesce(RxRingPtr, 1, 1);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing for recv");
		return XST_FAILURE;
	}

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
					XAE_RECEIVER_ENABLE_OPTION |
					XAE_TRANSMITTER_ENABLE_OPTION );
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
					XAE_JUMBO_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * Start the AxiEthernet and enable its ERROR interrupts
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);
	XAxiEthernet_IntEnable(&AxiEthernetInstance,
					XAE_INT_RECV_ERROR_MASK);

	/*
	 * Enable DMA receive related interrupts
	 */
	XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Allocate 1 RxBD.
	 */
	Status = XAxiDma_BdRingAlloc(RxRingPtr, 1, &BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error allocating RxBD");
		return XST_FAILURE;
	}

	/*
	 * Setup the BD.
	 */
	XAxiDma_BdSetBufAddr(BdPtr, (u32)&RxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(BdPtr, sizeof(RxFrame));
#else
	XAxiDma_BdSetLength(BdPtr, sizeof(RxFrame),
				RxRingPtr->MaxTransferLen);
#endif
	XAxiDma_BdSetCtrl(BdPtr, 0);

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(RxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error committing RxBD to HW");
		return XST_FAILURE;
	}

	/*
	 * Start DMA RX channel. Now it's ready to receive data.
	 */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable DMA transmit related interrupts
	 */
	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Allocate 1 TxBD
	 */
	Status = XAxiDma_BdRingAlloc(TxRingPtr, 1, &BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error allocating TxBD");
		return XST_FAILURE;
	}

	/*
	 * Setup the TxBD
	 */
	XAxiDma_BdSetBufAddr(BdPtr, (u32)&TxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(BdPtr, TxFrameLength);
#else
	XAxiDma_BdSetLength(BdPtr, TxFrameLength,
				TxRingPtr->MaxTransferLen);
#endif
	XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_TXSOF_MASK |
						XAXIDMA_BD_CTRL_TXEOF_MASK);

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(TxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error committing TxBD to HW");
		return XST_FAILURE;
	}

	/*
	 * Start DMA TX channel. Transmission starts at once.
	 */
	Status = XAxiDma_BdRingStart(TxRingPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (!FramesTx);

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 2 to HW, then there should be only 2
	 * ready for post processing.
	 */
	if (XAxiDma_BdRingFromHw(TxRingPtr, 1, &BdPtr) == 0) {
		AxiEthernetUtilErrorTrap("TxBDs were not ready for post processing");
		return XST_FAILURE;
	}

	/*
	 * Examine the TxBDs.
	 *
	 * There isn't much to do. The only thing to check would be DMA
	 * exception bits. But this would also be caught in the error handler.
	 * So we just return these BDs to the free list
	 */
	Status = XAxiDma_BdRingFree(TxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

	/*
	 * Wait for Rx indication
	 */
	while (!FramesRx);
	/*
	 * Now that the frame has been received, post process our RxBD.
	 * Since we have only submitted 1 to HW, then there should be only 1
	 * ready for post processing.
	 */
	if (XAxiDma_BdRingFromHw(RxRingPtr, 1, &BdPtr) == 0) {
		AxiEthernetUtilErrorTrap("RxBD was not ready for post processing");
		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	BdSts = XAxiDma_BdGetSts(BdCurPtr);

	if ((BdSts & XAXIDMA_BD_STS_ALL_ERR_MASK) ||
		(!(BdSts & XAXIDMA_BD_STS_COMPLETE_MASK))) {
			AxiEthernetUtilErrorTrap("Rx Error");
			return XST_FAILURE;
	}
	else {

		RxFrameLength =
		(XAxiDma_BdRead(BdCurPtr,XAXIDMA_BD_USR4_OFFSET)) & 0x0000FFFF;
	}

	/* Expected RX TPID+CFI+VID !!! */
	VTagCfiVid = (((u32)Tpid2 << 16) | ((u32)Cfi2 << 12) | RxStrpVid);

	/* Check on the VLAN CFI and VID */
	RxStatusControlWord = XAxiDma_BdGetAppWord(BdPtr,
						BD_VLAN_VID_OFFSET, &Valid);
	if(Valid) {
		RxCfiVid = RxStatusControlWord >> 16;
		RxCfiVid = Xil_Ntohs(RxCfiVid);
		if(RxCfiVid != (VTagCfiVid & 0x0000FFFF)) {
			AxiEthernetUtilErrorTrap("VLAN CFI and VID mismatch\n");
			return XST_FAILURE;
		}
	}
	else {
		AxiEthernetUtilErrorTrap("Status words not available from AXI DMA\n");
		return XST_FAILURE;
	}

	RxStatusControlWord = XAxiDma_BdGetAppWord(BdPtr,
						BD_VLAN_TPID_OFFSET, &Valid);
	if(Valid) {
		RxTpid = RxStatusControlWord >> 16;
		RxTpid = Xil_Ntohs(RxTpid);
		VTagCfiVid = ((VTagCfiVid & TPID_MASK) >> 16);
		if(RxTpid != VTagCfiVid) {

			AxiEthernetUtilErrorTrap("VLAN TPID mismatch\n");
			return XST_FAILURE;
		}
	}
	else {
		AxiEthernetUtilErrorTrap("Status words not available from AXI DMA\n");
		return XST_FAILURE;
	}

	if (AxiEthernetUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
		AxiEthernetUtilErrorTrap("Data mismatch");
		return XST_FAILURE;
	}

	/*
	 * Return the RxBD back to the channel for later allocation. Free the
	 * exact number we just post processed.
	 */
	Status = XAxiDma_BdRingFree(RxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}


/*****************************************************************************/
/**
*
* This is the DMA TX callback function to be called by TX interrupt handler.
* This function handles BDs finished by hardware.
*
* @param	TxRingPtr is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxCallBack(XAxiDma_BdRing * TxRingPtr)
{
	/*
	 * Disable the transmit related interrupts
	 */
	XAxiDma_BdRingIntDisable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);
	/*
	 * Increment the counter so that main thread knows something happened
	 */
	FramesTx++;
}

/*****************************************************************************/
/**
*
* This is the DMA TX Interrupt handler function.
*
* @param	TxRingPtr is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note 	This Interrupt handler MUST clear pending interrupts before
*		handling them by calling the call back. Otherwise the following
*		corner case could raise some issue:
*
* 		A packet got transmitted and a TX interrupt got asserted. If
* 		the interrupt handler calls the callback before clearing the
* 		interrupt, a new packet may get transmitted in the callback.
* 		This new packet then can assert one more TX interrupt before
* 		the control comes out of the callback function. Now when
* 		eventually control comes out of the callback function, it will
* 		never know about the second new interrupt and hence while
* 		clearing the interrupts, would clear the new interrupt as well
* 		and will never process it.
*		To avoid such cases, interrupts must be cleared before calling
*		the callback.
*
******************************************************************************/
static void TxIntrHandler(XAxiDma_BdRing * TxRingPtr)
{
	u32 IrqStatus;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_BdRingGetIrq(TxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(TxRingPtr, IrqStatus);
	/*
	 * If no interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		DeviceErrors++;
		AxiEthernetUtilErrorTrap
		("AXIDma: No interrupts asserted in TX status register");
		XAxiDma_Reset(&DmaInstance);
		if(!XAxiDma_ResetIsDone(&DmaInstance)) {
			AxiEthernetUtilErrorTrap ("AxiDMA: Error: Could not reset\n");
		}
		return;
	}

	/* If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		AxiEthernetUtilErrorTrap("AXIDMA: TX Error interrupts\n");

		/* Reset should never fail for transmit channel
		 */
		XAxiDma_Reset(&DmaInstance);
		if(!XAxiDma_ResetIsDone(&DmaInstance)) {

			AxiEthernetUtilErrorTrap ("AXIDMA: Error: Could not reset\n");
		}

		return;
	}

	/*
	 * If Transmit done interrupt is asserted, call TX call back function
	 * to handle the processed BDs and raise the according flag
	 */
	if ((IrqStatus & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		TxCallBack(TxRingPtr);
	}
}


/*****************************************************************************/
/**
*
* This is the DMA RX callback function to be called by RX interrupt handler.
* This function handles finished BDs by hardware, attaches new buffers to those
* BDs, and give them back to hardware to receive more incoming packets
*
* @param	RxRingPtr is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxCallBack(XAxiDma_BdRing * RxRingPtr)
{
	/*
	 * Disable the receive related interrupts
	 */
	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);
	/*
	 * Increment the counter so that main thread knows something
	 * happened
	 */
	FramesRx++;
}


/*****************************************************************************/
/**
*
* This is the Receive handler function for examples 1 and 2.
* It will increment a shared  counter, receive and validate the frame.
*
* @param	RxRingPtr is a pointer to the DMA ring instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(XAxiDma_BdRing * RxRingPtr)
{
	u32 IrqStatus;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_BdRingGetIrq(RxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(RxRingPtr, IrqStatus);
	/*
	 * If no interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		DeviceErrors++;
		AxiEthernetUtilErrorTrap
		("AXIDma: No interrupts asserted in RX status register");
		XAxiDma_Reset(&DmaInstance);
		if(!XAxiDma_ResetIsDone(&DmaInstance)) {
			AxiEthernetUtilErrorTrap ("Could not reset\n");
		}
		return;
	}

	/* If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		AxiEthernetUtilErrorTrap("AXIDMA: RX Error interrupts\n");

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(&DmaInstance);

		if(!XAxiDma_ResetIsDone(&DmaInstance)) {
			AxiEthernetUtilErrorTrap ("AXIDMA: Could not reset\n");
		}
		return;
	}
	/*
	 * If Reception done interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((IrqStatus & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		RxCallBack(RxRingPtr);
	}
}

/*****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments the
* the error counter so that the main thread knows the number of errors.
*
* @param	AxiEthernet is a reference to the AxiEthernet device instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void AxiEthernetErrorHandler(XAxiEthernet * AxiEthernet)
{
	u32 Pending = XAxiEthernet_IntPending(AxiEthernet);

	if (Pending & XAE_INT_RXRJECT_MASK) {
		AxiEthernetUtilErrorTrap("AxiEthernet: Rx packet rejected");
	}

	if (Pending & XAE_INT_RXFIFOOVR_MASK) {
		AxiEthernetUtilErrorTrap("AxiEthernet: Rx fifo over run");
	}

	XAxiEthernet_IntClear(AxiEthernet, Pending);

	/*
	 * Bump counter
	 */
	DeviceErrors++;
}


/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* Axi Ethernet.  This function is application-specific since the actual system
* may or may not have an interrupt controller.  The Axi Ethernet could be
* directly connected to a processor without an interrupt controller.  The user
* should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
* 		AxiEthernet component.
* @param	DmaInstancePtr is a pointer to the instance of the AXIDMA
*		component.
* @param	AxiEthernetDeviceId is Device ID of the Axi Ethernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiEthernetIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	DmaRxIntrId is the interrupt id for DMA Rx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMARX_INTR
* @param	DmaTxIntrId is the interrupt id for DMA Tx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMATX_INTR
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
static int AxiEthernetSetupIntrSystem(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId,
				u16 DmaTxIntrId)
{
	XAxiDma_BdRing * TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_BdRing * RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller and connect the ISR
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Unable to initialize the interrupt controller");
		return XST_FAILURE;
	}
#endif

#if XPAR_INTC_0_HAS_FAST == 1

	TxRingPtr_Fast = TxRingPtr;
	RxRingPtr_Fast = RxRingPtr;
	AxiEthernetInstancePtr_Fast = AxiEthernetInstancePtr;
	Status = XIntc_ConnectFastHandler(IntcInstancePtr, AxiEthernetIntrId,
						(XFastInterruptHandler) AxiEthernetErrorFastHandler);
	Status |= XIntc_ConnectFastHandler(IntcInstancePtr, DmaTxIntrId,
								(XFastInterruptHandler) TxIntrFastHandler);
	Status |= XIntc_ConnectFastHandler(IntcInstancePtr, DmaRxIntrId,
								(XFastInterruptHandler) RxIntrFastHandler);
#else
	Status = XIntc_Connect(IntcInstancePtr, AxiEthernetIntrId,
						(XInterruptHandler)
						AxiEthernetErrorHandler,
						AxiEthernetInstancePtr);
	Status |= XIntc_Connect(IntcInstancePtr, DmaTxIntrId,
						(XInterruptHandler) TxIntrHandler,
						TxRingPtr);
	Status |= XIntc_Connect(IntcInstancePtr, DmaRxIntrId,
					(XInterruptHandler) RxIntrHandler,
				RxRingPtr);
#endif


	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Unable to connect ISR to interrupt controller");
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error starting intc");
		return XST_FAILURE;
	}
#endif


	/*
	 * Enable interrupts from the hardware
	 */
	XIntc_Enable(IntcInstancePtr, AxiEthernetIntrId);
	XIntc_Enable(IntcInstancePtr, DmaTxIntrId);
	XIntc_Enable(IntcInstancePtr, DmaRxIntrId);
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


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, DmaTxIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, DmaRxIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, AxiEthernetIntrId, 0xA0, 0x3);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, DmaTxIntrId,
				(Xil_InterruptHandler)TxIntrHandler,
				TxRingPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, DmaRxIntrId,
				(Xil_InterruptHandler)RxIntrHandler,
				RxRingPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, AxiEthernetIntrId,
				(Xil_InterruptHandler)AxiEthernetErrorHandler,
				AxiEthernetInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, AxiEthernetIntrId);
	XScuGic_Enable(IntcInstancePtr, DmaTxIntrId);
	XScuGic_Enable(IntcInstancePtr, DmaRxIntrId);
#endif
#ifndef TESTAPP_GEN
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)INTC_HANDLER,
			(void *)(IntcInstancePtr));

	Xil_ExceptionEnable();

#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for AxiEthernet.
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		component.
* @param	AxiEthernetIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	DmaRxIntrId is the interrupt id for DMA Rx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMARX_INTR
* @param	DmaTxIntrId is the interrupt id for DMA Tx and is typically
*		taken from XPAR_<AXIETHERNET_instance>_CONNECTED_DMATX_INTR
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void AxiEthernetDisableIntrSystem(INTC *IntcInstancePtr,
					u16 AxiEthernetIntrId,
					u16 DmaRxIntrId,
					u16 DmaTxIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Disconnect the interrupts for the DMA TX and RX channels
	 */
	XIntc_Disconnect(IntcInstancePtr, DmaTxIntrId);
	XIntc_Disconnect(IntcInstancePtr, DmaRxIntrId);
	/*
	 * Disconnect and disable the interrupt for the AxiEthernet device
	 */
	XIntc_Disconnect(IntcInstancePtr, AxiEthernetIntrId);

#else
	/*
	 * Disconnect the interrupts for the DMA TX and RX channels
	 */
	XScuGic_Disconnect(IntcInstancePtr, DmaTxIntrId);
	XScuGic_Disconnect(IntcInstancePtr, DmaRxIntrId);

	/*
	 * Disconnect and disable the interrupt for the AxiEthernet device
	 */
	XScuGic_Disconnect(IntcInstancePtr, AxiEthernetIntrId);
#endif
}

#if XPAR_INTC_0_HAS_FAST == 1
/*****************************************************************************/
/**
*
* Fast Error Handler which calls AxiEthernetErrorHandler.
*
* @param	None
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void AxiEthernetErrorFastHandler(void)
{
	AxiEthernetErrorHandler((XAxiEthernet *)AxiEthernetInstancePtr_Fast);
}

/*****************************************************************************/
/**
*
* Fast Tramsmit Handler which calls TxIntrHandler.
*
* @param	None
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void TxIntrFastHandler(void)
{
	TxIntrHandler((XAxiDma_BdRing *)TxRingPtr_Fast);
}

/*****************************************************************************/
/**
*
* Fast Receive Handler which calls RxIntrHandler.
*
* @param	None
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void RxIntrFastHandler(void)
{
	RxIntrHandler((XAxiDma_BdRing *)RxRingPtr_Fast);
}

#endif
