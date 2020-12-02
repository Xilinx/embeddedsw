/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_class_audio.c
 *
 * This file contains the implementation of chapter 9 specific code for
 * the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who	Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   pm	20/02/20 First release
 *
 * </pre>
 *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xusbps_class_audio.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
//#define CH9_DEBUG
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 * This function is called by Chapter9 handler when class request is received
 * from Host.
 *
 * @param	InstancePtr is pointer to Usb_DevData instance.
 * @param	SetupData is the setup packet received from Host.
 *
 * @note	None.
 *
 *****************************************************************************/
void XUsbPs_ClassReq(XUsbPs *InstancePtr, XUsbPs_SetupData *SetupData)
{
	s32 Status;
	u8 Error = 0;
	u32 ReplyLen;
	static u8 Reply[XUSBPS_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
#ifndef XUSBPS_UAC1
	u8 UnitId = SetupData->wIndex >> 8;
#endif

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > XUSBPS_REQ_REPLY_LEN) {
		return;
	}

#ifdef CH9_DEBUG
	xil_printf("C:bmRequestType 0x%x\r\n", SetupData->bmRequestType);
	xil_printf("C:bRequest 0x%x\r\n", SetupData->bRequest);
	xil_printf("C:wValue 0x%x\r\n", SetupData->wValue);
	xil_printf("C:wIndex 0x%x\r\n", SetupData->wIndex);
	xil_printf("C:wLength 0x%x\r\n", SetupData->wLength);
#endif

	switch (SetupData->bRequest) {
#ifdef XUSBPS_UAC1
	case UAC1_SET_CUR:
		ReplyLen = SetupData->wLength;
		XUsbPs_EpDataBufferReceive((XUsbPs *)InstancePtr, 0, Reply,
						ReplyLen);

		XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0, NULL, 0);

		break;
	case UAC1_GET_CUR:
		ReplyLen = SetupData->wLength;
		Reply[0] = (u8)0x40;
		Reply[1] = (u8)0x1F;
		Reply[2] = (u8)0x00;

		Status = XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0,
				Reply, ReplyLen);
		if (XST_SUCCESS != Status) {
			/* Failure case needs to be handled */
			for (;;);
		}

		break;
	case UAC1_GET_MIN:
		ReplyLen = SetupData->wLength;
		Reply[0] = (u8)0x40;
		Reply[1] = (u8)0x1F;
		Reply[2] = (u8)0x00;

		Status = XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0,
				Reply, ReplyLen);
		if (XST_SUCCESS != Status) {
			/* Failure case needs to be handled */
			for (;;);
		}

		break;
	case UAC1_GET_MAX:
		ReplyLen = SetupData->wLength;
		Reply[0] = (u8)0x00;
		Reply[1] = (u8)0x77;
		Reply[2] = (u8)0x01;

		Status = XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0,
				Reply, ReplyLen);
		if (XST_SUCCESS != Status) {
			/* Failure case needs to be handled */
			for (;;);
		}

		break;
	case UAC1_GET_RES:
		ReplyLen = SetupData->wLength;
		Reply[0] = (u8)0x30;
		Reply[1] = (u8)0x00;

		Status = XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0,
				Reply, ReplyLen);
		if (XST_SUCCESS != Status) {
			/* Failure case needs to be handled */
			for (;;);
		}

		break;
#else	/*	XUSPBS_UAC2 */

	case UAC2_CS_CUR:
	switch(UnitId) {
		case USB_CLK_SRC_ID:
			switch(SetupData->wValue >> 8) {
			case UAC2_CS_CONTROL_SAM_FREQ:
				if ((SetupData->bmRequestType &
					XUSBPS_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					XUsbPs_EpDataBufferReceive(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);

					XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;

					Reply[0] = (u8)0x44;
					Reply[1] = (u8)0xAC;
					Reply[2] = (u8)0x00;
					Reply[3] = (u8)0x00;

					Status = XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
						for (;;);
					}
				}

				break;
			case UAC2_CS_CONTROL_CLOCK_VALID:
				ReplyLen = SetupData->wLength > 4 ? 4 :
					SetupData->wLength;
				/* Internal clock always valid */
				Reply[0] = (u8)0x01;

				Status = XUsbPs_EpBufferSend(
						(XUsbPs *)InstancePtr,
						0,
						Reply, ReplyLen);
				if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
					for (;;);
				}

				break;
			default:
				/* Unknown Control Selector for Clock Unit */
				Error = 1;
				break;
			}

			break;
		case USB_CLK_SEL_ID:
			if ((SetupData->bmRequestType &
					XUSBPS_ENDPOINT_DIR_MASK) == 0) {
				/* Set Request */
				ReplyLen = SetupData->wLength;
				XUsbPs_EpDataBufferReceive(
						(XUsbPs *)InstancePtr, 0,
						Reply, ReplyLen);

				XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, 0,
						NULL, 0);
			} else {
				/* Get Request */
				ReplyLen = SetupData->wLength > 4 ? 4 :
					SetupData->wLength;
				Reply[0] = (u8)0x01;

				Status = XUsbPs_EpBufferSend(
						(XUsbPs *)InstancePtr,
						0,
						Reply, ReplyLen);
				if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
					for (;;);
				}
			}

			break;
		case OUT_FETR_UNT_ID:
		case IN_FETR_UNT_ID:
		switch(SetupData->wValue >> 8) {
			case UAC2_FU_VOLUME_CONTROL:
				/* Feature not available */
				if ((SetupData->bmRequestType &
					XUSBPS_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					XUsbPs_EpDataBufferReceive(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);

					XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;
					Reply[0] = 0x00;
					Reply[1] = 0x00;

					Status = XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
						for (;;);
					}
				}

				break;
			case UAC2_FU_MUTE_CONTROL:
				/* Feature not available */
				if ((SetupData->bmRequestType &
					XUSBPS_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					XUsbPs_EpDataBufferReceive(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);

					XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;
					Reply[0] = 0x01;

					Status = XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
						for (;;);
					}
				}

				break;
			default:
				/* Unknown Control Selector for Feature Unit */
				Error = 1;
				break;
		}

		break;
	default:
		/* Unknown unit ID */
		Error = 1;
			break;
	}

	break;
	case UAC2_CS_RANGE:
		switch(UnitId) {
		case USB_CLK_SRC_ID:
			switch(SetupData->wValue >> 8) {
			case UAC2_CS_CONTROL_SAM_FREQ:
				ReplyLen = SetupData->wLength > 14 ? 14 :
					SetupData->wLength;
				Reply[0] = (u8)0x01;
				Reply[1] = (u8)0x00;
				Reply[2] = (u8)0x44;
				Reply[3] = (u8)0xAC;
				Reply[4] = (u8)0x00;
				Reply[5] = (u8)0x00;
				Reply[6] = (u8)0x44;
				Reply[7] = (u8)0xAC;
				Reply[8] = (u8)0x00;
				Reply[9] = (u8)0x00;
				Reply[10] = (u8)0x00;
				Reply[11] = (u8)0x00;
				Reply[12] = (u8)0x00;
				Reply[13] = (u8)0x00;

				Status = XUsbPs_EpBufferSend(
						(XUsbPs *)InstancePtr,
						0,
						Reply, ReplyLen);
				if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
					for (;;);
				}

				break;
			default:
				/* Unknown Clock Source Range Request */
				Error = 1;
				break;
			}

			break;
		case OUT_FETR_UNT_ID:
		case IN_FETR_UNT_ID:
			switch(SetupData->wValue >> 8) {
				case UAC2_FU_VOLUME_CONTROL:
					/* Feature not available */
					ReplyLen = SetupData->wLength >
						14 ? 14 :
						SetupData->wLength;
					Reply[0] = (u8)0x01;
					Reply[1] = (u8)0x00;
					Reply[2] = (u8)0x00;
					Reply[3] = (u8)0x81;
					Reply[4] = (u8)0x00;
					Reply[5] = (u8)0x00;
					Reply[6] = (u8)0x00;
					Reply[7] = (u8)0x01;

					Status = XUsbPs_EpBufferSend(
							(XUsbPs *)InstancePtr,
							0,
							Reply, ReplyLen);
					if (XST_SUCCESS != Status) {
					/* Failure case needs to be handled */
						for (;;);
					}

					break;
				default:
				/* Unknown Control Selector for Feature Unit */
					Error = 1;
					break;
			}

			break;
		default:
			/* Unknown unit ID */
			Error = 1;
				break;
		}

		break;
#endif  /* end of XUSBPS_UAC2 */

		default:
			Error = 1;
			break;
	}

	/* Set the send stall bit if there is an error */
	if (Error) {
#ifdef CH9_DEBUG
		printf("std dev req %d/%d error, stall 0 in out\n",
			SetupData->bRequest, (SetupData->wValue >> 8) & 0xff);
#endif
		XUsbPs_EpStall((XUsbPs *)InstancePtr, 0U,
			XUSBPS_EP_DIRECTION_IN | XUSBPS_EP_DIRECTION_OUT);
	}
}
