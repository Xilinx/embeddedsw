/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xusb_freertos_class_audio.c
*
* This file contains the implementation of the audio specific class code
* for the example.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rb   26/03/18 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "FreeRTOS.h"
#include "task.h"
#include "xusb_freertos_ch9_audio.h"
#include "xusb_freertos_class_audio.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

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
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	u32 ReplyLen;
	u8 Error = 0;
	static u8 Reply[USB_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
	u8 UnitId = SetupData->wIndex >> 8;

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > USB_REQ_REPLY_LEN)
		return;

	switch (SetupData->bRequest) {

	case UAC2_CS_CUR:
		switch (UnitId) {
		case USB_CLK_SRC_ID:
			switch (SetupData->wValue >> 8) {
			case UAC2_CS_CONTROL_SAM_FREQ:
				if ((SetupData->bRequestType &
							USB_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					EpBufferRecv(InstancePtr->PrivateData,
							0, Reply, ReplyLen);

					EpBufferSend(InstancePtr->PrivateData,
							0, NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;

					Reply[0] = (u8)0x44;
					Reply[1] = (u8)0xAC;
					Reply[2] = (u8)0x00;
					Reply[3] = (u8)0x00;

					EpBufferSend(InstancePtr->PrivateData,
							0, Reply, ReplyLen);
				}
				break;

			case UAC2_CS_CONTROL_CLOCK_VALID:
				ReplyLen = SetupData->wLength > 4 ? 4 :
					SetupData->wLength;
				/* Internal clock always valid */
				Reply[0] = (u8)0x01;

				EpBufferSend(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);
				break;

			default:
				/* Unknown Control Selector for Clock Unit */
				Error = 1;
				break;
			}

			break;

		case USB_CLK_SEL_ID:
			if ((SetupData->bRequestType &
						USB_ENDPOINT_DIR_MASK) == 0) {
				/* Set Request */
				ReplyLen = SetupData->wLength;
				EpBufferRecv(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);

				EpBufferSend(InstancePtr->PrivateData, 0,
						NULL, 0);
			} else {
				/* Get Request */
				ReplyLen = SetupData->wLength > 4 ? 4 :
					SetupData->wLength;
				Reply[0] = (u8)0x01;

				EpBufferSend(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);
			}
			break;

		case OUT_FETR_UNT_ID:
		case IN_FETR_UNT_ID:
			switch (SetupData->wValue >> 8) {
			case UAC2_FU_VOLUME_CONTROL:
				/* Feature not available */
				if ((SetupData->bRequestType &
							USB_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					EpBufferRecv(InstancePtr->PrivateData,
							0, Reply, ReplyLen);

					EpBufferSend(InstancePtr->PrivateData,
							0, NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;
					Reply[0] = 0x00;
					Reply[1] = 0x00;

					EpBufferSend(InstancePtr->PrivateData,
							0, Reply, ReplyLen);
				}
				break;

			case UAC2_FU_MUTE_CONTROL:
				/* Feature not available */
				if ((SetupData->bRequestType &
							USB_ENDPOINT_DIR_MASK) == 0) {
					/* Set Request */
					ReplyLen = SetupData->wLength;
					EpBufferRecv(InstancePtr->PrivateData,
							0, Reply, ReplyLen);

					EpBufferSend(InstancePtr->PrivateData,
							0, NULL, 0);
				} else {
					/* Get Request */
					ReplyLen = SetupData->wLength > 4 ? 4 :
						SetupData->wLength;
					Reply[0] = 0x01;

					EpBufferSend(InstancePtr->PrivateData,
							0, Reply, ReplyLen);
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
		switch (UnitId) {
		case USB_CLK_SRC_ID:
			switch (SetupData->wValue >> 8) {
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

				EpBufferSend(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);
				break;

			default:
				/* Unknown Clock Source Range Request */
				Error = 1;
				break;
			}
			break;

		case OUT_FETR_UNT_ID:
		case IN_FETR_UNT_ID:
			switch (SetupData->wValue >> 8) {
			case UAC2_FU_VOLUME_CONTROL:
				/* Feature not available */
				ReplyLen = SetupData->wLength > 14 ? 14 :
					SetupData->wLength;
				Reply[0] = (u8)0x01;
				Reply[1] = (u8)0x00;
				Reply[2] = (u8)0x00;
				Reply[3] = (u8)0x81;
				Reply[4] = (u8)0x00;
				Reply[5] = (u8)0x00;
				Reply[6] = (u8)0x00;
				Reply[7] = (u8)0x01;

				EpBufferSend(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);
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
	default:
		Error = 1;
		Ep0StallRestart(InstancePtr->PrivateData);
		break;
	}

	/* Set the send stall bit if there is an error */
	if (Error)
		Ep0StallRestart(InstancePtr->PrivateData);
}

/****************************************************************************/
/**
* This task implements audio record functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvRecordTask(void *pvParameters)
{
	u32 Size;
	u16 MaxPktSize = 1024;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct audio_dev *dev = (struct audio_dev *)(ch9_ptr->data_ptr);

	dev->index = 0;
	dev->residue = 0;
	dev->firstpkt = 1;

	dev->xSemaphoreRecord = xSemaphoreCreateBinary();

	SetEpInterval(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_IN, AUDIO_INTERVAL);

	/* Endpoint enables - not needed for Control EP */
	EpEnable(InstancePtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
			MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);

	StreamOn(InstancePtr->PrivateData, ISO_EP, USB_EP_DIR_OUT,
			BufferPtrTemp);

	while (1) {

		xSemaphoreTake(dev->xSemaphoreRecord, portMAX_DELAY);

		Size = dev->packetsize;
		dev->residue += dev->packetresidue;

		if ((dev->residue / dev->interval) >= dev->framesize) {
			Size += dev->framesize;
			dev->residue -= dev->framesize * dev->interval;
		}

		/* Buffer is completed, retransmitting the same file data */
		if ((dev->index + Size) > dev->disksize)
			dev->index = 0;

		if (EpBufferSend(InstancePtr->PrivateData, ISO_EP,
					dev->virtualdisk + dev->index,
					Size) == XST_SUCCESS) {
			dev->index += Size;

			if (dev->firstpkt) {
				Size = dev->packetsize;
				dev->residue += dev->packetresidue;

				if ((dev->residue / dev->interval) >=
						dev->framesize) {
					Size += dev->framesize;
					dev->residue -= dev->framesize *
						dev->interval;
				}

				if ((dev->index + Size) > dev->disksize)
					dev->index = 0;
				else
					dev->index += Size;

				dev->firstpkt = 0;
			}
		} else
			xSemaphoreGive(dev->xSemaphoreRecord);
	}
}

/****************************************************************************/
/**
* This task implements audio playback functionality
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvPlayBackTask(void *pvParameters)
{
	u16 MaxPktSize = 1024;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct audio_dev *dev = (struct audio_dev *)(ch9_ptr->data_ptr);

	dev->xSemaphorePlay = xSemaphoreCreateBinary();
#if defined(PLATFORM_ZYNQ)
	xSemaphoreGive(dev->xSemaphorePlay);
#endif

	dev->index = 0;
	dev->residue = 0;
	dev->firstpkt = 1;
	dev->bytesRecv = 0;

	SetEpInterval(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_OUT, AUDIO_INTERVAL);

	/* Endpoint enables - not needed for Control EP */
	EpEnable(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_OUT, MaxPktSize,
			USB_EP_TYPE_ISOCHRONOUS);

	StreamOn(InstancePtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
			BufferPtrTemp);

	while (1) {

		xSemaphoreTake(dev->xSemaphorePlay, portMAX_DELAY);
		if ((dev->index + dev->bytesRecv) > dev->disksize)
			dev->index = 0;

		memcpy(dev->virtualdisk + dev->index, BufferPtrTemp,
				dev->bytesRecv);
		dev->index += dev->bytesRecv;

		EpBufferRecv(InstancePtr->PrivateData, ISO_EP,
				BufferPtrTemp, 1024);
	}
}
