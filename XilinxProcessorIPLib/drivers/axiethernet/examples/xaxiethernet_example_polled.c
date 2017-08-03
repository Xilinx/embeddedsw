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
* @file xaxiethernet_example_polled.c
*
* Implements examples that utilize the Axi Ethernet's FIFO direct frame transfer
* mode in a polled fashion to send and receive frames.
*
* These examples demonstrate:
*
* - How to perform simple polling send and receive.
* - Advanced frame processing
* - Error handling
*
* Functional guide to example:
*
* - AxiEthernetSingleFramePolledExample() demonstrates the simplest way to send
*   and receive frames in polled mode.
*
* - AxiEthernetMultipleFramesPolledExample() demonstrates how to transmit a
*   "burst" of frames by queueing up several in the packet FIFO prior to
*   transmission.
*
* - AxiEthernetPollForTxStatus() demonstrates how to poll for transmit complete
*   status and how to handle error conditions.
*
* - AxiEthernetPollForRxStatus() demonstrates how to poll for receive status and
*   how to handle error conditions.
*
* - AxiEthernetResetDevice() demonstrates how to reset the driver/HW without
*   losing all configuration settings.
*
* Note that the advanced frame processing algorithms shown here are not limited
* to polled mode operation. The same techniques can be used for FIFO direct
* interrupt driven mode as well.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  4/30/10 First release based on the ll temac driver
* 5.4   ms   01/23/17 Modified xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings are
*                     available in all examples. This is a fix for CR-965028.
*       ms   04/05/17 Added tabspace for return statements in functions
*                     for proper documentation while generating doxygen.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet_example.h"
#include "xllfifo.h"
#include "xil_cache.h"

#ifdef XPAR_XUARTNS550_NUM_INSTANCES
#include "xuartns550_l.h"
#endif

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define AXIETHERNET_DEVICE_ID	XPAR_AXIETHERNET_0_DEVICE_ID
#define FIFO_DEVICE_ID		XPAR_AXI_FIFO_0_DEVICE_ID
#endif

/************************** Variable Definitions ****************************/

volatile int Padding;	/* For 1588 Packets we need to pad 8 bytes time stamp value */
volatile int ExternalLoopback; /* Variable for External loopback */
EthernetFrame TxFrame;		/* Transmit frame buffer */
EthernetFrame RxFrame;		/* Receive frame buffer */

XAxiEthernet AxiEthernetInstance;
XLlFifo FifoInstance;

/************************** Function Prototypes *****************************/

int AxiEthernetPolledExample(u16 AxiEthernetDeviceId, u16 FifoDeviceId);
int AxiEthernetSingleFramePolledExample();
int AxiEthernetMultipleFramesPolledExample();

int AxiEthernetPollForTxStatus();
int AxiEthernetPollForRxStatus();
int AxiEthernetResetDevice();

/*****************************************************************************/
/**
*
* This is the main function for the AxiEthernet example. This function is not
* included if the example is generated from the TestAppGen test  tool.
*
* @param	None.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
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
	 * Call the AxiEthernet polled example , specify the Device ID
	 * generated in xparameters.h
	 */
	Status = AxiEthernetPolledExample(AXIETHERNET_DEVICE_ID, FIFO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Axiethernet poll mode fifo Example Failed\r\n");
		return XST_FAILURE;
	}

	AxiEthernetUtilErrorTrap("Successfully ran Axiethernet poll mode fifo Example\r\n");
	AxiEthernetUtilErrorTrap("--- Exiting main() ---");
	return XST_SUCCESS;

}
#endif

/*****************************************************************************/
/**
*
* This function demonstrates the usage of the Axi Ethernet by sending and
* receiving frames in polled mode.
*
*
* @param	AxiEthernetDeviceId is device ID of the AxiEthernet Device ,
*		typically XPAR_<AXIETHERNET_instance>_DEVICE_ID value from
*		xparameters.h
* @param	FifoDeviceId is device ID of the Fifo device taken from
*		xparameters.h
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		AxiFifo hardware must be initialized before initializing
*		AxiEthernet. Since AxiFifo reset line is connected to the
*		AxiEthernet reset line, a reset of AxiFifo hardware during its
*		initialization would reset AxiEthernet.
*
******************************************************************************/
int AxiEthernetPolledExample(u16 AxiEthernetDeviceId, u16 FifoDeviceId)
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
	XLlFifo_Initialize(&FifoInstance, MacCfgPtr->AxiDevBaseAddress);

	/*
	 * Initialize AxiEthernet hardware.
	 */
	Status = XAxiEthernet_CfgInitialize(&AxiEthernetInstance, MacCfgPtr,
					MacCfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error in initialize");
		return XST_FAILURE;
	}

	/*
	 * Set the MAC  address
	 */
	Status = XAxiEthernet_SetMacAddress(&AxiEthernetInstance,
							(u8 *) AxiEthernetMAC);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting MAC address");
		return XST_FAILURE;
	}


	/*
	 * Set PHY to loopback, speed depends on phy type.
	 * MII is 100 and all others are 1000.
	 */
	if (XAxiEthernet_GetPhysicalInterface(&AxiEthernetInstance) ==
							XAE_PHY_TYPE_MII) {
		LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED;
	} else {
		LoopbackSpeed = AXIETHERNET_LOOPBACK_SPEED_1G;
	}
	Status = AxiEthernetUtilEnterLoopback(&AxiEthernetInstance,
								LoopbackSpeed);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error setting the PHY loopback");
		return XST_FAILURE;
	}

	/*
	 * Set PHY<-->MAC data clock
	 */
	Status =  XAxiEthernet_SetOperatingSpeed(&AxiEthernetInstance,
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

	/*
	 * Run the Single Frame polled example
	 */
	Status = AxiEthernetSingleFramePolledExample();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Run the Multiple Frames polled example
	 */
	Status = AxiEthernetMultipleFramesPolledExample();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;


}


/*****************************************************************************/
/**
*
* This function demonstrates the usage of the Axi Ethernet by sending and
* receiving a single frame in polled mode.
*
* @param	None.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetSingleFramePolledExample(void)
{
	u32 FifoFreeBytes;
	int PayloadSize = 100;
	u32 TxFrameLength;
	u32 RxFrameLength;

	/*
	 * Start the Axi Ethernet device
	 */
	XAxiEthernet_Start(&AxiEthernetInstance);

	/*
	 * Setup the packet to be transmitted
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/*
	 * Clear out the receive packet memory area
	 */
	AxiEthernetUtilFrameMemClear(&RxFrame);

	/*
	 * Calculate frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/*******************/
	/* Send the packet */
	/*******************/

	/*
	 * Wait for enough room in FIFO to become available
	 */
	do {
		FifoFreeBytes = XLlFifo_TxVacancy(&FifoInstance);
	} while (FifoFreeBytes < TxFrameLength);

	/*
	 * Write the frame data to FIFO
	 */
	XLlFifo_Write(&FifoInstance, TxFrame, TxFrameLength);

	/*
	 * Initiate transmit
	 */
	XLlFifo_TxSetLen(&FifoInstance, TxFrameLength);

	/*
	 * Wait for status of the transmitted packet
	 */
	switch (AxiEthernetPollForTxStatus()) {
	case XST_SUCCESS:/* Got a successful transmit status */
		break;

	case XST_NO_DATA:	/* Timed out */
		AxiEthernetUtilErrorTrap("Tx timeout");
		return XST_FAILURE;

	default:		/* Some other error */
		return XST_FAILURE;
	}

	/**********************/
	/* Receive the packet */
	/**********************/

	/*
	 * Wait for packet Rx
	 */
	switch (AxiEthernetPollForRxStatus()) {
	case XST_SUCCESS:/* Got a successful receive status */
		break;

	case XST_NO_DATA:	/* Timed out */
		AxiEthernetUtilErrorTrap("Rx timeout");
		return XST_FAILURE;

	default:		/* Some other error */
		return XST_FAILURE;
	}

	while(XLlFifo_RxOccupancy(&FifoInstance)) {
		/*
		 * A packet as arrived, get its length
		 */
		RxFrameLength = XLlFifo_RxGetLen(&FifoInstance);

		/*
		 * Read the received packet data
		 */
		XLlFifo_Read(&FifoInstance, &RxFrame, RxFrameLength);

		/*
		 * Verify the received frame length
		 */
		if ((RxFrameLength) != TxFrameLength) {
			AxiEthernetUtilErrorTrap("Receive length incorrect");
			return XST_FAILURE;
		}

		/*
		 * Validate frame data
		 */
		if (AxiEthernetUtilFrameVerify(&TxFrame, &RxFrame) != 0) {
			AxiEthernetUtilErrorTrap("Receive Data mismatch");
			return XST_FAILURE;
		}
	}

	/*
	 * Stop device
	 */
	XAxiEthernet_Stop(&AxiEthernetInstance);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This example uses polled mode to queue up multiple frames in the packet
* FIFOs before sending them in a single burst. Receive packets are handled in
* a similar way.
*
* @param	None.
*
* @return	-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None.
*
******************************************************************************/
int AxiEthernetMultipleFramesPolledExample(void)
{
	u32 FramesToLoopback;
	u32 PayloadSize;
	u32 TxFrameLength;
	u32 RxFrameLength;
	u32 FifoFreeBytes;
	u32 Index;

	/*
	 * Start the Axi Ethernet device
	 */
	XAxiEthernet_Start(&AxiEthernetInstance);

	/*
	 * Setup the number of frames to loopback (FramesToLoopback) and the
	 * size of the frame (PayloadSize) to loopback. The default settings
	 * should work for every case. Modifying the settings can cause
	 * problems, see discussion below:
	 *
	 * If PayloadSize is set small and FramesToLoopback high, then it is
	 * possible to cause the transmit status FIFO to overflow.
	 *
	 * If PayloadSize is set large and FramesToLoopback high, then it is
	 * possible to cause the transmit packet FIFO to overflow.
	 *
	 * Either of these scenarios may be worth trying out to observe how
	 * the driver reacts. The exact values to cause these types of errors
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
	 * Calculate Tx frame length (not including FCS)
	 */
	TxFrameLength = XAE_HDR_SIZE + PayloadSize;

	/*
	 * Setup the packet to be transmitted
	 */
	AxiEthernetUtilFrameHdrFormatMAC(&TxFrame, AxiEthernetMAC);
	AxiEthernetUtilFrameHdrFormatType(&TxFrame, PayloadSize);
	AxiEthernetUtilFrameSetPayloadData(&TxFrame, PayloadSize);

	/****************/
	/* Send packets */
	/****************/

	/*
	 * Since we may be interested to see what happens when FIFOs overflow,
	 * don't check for room in the transmit packet FIFO prior to writing
	 * to it.
	 */

	/*
	 * Write frame data to FIFO
	 * Fifo core only allows loading and sending one frame at a time.
	 */
	for (Index = 0; Index < FramesToLoopback; Index++) {
		/* Make sure there is room in the FIFO */
		do {
			FifoFreeBytes = XLlFifo_TxVacancy(&FifoInstance);
		} while (FifoFreeBytes < TxFrameLength);

		XLlFifo_Write(&FifoInstance, TxFrame, TxFrameLength);
		XLlFifo_TxSetLen(&FifoInstance, TxFrameLength);

		switch (AxiEthernetPollForTxStatus()) {
		case XST_SUCCESS:	/* Got a successful transmit status */
			break;

		case XST_NO_DATA:	/* Timed out */
			AxiEthernetUtilErrorTrap("Tx timeout");
			return XST_FAILURE;
			break;

		default:	/* Some other error */
			AxiEthernetResetDevice();
			return XST_FAILURE;
		}
	}

	/**********************/
	/* Receive the packet */
	/**********************/

	/*
	 * Wait for the packets to arrive
	 * The Fifo core only allows us to pull out one frame at a time.
	 */
	for (Index = 0; Index < FramesToLoopback; ) {
		/*
		 * Wait for packet Rx
		 */
		switch (AxiEthernetPollForRxStatus()) {
		case XST_SUCCESS:	/* Got a successful receive status */
			break;

		case XST_NO_DATA:	/* Timed out */
			AxiEthernetUtilErrorTrap("Rx timeout");
			return XST_FAILURE;
			break;

		default:	/* Some other error */
			AxiEthernetResetDevice();
			return XST_FAILURE;
		}

		while(XLlFifo_RxOccupancy(&FifoInstance)) {
			/*
			 * A packet has arrived, get its length
			 */
			RxFrameLength = XLlFifo_RxGetLen(&FifoInstance);

			/*
			 * Verify the received frame length
			 */
			if ((RxFrameLength) != TxFrameLength) {
				AxiEthernetUtilErrorTrap("Receive length incorrect");
				return XST_FAILURE;
			}
			/*
			 * Read the received packet data
			 */
			XLlFifo_Read(&FifoInstance, &RxFrame, RxFrameLength);

			if (AxiEthernetUtilFrameVerify
						(&TxFrame, &RxFrame) != 0) {
				AxiEthernetUtilErrorTrap("Receive Data Mismatch");
				return XST_FAILURE;
			}

			Index++;
		}
	}

	/*
	 * Stop device
	 */
	XAxiEthernet_Stop(&AxiEthernetInstance);

	return XST_SUCCESS;
}


/******************************************************************************/
/**
* This functions polls the Tx status and waits for an indication that a frame
* has been transmitted successfully or a transmit related error has occurred.
* If an error is reported, it handles all the possible  error conditions.
*
* @param	None.
*
* @return
*		- XST_SUCCESS, Tx has completed
*		- XST_NO_DATA, Timeout. Tx failure.
*		- XST_FIFO_ERROR, Error in the FIFO.
*
* @note		None.
*
******************************************************************************/
int AxiEthernetPollForTxStatus(void)
{
	int Status = XST_NO_DATA;
	int Attempts = 100000;	/*
				 * Number of attempts to get status before
				 * giving up
				 */

	/*
	 * Wait for transmit complete indication
	 */
	do {

		if (--Attempts <= 0)
			break;	/* Give up? */

		if (XLlFifo_Status(&FifoInstance) & XLLF_INT_TC_MASK) {
			XLlFifo_IntClear(&FifoInstance, XLLF_INT_TC_MASK);
			Status = XST_SUCCESS;
		}
		if (XLlFifo_Status(&FifoInstance) & XLLF_INT_ERROR_MASK) {
			Status = XST_FIFO_ERROR;
		}

	} while (Status == XST_NO_DATA);


	switch (Status) {
	case XST_SUCCESS:	/* Frame sent without error */
	case XST_NO_DATA:	/* Timeout */
		break;

	case XST_FIFO_ERROR:
		AxiEthernetUtilErrorTrap("FIFO error");
		AxiEthernetResetDevice();
		break;

	default:
		AxiEthernetUtilErrorTrap("Driver returned unknown transmit status");
		break;
	}

	return (Status);
}


/******************************************************************************/
/**
* This functions polls the Rx status and waits for an indication that a frame
* has arrived or a receive related error has occurred. If an error is reported,
* handle all the possible  error conditions.
*
* @param	None.
*
* @return
*		- XST_SUCCESS, a frame has been received
*		- XST_NO_DATA, Timeout. Rx failure.
*		- XST_FIFO_ERROR, Error in the FIFO.
*		- XST_DATA_LOST, a frame has been dropped
*
* @note     None.
*
******************************************************************************/
int AxiEthernetPollForRxStatus(void)
{
	int Status = XST_NO_DATA;
	int Attempts = 1000000;	/* Number of times to get a status before
				 * giving up
				 */

	/*
	 * There are two ways to poll for a received frame:
	 *
	 * XAxiEthernet_Recv() can be used and repeatedly called until it
	 * returns a length,  but this method does not provide any error
	 * detection.
	 *
	 * XAxiEthernet_FifoQueryRecvStatus() can be used and this function
	 * provides more information to handle error conditions.
	 */

	/*
	 * Wait for something to happen
	 */
	do {
		if (--Attempts <= 0)
			break;	/* Give up? */

		if (XLlFifo_Status(&FifoInstance) & XLLF_INT_RC_MASK) {
				Status = XST_SUCCESS;
		}
		if (XLlFifo_Status(&FifoInstance) & XLLF_INT_ERROR_MASK) {
			Status = XST_FIFO_ERROR;
		}
		if (XAxiEthernet_GetIntStatus(&AxiEthernetInstance) &
							XAE_INT_RXRJECT_MASK) {
			Status = XST_DATA_LOST;
		}
		/* When the RXFIFOOVR bit is set, the RXRJECT bit also
		 * gets set
		 */
		if (XAxiEthernet_GetIntStatus(&AxiEthernetInstance) &
						XAE_INT_RXFIFOOVR_MASK) {
			Status = XST_DATA_LOST;
		}
	} while (Status == XST_NO_DATA);

	switch (Status) {
	case XST_SUCCESS:	/* Frame has arrived */
	case XST_NO_DATA:	/* Timeout */
		break;

	case XST_DATA_LOST:
		AxiEthernetUtilErrorTrap("Frame was dropped");
		break;

	case XST_FIFO_ERROR:
		AxiEthernetUtilErrorTrap("FIFO error");
		AxiEthernetResetDevice();
		break;

	default:
		AxiEthernetUtilErrorTrap("Driver returned invalid transmit status");
		break;
	}

	return (Status);
}


/******************************************************************************/
/**
* This function resets the device but preserves the options set by the user.
*
* @param	None.
*
* @return	-XST_SUCCESS if reset is successful
*		-XST_FAILURE. if reset is not successful
*
* @note     None.
*
******************************************************************************/
int AxiEthernetResetDevice(void)
{
	int Status;
	u8 MacSave[6];
	u32 Options;

	/*
	 * Stop device
	 */
	XAxiEthernet_Stop(&AxiEthernetInstance);

	/*
	 * Save the device state
	 */
	XAxiEthernet_GetMacAddress(&AxiEthernetInstance, MacSave);
	Options = XAxiEthernet_GetOptions(&AxiEthernetInstance);

	/*
	 * Stop and reset both the fifo and the AxiEthernet the devices
	 */
	XLlFifo_Reset(&FifoInstance);
	XAxiEthernet_Reset(&AxiEthernetInstance);

	/*
	 * Restore the state
	 */
	Status = XAxiEthernet_SetMacAddress(&AxiEthernetInstance, MacSave);
	Status |= XAxiEthernet_SetOptions(&AxiEthernetInstance, Options);
	Status |= XAxiEthernet_ClearOptions(&AxiEthernetInstance, ~Options);
	if (Status != XST_SUCCESS) {
		AxiEthernetUtilErrorTrap("Error restoring state after reset");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
