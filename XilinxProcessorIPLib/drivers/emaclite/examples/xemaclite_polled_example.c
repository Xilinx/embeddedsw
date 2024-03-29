/******************************************************************************
* Copyright (C) 2004 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xemaclite_polled_example.c
*
* This file contains an example for using the EmacLite hardware and driver.
* This file contains an polled mode example outlining the transmission/reception
* of an Ethernet frame of 1000 bytes of payload.
*
* If the MDIO interface is NOT configured in the EmacLite core then this example
* will only transmit a frame.
* If the MDIO interface is configured in the EmacLite core then this example
* will enable the MAC loopback in the PHY device, then transmit the frame and
* compare the received frame.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a ecm  21/05/04 First release
* 1.01a sv   06/06/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  02/25/09 Updated to use PHY loop back if MDIO is configured in
*		      core and updated to be used in Test App
* 3.00a ktn  10/22/09 Updated example to use the macros that have been changed
*		      in the driver to remove _m from the name of the macro.
* 3.01a ktn  07/08/10 Updated example to support Little Endian MicroBlaze.
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.4  rsp   11/04/18 Fix poll example failure when I/D cache is disabled.
*                     It's a corner case when on network there is a packet
*                     reception before send i.e during phy loopback setup.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xemaclite_example.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The Size of the Test Frame.
 */
#define EMACLITE_TEST_FRAME_SIZE	1000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef SDT
int EmacLitePolledExample(UINTPTR BaseAddress);
#else
int EmacLitePolledExample(u16 DeviceId);
#endif

static int EmacLiteSendFrame(XEmacLite *InstancePtr, u32 PayloadSize);

static int EmacLiteRecvFrame(u32 PayloadSize);

/************************** Variable Definitions *****************************/

/*
 * Set up valid local and remote MAC addresses. This loop back test uses the
 * LocalAddress both as a source and destination MAC address.
 */
static u8 LocalAddress[XEL_MAC_ADDR_SIZE] = {
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x03
};
static u8 RemoteAddress[XEL_MAC_ADDR_SIZE] = {
	0x00, 0x10, 0xa4, 0xb6, 0xfd, 0x09
};

/****************************************************************************/
/**
*
* This function is the main function of the EmacLite polled example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE .
*
* @note		None.
*
*****************************************************************************/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	/*
	 * Run the EmacLite Polled example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
#ifdef SDT
	Status = EmacLitePolledExample(EMACLITE_BASEADDR);
#else
	Status = EmacLitePolledExample(EMAC_DEVICE_ID);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Emaclite polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Emaclite polled Example\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* The main entry point for the EmacLite driver example in polled mode.
*
* This function will transmit/receive the Ethernet frames and verify the
* data in the received frame (if the MDIO interface is configured in the
* EmacLite core).
* This function simply transmits a frame if the MDIO interface is not
* configured in the EmacLite core.
*
* @param	DeviceId is device ID of the XEmacLite Device , typically
*		XPAR_<EMAC_instance>_DEVICE_ID value from xparameters.h.
*
* @return	XST_SUCCESS to indicate success, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
#ifdef SDT
int EmacLitePolledExample(UINTPTR BaseAddress)
#else
int EmacLitePolledExample(u16 DeviceId)
#endif
{
	int Status;
	XEmacLite *EmacLiteInstPtr = &EmacLiteInstance;
	u32 PhyAddress = 0;
	RecvFrameLength = 0;
	XEmacLite_Config *ConfigPtr;

	/*
	 * Initialize the EmacLite device.
	 */
#ifdef SDT
	ConfigPtr = XEmacLite_LookupConfig(BaseAddress);
#else
	ConfigPtr = XEmacLite_LookupConfig(DeviceId);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XEmacLite_CfgInitialize(EmacLiteInstPtr,
					 ConfigPtr,
					 ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the MAC address.
	 */
	XEmacLite_SetMacAddress(EmacLiteInstPtr, LocalAddress);

	/*
	 * Check if there is a TX buffer available, if there isn't it is an
	 * error.
	 */
	if (XEmacLite_TxBufferAvailable(EmacLiteInstPtr) != TRUE) {
		return XST_FAILURE;
	}

	/*
	 * If the MDIO is configured in the device.
	 */
	if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		/*
		 * Detect the PHY device and enable the MAC Loop back
		 * in the PHY.
		 */
		PhyAddress = EmacLitePhyDetect(EmacLiteInstPtr);
		Status = EmacLiteEnablePhyLoopBack(EmacLiteInstPtr,
						   PhyAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	/*
	 * Empty any existing receive frames.
	 */
	XEmacLite_FlushReceive(EmacLiteInstPtr);

	/*
	 * Reset the receive frame length to zero.
	 */
	RecvFrameLength = 0;
	Status = EmacLiteSendFrame(EmacLiteInstPtr, EMACLITE_TEST_FRAME_SIZE);
	if (Status != XST_SUCCESS) {
		if (XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
			/*
			 * Disable the MAC Loop back in the PHY.
			 */
			EmacLiteDisablePhyLoopBack(EmacLiteInstPtr,
						   PhyAddress);
			return XST_FAILURE;
		}
	}

	/*
	 * If the MDIO is not configured in the core then return XST_SUCCESS
	 * as the frame has been transmitted.
	 */
	if (!XEmacLite_IsMdioConfigured(EmacLiteInstPtr)) {
		return XST_SUCCESS;
	}


	/*
	 * Poll for receive packet.
	 */
	while ((volatile u32)RecvFrameLength == 0)  {
		RecvFrameLength = XEmacLite_Recv(EmacLiteInstPtr,
						 (u8 *)RxFrame);
	}

	/*
	 * Check the received frame.
	 */
	Status = EmacLiteRecvFrame(EMACLITE_TEST_FRAME_SIZE);
	if ((Status != XST_SUCCESS) && (Status != XST_NO_DATA)) {
		/*
		 * Disable the MAC Loop back in the PHY.
		 */
		EmacLiteDisablePhyLoopBack(EmacLiteInstPtr, PhyAddress);
		return XST_FAILURE;
	}


	/*
	 * Disable the MAC Loop back in the PHY.
	 */
	EmacLiteDisablePhyLoopBack(EmacLiteInstPtr, PhyAddress);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function sends a frame of given size.
*
* @param	XEmacInstancePtr is a pointer to the XEmacLite instance.
* @param	PayloadSize is the size of the frame to create. The size only
*		reflects the payload size, it does not include the Ethernet
*		header size (14 bytes) nor the Ethernet CRC size (4 bytes).
*
* @return	XST_SUCCESS if successful, else a driver-specific return code.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteSendFrame(XEmacLite *InstancePtr, u32 PayloadSize)
{
	u8 *FramePtr;
	int Index;
	FramePtr = (u8 *)TxFrame;

	/*
	 * Set up the destination address as the local address for
	 * Phy Loopback.
	 */
	if (XEmacLite_IsMdioConfigured(InstancePtr)) {

		*FramePtr++ = LocalAddress[0];
		*FramePtr++ = LocalAddress[1];
		*FramePtr++ = LocalAddress[2];
		*FramePtr++ = LocalAddress[3];
		*FramePtr++ = LocalAddress[4];
		*FramePtr++ = LocalAddress[5];
	} else {
		/*
		 * Fill in the valid Destination MAC address if
		 * the Loopback is not enabled.
		 */
		*FramePtr++ = RemoteAddress[0];
		*FramePtr++ = RemoteAddress[1];
		*FramePtr++ = RemoteAddress[2];
		*FramePtr++ = RemoteAddress[3];
		*FramePtr++ = RemoteAddress[4];
		*FramePtr++ = RemoteAddress[5];
	}

	/*
	 * Fill in the source MAC address.
	 */
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Set up the type/length field - be sure its in network order.
	 */
	*((u16 *)FramePtr) = Xil_Htons(PayloadSize);
	FramePtr++;
	FramePtr++;

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	for (Index = 0; Index < PayloadSize; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/*
	 * Now send the frame.
	 */
	return XEmacLite_Send(InstancePtr, (u8 *)TxFrame,
			      PayloadSize + XEL_HEADER_SIZE);

}

/******************************************************************************/
/**
*
* This function receives a frame of given size. This function assumes interrupt
* mode, receives the frame and verifies its contents.
*
* @param	PayloadSize is the size of the frame to receive.
*		The size only reflects the payload size, it does not include the
*		Ethernet header size (14 bytes) nor the Ethernet CRC size (4
*		bytes).
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note		None.
*
******************************************************************************/
static int EmacLiteRecvFrame(u32 PayloadSize)
{
	u8 *FramePtr;

	/*
	 * This assumes MAC does not strip padding or CRC.
	 */
	if (RecvFrameLength != 0) {
		int Index;

		/*
		 * Verify length, which should be the payload size.
		 */
		if ((RecvFrameLength - (XEL_HEADER_SIZE + XEL_FCS_SIZE)) !=
		    PayloadSize) {
			return XST_LOOPBACK_ERROR;
		}

		/*
		 * Verify the contents of the Received Frame.
		 */
		FramePtr = (u8 *)RxFrame;
		FramePtr += XEL_HEADER_SIZE;	/* Get past the header */

		for (Index = 0; Index < PayloadSize; Index++) {
			if (*FramePtr++ != (u8)Index) {
				return XST_LOOPBACK_ERROR;
			}
		}
	}

	return XST_SUCCESS;
}
