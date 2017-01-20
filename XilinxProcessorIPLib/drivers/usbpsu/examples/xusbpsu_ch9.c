/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xusbpsu_ch9.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg   06/06/16 First release
* 1.1	vak  30/11/16 Addded DFU support
*
*
* </pre>
*
*****************************************************************************/
#include "xusbpsu_ch9.h"
#include "xil_cache.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

#define XUSBPSU_REQ_REPLY_LEN	1024	/**< Max size of reply buffer. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XUsbPsu_StdDevReq(struct XUsbPsu *InstancePtr,
								SetupPacket *SetupData);
void XUsbPsu_ClassReq(struct XUsbPsu *InstancePtr,
								SetupPacket *SetupData);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* This function handles a Setup Data packet from the host.
*
* @param	InstancePtr is a pointer to XUsbPsu instance of the controller.
* @param	SetupData is the structure containing the setup request.
*
* @return
*		- XST_SUCCESS if the function is successful.
*		- XST_FAILURE if an Error occured.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_Ch9Handler(struct XUsbPsu *InstancePtr,
			SetupPacket *SetupData)
{
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);

	switch (SetupData->bRequestType & XUSBPSU_REQ_TYPE_MASK) {
	case XUSBPSU_CMD_STDREQ:
		XUsbPsu_StdDevReq(InstancePtr, SetupData);
		break;

	case XUSBPSU_CMD_CLASSREQ:
		ch9_ptr->ch9_func.XUsbPsu_ClassReq(InstancePtr, SetupData);
		break;

	case XUSBPSU_CMD_VENDREQ:

#ifdef CH9_DEBUG
		printf("vendor request %x\n", SetupData->bRequest);
#endif
		break;

	default:
		/* Stall on Endpoint 0 */
#ifdef CH9_DEBUG
		printf("unknown class req, stalling at %s\n", __func__);
#endif
		XUsbPsu_EpSetStall(InstancePtr, 0, XUSBPSU_EP_DIR_OUT);
		break;
	}
}

/*****************************************************************************/
/**
* This function handles a standard device request.
*
* @param	InstancePtr is a pointer to XUsbPsu instance of the controller.
* @param	SetupData is a pointer to the data structure containing the
*		setup request.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XUsbPsu_StdDevReq(struct XUsbPsu *InstancePtr,
			      SetupPacket *SetupData)
{
	int Status;
	int Error = 0;
	int ReplyLen;
	static u8 Reply[XUSBPSU_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
	static u8 TmpBuffer[10] ALIGNMENT_CACHELINE;
	USBCH9_DATA *usb_data =
			(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
	u8 EpNum = SetupData->wIndex & XUSBPSU_ENDPOINT_NUMBER_MASK;
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = !!(SetupData->wIndex & XUSBPSU_ENDPOINT_DIR_MASK);

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > XUSBPSU_REQ_REPLY_LEN) {
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


	case XUSBPSU_REQ_GET_STATUS:

		switch(SetupData->bRequestType & XUSBPSU_STATUS_MASK) {
		case XUSBPSU_STATUS_DEVICE:
#ifdef CH9_DEBUG
			printf("GET STATUS DEVICE\r\n");
#endif

			/* It seems we do not have to worry about zeroing out
			 * the rest of the reply buffer even though we are only
			 * using the first two bytes.
			 */
			*((u16 *) &Reply[0]) = 0x0100; /* Self powered */
			break;

		case XUSBPSU_STATUS_INTERFACE:
#ifdef CH9_DEBUG
			printf("GET STATUS INTERFACE\r\n");
#endif
			*((u16 *) &Reply[0]) = 0x0;
			break;

		case XUSBPSU_STATUS_ENDPOINT:
#ifdef CH9_DEBUG
			printf("GET STATUS ENDPOINT\r\n");
#endif
			*((u16 *) &Reply[0]) = XUsbPsu_IsEpStalled(InstancePtr, EpNum,
					Direction);
			break;
		default:
#ifdef CH9_DEBUG
			printf("unknown request for status %x\r\n",
				SetupData->bRequestType);
#endif
			break;
		}

		XUsbPsu_EpBufferSend(InstancePtr, 0, Reply, SetupData->wLength);
		break;

	case XUSBPSU_REQ_SET_ADDRESS:

		/* With bit 24 set the address value is held in a shadow
		 * register until the status phase is acked. At which point it
		 * address value is written into the address register.
		 */
		XUsbPsu_SetDeviceAddress(InstancePtr, SetupData->wValue);
#ifdef CH9_DEBUG
		printf("SET ADDRESS: %d\r\n", SetupData->wValue);
#endif
		break;

	case XUSBPSU_REQ_GET_INTERFACE:
#ifdef CH9_DEBUG
		printf("GET INTERFACE %d/%d/%d\r\n",
			SetupData->wIndex, SetupData->wLength, 0);
#endif
		break;

	case XUSBPSU_REQ_GET_DESCRIPTOR:
		/* Get descriptor type. */
		switch ((SetupData->wValue >> 8) & 0xff) {

		case XUSBPSU_TYPE_DEVICE_DESC:
		case XUSBPSU_TYPE_DEVICE_QUALIFIER:
			/*
			 * Set up the reply buffer with the device descriptor
			 * data.
			 */
			ReplyLen = usb_data->ch9_func.
					XUsbPsu_Ch9SetupDevDescReply(
						InstancePtr, Reply,
						XUSBPSU_REQ_REPLY_LEN);

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;
#ifdef CH9_DEBUG
			printf("GET DEV DESC %d/%d\r\n", ReplyLen,
						SetupData->wLength);
#endif
			if(((SetupData->wValue >> 8) & 0xff) ==
					XUSBPSU_TYPE_DEVICE_QUALIFIER) {
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
			Status = XUsbPsu_EpBufferSend(InstancePtr, 0,
					Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		case XUSBPSU_TYPE_CONFIG_DESC:

			/* Set up the reply buffer with the configuration
			 * descriptor data.
			 */
			ReplyLen = usb_data->ch9_func.
					XUsbPsu_Ch9SetupCfgDescReply(
						InstancePtr, Reply,
						XUSBPSU_REQ_REPLY_LEN);

#ifdef CH9_DEBUG
			printf("GET CONFIG DESC %d/%d\r\n", ReplyLen,
                   SetupData->wLength);
#endif
			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPsu_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		case XUSBPSU_TYPE_STRING_DESC:
            /* Set up the reply buffer with the configuration
			 * descriptor data.
			 */
			ReplyLen = usb_data->ch9_func.
					XUsbPsu_Ch9SetupStrDescReply(
						InstancePtr, Reply, 128,
						SetupData->wValue & 0xFF);

#ifdef CH9_DEBUG
		    printf("GET STRING DESC %d/%d\r\n", ReplyLen,
                   SetupData->wLength);
#endif
			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPsu_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}

			break;

		case XUSBPSU_TYPE_BOS_DESC:
            /* Set up the reply buffer with the BOS descriptor
			 * data.
			 */
			ReplyLen = usb_data->ch9_func.
					XUsbPsu_Ch9SetupBosDescReply(Reply,
							XUSBPSU_REQ_REPLY_LEN);

#ifdef CH9_DEBUG
			printf("GET BOS DESC %d/%d\r\n", ReplyLen,
                   SetupData->wLength);
#endif

			ReplyLen = ReplyLen > SetupData->wLength ?
						SetupData->wLength : ReplyLen;

			Status = XUsbPsu_EpBufferSend(InstancePtr, 0,
							Reply, ReplyLen);
			if (XST_SUCCESS != Status) {
				/* Failure case needs to be handled */
				for (;;);
			}
			break;

		default:
			Error = 1;
			break;
		}
		break;

	case XUSBPSU_REQ_SET_CONFIGURATION:

		/*
		 * Only allow configuration index 1 as this is the only one we
		 * have.
		 */
		if ((SetupData->wValue & 0xff) != 1) {
			Error = 1;
#ifdef CH9_DEBUG
			printf("SET CONFIG fail\r\n");
#endif
			break;
		}

#ifdef CH9_DEBUG
		printf("SET CONFIG\r\n");
#endif

		if(InstancePtr->Speed == XUSBPSU_SPEED_SUPER) {
#ifdef XUSBPSU_LPM_MODE
			XUsbPsu_AcceptU1U2Sleep(InstancePtr);
#endif
		}

		usb_data->ch9_func.
			XUsbPsu_SetConfiguration(InstancePtr, SetupData);
		usb_data->ch9_func.
			XUsbPsu_SetConfigurationApp(InstancePtr, SetupData);
		break;

	case XUSBPSU_REQ_GET_CONFIGURATION:
#ifdef CH9_DEBUG
		printf("GET CONFIGURATION\r\n");
#endif
		break;

	case XUSBPSU_REQ_CLEAR_FEATURE:
#ifdef CH9_DEBUG
		printf("CLEAR FEATURE\r\n");
#endif

		switch(SetupData->bRequestType & XUSBPSU_STATUS_MASK) {
		case XUSBPSU_STATUS_ENDPOINT:
			if (SetupData->wValue == XUSBPSU_ENDPOINT_HALT)
				XUsbPsu_EpClearStall(InstancePtr, EpNum,
						Direction);
			break;

		case XUSBPSU_STATUS_DEVICE:
			if(InstancePtr->Speed == XUSBPSU_SPEED_SUPER) {
				if(SetupData->wValue == XUSBPSU_U1_ENABLE) {
					XUsbPsu_U1SleepDisable(InstancePtr);
				} else if(SetupData->wValue == XUSBPSU_U2_ENABLE) {
					XUsbPsu_U2SleepDisable(InstancePtr);
				}
			}
			break;

		default:
			Error = 1;
			break;
		}
		break;

	case XUSBPSU_REQ_SET_FEATURE:
#ifdef CH9_DEBUG
		printf("SET FEATURE\r\n");
#endif
		switch(SetupData->bRequestType & XUSBPSU_STATUS_MASK) {
		case XUSBPSU_STATUS_ENDPOINT:
			if (SetupData->wValue == XUSBPSU_ENDPOINT_HALT)
				XUsbPsu_EpSetStall(InstancePtr, EpNum,
						Direction);
			break;

		case XUSBPSU_STATUS_DEVICE:
			if(InstancePtr->Speed == XUSBPSU_SPEED_SUPER) {
				if(SetupData->wValue == XUSBPSU_U1_ENABLE) {
#ifdef XUSBPSU_LPM_MODE
					XUsbPsu_U1SleepEnable(InstancePtr);
#endif
				} else if (SetupData->wValue == XUSBPSU_U2_ENABLE) {
#ifdef XUSBPSU_LPM_MODE
					XUsbPsu_U2SleepEnable(InstancePtr);
#endif
				}
			}

			if (SetupData->wValue == XUSBPSU_TEST_MODE) {
				int TestSel = (SetupData->wIndex >> 8) & 0xFF;

				usleep(1000);

				switch (TestSel) {
				case XUSBPSU_TEST_J:
				case XUSBPSU_TEST_K:
				case XUSBPSU_TEST_SE0_NAK:
				case XUSBPSU_TEST_PACKET:
				case XUSBPSU_TEST_FORCE_ENABLE:
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
	case XUSBPSU_REQ_SET_INTERFACE:

#ifdef CH9_DEBUG
		printf("SET INTERFACE %d/%d\n", SetupData->wValue, SetupData->wIndex);
#endif

		/* Call the set interface handler, if any*/
		if (usb_data->ch9_func.XUsbPsu_SetInterfaceHandler != NULL) {
			usb_data->ch9_func.XUsbPsu_SetInterfaceHandler(
							InstancePtr, SetupData);
		}
		break;

	case XUSBPSU_REQ_SET_SEL:
#ifdef CH9_DEBUG
		printf("SET SEL \r\n");
#endif

		XUsbPsu_EpBufferRecv(InstancePtr, 0, TmpBuffer, 6);
		XUsbPsu_SetU1SleepTimeout(InstancePtr, 0x0A);
		XUsbPsu_SetU2SleepTimeout(InstancePtr, 0x04);
		break;

	case XUSBPSU_REQ_SET_ISOCH_DELAY:
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
		XUsbPsu_EpSetStall(InstancePtr, 0, XUSBPSU_EP_DIR_OUT);
	}
}
