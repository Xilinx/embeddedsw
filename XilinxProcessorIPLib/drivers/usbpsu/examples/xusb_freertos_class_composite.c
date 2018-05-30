/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_class_composite.c
 *
 * This file contains the implementation of the composite device specific
 * class code for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   28/03/18 First release
 * 1.5   vak  13/02/19 Added support for versal
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 *
 *</pre>
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"	/* XPAR parameters */
#include "xusb_freertos_ch9_composite.h"
#include "xusb_freertos_class_composite.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

static inline void Usb_DfuWaitForReset(struct dfu_if *DFU)
{
	/* This bit would be cleared when reset happens*/
	DFU->dfu_wait_for_interrupt = 1;
	dmb();
	while (DFU->dfu_wait_for_interrupt == 0) {
		/* Wait for an reset event */
		;
	}
}

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
static u8 txBuffer[128];
#pragma data_alignment = 64
static u8 MaxLUN = 0;
#pragma data_alignment = 64
static u8 ClassData[10];
#pragma data_alignment = 64
static u8 BufferPtrTemp[1024];
u8 DetachCounter = 0;
#else
#pragma data_alignment = 32
static u8 txBuffer[128];
#pragma data_alignment = 32
static u8 MaxLUN = 0;
#pragma data_alignment = 32
static u8 ClassData[10];
#pragma data_alignment = 32
static u8 BufferPtrTemp[1024];
u8 DetachCounter = 0;
#endif
#else
u8 DetachCounter = 0;
static u8 txBuffer[128] ALIGNMENT_CACHELINE;
static u8 MaxLUN ALIGNMENT_CACHELINE = 0;
static u8 ClassData[10];
static u8 BufferPtrTemp[1024];
#endif

/* Pre-manufactured response to the SCSI Inquiry command. */
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
#else
#pragma data_alignment = 32
#endif
const static SCSI_INQUIRY scsiInquiry[] = {
#else
const static SCSI_INQUIRY scsiInquiry[] ALIGNMENT_CACHELINE = {
#endif
	{
		0x00,
		0x80,
		0x00,
		0x01,
		0x1f,
		0x00,
		0x00,
		0x00,
		{"Xilinx  "},		/* Vendor ID:  must be  8 characters long. */
		{"PS USB VirtDisk"},	/* Product ID: must be 16 characters long. */
		{"1.00"}		/* Revision:   must be  4 characters long. */
	},
	{
		0x00,
		0x80,
		0x02,
		0x02,
		0x1F,
		0x00,
		0x00,
		0x00,
		{"Xilinx  "},		/* Vendor ID:  must be  8 characters long. */
		{"PS USB VirtDisk"},	/* Product ID: must be 16 characters long. */
		{"1.00"}		/* Revision:   must be  4 characters long. */
	}
};

/*****************************************************************************/
/**
* This function handles setting of DFU state.
*
* @param	DFU is a pointer to DFU instance of the controller
* @param	dfu_state is a value of the DFU state to be set
*
* @return	- XST_SUCCESS if the function is successful.
*		- XST_FAILURE if an Error occurred.
*
* @note		None.
*
******************************************************************************/
s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state)
{
	switch (dfu_state) {

		case STATE_APP_IDLE:
			DFU->curr_state = STATE_APP_IDLE;
			DFU->next_state = STATE_APP_DETACH;
			DFU->status = DFU_STATUS_OK;
			/* Set to runtime mode by default*/
			DFU->is_dfu = 0;
			DFU->runtime_to_dfu = 0;
			break;

		case STATE_APP_DETACH:
			if (DFU->curr_state == STATE_APP_IDLE) {
				DFU->curr_state = STATE_APP_DETACH;
				DFU->next_state = STATE_DFU_IDLE;

#ifdef DFU_DEBUG
				xil_printf("Waiting for USB reset to happen"
						"to enter into DFU_IDLE state....\n");
#endif

				/* Wait For USB Reset to happen */
				Usb_DfuWaitForReset(DFU);

#ifdef DFU_DEBUG
				xil_printf("Got reset, entering DFU_IDLE state\n");
#endif
				/* Setting DFU mode */
				DFU->is_dfu = 1;

				/* Set this flag to indicate we are
				 * going from runtime to dfu mode
				 */
				DFU->runtime_to_dfu = 1;

				/* fall through */
			} else if (DFU->curr_state == STATE_DFU_IDLE) {
#ifdef DFU_DEBUG
				xil_printf("Waiting for USB reset to"
						"happen to enter into APP_IDLE state....\n");
#endif
				/* Wait For USB Reset to happen */
				Usb_DfuWaitForReset(DFU);

				DFU->curr_state = STATE_APP_IDLE;
				DFU->next_state = STATE_APP_DETACH;
				DFU->status = DFU_STATUS_OK;
				DFU->is_dfu = 0;
				break;

			} else {
				goto stall;
			}

		case STATE_DFU_IDLE:
			DFU->curr_state = STATE_DFU_IDLE;
			DFU->next_state = STATE_DFU_DOWNLOAD_SYNC;
			DFU->is_dfu = 1;
			break;

		case STATE_DFU_DOWNLOAD_SYNC:
			DFU->curr_state = STATE_DFU_DOWNLOAD_SYNC;
			break;

		case STATE_DFU_DOWNLOAD_BUSY:
		case STATE_DFU_DOWNLOAD_IDLE:
		case STATE_DFU_MANIFEST_SYNC:
		case STATE_DFU_MANIFEST:
		case STATE_DFU_MANIFEST_WAIT_RESET:
		case STATE_DFU_UPLOAD_IDLE:
		case STATE_DFU_ERROR:
		default:
stall:
			/* Unsupported command. Stall the end point */
			DFU->curr_state = STATE_DFU_ERROR;
			EpSetStall(DFU->InstancePtr->PrivateData, 0, USB_EP_DIR_IN);
			return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function updates the current state while downloading a file
*
* @param	DFU is a pointer to DFU instance of the controller
* @param	status is a pointer of the DFU status
*
* @note		None.
*
******************************************************************************/
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status)
{

	if (DFU->got_dnload_rqst != 1)
		return;

	switch (DFU->curr_state) {

		case STATE_DFU_IDLE:
			DFU->curr_state = STATE_DFU_DOWNLOAD_SYNC;
			break;

		case STATE_DFU_DOWNLOAD_SYNC:
			DFU->curr_state = STATE_DFU_DOWNLOAD_BUSY;
			break;

		default:
			break;
	}
}

/*****************************************************************************/
/**
* This function handles getting of DFU status.
*
* @param	DFU is a pointer to DFU instance of the controller
* @param	status is the pointer of the DFU status
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void USB_DfuGetStatus(struct dfu_if *DFU, u8 *status)
{
	/* update the download state */
	USB_DfuSetDwloadState(DFU, status);

	/* DFU status */
	status[0] = (u8)DFU->status;
	/* timeout to wait until next request */
	status[1] = (u8)(0);
	status[2] = (u8)0;
	status[3] = (u8)0;
	/* DFU current state */
	status[4] = (u8)DFU->curr_state;
	status[5] = (u8)0;
}

/*****************************************************************************/
/**
* This function handles USB disconnect, called from driver.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_DisconnectHandler(struct Usb_DevData *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);

	if (DFU->dfu_wait_for_interrupt == 1) {
		/* Tell DFU that we got disconnected */
		DFU->dfu_wait_for_interrupt = 0;
	}
	if (DFU->is_dfu == 1) {
		/* Switch to run time mode */
		DFU->is_dfu = 0;
	}
}

/*****************************************************************************/
/**
* This function handles USB reset, called from driver.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_ResetHandler(struct Usb_DevData *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);

	if (DFU->dfu_wait_for_interrupt == 1) {
		/* Tell DFU that we got reset signal */
		DFU->dfu_wait_for_interrupt = 0;
	}
}

/*****************************************************************************/
/**
* This function handles set interface request.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_SetIntf(struct Usb_DevData *InstancePtr,  SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);

	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *interface = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct dfu_if *f_dfu = &interface->f_dfu;
	BaseType_t xHigherPriorityTaskWoken;

	switch (SetupData->wIndex & 0xff) {

	case DFU_INTF:		/* DFU */
		/* Setting the alternate setting requested */
		f_dfu->current_inf = SetupData->wValue;
		if ((f_dfu->current_inf >= DFU_ALT_SETTING) ||
				(f_dfu->runtime_to_dfu == 1)) {

			/* Clear the flag , before entering into
			 * DFU mode from runtime mode
			 */
			if (f_dfu->runtime_to_dfu == 1)
				f_dfu->runtime_to_dfu = 0;

			/* Entering DFU_IDLE state */
			Usb_DfuSetState(f_dfu, STATE_DFU_IDLE);
		} else {
			/* Entering APP_IDLE state */
			Usb_DfuSetState(f_dfu, STATE_APP_IDLE);
		}
		break;

	case AUDIO_INTF_IN:	/* AUDIO */
		if ((SetupData->wValue & 0xff) == 1) {

			xTaskNotifyFromISR(interface->xMainTask, RECORD_START,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
		} else {

			xTaskNotifyFromISR(interface->xMainTask, RECORD_STOP,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;

	case AUDIO_INTF_OUT:
		if ((SetupData->wValue & 0xff) == 1) {

			xTaskNotifyFromISR(interface->xMainTask, PLAY_START,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
		} else {

			xTaskNotifyFromISR(interface->xMainTask, PLAY_STOP,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;

	default:
		xil_printf("Invalid number for set interface request\r\n");
		break;
	}
}

/*****************************************************************************/
/**
* This function handles UAC1 class request.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Usb_AudioClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	u32 ReplyLen;
	u8 Error = 0;

#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
#else
#pragma data_alignment = 32
#endif
	static u8 Reply[USB_REQ_REPLY_LEN];
#else
	static u8 Reply[USB_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;
#endif
	u8 UnitId = SetupData->wIndex >> 8;

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen...
	 */
	if (SetupData->wLength > USB_REQ_REPLY_LEN) {
		return;
	}

	switch (SetupData->bRequest) {

	case UAC2_CS_CUR:
		switch(UnitId) {
		case USB_CLK_SRC_ID:
			switch(SetupData->wValue >> 8) {
			case UAC2_CS_CONTROL_SAM_FREQ:
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

					Reply[0] = (u8)0x44;
					Reply[1] = (u8)0xAC;
					Reply[2] = (u8)0x00;
					Reply[3] = (u8)0x00;

					EpBufferSend(InstancePtr->PrivateData, 0,
							Reply, ReplyLen);
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
			if ((SetupData->bRequestType & USB_ENDPOINT_DIR_MASK) == 0) {
				/* Set Request */
				ReplyLen = SetupData->wLength;
				EpBufferRecv(InstancePtr->PrivateData, 0,
						Reply, ReplyLen);

				EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
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
			switch(SetupData->wValue >> 8) {
			case UAC2_FU_VOLUME_CONTROL:
				/* Feature not available */
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
					Reply[0] = 0x00;
					Reply[1] = 0x00;

					EpBufferSend(InstancePtr->PrivateData, 0,
							Reply, ReplyLen);
				}
				break;

			case UAC2_FU_MUTE_CONTROL:
				/* Feature not available */
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
					Reply[0] = 0x01;

					EpBufferSend(InstancePtr->PrivateData, 0,
							Reply, ReplyLen);
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
			switch(SetupData->wValue >> 8) {
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

/*****************************************************************************/
/**
* This function handles DFU heart and soul of DFU state machine.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void Usb_DfuClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);
	u32 txBytesLeft;
	u32 rxBytesLeft;
	s32 result = -1;

#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
#else
#pragma data_alignment = 32
#endif
	static u8 DFUReply[6];
#else
	static u8 DFUReply[6] ALIGNMENT_CACHELINE;
#endif
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *f = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct dfu_if *dfu = &(f->f_dfu);

	switch (SetupData->bRequest) {

		case DFU_DETACH:
#ifdef DFU_DEBUG
			xil_printf("Got DFU_DETACH\n");
#endif
			Usb_DfuSetState(dfu, STATE_APP_DETACH);
			break;

		case DFU_DNLOAD:
			if (dfu->got_dnload_rqst == 0)
				dfu->got_dnload_rqst = 1;

			if ((dfu->total_transfers == 0) && (SetupData->wValue == 0)) {
				/* we are the start of the data, clear
				 * the download counter
				 */
				dfu->total_bytes_dnloaded = 0;
			}

			rxBytesLeft = (SetupData->wLength);
			dfu->total_transfers++;

			if (rxBytesLeft > 0) {
				result = EpBufferRecv(InstancePtr->PrivateData, 0,
						(dfu->disk) +
						dfu->total_bytes_dnloaded,
						rxBytesLeft);
				dfu->total_bytes_dnloaded += rxBytesLeft;
			} else if ((dfu->got_dnload_rqst == 1) && (rxBytesLeft == 0)) {
				dfu->curr_state = STATE_DFU_IDLE;
				dfu->got_dnload_rqst = 0;
				dfu->total_transfers = 0;
			}

			if ((dfu->got_dnload_rqst == 1) && (result == 0)) {
				dfu->curr_state = STATE_DFU_DOWNLOAD_IDLE;
				dfu->got_dnload_rqst = 0;
			}
			break;

		case DFU_UPLOAD:

			if ((dfu->total_transfers == 0) && (SetupData->wValue == 0)) {
				/* we are the start of the data, clear the
				 * download counter
				 */
				dfu->total_bytes_uploaded = 0;
			}

			txBytesLeft = (SetupData->wLength);
			dfu->total_transfers++;

			if (dfu->total_bytes_uploaded < dfu->total_bytes_dnloaded) {
				if ((dfu->total_bytes_uploaded + txBytesLeft) >
						dfu->total_bytes_dnloaded) {
					/* Upload only remaining bytes */
					txBytesLeft =
						(dfu->total_bytes_dnloaded -
						 dfu->total_bytes_uploaded);
				}
				result = EpBufferSend(InstancePtr->PrivateData, 0,
						(dfu->disk) +
						dfu->total_bytes_uploaded,
						txBytesLeft);
				dfu->total_bytes_uploaded += txBytesLeft;
			} else {
				/* Send a short packet */
				result = EpBufferSend(InstancePtr->PrivateData, 0,
						(dfu->disk) +
						dfu->total_bytes_uploaded, 0);
				dfu->total_bytes_uploaded = 0;
				dfu->total_transfers = 0;
			}
			dfu->curr_state = STATE_DFU_IDLE;
			break;

		case DFU_GETSTATUS:
			USB_DfuGetStatus(dfu, (u8 *)DFUReply);
			EpBufferSend(InstancePtr->PrivateData,
					0, DFUReply, SetupData->wLength);
			break;

		case DFU_CLRSTATUS:
			if (dfu->curr_state == STATE_DFU_ERROR) {
				EpClearStall(InstancePtr->PrivateData, 0, USB_EP_DIR_IN);
				Usb_DfuSetState(dfu, STATE_DFU_IDLE);
			}
			break;

		case DFU_GETSTATE:
			DFUReply[0] = (u8)dfu->curr_state;
			EpBufferSend(InstancePtr->PrivateData,
					0, DFUReply, SetupData->wLength);
			break;

		case DFU_ABORT:
			/* Stop current download transfer */
			if (dfu->got_dnload_rqst == 1) {
				StopTransfer(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				Usb_DfuSetState(dfu, STATE_DFU_IDLE);
			}
		default:

			/* Unsupported command. Stall the end point. */
			dfu->curr_state = STATE_DFU_ERROR;
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_IN);
			break;
	}
}

/*****************************************************************************/
/**
* This function is class handler for Mass storage and is called when
* Setup packet received is for Class request(not Standard Device request)
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void Usb_StorageClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	switch(SetupData->bRequest) {
	case USB_CLASSREQ_MASS_STORAGE_RESET:
		/* For Control transfers, Status Phase is handled by driver */

		EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
		break;

	case USB_CLASSREQ_GET_MAX_LUN:
		EpBufferSend(InstancePtr->PrivateData, 0, &MaxLUN, 1);
		break;

	default:
		/*
		 * Unsupported command. Stall the end point.
		 */
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
		break;
	}
}

/****************************************************************************/
/**
* This function is class handler for HID and is called when
* Setup packet received is for Class request(not a Standard Device request)
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void Usb_KeyboardClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	uint16_t ReplyLen = 0;

	switch (SetupData->bRequest) {
		case  SET_IDLE_REQUEST:
			EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);
			break;

		case GET_REPORT_REQUEST:
			/* send an empty report */
			ReplyLen = SetupData->wLength > REPORT_LENGTH ?
				(u16)REPORT_LENGTH : SetupData->wLength;
			memset(ClassData, 0x0, ReplyLen);

			EpBufferSend(InstancePtr->PrivateData, KEYBOARD_EP,
					(u8 *)ClassData, ReplyLen);
			break;

		default:
			xil_printf("Keyboard Unknown class request 0x%x\r\n",
					SetupData->bRequest);
			Ep0StallRestart(InstancePtr->PrivateData);
	}
}

/****************************************************************************/
/**
* This function is class request handler for composite device
* Setup packet received is for Class request(not a Standard Device request)
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);

	switch (SetupData->wIndex & 0xff) {
		case DFU_INTF:
			Usb_DfuClassReq(InstancePtr, SetupData);
			break;

		case AUDIO_INTF:
		case AUDIO_INTF_OUT:
		case AUDIO_INTF_IN:
			Usb_AudioClassReq(InstancePtr, SetupData);
			break;

		case STORAGE_INTF:
			Usb_StorageClassReq(InstancePtr, SetupData);
			break;

		case KEYBOARD_INTF:
			Usb_KeyboardClassReq(InstancePtr, SetupData);
			break;

		default:
			xil_printf("Invalid class request\r\n");
			break;
	}
}

/*****************************************************************************/
/**
* This function handles Reduced Block Command (RBC) requests from the host.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller.
* @param	f is a pointer to storage interface
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void ParseCBW(struct Usb_DevData *InstancePtr, struct storage_if *f)
{
	u32	Offset;
	u8	Index;

	switch (f->cbw.CBWCB[0]) {
	case USB_RBC_INQUIRY:
		f->phase = USB_EP_STATE_DATA_IN;
		Index = (IsSuperSpeed(InstancePtr) != XST_SUCCESS) ? 0 : 1;
		f->currTrans.Ptr = (u8 *) &scsiInquiry[Index];
		f->currTrans.Length = sizeof(scsiInquiry[Index]);
		break;

	case USB_UFI_GET_CAP_LIST: {
		SCSI_CAP_LIST	*CapList;

		CapList = (SCSI_CAP_LIST *) txBuffer;
		CapList->listLength	= 8;
		CapList->descCode	= 3;
		CapList->numBlocks	= htonl(STORAGE_NUM_BLOCKS);
		CapList->blockLength = htons(STORAGE_BLOCK_SIZE);
		f->phase = USB_EP_STATE_DATA_IN;
		f->currTrans.Ptr = txBuffer;
		f->currTrans.Length =  sizeof(SCSI_CAP_LIST);
	}
		break;

	case USB_RBC_READ_CAP: {
		SCSI_READ_CAPACITY	*Cap;

		Cap = (SCSI_READ_CAPACITY *) txBuffer;
		Cap->numBlocks = htonl(STORAGE_NUM_BLOCKS - 1);
		Cap->blockSize = htonl(STORAGE_BLOCK_SIZE);
		f->phase = USB_EP_STATE_DATA_IN;
		f->currTrans.Ptr = txBuffer;
		f->currTrans.Length = sizeof(SCSI_READ_CAPACITY);
	}
		break;

	case USB_RBC_READ:
		Offset = htonl(((SCSI_READ_WRITE *) &f->cbw.CBWCB)-> block) *
			STORAGE_BLOCK_SIZE;
		f->phase = USB_EP_STATE_DATA_IN;
		f->currTrans.Ptr = f->disk + Offset;
		f->currTrans.Length = htons(((SCSI_READ_WRITE *) &f->cbw.CBWCB)->
				length) * STORAGE_BLOCK_SIZE;
		break;

	case USB_RBC_MODE_SENSE:
		memcpy(txBuffer, "\003\000\000\000", 4);
		f->phase = USB_EP_STATE_DATA_IN;
		f->currTrans.Ptr = txBuffer;
		f->currTrans.Length = 4;
		break;

	case USB_RBC_MODE_SELECT:
		f->phase = USB_EP_STATE_DATA_OUT;
		f->currTrans.Ptr = txBuffer;
		f->currTrans.Length = 24;
		break;

	case USB_RBC_TEST_UNIT_READY:
		f->phase = USB_EP_STATE_STATUS;
		f->currTrans.Length = 0;
		break;

	case USB_RBC_MEDIUM_REMOVAL:
		f->phase = USB_EP_STATE_STATUS;
		f->currTrans.Length = 0;
		break;

	case USB_RBC_VERIFY:
		f->phase = USB_EP_STATE_STATUS;
		f->currTrans.Length = 0;
		break;

	case USB_RBC_WRITE:
		Offset = htonl(((SCSI_READ_WRITE *) &f->cbw.CBWCB)-> block) *
			STORAGE_BLOCK_SIZE;
		f->phase = USB_EP_STATE_DATA_OUT;
		f->currTrans.Ptr = f->disk + Offset;
		f->currTrans.Length = htons(((SCSI_READ_WRITE *) &f->cbw.CBWCB)->length) *
			STORAGE_BLOCK_SIZE;
		break;

	case USB_RBC_STARTSTOP_UNIT: {
		u8 immed;

		immed = ((SCSI_START_STOP *) &f->cbw.CBWCB)->immed;
		/* If the immediate bit is 0 we are supposed to send
		 * a success status.
		 */
		if (0 == (immed & 0x01)) {
			f->phase = USB_EP_STATE_STATUS;
			f->currTrans.Length = 0;
		}
		break;
	}

	case USB_SYNC_SCSI:
		f->phase = USB_EP_STATE_STATUS;
		f->currTrans.Length = 0;
		break;
	}
}


/****************************************************************************/
/**
* This function is used to send SCSI Command Status Wrapper to Host.
*
* @param	InstancePtr is pointer to Usb_DevData instance.
* @param	Length is the data residue.
*
* @return	None
*
* @note
*
*****************************************************************************/
void SendCSW(struct Usb_DevData *InstancePtr, struct storage_if *f, u32 Length)
{
	f->csw.dCSWSignature = 0x53425355;
	f->csw.dCSWTag = f->cbw.dCBWTag;
	f->csw.dCSWDataResidue = Length;
	f->csw.bCSWStatus = 0;
	f->phase = USB_EP_STATE_STATUS;

	EpBufferSend(InstancePtr->PrivateData, STORAGE_EP, (void *) &f->csw, 13);
}

/****************************************************************************/
/**
* This task implements keyboard functionality. task will get
* host and act accordingly
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvKeyboardTask(void *pvParameters)
{
	u16 MaxPktSize;
	u8 SendKey = 1;
	u8 NoKeyData[8]= {0,0,0,0,0,0,0,0};
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev= (struct composite_dev *)(ch9_ptr->data_ptr);
	struct keyboard_if *f = &(dev->f_keyboard);

	if ((InstancePtr->Speed == USB_SPEED_SUPER) ||
			(InstancePtr->Speed == USB_SPEED_HIGH) )
		MaxPktSize = 0x400;
	else
		MaxPktSize = 0x40;

	f->xSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(f->xSemaphore);

	EpEnable(InstancePtr->PrivateData, KEYBOARD_EP, USB_EP_DIR_IN,
			MaxPktSize, USB_EP_TYPE_INTERRUPT);

	while (1) {

		xSemaphoreTake(f->xSemaphore, portMAX_DELAY);
		if (SendKey) {
			static char KeyData[8] = {2,0,0,0,0,0,0,0};

			/* get key from serial and send key */
			KeyData[2] = (inbyte() - ('a' - 4));
			EpBufferSend(InstancePtr->PrivateData, KEYBOARD_EP,
					(u8 *)&KeyData[0], 8);
			SendKey = 0;
		} else {
			/* send key release */
			EpBufferSend(InstancePtr->PrivateData, KEYBOARD_EP,
					(u8 *)&NoKeyData[0], 8);
			SendKey = 1;
		}
	}
}

/****************************************************************************/
/**
* This task implements SCSI command functionality. task will get command from
* host and act accordingly
*
* @param	pvParameters private parameters.
*
* @note		None.
*
*****************************************************************************/
void prvSCSITask( void *pvParameters)
{
	s32 RetVal;
	u16 MaxPktSize;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)ch9_ptr->data_ptr;
	struct storage_if *f = &dev->f_storage;

	if(InstancePtr->Speed == USB_SPEED_SUPER)
		MaxPktSize = 1024;
	else
		MaxPktSize = 512;

	f->xSemaphore = xSemaphoreCreateBinary();

	/* Endpoint enables - not needed for Control EP */
	RetVal = EpEnable(InstancePtr->PrivateData, STORAGE_EP, USB_EP_DIR_IN,
			MaxPktSize, USB_EP_TYPE_BULK);
	if (RetVal != XST_SUCCESS) {
		xil_printf("failed to enable BULK IN Ep\r\n");
	}

	RetVal = EpEnable(InstancePtr->PrivateData, STORAGE_EP, USB_EP_DIR_OUT,
			MaxPktSize, USB_EP_TYPE_BULK);
	if (RetVal != XST_SUCCESS) {
		xil_printf("failed to enable BULK OUT Ep\r\n");
	}

	xSemaphoreGive(f->xSemaphore);

	while (1) {

		xSemaphoreTake(f->xSemaphore, portMAX_DELAY);
		f->phase = USB_EP_STATE_COMMAND;
		EpBufferRecv(InstancePtr->PrivateData, STORAGE_EP,
				(u8*)&(f->cbw), sizeof(f->cbw));

		xSemaphoreTake(f->xSemaphore, portMAX_DELAY);
		ParseCBW(InstancePtr, f);

		if (f->phase == USB_EP_STATE_DATA_IN) {
			EpBufferSend(InstancePtr->PrivateData, STORAGE_EP,
					f->currTrans.Ptr, f->currTrans.Length);

		} else if (f->phase == USB_EP_STATE_DATA_OUT) {
			EpBufferRecv(InstancePtr->PrivateData, STORAGE_EP,
					f->currTrans.Ptr, f->currTrans.Length);
		} else {
			xSemaphoreGive(f->xSemaphore);
		}

		xSemaphoreTake(f->xSemaphore, portMAX_DELAY);
		SendCSW(InstancePtr, f, 0);
	}
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
void prvRecordTask( void *pvParameters)
{
	u32 Size;
	u16 MaxPktSize = 1024;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &dev->f_audio;

	f->index = 0;
	f->residue = 0;
	f->firstpkt = 1;

	f->xSemaphoreRecord = xSemaphoreCreateBinary();

	SetEpInterval(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_IN, AUDIO_INTERVAL);

	/* Endpoint enables - not needed for Control EP */
	EpEnable(InstancePtr->PrivateData, ISO_EP, USB_EP_DIR_IN,
			MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);

	while(1) {

		xSemaphoreTake(f->xSemaphoreRecord, portMAX_DELAY);

		Size = f->packetsize;
		f->residue += f->packetresidue;

		if ((f->residue / f->interval) >= f->framesize) {
			Size += f->framesize;
			f->residue -= f->framesize * f->interval;
		}

		/* Buffer is completed, retransmitting the same file data */
		if ((f->index + Size) > f->disksize)
			f->index = 0;

		if (EpBufferSend(InstancePtr->PrivateData, ISO_EP,
					f->disk + f->index, Size) == XST_SUCCESS) {
			f->index += Size;

			if (f->firstpkt) {
				Size = f->packetsize;
				f->residue += f->packetresidue;

				if ((f->residue / f->interval) >= f->framesize) {
					Size += f->framesize;
					f->residue -= f->framesize * f->interval;
				}

				/* Buffer is completed, retransmitting the same file data */
				if ((f->index + Size) > f->disksize)
					f->index = 0;
				else
					f->index += Size;

				f->firstpkt = 0;
			}
		}
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
void prvPlayBackTask( void *pvParameters)
{
	u16 MaxPktSize = 1024;
	struct Usb_DevData *InstancePtr = pvParameters;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct composite_dev *dev = (struct composite_dev *)(ch9_ptr->data_ptr);
	struct audio_if *f = &dev->f_audio;

	f->xSemaphorePlay = xSemaphoreCreateBinary();

	f->index = 0;
	f->residue = 0;
	f->firstpkt = 1;
	f->bytesRecv = 0;

	SetEpInterval(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_OUT, AUDIO_INTERVAL);

	/* Endpoint enables - not needed for Control EP */
	EpEnable(InstancePtr->PrivateData, ISO_EP,
			USB_EP_DIR_OUT, MaxPktSize,
			USB_EP_TYPE_ISOCHRONOUS);

	while (1) {

		xSemaphoreTake(f->xSemaphorePlay, portMAX_DELAY);
		if ((f->index + f->bytesRecv) > f->disksize)
			f->index = 0;

		memcpy(f->disk + f->index, BufferPtrTemp, f->bytesRecv);
		f->index += f->bytesRecv;

		EpBufferRecv(InstancePtr->PrivateData, ISO_EP,
				BufferPtrTemp, 1024);
	}
}
