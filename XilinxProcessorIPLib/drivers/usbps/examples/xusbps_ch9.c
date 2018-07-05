/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
 * @file xusbps_ch9.c
 *
 * This file contains the implementation of the chapter 9 code for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.00a jz  10/10/10 First release
 * 1.04a nm  02/05/13 Fixed CR# 696550.
 *		      Added template code for Vendor request.
 * 1.04a nm  03/04/13 Fixed CR# 704022. Implemented TEST_MODE Feature.
 * 1.06a kpc 11/11/13 Always use global memory for dma operations
 * 2.1   kpc 4/29/14  Align dma buffers to cache line boundary
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/


#include "xparameters.h"	/* XPAR parameters */
#include "xusbps.h"		/* USB controller driver */
#include "xusbps_hw.h"		/* USB controller driver */

#include "xusbps_ch9.h"
#include "xil_printf.h"
#include "xil_cache.h"

/*default class is storage class */
#include "xusbps_class_storage.h"
#include "sleep.h"

/* #define CH9_DEBUG */

#ifdef CH9_DEBUG
#include <stdio.h>
#define printf xil_printf
#endif

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XUsbPs_StdDevReq(XUsbPs *InstancePtr,
			      XUsbPs_SetupData *SetupData);

static int XUsbPs_HandleVendorReq(XUsbPs *InstancePtr,
					XUsbPs_SetupData *SetupData);

/************************** Variable Definitions *****************************/

#ifdef __ICCARM__
#pragma data_alignment = 32
static u8 Response;
#pragma data_alignment = 4
#else
static u8 Response ALIGNMENT_CACHELINE;
#endif

/*****************************************************************************/
/**
* This function handles a Setup Data packet from the host.
*
* @param	InstancePtr is a pointer to XUsbPs instance of the controller.
* @param	SetupData is the structure containing the setup request.
*
* @return
*		- XST_SUCCESS if the function is successful.
*		- XST_FAILURE if an Error occured.
*
* @note		None.
*
******************************************************************************/
int XUsbPs_Ch9HandleSetupPacket(XUsbPs *InstancePtr,
				 XUsbPs_SetupData *SetupData)
{
	int Status = XST_SUCCESS;

#ifdef CH9_DEBUG
	printf("Handle setup packet\n");
#endif

	switch (SetupData->bmRequestType & XUSBPS_REQ_TYPE_MASK) {
	case XUSBPS_CMD_STDREQ:
		XUsbPs_StdDevReq(InstancePtr, SetupData);
		break;

	case XUSBPS_CMD_CLASSREQ:
		XUsbPs_ClassReq(InstancePtr, SetupData);
		break;

	case XUSBPS_CMD_VENDREQ:

#ifdef CH9_DEBUG
		printf("vendor request %x\n", SetupData->bRequest);
#endif
		Status = XUsbPs_HandleVendorReq(InstancePtr, SetupData);
		break;

	default:
		/* Stall on Endpoint 0 */
#ifdef CH9_DEBUG
		printf("unknown class req, stall 0 in out\n");
#endif
		XUsbPs_EpStall(InstancePtr, 0, XUSBPS_EP_DIRECTION_IN |
						XUSBPS_EP_DIRECTION_OUT);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function handles a standard device request.
*
* @param	InstancePtr is a pointer to XUsbPs instance of the controller.
* @param	SetupData is a pointer to the data structure containing the
*		setup request.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XUsbPs_StdDevReq(XUsbPs *InstancePtr,
			      XUsbPs_SetupData *SetupData)
{
	int Status;
	int Error = 0;

	XUsbPs_Local	*UsbLocalPtr;

	int ReplyLen;
#ifdef __ICCARM__
#pragma data_alignment = 32
static u8  	Reply[XUSBPS_REQ_REPLY_LEN];
#pragma data_alignment = 4
#else
	static u8  	Reply[XUSBPS_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
#endif

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > XUSBPS_REQ_REPLY_LEN) {
		return;
	}

	UsbLocalPtr = (XUsbPs_Local *) InstancePtr->UserDataPtr;

#ifdef CH9_DEBUG
	printf("std dev req %d\n", SetupData->bRequest);
#endif

	switch (SetupData->bRequest) {

	case XUSBPS_REQ_GET_STATUS:

		switch(SetupData->bmRequestType & XUSBPS_STATUS_MASK) {
		case XUSBPS_STATUS_DEVICE:
			/* It seems we do not have to worry about zeroing out the rest
			 * of the reply buffer even though we are only using the first
			 * two bytes.
			 */
			*((u16 *) &Reply[0]) = 0x0100; /* Self powered */
			break;

		case XUSBPS_STATUS_INTERFACE:
			*((u16 *) &Reply[0]) = 0x0;
			break;

		case XUSBPS_STATUS_ENDPOINT:
			{
			u32 Status;
			int EpNum = SetupData->wIndex;

			Status = XUsbPs_ReadReg(InstancePtr->Config.BaseAddress,
					XUSBPS_EPCRn_OFFSET(EpNum & 0xF));

			if(EpNum & 0x80) { /* In EP */
				if(Status & XUSBPS_EPCR_TXS_MASK) {
					*((u16 *) &Reply[0]) = 0x0100;
				}else {
					*((u16 *) &Reply[0]) = 0x0000;
				}
			} else {	/* Out EP */
				if(Status & XUSBPS_EPCR_RXS_MASK) {
					*((u16 *) &Reply[0]) = 0x0100;
				}else {
					*((u16 *) &Reply[0]) = 0x0000;
				}
			}
			break;
			}

		default:
			;
#ifdef CH9_DEBUG
			printf("unknown request for status %x\n", SetupData->bmRequestType);
#endif
		}
		XUsbPs_EpBufferSend(InstancePtr, 0, Reply, SetupData->wLength);
		break;

	case XUSBPS_REQ_SET_ADDRESS:

		/* With bit 24 set the address value is held in a shadow
		 * register until the status phase is acked. At which point it
		 * address value is written into the address register.
		 */
		XUsbPs_SetDeviceAddress(InstancePtr, SetupData->wValue);
#ifdef CH9_DEBUG
		printf("Set address %d\n", SetupData->wValue);
#endif
		/* There is no data phase so ack the transaction by sending a
		 * zero length packet.
		 */
		XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
		break;

	case XUSBPS_REQ_GET_INTERFACE:
#ifdef CH9_DEBUG
		printf("Get interface %d/%d/%d\n",
			SetupData->wIndex, SetupData->wLength,
			InstancePtr->CurrentAltSetting);
#endif
		Response = (u8)InstancePtr->CurrentAltSetting;

		/* Ack the host */
		XUsbPs_EpBufferSend(InstancePtr, 0, &Response, 1);

		break;

	case XUSBPS_REQ_GET_DESCRIPTOR:
#ifdef CH9_DEBUG
		printf("Get desc %x/%d\n", (SetupData->wValue >> 8) & 0xff,
				SetupData->wLength);
#endif

		/* Get descriptor type. */
		switch ((SetupData->wValue >> 8) & 0xff) {

		case XUSBPS_TYPE_DEVICE_DESC:
		case XUSBPS_TYPE_DEVICE_QUALIFIER:

			/* Set up the reply buffer with the device descriptor
			 * data.
			 */
			ReplyLen = XUsbPs_Ch9SetupDevDescReply(
						Reply, XUSBPS_REQ_REPLY_LEN);

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			if(((SetupData->wValue >> 8) & 0xff) ==
					XUSBPS_TYPE_DEVICE_QUALIFIER) {
				Reply[0] = (u8)ReplyLen;
				Reply[1] = (u8)0x6;
				Reply[2] = (u8)0x0;
				Reply[3] = (u8)0x2;
				Reply[4] = (u8)0xFF;
				Reply[5] = (u8)0x00;
				Reply[6] = (u8)0x0;
				Reply[7] = (u8)0x10;
				Reply[8] = (u8)0;
				Reply[9] = (u8)0x0;
			}
			Status = XUsbPs_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		case XUSBPS_TYPE_CONFIG_DESC:

			/* Set up the reply buffer with the configuration
			 * descriptor data.
			 */
			ReplyLen = XUsbPs_Ch9SetupCfgDescReply(
						Reply, XUSBPS_REQ_REPLY_LEN);

#ifdef CH9_DEBUG
			printf("get config %d/%d\n", ReplyLen, SetupData->wLength);
#endif

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPs_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;


		case XUSBPS_TYPE_STRING_DESC:

			/* Set up the reply buffer with the string descriptor
			 * data.
			 */
			ReplyLen = XUsbPs_Ch9SetupStrDescReply(
						Reply, XUSBPS_REQ_REPLY_LEN,
						SetupData->wValue & 0xFF);

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPs_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

#ifdef MOUSE_SIMULATION
		case XUSBPS_TYPE_HID_DESC:

			/* Set up the reply buffer with the HID descriptor
			 * data.
			 */
			ReplyLen = XUsbPs_Ch9SetupHidDescReply(
						Reply, XUSBPS_REQ_REPLY_LEN);

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPs_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		case XUSBPS_TYPE_REPORT_DESC:

			/* Set up the reply buffer with the report descriptor
			 * data.
			 */
			ReplyLen = XUsbPs_Ch9SetupReportDescReply(
						Reply, XUSBPS_REQ_REPLY_LEN);
#ifdef CH9_DEBUG
			printf("report desc len %d\n", ReplyLen);
#endif

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPs_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;
#endif /* MOUSE_SIMULATION */

		default:
			Error = 1;
			break;
		}
		break;


	case XUSBPS_REQ_SET_CONFIGURATION:

		/*
		 * Only allow configuration index 1 as this is the only one we
		 * have.
		 */
		if ((SetupData->wValue & 0xff) != 1) {
			Error = 1;
			break;
		}

		UsbLocalPtr->CurrentConfig = SetupData->wValue & 0xff;


		/* Call the application specific configuration function to
		 * apply the configuration with the given configuration index.
		 */
		XUsbPs_SetConfiguration(InstancePtr,
						UsbLocalPtr->CurrentConfig);

		/* There is no data phase so ack the transaction by sending a
		 * zero length packet.
		 */
		XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
		break;


	case XUSBPS_REQ_GET_CONFIGURATION:
		Response = (u8)InstancePtr->CurrentAltSetting;
		XUsbPs_EpBufferSend(InstancePtr, 0,
					&Response, 1);
		break;


	case XUSBPS_REQ_CLEAR_FEATURE:
		switch(SetupData->bmRequestType & XUSBPS_STATUS_MASK) {
		case XUSBPS_STATUS_ENDPOINT:
			if(SetupData->wValue == XUSBPS_ENDPOINT_HALT) {
				int EpNum = SetupData->wIndex;

				if(EpNum & 0x80) {	/* In ep */
					XUsbPs_ClrBits(InstancePtr,
						XUSBPS_EPCRn_OFFSET(EpNum & 0xF),
						XUSBPS_EPCR_TXS_MASK);
				}else { /* Out ep */
					XUsbPs_ClrBits(InstancePtr,
						XUSBPS_EPCRn_OFFSET(EpNum),
						XUSBPS_EPCR_RXS_MASK);
				}
			}
			/* Ack the host ? */
			XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
			break;

		default:
			Error = 1;
			break;
		}

		break;

	case XUSBPS_REQ_SET_FEATURE:
		switch(SetupData->bmRequestType & XUSBPS_STATUS_MASK) {
		case XUSBPS_STATUS_ENDPOINT:
			if(SetupData->wValue == XUSBPS_ENDPOINT_HALT) {
				int EpNum = SetupData->wIndex;

				if(EpNum & 0x80) {	/* In ep */
					XUsbPs_SetBits(InstancePtr,
						XUSBPS_EPCRn_OFFSET(EpNum & 0xF),
						XUSBPS_EPCR_TXS_MASK);

				}else { /* Out ep */
					XUsbPs_SetBits(InstancePtr,
						XUSBPS_EPCRn_OFFSET(EpNum),
						XUSBPS_EPCR_RXS_MASK);
				}
			}
			/* Ack the host ? */
			XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);

			break;
		case XUSBPS_STATUS_DEVICE:
			if (SetupData->wValue == XUSBPS_TEST_MODE) {
				int TestSel = (SetupData->wIndex >> 8) & 0xFF;

				/* Ack the host, the transition must happen
					after status stage and < 3ms */
				XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
				usleep(1000);

				switch (TestSel) {
				case XUSBPS_TEST_J:
				case XUSBPS_TEST_K:
				case XUSBPS_TEST_SE0_NAK:
				case XUSBPS_TEST_PACKET:
				case XUSBPS_TEST_FORCE_ENABLE:
					XUsbPs_SetBits(InstancePtr, \
						XUSBPS_PORTSCR1_OFFSET, \
				 		TestSel << 16);
					break;
				default:
					/* Unsupported test selector */
					break;
				}
				break;
			}

		default:
			Error = 1;
			break;
		}

		break;


	/* For set interface, check the alt setting host wants */
	case XUSBPS_REQ_SET_INTERFACE:

#ifdef CH9_DEBUG
		printf("set interface %d/%d\n", SetupData->wValue, SetupData->wIndex);
#endif
		/* Not supported */
		/* XUsbPs_SetInterface(InstancePtr, SetupData->wValue, SetupData->wIndex); */

		/* Ack the host after device finishes the operation */
		Error = XUsbPs_EpBufferSend(InstancePtr, 0, NULL, 0);
		if(Error) {
#ifdef CH9_DEBUG
			printf("EpBufferSend failed %d\n", Error);
#endif
		}
        break;

	default:
		Error = 1;
		break;
	}

	/* Set the send stall bit if there was an error */
	if (Error) {
#ifdef CH9_DEBUG
		printf("std dev req %d/%d error, stall 0 in out\n",
			SetupData->bRequest, (SetupData->wValue >> 8) & 0xff);
#endif
		XUsbPs_EpStall(InstancePtr, 0, XUSBPS_EP_DIRECTION_IN |
						XUSBPS_EP_DIRECTION_OUT);
	}
}

/*****************************************************************************/
/**
* This function handles a vendor request.
*
* @param	InstancePtr is a pointer to XUsbPs instance of the controller.
* @param	SetupData is a pointer to the data structure containing the
*		setup request.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if an Error occured.
*
* @note	
*		This function is a template to handle vendor request for control
*		IN and control OUT endpoints. The control OUT endpoint can
*		receive only 64 bytes of data per dTD. For receiving more than
*		64 bytes of vendor data on control OUT endpoint, change the
*		buffer size of the control OUT endpoint. Otherwise the results
*		are unexpected.
*
******************************************************************************/
static int XUsbPs_HandleVendorReq(XUsbPs *InstancePtr,
					XUsbPs_SetupData *SetupData)
{
	u8      *BufferPtr;
	u32     BufferLen;
	u32     Handle;
	u32	Reg;
#ifdef __ICCARM__
#pragma data_alignment = 32
const static u8	Reply[8] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
#pragma data_alignment = 4
#else
	const static u8	Reply[8] ALIGNMENT_CACHELINE =
							{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
#endif
	u8	EpNum = 0;
	int 	Status;
	int 	Direction;
	int 	Timeout;

	/* Check the direction, USB 2.0 section 9.3 */
	Direction = SetupData->bmRequestType & (1 << 7);

	if (!Direction) {
		/* Control OUT vendor request */
		if (SetupData->wLength > 0) {
			/* Re-Prime the endpoint to receive Setup DATA */
			XUsbPs_EpPrime(InstancePtr, 0, XUSBPS_EP_DIRECTION_OUT);

			/* Check whether EP prime is successful or not */
			Timeout = XUSBPS_TIMEOUT_COUNTER;
			do {
			Reg = XUsbPs_ReadReg(InstancePtr->Config.BaseAddress,
							XUSBPS_EPPRIME_OFFSET);
			} while(((Reg & (1 << EpNum)) == 1) && --Timeout);

			if (!Timeout) {
				return XST_FAILURE;
			}

			/* Get the Setup DATA, don't wait for the interrupt */
			Timeout = XUSBPS_TIMEOUT_COUNTER;
			do {
				Status = XUsbPs_EpBufferReceive(InstancePtr,
					EpNum, &BufferPtr, &BufferLen, &Handle);
			} while((Status != XST_SUCCESS) && --Timeout);
		
			if (!Timeout) {
				return XST_FAILURE;
			}

			Xil_DCacheInvalidateRange((unsigned int)BufferPtr,
								BufferLen);
#ifdef CH9_DEBUG
			int 	Len;
			xil_printf("Vendor data:\r\n");
			for(Len = 0;Len < BufferLen;Len++)
				xil_printf("%02x ",BufferPtr[Len]);
#endif
	
			if (Status == XST_SUCCESS) {
				/* Zero length ACK */
				Status = XUsbPs_EpBufferSend(InstancePtr, EpNum,
								NULL, 0);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
			}
		}
	} else {
		if (SetupData->wLength > 0) {
			/* Control IN vendor request */
			Status = XUsbPs_EpBufferSend(InstancePtr, EpNum, Reply,
							SetupData->wLength);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}
	return XST_SUCCESS;
}
