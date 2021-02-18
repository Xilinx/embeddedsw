/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_example_util.c
*
* This file implements the utility functions for the Xxv Ethernet and USXGMII
* example code. It contains functions to setup USXGMII autonegotiation at
* desired settings and to bypass autonegotiation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   06/16/17 First release
*       hk   02/15/18 Add support for USXGMII
* 1.5   rsp  06/08/20 Fix Xil_poll_timeout compilation error on microblaze.
* 1.6	sk   02/18/21 Use UINTPTR instead of u32 for BaseAddress variable.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xxxvethernet_example.h"
#if !defined(__PPC__)
#include "sleep.h"
#endif

/************************** Variable Definitions ****************************/

/*
 * Local MAC address
 */
char XxvEthernetMAC[6] = { 0x00, 0x0A, 0x35, 0x01, 0x02, 0x03 };
char DestAddr[6] = { 0x00, 0x00, 0x53, 0x0e, 0x9f, 0xb0 };


/******************************************************************************/
/**
*
* Set the MAC addresses in the frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	DestAddr is the Destination MAC address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XxvEthernetUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr, char *SrcAddr)
{
	char *Frame = (char *) FramePtr;
	int Index;

	/*
	 * Destination address
	 */
	for (Index = 0; Index < XXE_MAC_ADDR_SIZE; Index++) {
		*Frame++ = DestAddr[Index];
	}

	/*
	 * Source address
	 */
	for (Index = 0; Index < XXE_MAC_ADDR_SIZE; Index++) {
		*Frame++ = SrcAddr[Index];
	}
}

/******************************************************************************/
/**
*
* Set the frame type for the specified frame.
*
* @param	FramePtr is the pointer to the frame.
* @param	FrameType is the Type to set in frame.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XxvEthernetUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12;

	FrameType = Xil_Htons(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/******************************************************************************/
/**
* This function places a pattern in the payload section of a frame. The pattern
* is a  8 bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, then the pattern changes to a 16 bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param	FramePtr is a pointer to the frame to change.
* @param	PayloadSize is the number of bytes in the payload that will be
*		set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XxvEthernetUtilFrameSetPayloadData(EthernetFrame *FramePtr,
							int PayloadSize)
{
	unsigned BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XXE_HDR_SIZE;

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft)
			break;

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/******************************************************************************/
/**
* This function verifies the frame data against a CheckFrame.
*
* Validation occurs by comparing the ActualFrame to the header of the
* CheckFrame. If the headers match, then the payload of ActualFrame is
* verified for the same pattern Util_FrameSetPayloadData() generates.
*
* @param	CheckFrame is a pointer to a frame containing the 14 byte header
*		that should be present in the ActualFrame parameter.
* @param	ActualFrame is a pointer to a frame to validate.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE in case of failure.
*
* @note		None.
*
******************************************************************************/
int XxvEthernetUtilFrameVerify(EthernetFrame * CheckFrame,
			 EthernetFrame * ActualFrame)
{
	unsigned char *CheckPtr = (unsigned char *) CheckFrame;
	unsigned char *ActualPtr = (unsigned char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;
	int Index;



	CheckPtr = CheckPtr + 8;
	ActualPtr = ActualPtr + 8;

	/*
	 * Compare the headers
	 */
	for (Index = 0; Index < XXE_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}

	Index = 0;

	BytesLeft = *(u16 *) &ActualPtr[6];
	BytesLeft = Xil_Ntohs(BytesLeft);


	/*
	 * Get the length of the payload, do not use VLAN TPID here.
	 * TPID needs to be verified.
	 */
	while ((0x8100 == BytesLeft) || (0x88A8 == BytesLeft) ||
	       (0x9100 == BytesLeft) || (0x9200 == BytesLeft)) {
		Index++;
		BytesLeft = *(u16 *) &ActualPtr[12+(4*Index)];
		BytesLeft = Xil_Ntohs(BytesLeft);
	}

	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[6+(4*Index)];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (u8) Counter++) {

			return XST_FAILURE;
		}
		BytesLeft--;
	}


	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (u8) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft)
			break;

		if (*ActualPtr++ != (u8) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function sets all bytes of a frame to 0.
*
* @param	FramePtr is a pointer to the frame itself.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XxvEthernetUtilFrameMemClear(EthernetFrame * FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/*
	 * Frame should be an integral number of words
	 */
	while (WordsLeft--) {
		*Data32Ptr++ = 0;
	}
}

/******************************************************************************/
/**
*
* This function sets 10G ethernet in local loopback
*
* @param	XxvEthernetInstancePtr is a pointer to the instance of the
*		XxvEthernet component.
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
int XxvEthernetUtilEnterLocalLoopback(XXxvEthernet *XxvEthernetInstancePtr)
{
	UINTPTR BaseAddress = XxvEthernetInstancePtr->Config.BaseAddress;

	XXxvEthernet_WriteReg(BaseAddress, XXE_MODE_OFFSET,
			XXxvEthernet_ReadReg(BaseAddress, XXE_MODE_OFFSET) |
			XXE_MODE_LCLLPBK_MASK);

	/* Need to perform a GT reset after setting local loopback bit */
	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x1);
	usleep(100);
	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x0);

	return 0;
}

/******************************************************************************/
/**
*
* This function sets up USXGMII at specified speed.
* Autonegotiation is enabled with the selected settings.
* If autonegotiation fails, it is restarted after a timeout.
* The function returns success after autonegotiation is completed.
* This function enables RX and performs a GT reset first because
* USXGMII requires the same.
*
* @param	XxvEthernetInstancePtr is a pointer to the instance of the
*		XxvEthernet component.
* @param	Speed - can be 1G or 2.5G - use RATE_1G or RATE_2G5
* @param	Duplex - 1 for full duplex, 0 for half duplex
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
int XxvEthernetUtilUsxgmiiSetup(XXxvEthernet *XxvEthernetInstancePtr, u32 Rate, u32 Duplex)
{
	UINTPTR BaseAddress = XxvEthernetInstancePtr->Config.BaseAddress;
	u32 Status;
	u32 ANSR;
	int to = 0, to_cnt = 0;

	/* Enable receiver because USXGMII setup needs it */
	XXxvEthernet_SetOptions(XxvEthernetInstancePtr, XXE_RECEIVER_ENABLE_OPTION);

	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x1);

	usleep(100);
	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x0);

	Status = XXxvEthernet_SetUsxgmiiRateAndDuplex(XxvEthernetInstancePtr, Rate, Duplex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* AN enable */
	XXxvEthernet_SetUsxgmiiAnEnable(XxvEthernetInstancePtr);
	XXxvEthernet_UsxgmiiAnMainRestart(XxvEthernetInstancePtr);

	do {
		to = Xil_poll_timeout(XXxvEthernet_GetUsxgmiiAnSts, XxvEthernetInstancePtr,
							ANSR, (ANSR & XXE_USXGMII_AN_COMP_MASK) != 0, 10000);
		if(to == -1) {
			XXxvEthernet_UsxgmiiAnMainRestart(XxvEthernetInstancePtr);
			xil_printf("restarting AN \n\r");
			to_cnt++;
		} else {
			break;
		}
	}while(to_cnt < 3);

	if(to_cnt >= 3) {
		return XST_FAILURE;
	}

	xil_printf("USXGMII setup at %dMbps \n\r", Rate);

	return 0;
}

/******************************************************************************/
/**
*
* This function sets up USXGMII at specified speed.
* Autonegotiation is bypassed and the status is checked for completion before
* returning successfully.
* This function enables RX and performs a GT reset first because
* USXGMII requires the same.
*
* @param	XxvEthernetInstancePtr is a pointer to the instance of the
*		XxvEthernet component.
* @param	Speed - can be 1G or 2.5G - use RATE_1G or RATE_2G5
* @param	Duplex - 1 for full duplex, 0 for half duplex
*
* @return	- XST_SUCCESS if successful.
*		- XST_FAILURE, in case of failure..
*
* @note		None.
*
******************************************************************************/
int XxvEthernetUtilUsxgmiiSetupBypassAN(XXxvEthernet *XxvEthernetInstancePtr, u32 Rate, u32 Duplex)
{
	UINTPTR BaseAddress = XxvEthernetInstancePtr->Config.BaseAddress;
	u32 Status;
	u32 ANSR;
	int to = 0, to_cnt = 0;

	/* Enable receiver because USXGMII setup needs it */
	XXxvEthernet_SetOptions(XxvEthernetInstancePtr, XXE_RECEIVER_ENABLE_OPTION);

	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x1);

	usleep(100);
	XXxvEthernet_WriteReg(BaseAddress, XXE_GRR_OFFSET, 0x0);

	Status = XXxvEthernet_SetUsxgmiiRateAndDuplex(XxvEthernetInstancePtr, Rate, Duplex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* AN bypass */
	XXxvEthernet_SetUsxgmiiAnBypass(XxvEthernetInstancePtr);
	XXxvEthernet_UsxgmiiAnMainRestart(XxvEthernetInstancePtr);

	do {
		to = Xil_poll_timeout(XXxvEthernet_GetUsxgmiiAnSts, XxvEthernetInstancePtr,
							ANSR, (ANSR & XXE_USXGMII_AN_COMP_MASK) != 0, 10000);
		if(to == -1) {
			XXxvEthernet_UsxgmiiAnMainRestart(XxvEthernetInstancePtr);
			xil_printf("restarting AN \n\r");
			to_cnt++;
		} else {
			break;
		}
	}while(to_cnt < 3);

	if(to_cnt >= 3) {
		return XST_FAILURE;
	}

	xil_printf("USXGMII setup at %dMbps \n\r", Rate);

	return 0;
}

/******************************************************************************/
/**
* This function is called by example code when an error is detected. It
* can be set as a breakpoint with a debugger or it can be used to print out the
* given message if there is a UART or STDIO device.
*
* @param	Message is the text explaining the error
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XxvEthernetUtilErrorTrap(char *Message)
{
	static int Count = 0;

	Count++;

#ifdef STDOUT_BASEADDRESS
	xil_printf("%s\r\n", Message);
#endif
}
