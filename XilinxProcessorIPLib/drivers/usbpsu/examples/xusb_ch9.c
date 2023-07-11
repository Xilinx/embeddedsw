/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_ch9.c
 *
 * This file contains the implementation of chapter 9 specific code for
 * the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   sg  06/06/16  First release
 * 1.1	vak  30/11/16  Added DFU support
 * 1.4   BK  12/01/18  Renamed the file and added changes to have a common
 *		       example for all USB IPs.
 * 1.5	vak  13/02/19  Added support for versal
 * 1.5  vak  03/25/19  Fixed incorrect data_alignment pragma directive for IAR
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xusb_ch9.h"
#include "xil_cache.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void Usb_StdDevReq(struct Usb_DevData *InstancePtr,
			  SetupPacket *SetupData);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* This function handles a Setup Data packet from the host.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller.
* @param	SetupData is the structure containing the setup request.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Ch9Handler(struct Usb_DevData *InstancePtr,
		SetupPacket *SetupData)
{
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);

#ifdef CH9_DEBUG
	printf("Handle setup packet\n");
#endif

	switch (SetupData->bRequestType & USB_REQ_TYPE_MASK) {
		case USB_CMD_STDREQ:
			Usb_StdDevReq(InstancePtr, SetupData);
			break;

		case USB_CMD_CLASSREQ:
			ch9_ptr->ch9_func.Usb_ClassReq(InstancePtr, SetupData);
			break;

		case USB_CMD_VENDREQ:

#ifdef CH9_DEBUG
			printf("vendor request %x\n", SetupData->bRequest);
#endif
			break;

		default:
			/* Stall on Endpoint 0 */
#ifdef CH9_DEBUG
			printf("unknown class req, stalling at %s\n", __func__);
#endif
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
	}

}

/*****************************************************************************/
/**
* This function handles a standard device request.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller.
* @param	SetupData is a pointer to the data structure containing the
*			setup request.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Usb_StdDevReq(struct Usb_DevData *InstancePtr,
			  SetupPacket *SetupData)
{
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
	static u8 Reply[USB_REQ_REPLY_LEN];
#pragma data_alignment = 64
	static u8 TmpBuffer[10];
#else
#pragma data_alignment = 32
	static u8 Reply[USB_REQ_REPLY_LEN];
#pragma data_alignment = 32
	static u8 TmpBuffer[10];
#endif
#else
	static u8 Reply[USB_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
	static u8 TmpBuffer[10] ALIGNMENT_CACHELINE;
#endif

	s32 Status;
	u8 Error = 0;
	u32 ReplyLen;
	USBCH9_DATA *usb_data =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	u8 EpNum = SetupData->wIndex & USB_ENDPOINT_NUMBER_MASK;
	/*
	 * Direction -- USB_EP_DIR_IN or USB_EP_DIR_OUT
	 */
	u8 Direction = !!(SetupData->wIndex & USB_ENDPOINT_DIR_MASK);

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > USB_REQ_REPLY_LEN) {
		return;
	}

#ifdef CH9_DEBUG
	printf("bmRequestType 0x%x\r\n", SetupData->bRequestType);
	printf("bRequest 0x%x\r\n", SetupData->bRequest);
	printf("wValue 0x%x\r\n", SetupData->wValue);
	printf("wIndex 0x%x\r\n", SetupData->wIndex);
	printf("wLength 0x%x\r\n", SetupData->wLength);
#endif

	switch (SetupData->bRequest) {

		case USB_REQ_GET_STATUS:

			switch (SetupData->bRequestType & USB_STATUS_MASK) {
				case USB_STATUS_DEVICE:
#ifdef CH9_DEBUG
					printf("GET STATUS DEVICE\r\n");
#endif

					/* It seems we do not have to worry about zeroing out
					 * the rest of the reply buffer even though we are only
					 * using the first two bytes.
					 */
					*((u16 *) &Reply[0]) = 0x0100; /* Self powered */
					break;

				case USB_STATUS_INTERFACE:
#ifdef CH9_DEBUG
					printf("GET STATUS INTERFACE\r\n");
#endif
					*((u16 *) &Reply[0]) = 0x0;
					break;

				case USB_STATUS_ENDPOINT:
#ifdef CH9_DEBUG
					printf("GET STATUS ENDPOINT\r\n");
#endif
					*((u16 *) &Reply[0]) = IsEpStalled(InstancePtr->PrivateData,
									   EpNum, Direction);
					break;

				default:
#ifdef CH9_DEBUG
					printf("unknown request for status %x\r\n",
					       SetupData->bRequestType);
#endif
					break;
			}

			EpBufferSend(InstancePtr->PrivateData, 0, Reply, SetupData->wLength);
			break;

		case USB_REQ_SET_ADDRESS:

			/* With bit 24 set the address value is held in a shadow
			 * register until the status phase is acked. At which point it
			 * address value is written into the address register.
			 */
			SetDeviceAddress(InstancePtr->PrivateData, SetupData->wValue);
#ifdef CH9_DEBUG
			printf("SET ADDRESS: %d\r\n", SetupData->wValue);
#endif

			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case USB_REQ_GET_INTERFACE:
#ifdef CH9_DEBUG
			printf("GET INTERFACE %d/%d/%d\r\n",
			       SetupData->wIndex, SetupData->wLength, 0);
#endif
			break;

		case USB_REQ_GET_DESCRIPTOR:
			/* Get descriptor type. */
			switch ((SetupData->wValue >> 8) & 0xff) {

				case USB_TYPE_DEVICE_DESC:
				case USB_TYPE_DEVICE_QUALIFIER:
					/*
					 * Set up the reply buffer with the device descriptor
					 * data.
					 */
					ReplyLen = usb_data->ch9_func.
						   Usb_Ch9SetupDevDescReply(
							   InstancePtr, Reply,
							   USB_REQ_REPLY_LEN);

					ReplyLen = ReplyLen > SetupData->wLength ?
						   SetupData->wLength : ReplyLen;
#ifdef CH9_DEBUG
					printf("GET DEV DESC %d/%d\r\n", ReplyLen,
					       SetupData->wLength);
#endif
					if (((SetupData->wValue >> 8) & 0xff) ==
					    USB_TYPE_DEVICE_QUALIFIER) {
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
					Status = EpBufferSend(InstancePtr->PrivateData, 0,
							      Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
						/* Failure case needs to be handled */
						for (;;);
					}
					break;

				case USB_TYPE_CONFIG_DESC:

					/* Set up the reply buffer with the configuration
					 * descriptor data.
					 */
					ReplyLen = usb_data->ch9_func.
						   Usb_Ch9SetupCfgDescReply(
							   InstancePtr, Reply,
							   USB_REQ_REPLY_LEN);

#ifdef CH9_DEBUG
					printf("GET CONFIG DESC %d/%d\r\n", ReplyLen,
					       SetupData->wLength);
#endif
					ReplyLen = ReplyLen > SetupData->wLength ?
						   SetupData->wLength : ReplyLen;

					Status = EpBufferSend(InstancePtr->PrivateData, 0,
							      Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
						/* Failure case needs to be handled */
						for (;;);
					}
					break;

				case USB_TYPE_STRING_DESC:
					/* Set up the reply buffer with the configuration
						 * descriptor data.
						 */
					ReplyLen = usb_data->ch9_func.
						   Usb_Ch9SetupStrDescReply(
							   InstancePtr, Reply, 128,
							   SetupData->wValue & 0xFF);

#ifdef CH9_DEBUG
					printf("GET STRING DESC %d/%d\r\n", ReplyLen,
					       SetupData->wLength);
#endif
					ReplyLen = ReplyLen > SetupData->wLength ?
						   SetupData->wLength : ReplyLen;

					Status = EpBufferSend(InstancePtr->PrivateData, 0,
							      Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
						/* Failure case needs to be handled */
						for (;;);
					}

					break;

				case USB_TYPE_BOS_DESC:
					/* Set up the reply buffer with the BOS descriptor
						 * data.
						 */
					ReplyLen = usb_data->ch9_func.
						   Usb_Ch9SetupBosDescReply(Reply,
									    USB_REQ_REPLY_LEN);

#ifdef CH9_DEBUG
					printf("GET BOS DESC %d/%d\r\n", ReplyLen,
					       SetupData->wLength);
#endif

					ReplyLen = ReplyLen > SetupData->wLength ?
						   SetupData->wLength : ReplyLen;

					Status = EpBufferSend(InstancePtr->PrivateData, 0,
							      Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
						/* Failure case needs to be handled */
						for (;;);
					}
					break;

				default:
					if (usb_data->ch9_func.Usb_GetDescReply) {
						/* send any class dependent descriptors */
						ReplyLen = usb_data->ch9_func.Usb_GetDescReply(InstancePtr,
								SetupData, Reply);
						if ( ReplyLen == 0 ) {
							Error = 1;
						} else {
							Status = EpBufferSend(InstancePtr->PrivateData, 0,
									      Reply, ReplyLen);
							if (XST_SUCCESS != Status) {
								/* Failure case needs to be handled */
								for (;;);
							}
						}
					} else {
						Error = 1;
					}
					break;
			}
			break;

		case USB_REQ_SET_CONFIGURATION:

#ifdef CH9_DEBUG
			printf("SET CONFIG\r\n");
#endif

			if (InstancePtr->Speed == USB_SPEED_SUPER) {
#ifdef USB_LPM_MODE
				AcceptU1U2Sleep(InstancePtr->PrivateData);
#endif
			}

			usb_data->ch9_func.
			Usb_SetConfiguration(InstancePtr, SetupData);
			usb_data->ch9_func.
			Usb_SetConfigurationApp(InstancePtr, SetupData);

			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case USB_REQ_GET_CONFIGURATION:
#ifdef CH9_DEBUG
			printf("GET CONFIGURATION\r\n");
#endif

			/* When we run CV test suite application in Windows, need to
			 * add GET_CONFIGURATION command to pass test suite
			 */
			*((u8 *) &Reply[0]) = GetConfigDone(InstancePtr->PrivateData);
			Status = EpBufferSend(InstancePtr->PrivateData, 0, Reply,
					      SetupData->wLength);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		case USB_REQ_CLEAR_FEATURE:
#ifdef CH9_DEBUG
			printf("CLEAR FEATURE\r\n");
#endif
			switch (SetupData->bRequestType & USB_STATUS_MASK) {
				case USB_STATUS_ENDPOINT:
					if (SetupData->wValue == USB_ENDPOINT_HALT) {
						EpClearStall(InstancePtr->PrivateData, EpNum, Direction);
					}
					break;

				case USB_STATUS_DEVICE:
					if (InstancePtr->Speed == USB_SPEED_SUPER) {
						if (SetupData->wValue == USB_U1_ENABLE) {
							U1SleepDisable(InstancePtr->PrivateData);
						} else if (SetupData->wValue == USB_U2_ENABLE) {
							U2SleepDisable(InstancePtr->PrivateData);
						}
					}

					EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
					break;

				default:
					Error = 1;
					break;
			}
			break;

		case USB_REQ_SET_FEATURE:
#ifdef CH9_DEBUG
			printf("SET FEATURE\r\n");
#endif
			switch (SetupData->bRequestType & USB_STATUS_MASK) {
				case USB_STATUS_ENDPOINT:
					if (SetupData->wValue == USB_ENDPOINT_HALT)
						EpSetStall(InstancePtr->PrivateData, EpNum,
							   Direction);

					EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);

					break;

				/* When we run CV test suite application in Windows, need to
				 * add INTRF_FUNC_SUSNPEND command to pass test suite
				 */
				case USB_STATUS_INTERFACE:
					switch (SetupData->wValue) {
						case USB_INTRF_FUNC_SUSPEND:
							/* enable Low power suspend */
							/* enable remote wakeup */
							break;
						default:
							Error = 1;
					}
					break;

				case USB_STATUS_DEVICE:
					if (InstancePtr->Speed == USB_SPEED_SUPER) {
						if (SetupData->wValue == USB_U1_ENABLE) {
#ifdef USB_LPM_MODE
							U1SleepEnable(InstancePtr->PrivateData);
#endif
						} else if (SetupData->wValue == USB_U2_ENABLE) {
#ifdef USB_LPM_MODE
							U2SleepEnable(InstancePtr->PrivateData);
#endif
						}
					}

					if (SetupData->wValue == USB_TEST_MODE) {
						u32 TestSel = (SetupData->wIndex >> 8) & 0xFF;

						EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
						usleep(1000);

						switch (TestSel) {
							case USB_TEST_J:
							case USB_TEST_K:
							case USB_TEST_SE0_NAK:
							case USB_TEST_PACKET:
							case USB_TEST_FORCE_ENABLE:
								//Set Bits in PORTSCR
								SetBits(InstancePtr->PrivateData, TestSel << 16);
								break;
							default:
								/* Unsupported test selector */
								break;
						}
						break;
					}
					break;

				default:
					Error = 1;
					break;
			}

			break;

		/* For set interface, check the alt setting host wants */
		case USB_REQ_SET_INTERFACE:

#ifdef CH9_DEBUG
			printf("SET INTERFACE %d/%d\n", SetupData->wValue, SetupData->wIndex);
#endif

			/* Call the set interface handler, if any*/
			if (usb_data->ch9_func.Usb_SetInterfaceHandler != NULL) {
				usb_data->ch9_func.Usb_SetInterfaceHandler(
					InstancePtr, SetupData);
			}

			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case USB_REQ_SET_SEL:
#ifdef CH9_DEBUG
			printf("SET SEL \r\n");
#endif

			EpBufferRecv(InstancePtr->PrivateData, 0, TmpBuffer, 6);
			SetU1SleepTimeout(InstancePtr->PrivateData, 0x0A);
			SetU2SleepTimeout(InstancePtr->PrivateData, 0x04);
			break;

		case USB_REQ_SET_ISOCH_DELAY:
#ifdef CH9_DEBUG
			printf("SET ISOCH DELAY \r\n");
#endif
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
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
	}
}
