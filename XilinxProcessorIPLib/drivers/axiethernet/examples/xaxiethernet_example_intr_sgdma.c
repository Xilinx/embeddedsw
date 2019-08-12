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
* @file xaxiethernet_example_intr_sgdma.c
*
* Implements examples that utilize the Axi Ethernet's interrupt driven SGDMA
* packet transfer mode to send and receive frames.
*
* These examples demonstrate:
*
* - How to perform simple send and receive
* - Interrupt coalescing
* - Checksum offload
* - Error handling
* - Device reset
*
* Functional guide to example:
*
* - AxiEthernetSgDmaIntrSingleFrameExample demonstrates the simplest way to
*   send and receive frames in in interrupt driven SGDMA mode.
*
* - AxiEthernetSgDmaIntrCoalescingExample demonstrates how to use interrupt
*   coalescing to increase throughput.
*
* - AxiEthernetSgDmaPartialChecksumOffloadExample demonstrates the partial
*   checksum offloading. The HW must be setup for partial checksum
*   offloading for this example to execute.
*
* - AxiEthernetSgDmaFullChecksumOffloadExample demonstrates the full
*   checksum offloading. The HW must be setup for full checksum offloading
*   for this example to execute.
*
* - AxiEthernetAxiEthernetErrorHandler() demonstrates how to manage
*   asynchronous errors.
*
* - AxiEthernetResetDevice() demonstrates how to reset the driver/HW without
*   losing all configuration settings.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  4/30/10  First release based on the ll temac driver
* 1.01a asa  12/10/10 Added full checksum offload example.
* 		      		  Changes made to enable the AXIDMA Tx/Rx ring interrupts
*		      		  before allocation of Tx/Rx BDs for all examples.
* 3.00a asa  6/25/12  Modified XAxiDma_BdSetLength API call to support new
*		      		  AXI DMA driver version 7.00a. Removed the calls to
*		      		  XAxiDma_BdRingStart for the coalesce and checksum
*		      		  offload examples. They are not required with the new
*		      		  AxiDMA version.
* 3.00a bss  10/22/12 Added support for Fast Interrupt Handlers.
* 3.01a srt  02/14/13 Added support for Zynq (CR 681136).
* 5.4   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     for proper documentation while generating doxygen.
* 5.8   rsp  07/23/18 Fix gcc '[-Wint-conversion]' warning.
*       rsp  10/22/18 Set RX BD length to jumbo frame size. It fixes 'Error
*                     committing RxBD to HW' error for designs having length
*                     register width < 19 bits.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#include "xaxidma.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "stdio.h"		/* stdio */

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

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

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#define RXBD_CNT				128	/* Number of RxBDs to use */
#define TXBD_CNT				128	/* Number of TxBDs to use */
#define BD_ALIGNMENT			XAXIDMA_BD_MINIMUM_ALIGNMENT
									/* Byte alignment of BDs */
#define PAYLOAD_SIZE			1000 /* Payload size used in examples */
#define BD_USR0_OFFSET			0 /* AXI4-Stream Control Word offset from
								   * the start of app user words in BD. Offset
								   * 0 means, Control Word 0, used for enabling
								   * checksum offloading.
								   */
#define BD_USR1_OFFSET			1 /* AXI4-Stream Control Word offset from
								   * the start of app user words in BD. Offset
								   * 1 means, Control Word 1, used for
								   * mentioning checksum begin and checksum
								   * insert points
								   */
#define BD_USR2_OFFSET			2 /* AXI4-Stream Control Word offset from
								   * the start of app user words in BD. Offset
								   * 2 means, Control Word 2, used for
								   * mentioning checksum seed.
								   */

#define PARTIAL_CSUM_ENABLE		0x00000001 /* Option for partial csum enable */
#define FULL_CSUM_ENABLE		0x00000002 /* Option for full csum enable */
#define TX_CS_INIT_OFFSET		16	/* Offset in the control word where
									 * byte offset in the ethernet frame
									 * to start CSUM calculation need to
									 * be inserted
									 */
#define RCV_FRM_NOT_CORRUPTED	0xFFFF	/* If the received frame is not
										 * corrupted, for partial csum
										 * offloading case, the control
										 * word 3 would return 0xFFFF
										 */

#define IP_VERSION_4			0x800	/* For IPV4, the Ethernet frame
										 * type/length field will have a
										 * value of 0x800
										 */
#define IP_HEADER_VERSION		0x04	/* For IPv4, the version entry in
										 * IP header is always 4
										 */
#define IP_HEADER_LEN_IN_WORDS	0x05	/* For our case, the header length
										 * is always 20 bytes (5 words
										 */
#define IP_HEADER_LENGTH		20	/* IP header length in bytes. Used
									 * as offset to kigure out the start
									 * of TCP header.
									 */
#define IP_HEADER_PROTOCOL_TCP	0x6	/* IP header protocol entry. For TCP
									 * packets, it is 6.
									 */
#define IP_HEADER_PROTOCOL_UDP	0x17 /* IP header protocol entry. For UDP
									  * packets, it is 17.
									  */
#define TCP_HEADER_LEN_IN_WORDS	0x5	/* For our case, the header length
									 * is always 20 bytes (5 words)
									 */
#define TCP_HEADER_LENGTH		20	/* IP header length in bytes. Used
									 * as offset to kigure out the start
									 * of TCP header.
									 */
#define FULL_CSUM_STATUS_MASK	0x00000038 /* Mask to extract full checksum
										* status field from AXI4 Stream
										* Status Word 2.
										*/

#define FULL_CSUM_VALIDATED		0x00000002 /* If bits 3-5 in AXI4 Status word
										* have a value of 0x010, it means
										* both IP and TCP checksums have
										* been found to be correct.
										*/
#define IP_TCP_CSUMS_NOT_CHECKED			0x00000000
#define IP_CSUM_OK_TCP_NOT_CHECKED			0x00000001
#define TCP_CSUM_NOT_CHECKED_IP_NOT_OK		0x00000005
#define IP_CSUM_OK_TCP_NOT_OK				0x00000006

#define DUMMY_TCP_PORT_1					0x1111
#define DUMMY_TCP_PORT_2					0x1112
/*
 * Number of bytes to reserve for BD space for the number of BDs desired
 */
#define RXBD_SPACE_BYTES (XAxiDma_BdRingMemCalc(BD_ALIGNMENT, RXBD_CNT))
#define TXBD_SPACE_BYTES (XAxiDma_BdRingMemCalc(BD_ALIGNMENT, TXBD_CNT))

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
static volatile int DeviceErrors;/* Num of errors detected in the device */
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
int AxiEthernetSgDmaIntrExample(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetDeviceId,
				u16 AxiDmaDeviceId,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId,
				u16 DmaTxIntrId);
int AxiEthernetSgDmaIntrSingleFrameExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr);
int AxiEthernetSgDmaIntrCoalescingExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr);
int AxiEthernetSgDmaPartialChecksumOffloadExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr);

int AxiEthernetSgDmaFullChecksumOffloadExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr);

/*
 * Interrupt setup and Callbacks for examples
 */

static int AxiEthernetSetupIntrSystem(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XAxiDma *DmaInstancePtr,
				u16 AxiEthernetIntrId,
				u16 DmaRxIntrId,
				u16 DmaTxIntrId);
static void AxiEthernetDisableIntrSystem(INTC *IntcInstancePtr,
				   u16 AxiEthernetIntrId,
				   u16 DmaRxIntrId, u16 DmaTxIntrId);

void AxiEthernetPHYRegistersDump(XAxiEthernet * AxiEthernetInstancePtr);

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
	 * Call the Axi Ethernet SGDMA interrupt example , specify the
	 * parameters generated in xparameters.h.
	 */
	Status = AxiEthernetSgDmaIntrExample(&IntcInstance,
						&AxiEthernetInstance,
						&DmaInstance,
						AXIETHERNET_DEVICE_ID,
						AXIDMA_DEVICE_ID,
						AXIETHERNET_IRPT_INTR,
						DMA_RX_IRPT_INTR,
						DMA_TX_IRPT_INTR);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Axiethernet intr sgdma Example Failed\r\n");
		AxiEthernetUtilErrorTrap("--- Exiting main() ---");
		return XST_FAILURE;
	}

	AxiEthernetUtilErrorTrap("Successfully ran Axiethernet intr sgdma Example\r\n");
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
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		AxiDma hardware must be initialized before initializing
*		AxiEthernet. Since AxiDma reset line is connected to the
*		AxiEthernet reset line, a reset of AxiDma hardware during its
*		initialization would reset AxiEthernet.
*
******************************************************************************/
int AxiEthernetSgDmaIntrExample(INTC *IntcInstancePtr,
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
	XAxiDma_Config* DmaConfig;
	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd BdTemplate;

	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Get the configuration of AxiEthernet hardware.
	 */
	MacCfgPtr = XAxiEthernet_LookupConfig(AxiEthernetDeviceId);

	/*
	 * Check whether DMA is present or not
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

#if defined(__aarch64__)
	Xil_SetTlbAttributes((UINTPTR) TxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
	Xil_SetTlbAttributes((UINTPTR) RxBdSpace, NORM_NONCACHE | INNER_SHAREABLE);
#endif
	/*
	 * Setup RxBD space.
	 *
	 * We have already defined a properly aligned area of memory to store
	 * RxBDs at the beginning of this source code file so just pass its
	 * address into the function. No MMU is being used so the physical and
	 * virtual addresses are the same.
	 *
	 * Setup a BD template for the Rx channel. This template will be
	 * copied to every RxBD. We will not have to explicitly set these
	 * again.
	 */

	/*
	 * Disable all RX interrupts before RxBD space setup
	 */
	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Create the RxBD ring
	 */
	Status = XAxiDma_BdRingCreate(RxRingPtr, (UINTPTR) &RxBdSpace,
				     (UINTPTR) &RxBdSpace, BD_ALIGNMENT, RXBD_CNT);
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
	Status = XAxiDma_BdRingCreate(TxRingPtr, (UINTPTR) &TxBdSpace,
				(UINTPTR) &TxBdSpace, BD_ALIGNMENT, TXBD_CNT);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting up TxBD space");
		return XST_FAILURE;
	}
	/*
	 * We reuse the bd template, as the same one will work for both rx
	 * and tx.
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
	if (XAxiEthernet_GetPhysicalInterface(AxiEthernetInstancePtr) ==
						XAE_PHY_TYPE_MII) {
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
						AxiEthernetIntrId,
						DmaRxIntrId,
						DmaTxIntrId);


	/****************************/
	/* Run through the examples */
	/****************************/

	/*
	 * Run the AxiEthernet DMA Single Frame Interrupt example
	 */
	Status = AxiEthernetSgDmaIntrSingleFrameExample(AxiEthernetInstancePtr,
						  DmaInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Run the Axi Ethernet DMA Interrupt Coalescing example
	 */
	Status = AxiEthernetSgDmaIntrCoalescingExample(AxiEthernetInstancePtr,
						 DmaInstancePtr);
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
		Status = AxiEthernetSgDmaPartialChecksumOffloadExample
			(AxiEthernetInstancePtr,DmaInstancePtr);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
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
		Status = AxiEthernetSgDmaFullChecksumOffloadExample
		(AxiEthernetInstancePtr,DmaInstancePtr);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/*
	 * Disable the interrupts for the Axi Ethernet device
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
* This function demonstrates the usage of the Axi Ethernet by sending and
* receiving a single frame in SGDMA interrupt mode.
* The source packet will be described by two descriptors. It will be received
* into a buffer described by a single descriptor.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSgDmaIntrSingleFrameExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd *Bd1Ptr;
	XAxiDma_Bd *Bd2Ptr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdSts;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;
	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	/*
	 * Calculate the frame length (not including FCS)
	 */
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
	 * Start the Axi Ethernet device and enable the ERROR interrupt
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);
	XAxiEthernet_IntEnable(AxiEthernetInstancePtr,
					XAE_INT_RECV_ERROR_MASK);

	/*
	 * Enable DMA RX interrupt.
	 *
	 * Interrupt coalescing parameters are left at their default settings
	 * which is to interrupt the processor after every frame has been
	 * processed by the DMA engine.
	 */
	XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Allocate 1 RxBD.
	 */
	Status = XAxiDma_BdRingAlloc(RxRingPtr, 1, &Bd1Ptr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error allocating RxBD");
		return XST_FAILURE;
	}

	/*
	 * Setup the BD.
	 */
	XAxiDma_BdSetBufAddr(Bd1Ptr, (UINTPTR)&RxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(Bd1Ptr, XAE_MAX_JUMBO_FRAME_SIZE);
#else
	XAxiDma_BdSetLength(Bd1Ptr, XAE_MAX_JUMBO_FRAME_SIZE,
				RxRingPtr->MaxTransferLen);
#endif
	XAxiDma_BdSetCtrl(Bd1Ptr, 0);

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(RxRingPtr, 1, Bd1Ptr);
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
	 * Enable DMA transmit interrupts
	 */
	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Allocate, setup, and enqueue 2 TxBDs. The first BD will describe
	 * the first 32 bytes of TxFrame and the second BD will describe the
	 * rest of the frame.
	 *
	 * The function below will allocate two adjacent BDs with Bd1Ptr being
	 * set as the lead BD.
	 */
	Status = XAxiDma_BdRingAlloc(TxRingPtr, 2, &Bd1Ptr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error allocating TxBD");
		return XST_FAILURE;
	}

	/*
	 * Setup TxBD #1
	 */
	XAxiDma_BdSetBufAddr(Bd1Ptr, (UINTPTR)&TxFrame);

#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(Bd1Ptr, 32);
#else
	XAxiDma_BdSetLength(Bd1Ptr, 32,
			TxRingPtr->MaxTransferLen);
#endif

	XAxiDma_BdSetCtrl(Bd1Ptr, XAXIDMA_BD_CTRL_TXSOF_MASK);

	/*
	 * Setup TxBD #2
	 */
	Bd2Ptr = (XAxiDma_Bd *)XAxiDma_BdRingNext(TxRingPtr, Bd1Ptr);
	XAxiDma_BdSetBufAddr(Bd2Ptr, (UINTPTR) (&TxFrame) + 32);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(Bd2Ptr, TxFrameLength - 32);
#else
	XAxiDma_BdSetLength(Bd2Ptr, TxFrameLength - 32,
				TxRingPtr->MaxTransferLen);
#endif

	XAxiDma_BdSetCtrl(Bd2Ptr, XAXIDMA_BD_CTRL_TXEOF_MASK);

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(TxRingPtr, 2, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		/*
		 * Undo BD allocation and exit
		 */
		XAxiDma_BdRingUnAlloc(TxRingPtr, 2, Bd1Ptr);
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
	 * Since we have only submitted 2 to HW, then there should be only 2 ready
	 * for post processing.
	 */
	if (XAxiDma_BdRingFromHw(TxRingPtr, 2, &Bd1Ptr) == 0) {
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
	Status = XAxiDma_BdRingFree(TxRingPtr, 2, Bd1Ptr);
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
	if (XAxiDma_BdRingFromHw(RxRingPtr, 1, &Bd1Ptr) == 0) {
		AxiEthernetUtilErrorTrap("RxBD was not ready for post processing");
		return XST_FAILURE;
	}
	BdCurPtr = Bd1Ptr;
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

	if (RxFrameLength != TxFrameLength) {
		AxiEthernetUtilErrorTrap("Length mismatch");
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
	Status = XAxiDma_BdRingFree(RxRingPtr, 1, Bd1Ptr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
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
* This example sends frames with the interrupt coalescing settings altered
* from their defaults.
*
* The default settings will interrupt the processor after every frame has been
* sent. This example will increase the threshold resulting in lower CPU
* utilization since it spends less time servicing interrupts.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
* @param	DmaInstancePtr   is a pointer to the instance of the Dma
*		component.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note     None.
*
******************************************************************************/
int AxiEthernetSgDmaIntrCoalescingExample(XAxiEthernet *AxiEthernetInstancePtr,
						XAxiDma *DmaInstancePtr)
{
	int Status;
	u32 TxFrameLength;
	int PayloadSize = 100;
	u32 Index;
	u32 NumBd;
	u16 Threshold = 10;
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;

	/*
	 * Calculate the frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/*
	 * Setup packet to be transmitted. The same packet will be
	 * transmitted over and over
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Flush the TX frame before giving it to DMA TX channel to transmit.
	 */
	Xil_DCacheFlushRange((UINTPTR)&TxFrame, TxFrameLength);

	/*
	 * Make sure Tx and Rx are enabled
	 */
	Status = XAxiEthernet_SetOptions(AxiEthernetInstancePtr,
				     XAE_TRANSMITTER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting options");
		return XST_FAILURE;
	}

	/*
	 * We don't care about the receive channel for this example,
	 * so turn it off, in case it was turned on earlier.
	 */

	Status = XAxiEthernet_ClearOptions(AxiEthernetInstancePtr,
				       XAE_RECEIVER_ENABLE_OPTION);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error clearing option");
		return XST_FAILURE;
	}

	/*
	 * Set the interrupt coalescing parameters for the test. The waitbound
	 * timer is set to 1 (ms) to catch the last few frames.
	 *
	 * If you set variable Threshold to some value larger than TXBD_CNT,
	 * then there can never be enough frames sent to meet the threshold.
	 * In this case the waitbound timer will always cause the interrupt to
	 * occur.
	 */
	Status = XAxiDma_BdRingSetCoalesce(TxRingPtr, Threshold, 255);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting coalescing settings");
		return XST_FAILURE;
	}

	/*
	 * Enable the send interrupts. Nothing should be transmitted yet as the
	 * device has not been started
	 */
	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/*
	 * Prime the engine, allocate all BDs and assign them to the same buffer
	 */
	Status = XAxiDma_BdRingAlloc(TxRingPtr, Threshold, &BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error allocating TxBDs prior to starting");
		return XST_FAILURE;
	}

	/*
	 * Setup the TxBDs
	 */
	BdCurPtr = BdPtr;

	for (Index = 0; Index < Threshold; Index++) {
		XAxiDma_BdSetBufAddr(BdCurPtr, (UINTPTR)&TxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
		XAxiDma_BdSetLength(BdCurPtr, TxFrameLength);
#else
		XAxiDma_BdSetLength(BdCurPtr, TxFrameLength,
					TxRingPtr->MaxTransferLen);
#endif

		XAxiDma_BdSetCtrl(BdCurPtr, XAXIDMA_BD_CTRL_TXSOF_MASK |
				     XAXIDMA_BD_CTRL_TXEOF_MASK);
		BdCurPtr = (XAxiDma_Bd *) XAxiDma_BdRingNext(TxRingPtr, BdCurPtr);
	}

	/*
	 *  Enqueue all TxBDs to HW
	 */
	Status = XAxiDma_BdRingToHw(TxRingPtr, Threshold, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error committing TxBDs prior to starting");
		return XST_FAILURE;
	}

	/*
	 * Start the device. Transmission commences now!
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Wait for the interrupt
	 */
	while (!FramesTx);

	NumBd = XAxiDma_BdRingFromHw(TxRingPtr, Threshold, &BdPtr);
	if (NumBd != Threshold) {
		AxiEthernetUtilErrorTrap("Error in interrupt coalescing");
	}
	else {
		/*
		 * Don't bother to check the BDs status, just free them
		 */
		Status = XAxiDma_BdRingFree(TxRingPtr, Threshold, BdPtr);
		if (Status != XST_SUCCESS) {
			AxiEthernetUtilErrorTrap("Error freeing TxBDs");
		}
	}

	/*
	 * Done sending frames. Stop the device
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
* The transmit frame will be checksummed over the entire Ethernet payload
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
int AxiEthernetSgDmaPartialChecksumOffloadExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;
	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdSts;

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
	DeviceErrors = 0;

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
	XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR)&RxFrame);
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
	XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR)&TxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(BdPtr, TxFrameLength);
#else
	XAxiDma_BdSetLength(BdPtr, TxFrameLength,
				TxRingPtr->MaxTransferLen);
#endif

	XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_TXSOF_MASK |
						XAXIDMA_BD_CTRL_TXEOF_MASK);

	/*
	 * Setup TxBd checksum offload attributes.
	 * Note that the checksum offload values can be set globally for all
	 * TxBds when XAxiDma_BdRingClone() is called to setup Tx BD space.
	 * This would eliminate the need to set them here.
	 */
	/* Enable hardware checksum computation for the buffer descriptor */
	Status = XAxiDma_BdSetAppWord(BdPtr,BD_USR0_OFFSET,PARTIAL_CSUM_ENABLE);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling partial csum offloading");
		return XST_FAILURE;
	}

	/* Write Start Offset and Insert Offset into BD */
	Status = XAxiDma_BdSetAppWord(BdPtr,BD_USR1_OFFSET,
		((XAE_HDR_SIZE << TX_CS_INIT_OFFSET) | (TxFrameLength - 2)));
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in setting csum insert point");
		return XST_FAILURE;
	}

	/* Write 0, as the seed value, to the BD */
	Status = XAxiDma_BdSetAppWord(BdPtr,BD_USR2_OFFSET,0);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling partial csum offloading");
		return XST_FAILURE;
	}

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(TxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error committing TxBD to HW");
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (!FramesTx);
	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 2 to HW, then there should be only 2 ready
	 * for post processing.
	 */
	if (XAxiDma_BdRingFromHw(TxRingPtr, 1, &BdPtr) == 0) {
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
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	 */
	if (XAxiDma_BdRingFromHw(RxRingPtr, 1, &BdPtr) == 0) {
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

	if (RxFrameLength != TxFrameLength) {
		AxiEthernetUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}

	/*
	 * Verify the checksum as computed by HW. It should add up to 0xFFFF
	 * if frame was uncorrupted
	 */
	if ((u16) (XAxiDma_BdRead(BdPtr, XAXIDMA_BD_USR3_OFFSET))
		!= RCV_FRM_NOT_CORRUPTED) {
		AxiEthernetUtilErrorTrap("Rx checksum incorrect");
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
int AxiEthernetSgDmaFullChecksumOffloadExample(XAxiEthernet
			*AxiEthernetInstancePtr, XAxiDma *DmaInstancePtr)
{
	int Status;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int PayloadSize = 64;
	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(DmaInstancePtr);
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(DmaInstancePtr);
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdSts;
	u8 DummyIPAddr[4];
	u32 FullCsumStatus;
	u8 *IPHdrPntr;
	u8 *TCPHdrPntr;
	u16 TempShortToCopy;
	u8 *PtrToCopyShort;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesTx = 0;
	DeviceErrors = 0;

	memset(RxFrame,0,sizeof(RxFrame));
	memset(TxFrame,0,sizeof(TxFrame));

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + IP_HEADER_LENGTH + TCP_HEADER_LENGTH +
								PayloadSize ;

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
	XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR)&RxFrame);
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
	XAxiDma_BdSetBufAddr(BdPtr, (UINTPTR)&TxFrame);
#ifndef XPAR_AXIDMA_0_ENABLE_MULTI_CHANNEL
	XAxiDma_BdSetLength(BdPtr, TxFrameLength);
#else
	XAxiDma_BdSetLength(BdPtr, TxFrameLength,
				TxRingPtr->MaxTransferLen);
#endif

	XAxiDma_BdSetCtrl(BdPtr, XAXIDMA_BD_CTRL_TXSOF_MASK |
						XAXIDMA_BD_CTRL_TXEOF_MASK);

	/*
	 * Setup TxBd checksum offload attributes.
	 */
	/* Enable hardware checksum computation for the buffer descriptor */
	Status = XAxiDma_BdSetAppWord(BdPtr, BD_USR0_OFFSET, FULL_CSUM_ENABLE);
	if(Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in enabling full csum offloading");
		return XST_FAILURE;
	}

	/*
	 * Enqueue to HW
	 */
	Status = XAxiDma_BdRingToHw(TxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error committing TxBD to HW");
		return XST_FAILURE;
	}

	/*
	 * Wait for transmission to complete
	 */
	while (!FramesTx);

	/*
	 * Now that the frame has been sent, post process our TxBDs.
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	 */
	if (XAxiDma_BdRingFromHw(TxRingPtr, 1, &BdPtr) == 0) {
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
	 * Since we have only submitted 1 to HW, then there should be only 1 ready
	 * for post processing.
	 */
	if (XAxiDma_BdRingFromHw(RxRingPtr, 1, &BdPtr) == 0) {
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
	BdCurPtr = BdPtr;
	BdSts = XAxiDma_BdGetSts(BdCurPtr);
	if ((BdSts & XAXIDMA_BD_STS_ALL_ERR_MASK) ||
		(!(BdSts & XAXIDMA_BD_STS_COMPLETE_MASK))) {
			AxiEthernetUtilErrorTrap("Rx Error");
			return XST_FAILURE;
	}
	else {

		RxFrameLength =
			(XAxiDma_BdRead(BdCurPtr,XAXIDMA_BD_USR4_OFFSET)) &
								0x0000FFFF;
	}

	if (RxFrameLength != TxFrameLength) {
		AxiEthernetUtilErrorTrap("Length mismatch");
		return XST_FAILURE;
	}

	/*
	 * Read the full checksum validation status from AXI4 Status Word.
	 */
	FullCsumStatus = (((XAxiDma_BdRead(BdPtr, XAXIDMA_BD_USR2_OFFSET)) &
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
	Status = XAxiDma_BdRingFree(RxRingPtr, 1, BdPtr);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error freeing up TxBDs");
		return XST_FAILURE;
	}

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
* This is the DMA TX callback function to be called by TX interrupt handler.
* This function handles BDs finished by hardware.
*
* @param    TxRingPtr is a pointer to TX channel of the DMA engine.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
static void TxCallBack(XAxiDma_BdRing *TxRingPtr)
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
* @note		This Interrupt handler MUST clear pending interrupts before
*		handling them by calling the call back. Otherwise the following
*		corner case could raise some issue:
*
*		A packet got transmitted and a TX interrupt got asserted. If
*		the interrupt handler calls the callback before clearing the
*		interrupt, a new packet may get transmitted in the callback.
*		This new packet then can assert one more TX interrupt before
*		the control comes out of the callback function. Now when
*		eventually control comes out of the callback function, it will
*		never know about the second new interrupt and hence while
*		clearing the interrupts, would clear the new interrupt as well
*		and will never process it.
*		To avoid such cases, interrupts must be cleared before calling
*		the callback.
*
******************************************************************************/
static void TxIntrHandler(XAxiDma_BdRing *TxRingPtr)
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

	/*
	 * If error interrupt is asserted, raise error flag, reset the
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
static void RxCallBack(XAxiDma_BdRing *RxRingPtr)
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
static void RxIntrHandler(XAxiDma_BdRing *RxRingPtr)
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

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		AxiEthernetUtilErrorTrap("AXIDMA: RX Error interrupts\n");

		/*
		 * Reset could fail and hang
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
* @param	AxiEthernet is a reference to the Axi Ethernet device instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void AxiEthernetErrorHandler(XAxiEthernet *AxiEthernet)
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
								(XInterruptHandler)AxiEthernetErrorHandler,
									AxiEthernetInstancePtr);
	Status |= XIntc_Connect(IntcInstancePtr, DmaTxIntrId,
								(XInterruptHandler) TxIntrHandler, TxRingPtr);
	Status |= XIntc_Connect(IntcInstancePtr, DmaRxIntrId,
								(XInterruptHandler) RxIntrHandler, RxRingPtr);

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
