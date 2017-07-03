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
/**
*
* @file xaxiethernet_example_intr_fifo.c
*
* Implements examples that utilize the Axi Ethernet's interrupt driven FIFO
* direct packet transfer mode to send and receive frames.
*
* These examples demonstrate:
*
* - How to perform simple send and receive.
* - Advanced frame processing
* - Error handling
* - Device reset
*
* Functional guide to example:
*
* - AxiEthernetSingleFrameIntrExample() demonstrates the simplest way to send
*   and receive frames in interrupt driven FIFO direct mode.
*
* - AxiEthernetSingleFrameNonContIntrExample demonstrates how to handle frames
*   that are stored in more than one memory location.
*
* - AxiEthernetMultipleFramesIntrExample demonstrates how to defer frame
*   reception so that CPU intensive receive functions are not performed in
*   interrupt context.
*
* - AxiEthernetErrorHandler() demonstrates how to manage asynchronous errors.
*
* - AxiEthernetResetDevice() demonstrates how to reset the driver/HW while
    maintaining  driver/HW state.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  4/30/10  First release based on the ll temac driver
* 3.00a bss  10/22/12 Added support for Fast Interrupt Handlers.
* 3.01a srt  02/14/13 Added support for Zynq (CR 681136)
* 3.02a srt  08/06/13 Fixed CR 727634 -
*			Modified FifoHandler() logic to reflect the bit
*			changes in the Interrupt Status Register as per
*			the latest AXI FIFO stream IP.
* 5.4   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
*       ms   04/05/17 Added return tags in functions for proper documentation
*                     while generating doxygen.
* 5.6	adk  03/07/17 Fixed CR#979023 Example failed to compile.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#include "xllfifo.h"
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
#define FIFO_DEVICE_ID		XPAR_AXI_FIFO_0_DEVICE_ID
#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define AXIETHERNET_IRPT_INTR	XPAR_INTC_0_AXIETHERNET_0_VEC_ID
#define FIFO_IRPT_INTR		XPAR_INTC_0_LLFIFO_0_VEC_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define AXIETHERNET_IRPT_INTR	XPAR_FABRIC_AXIETHERNET_0_VEC_ID
#define FIFO_IRPT_INTR		XPAR_FABRIC_LLFIFO_0_VEC_ID
#endif
#endif

#define PAYLOAD_SIZE		100	/* Payload size used in examples */
/*************************** Variable Definitions ****************************/

static EthernetFrame TxFrame;	/* Frame used to send with */
static EthernetFrame RxFrame;	/* Frame used to receive data */
volatile static int DeferRx = 0;

/*
 * Counters setup to be incremented by callbacks
 */
static volatile int FramesRx;		/* Number of received frames */
static volatile int FramesRxInts;	/* Number of ints for received
					 *  frames
					 */
static volatile int FramesTxInts;	/* Number of ints for sent frames */
static volatile int DeviceErrors;	/* Num of errors detected in
					 * the device
					 */
static volatile int FrameDataErrors;	/* Num of times frame data check
					 * failed
					 */
volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */
volatile int ExternalLoopback; /* Variable for External loopback */
XAxiEthernet AxiEthernetInstance;
XLlFifo FifoInstance;


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

#if XPAR_INTC_0_HAS_FAST == 1

/* Variables for Fast Interrupt Handlers */
XAxiEthernet *AxiEthernetInstancePtr_Fast;
XLlFifo *Fifo_Fast;

/******  Fast Interrupt Handlers prototypes  ******/

static void AxiEthernetErrorFastHandler(void) __attribute__ ((fast_interrupt));

static void FifoFastHandler(void) __attribute__ ((fast_interrupt));

#else

static void AxiEthernetErrorHandler(XAxiEthernet *AxiEthernet);
static void FifoHandler(XLlFifo *Fifo);

#endif


/*************************** Function Prototypes *****************************/

/*
 * The different examples given in this file
 */
int AxiEthernetFifoIntrExample(INTC *IntcInstancePtr,
			 XAxiEthernet *AxiEthernetInstancePtr,
			 XLlFifo *FifoInstancePtr,
			 u16 AxiEthernetDeviceId,
			 u16 FifoDeviceId, u16 AxiEthernetIntrId,
			 u16 FifoIntrId);
int AxiEthernetSingleFrameIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
				XLlFifo *FifoInstancePtr);
int AxiEthernetSingleFrameNonContIntrExample(XAxiEthernet
			*AxiEthernetInstancePtr, XLlFifo *FifoInstancePtr);
int AxiEthernetMultipleFramesIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
				   XLlFifo *FifoInstancePtr);


/*
 * The Interrupt setup and callbacks
 */
static int AxiEthernetSetupIntrSystem(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XLlFifo *FifoInstancePtr,
				u16 AxiEthernetIntrId, u16 FifoIntrId);
static void AxiEthernetDisableIntrSystem(INTC *IntcInstancePtr,
				   u16 AxiEthernetIntrId, u16 FifoIntrId);

static void FifoRecvHandler(XLlFifo *Fifo);

/*
 * Utility routines
 */
static int AxiEthernetResetDevice(XAxiEthernet *AxiEthernetInstancePtr,
					XLlFifo *FifoInstancePtr);


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
*		- XST_FAILURE.to indicate failure.
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
	 * Call the Axi Ethernet interrupt example , specify the parameters
	 * generated in xparameters.h
	 */
	Status = AxiEthernetFifoIntrExample(&IntcInstance,
					&AxiEthernetInstance,
					&FifoInstance,
					AXIETHERNET_DEVICE_ID,
					FIFO_DEVICE_ID,
					AXIETHERNET_IRPT_INTR, FIFO_IRPT_INTR);

	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Axiethernet intr fifo Example Failed\r\n");
	} else {
		AxiEthernetUtilErrorTrap("Successfully ran Axiethernet intr fifo Example\r\n");
	}

	AxiEthernetUtilErrorTrap("--- Exiting main() ---");


	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif



/*****************************************************************************/
/**
*
* This function demonstrates the usage usage of the Axi Ethernet by sending
* and receiving frames in interrupt driven fifo mode.
*
*
* @param	IntcInstancePtr is a pointer to the instance of the Intc
*		component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		Axi Ethernet component.
* @param	FifoInstancePtr is a pointer to the instance of the AXIFIFO
*		component.
* @param	AxiEthernetDeviceId is Device ID of the Axi Ethernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	FifoDeviceId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	AxiEthernetIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	FifoIntrId is the interrupt id for fifo.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		AxiFifo hardware must be initialized before initializing
*		AxiEthernet. Since AxiFifo reset line is connected to the
*		AxiEthernet reset line, a reset of AxiFifo hardware during its
*		initialization would reset AxiEthernet.
*
******************************************************************************/
int AxiEthernetFifoIntrExample(INTC *IntcInstancePtr,
			 XAxiEthernet *AxiEthernetInstancePtr,
			 XLlFifo *FifoInstancePtr,
			 u16 AxiEthernetDeviceId,
			 u16 FifoDeviceId, u16 AxiEthernetIntrId,
			 u16 FifoIntrId)
{
	int Status;
	XAxiEthernet_Config *MacCfgPtr;
	int LoopbackSpeed;


	/*************************************/
	/* Setup device for first-time usage */
	/*************************************/

	/*
	 *  Get the configuration of AxiEthernet hardware.
	 */
	MacCfgPtr = XAxiEthernet_LookupConfig(AxiEthernetDeviceId);

	/*
	 * Check whether AXIFIFO is present or not
	 */
	if(MacCfgPtr->AxiDevType != XPAR_AXI_FIFO) {
		AxiEthernetUtilErrorTrap
			("Device HW not configured for FIFO mode\r\n");
		return XST_FAILURE;
	}

	/*
	 * Initialize AXIFIFO hardware. AXIFIFO must be initialized before
	 * AxiEthernet. During AXIFIFO initialization, AXIFIFO hardware is
	 * reset, and since AXIFIFO reset line is connected to AxiEthernet,
	 * this would ensure a reset of AxiEthernet.
	 */
	XLlFifo_Initialize(FifoInstancePtr, MacCfgPtr->AxiDevBaseAddress);

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
	 * Set the MAC  address
	 */
	Status = XAxiEthernet_SetMacAddress(AxiEthernetInstancePtr,
							(u8 *) AxiEthernetMAC);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting MAC address");
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
	Status = AxiEthernetUtilEnterLoopback(AxiEthernetInstancePtr,
							LoopbackSpeed);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting the PHY loopback");
		return XST_FAILURE;
	}

	/*
	 * Set PHY<-->MAC data clock
	 */
	Status = XAxiEthernet_SetOperatingSpeed(AxiEthernetInstancePtr,
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

	/* Clear any pending FIFO interrupts from any previous
	 * examples (e.g., polled)
	 */
	XLlFifo_IntClear(FifoInstancePtr, XLLF_INT_ALL_MASK);

	/*
	 * Connect to the interrupt controller and enable interrupts
	 */
	Status = AxiEthernetSetupIntrSystem(IntcInstancePtr,
				      AxiEthernetInstancePtr,
				      FifoInstancePtr, AxiEthernetIntrId,
				      FifoIntrId);

	/****************************/
	/* Run through the examples */
	/****************************/


	/*
	 * Run the AxiEthernet Single Frame Interrupt example
	 */
	Status = AxiEthernetSingleFrameIntrExample(AxiEthernetInstancePtr,
							FifoInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the AxiEthernet Single Frame Non Continuous Interrupt example
	 */
	Status = AxiEthernetSingleFrameNonContIntrExample
				(AxiEthernetInstancePtr, FifoInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	DeferRx = 1;
	Status = AxiEthernetMultipleFramesIntrExample(AxiEthernetInstancePtr,
							   FifoInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = AxiEthernetResetDevice(AxiEthernetInstancePtr,
							FifoInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Disable the interrupts for the AxiEthernet device
	 */
	AxiEthernetDisableIntrSystem(IntcInstancePtr, AxiEthernetIntrId,
								FifoIntrId);


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
* receiving a single frame in interrupt mode.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	FifoInstancePtr is a pointer to the instance of the Fifo
*		component.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSingleFrameIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
					XLlFifo *FifoInstancePtr)
{
	u32 FifoFreeBytes;
	u32 TxFrameLength;
	int PayloadSize = PAYLOAD_SIZE;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesRxInts = 0;
	FramesTxInts = 0;
	DeviceErrors = 0;
	FrameDataErrors = 0;

	/*
	 * Setup packet to be transmitted
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/****************/
	/* Setup device */
	/****************/

	/*
	 * Start the device
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Enable the interrupts
	 */
	XLlFifo_IntEnable(FifoInstancePtr, XLLF_INT_ALL_MASK);
	XAxiEthernet_IntEnable(AxiEthernetInstancePtr,
			   XAE_INT_RXRJECT_MASK | XAE_INT_RXFIFOOVR_MASK);


		/*******************/
		/* Send the packet */
		/*******************/

	/*
	 * Find out how much room is in the FIFO
	 * Vacancy is a value in 32 bit words. Multiply by 4 to get bytes.
	 */
	FifoFreeBytes = XLlFifo_TxVacancy(FifoInstancePtr) * 4;
	if (FifoFreeBytes < TxFrameLength) {
		AxiEthernetUtilErrorTrap("Not enough room in FIFO for frame");
		return XST_FAILURE;
	}

	/*
	 * Write frame data to FIFO
	 */
	XLlFifo_Write(FifoInstancePtr, TxFrame, TxFrameLength);

	/*
	 * Initiate the transmit
	 */
	XLlFifo_TxSetLen(FifoInstancePtr, TxFrameLength);

	/*
	 * Wait for receive indication or error
	 */
	while ((FramesRx == 0) && (DeviceErrors == 0));

	/*
	 * Stop the device
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This example sends a packet from non-contiguous memory locations. The header
* is stored in one area. The payload data is calculated and written to the
* packet  FIFO one byte at a time.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	FifoInstancePtr is a pointer to the instance of the Fifo
*		component.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSingleFrameNonContIntrExample(XAxiEthernet
						*AxiEthernetInstancePtr,
						XLlFifo *FifoInstancePtr)
{
	u32 FifoFreeBytes;
	int PayloadSize = 20;
	u8 PayloadData;
	u32 TxFrameLength;

	/*
	 * Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesRxInts = 0;
	FramesTxInts = 0;
	DeviceErrors = 0;
	FrameDataErrors = 0;

	/*
	 * Setup the transmit packet header
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);

	/*
	 * Clear out receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/****************/
	/* Setup device */
	/****************/

	/*
	 * Start the device
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Enable interrupts
	 */
	XLlFifo_IntEnable(FifoInstancePtr, XLLF_INT_ALL_MASK);

	/*******************/
	/* Send the packet */
	/*******************/

	/*
	 * Make sure there is enough room for a full sized frame
	 * Vacancy is a value in 32 bit words. Multiply by 4 to get bytes.
	 */
	FifoFreeBytes = XLlFifo_TxVacancy(FifoInstancePtr) * 4;

	if (FifoFreeBytes < (XAE_MTU + XAE_HDR_SIZE)) {
		AxiEthernetUtilErrorTrap("Not enough room in FIFO for frame");
		return XST_FAILURE;
	}

	/*
	 * Write the header data
	 */
	XLlFifo_Write(FifoInstancePtr, TxFrame, XAE_HDR_SIZE);

	/*
	 * Write payload one byte at a time. Set the payload like the
	 * AxiEthernetUtilFrameSetPayloadData() function would. This is done
	 * so that the received packet will pass validation in
	 * AxiEthernetRecvHandler().
	 *
	 * Keep PayloadSize less than 255 since
	 * AxiEthernetUtilFrameSetPayloadData() switches to a 16 bit
	 * counter at 256.
	 *
	 * This is not the fastest way to send a frame of data but it does
	 * illustrate the flexibility of the API.
	 */
	PayloadData = 0;
	while ((PayloadData < PayloadSize) && (DeviceErrors == 0)) {
		XLlFifo_Write(FifoInstancePtr, &PayloadData, 1);
		PayloadData++;
	}

	/*
	 * Did it all get written without error
	 */
	if (DeviceErrors != 0) {
		AxiEthernetUtilErrorTrap
			("Error writing payload to FIFO, reset recommended");
		return XST_FAILURE;
	}

	/*
	 * Now begin transmission
	 */
	XLlFifo_TxSetLen(FifoInstancePtr, TxFrameLength);

	/*
	 * Wait for receive indication or error
	 */
	while ((FramesRx == 0) && (DeviceErrors == 0));

	/*
	 * Stop the device
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This example sends and receives a batch of frames. Frame reception is handled
* in this function and not in the callback function.
*
* Use this method of reception when interrupt latency is important.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	FifoInstancePtr is a pointer to the instance of the Fifo
*		component.
*
* @return
*		- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		None.
*
******************************************************************************/
int AxiEthernetMultipleFramesIntrExample(XAxiEthernet *AxiEthernetInstancePtr,
						XLlFifo *FifoInstancePtr)
{
	int FramesToLoopback;
	int PayloadSize;
	u32 TxFrameLength;
	u32 RxFrameLength;
	int Index;

	/*
	 *  Clear variables shared with callbacks
	 */
	FramesRx = 0;
	FramesRxInts = 0;
	FramesTxInts = 0;
	DeviceErrors = 0;
	FrameDataErrors = 0;

	/*
	 * Setup the number of frames to loopback and the size of the frame to
	 * loopback. The default settings should work for every case. Modifying
	 * the settings can cause problems, see discussion below:
	 *
	 * If PayloadSize is set small and FramesToLoopback high, then it is
	 * possible to cause the transmit status FIFO to overflow.
	 *
	 * If PayloadSize is set large and FramesToLoopback high, then it is
	 * possible to cause the transmit packet FIFO to overflow.
	 *
	 * Either of these scenarios may be worth trying out to observe how the
	 * driver reacts. The exact values to cause these types of errors
	 * will vary due to the sizes of the FIFOs selected at hardware build
	 * time. But the following settings should create problems for all
	 * FIFO sizes:
	 *
	 * Transmit status FIFO overflow
	 *    PayloadSize = 1
	 *    FramesToLoopback = 1000
	 *
	 * Transmit packet FIFO overflow
	 *    PayloadSize = 1500
	 *    FramesToLoopback = 16
	 *
	 * These values should always work without error
	 *    PayloadSize = 100
	 *    FramesToLoopback = 5
	 */
	PayloadSize = 100;
	FramesToLoopback = 5;

	/*
	 * Setup the transmit packet
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

		/****************/
		/* Setup device */
		/****************/

	/*
	 * Start the device
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	/*
	 * Enable the interrupts
	 */
	XLlFifo_IntEnable(FifoInstancePtr, XLLF_INT_ALL_MASK);


	/****************/
	/* Send packets */
	/****************/

	/*
	 * Since we may be interested to see what happens when FIFOs overflow,
	 * don't check for room in the transmit packet FIFO prior to writing
	 * to it.
	 */

	/*
	 * With the xps_ll_fifo core we can't stuff the fifo with data from
	 * multiple packets and then send them. Instead, the code needs to
	 * write the data, and then immediately send the packet before
	 * writting the data for the next packet.
	 */
	for (Index = 0; Index < FramesToLoopback; Index++) {
		/*
		 * Write frame data to FIFO
		 */
		XLlFifo_Write(FifoInstancePtr, TxFrame, TxFrameLength);
		/*
		 * Initiate the transmission
		 */
		XLlFifo_TxSetLen(FifoInstancePtr, TxFrameLength);
	}

	/*******************/
	/* Receive packets */
	/*******************/

	/*
	 * Now wait for frames to be received. When the callback is executed,
	 * it will disable interrupts and set a shared variable which will
	 * trigger this routine to process received frames
	 */
	for (Index = 0; Index < FramesToLoopback; Index++) {
		/*
		 * Wait
		 */
		while (FramesRxInts == 0);

			/*
		 * Frame has arrived, so get the length
		 */
		RxFrameLength = XLlFifo_RxGetLen(FifoInstancePtr);

		/*
		 * Decision time: We can re-enable receive interrupts here or
		 * after we read the frame out of the FIFO. This is a matter
		 * of preference and goals of an application using the driver.
		 */
		XLlFifo_IntEnable(FifoInstancePtr, XLLF_INT_RC_MASK);

		/*
		 * Frame size as expected?
		 */
		if ((RxFrameLength) != TxFrameLength) {
			AxiEthernetUtilErrorTrap("Receive length incorrect");
		}

		/*
		 * Clear out receive packet memory area
		 */
		AxiEthernetUtilFrameMemClear(&RxFrame);

		/*
		 * Read frame from packet FIFO
		 */
		XLlFifo_Read(FifoInstancePtr, &RxFrame, RxFrameLength);
		/*
		 * Verify the received data
		 */
		if (AxiEthernetUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
			AxiEthernetUtilErrorTrap("Data mismatch");
			return XST_FAILURE;
		}
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
* This is the Receive handler callback function for examples 1 and 2.
* It will increment a shared  counter, receive and validate the frame.
*
* @param	Fifo is a reference to the Fifo device instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoRecvHandler(XLlFifo *Fifo)
{
	u32 FrameLength;

	/*
	 * We get the interrupt only once for multiple frames received.
	 * So get all the frames we can.
	 */
	/* While there is data in the fifo ... */
	while (XLlFifo_RxOccupancy(Fifo)) {
		/*
		 * Get the packet length
		 */
		FrameLength = XLlFifo_RxGetLen(Fifo);

		XLlFifo_Read(Fifo, RxFrame, FrameLength);
		/*
		 * Validate the packet data against the header of the TxFrame.
		 * The payload data should as placed by
		 * AxiEthernetUtilFrameSetPayloadData()
		 */
		if (AxiEthernetUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
			FrameDataErrors++;
			AxiEthernetUtilErrorTrap("Data mismatch");
			return;
		}
		/*
		 * Bump counter
		 */
		FramesRx++;
	}
}

/*****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments the
* the error counter so that the main thread knows the number of errors.
*
* @param	Fifo is a reference to the AxiEthernet device instance.
*
* @param	Pending is a bitmask of the pending interrupts.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoErrorHandler(XLlFifo *Fifo, u32 Pending)
{
	int timeout_counter;

	if (Pending & XLLF_INT_RPURE_MASK) {
		AxiEthernetUtilErrorTrap("Fifo: Rx under-read error");
	}
	if (Pending & XLLF_INT_RPORE_MASK) {
		AxiEthernetUtilErrorTrap("Fifo: Rx over-read error");
	}
	if (Pending & XLLF_INT_RPUE_MASK) {
		AxiEthernetUtilErrorTrap("Fifo: Rx fifo empty");
	}
	if (Pending & XLLF_INT_TPOE_MASK) {
		AxiEthernetUtilErrorTrap("Fifo: Tx fifo overrun");
	}
	if (Pending & XLLF_INT_TSE_MASK) {
		AxiEthernetUtilErrorTrap("Fifo: Tx length mismatch");
	}

	/*
	 * Reset the tx or rx side of the fifo as needed
	 */
	if (Pending & XLLF_INT_RXERROR_MASK) {
		XLlFifo_IntClear(Fifo, XLLF_INT_RRC_MASK);
		XLlFifo_RxReset(Fifo);

		timeout_counter = 10000;

		while ((XLlFifo_Status(Fifo) & XLLF_INT_RRC_MASK) == 0) {
			timeout_counter--;
			if (timeout_counter == 0) {
				XLlFifo_Reset(Fifo);
			/* we've reset the whole core so just exit out */
				goto feh_exit;
			}
		}
	}

	if (Pending & XLLF_INT_TXERROR_MASK) {
		XLlFifo_IntClear(Fifo, XLLF_INT_TRC_MASK);
		XLlFifo_TxReset(Fifo);

		timeout_counter = 10000;

		while ((XLlFifo_Status(Fifo) & XLLF_INT_TRC_MASK) == 0) {
			timeout_counter--;
			if (timeout_counter == 0) {
				XLlFifo_Reset(Fifo);

			/* we've reset the whole core so just exit out */
				goto feh_exit;
			}
		}
	}

	feh_exit:
	/*
	 * Bump counter
	 */
	DeviceErrors++;
}


/*****************************************************************************/
/**
*
* This is the Fifo handler function and  will increment a shared
* counter that can be tested by the main thread of operation.
*
* @param	Fifo is a reference to the Fifo instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoHandler(XLlFifo *Fifo)
{
	u32 Pending = XLlFifo_IntPending(Fifo);

	while (Pending) {
		if (Pending & XLLF_INT_RC_MASK) {
			/*
			 * Receive the frame, unless we are deferring the
			 * receive.
			 */
			if (DeferRx) {
				FramesRxInts++;	/* We can count the interrupts,
						 * but in the handler we don't
						 * exactly know how many frames
						 * as we could get one int for
						 * multiple frames.
						 */
				/*
				 * use for example 3: Disable receive
				 * interrupts to defer frame reception
				 * to the example function.
				 */
				XLlFifo_IntDisable(Fifo, XLLF_INT_RC_MASK);
			}
			else {
				FifoRecvHandler(Fifo);
			}
			XLlFifo_IntClear(Fifo, XLLF_INT_RC_MASK);
		}
		else if (Pending & XLLF_INT_TC_MASK) {
			FramesTxInts++;	/* We can count the interrupts, but in
					 * the handler we don't exactly know
					 * how many frames as we could get one
					 * int for multiple frames.
					 */
			XLlFifo_IntClear(Fifo, XLLF_INT_TC_MASK);
		}
		else if (Pending & XLLF_INT_ERROR_MASK){
			FifoErrorHandler(Fifo, Pending);
			XLlFifo_IntClear(Fifo, XLLF_INT_ERROR_MASK);
		} else {
			XLlFifo_IntClear(Fifo, Pending);
		}
		Pending = XLlFifo_IntPending(Fifo);
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

/******************************************************************************/
/**
* This function resets the device but preserves the options set by the user.
*
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	FifoInstancePtr is a pointer to the instance of the Fifo
*		component.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		None.
*
******************************************************************************/
static int AxiEthernetResetDevice(XAxiEthernet *AxiEthernetInstancePtr,
					XLlFifo *FifoInstancePtr)
{
	int Status;
	u8 MacSave[6];
	u32 Options;

	/*
	 * Stop the Axi Ethernet device
	 */
	XAxiEthernet_Stop(AxiEthernetInstancePtr);


	/*
	 * Save the device state
	 */
	XAxiEthernet_GetMacAddress(AxiEthernetInstancePtr, MacSave);
	Options = XAxiEthernet_GetOptions(AxiEthernetInstancePtr);

	/*
	 * Stop and reset the Axi Ethernet device
	 */
	XAxiEthernet_Reset(AxiEthernetInstancePtr);

	/*
	 * reset the fifo
	 */
	XLlFifo_Reset(FifoInstancePtr);

	/*
	 * Restore the state
	 */
	Status = XAxiEthernet_SetMacAddress(AxiEthernetInstancePtr, MacSave);
	Status |= XAxiEthernet_SetOptions(AxiEthernetInstancePtr, Options);
	Status |= XAxiEthernet_ClearOptions(AxiEthernetInstancePtr, ~Options);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error restoring state after reset");
		return XST_FAILURE;
	}

	/*
	 * Restart the device
	 */
	XAxiEthernet_Start(AxiEthernetInstancePtr);

	return XST_SUCCESS;
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
* @param	IntcInstancePtr is a pointer to the instance of the Intc component.
* @param	AxiEthernetInstancePtr is a pointer to the instance of the
*		AxiEthernet component.
* @param	FifoInstancePtr is a pointer to the instance of the AXIFIFO
*		component.
* @param	AxiEthernetDeviceId is Device ID of the Axi Ethernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	AxiEthernetIntrId is the Interrupt ID and is typically
*		XPAR_<INTC_instance>_<AXIETHERNET_instance>_VEC_ID
*		value from xparameters.h.
* @param	FifoIntrId is the interrupt id fifo.
*
* @return	- XST_SUCCESS to indicate success.
*		- XST_FAILURE.to indicate failure.
*
* @note		None.
*
******************************************************************************/
static int AxiEthernetSetupIntrSystem(INTC *IntcInstancePtr,
				XAxiEthernet *AxiEthernetInstancePtr,
				XLlFifo *FifoInstancePtr,
				u16 AxiEthernetIntrId, u16 FifoIntrId)
{
	int Status;
#ifdef XPAR_INTC_0_DEVICE_ID
#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller and connect the ISR
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap
			("Unable to intialize the interrupt controller");
		return XST_FAILURE;
	}
#endif

#if XPAR_INTC_0_HAS_FAST == 1
	AxiEthernetInstancePtr_Fast = AxiEthernetInstancePtr;
	Fifo_Fast = FifoInstancePtr;
	Status = XIntc_ConnectFastHandler(IntcInstancePtr, AxiEthernetIntrId,
						(XFastInterruptHandler) AxiEthernetErrorFastHandler);
	Status |= XIntc_ConnectFastHandler(IntcInstancePtr, FifoIntrId,
								(XFastInterruptHandler) FifoFastHandler);
#else
	Status = XIntc_Connect(IntcInstancePtr, AxiEthernetIntrId,
			       (XInterruptHandler) AxiEthernetErrorHandler,
			       AxiEthernetInstancePtr);
	Status |= XIntc_Connect(IntcInstancePtr, FifoIntrId,
			      (XInterruptHandler) FifoHandler,
							FifoInstancePtr);
#endif
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap
			("Unable to connect ISR to interrupt controller");
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
	XIntc_Enable(IntcInstancePtr, FifoIntrId);
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


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, FifoIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, AxiEthernetIntrId, 0xA0, 0x3);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, FifoIntrId,
				(Xil_InterruptHandler)FifoHandler,
				FifoInstancePtr);
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
	XScuGic_Enable(IntcInstancePtr, FifoIntrId);
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
* @param	FifoIntrId is the interrupt id for fifo.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void AxiEthernetDisableIntrSystem(INTC *IntcInstancePtr,
				   u16 AxiEthernetIntrId, u16 FifoIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Disconnect and disable the interrupt for the AxiEthernet device
	 */
	XIntc_Disconnect(IntcInstancePtr, AxiEthernetIntrId);
	/*
	 * Disconnect and disable the interrupt for the Fifo device
	 */
	XIntc_Disconnect(IntcInstancePtr, FifoIntrId);

#else
	/*
	 * Disconnect and disable the interrupt for the AxiEthernet device
	 */
	XScuGic_Disconnect(IntcInstancePtr, AxiEthernetIntrId);
	/*
	 * Disconnect and disable the interrupt for the Fifo device
	 */
	XScuGic_Disconnect(IntcInstancePtr, FifoIntrId);
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
* Fast FIFO Handler which calls FifoHandler.
*
* @param	None
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FifoFastHandler(void)
{
	FifoHandler((XLlFifo *)Fifo_Fast);
}

#endif
