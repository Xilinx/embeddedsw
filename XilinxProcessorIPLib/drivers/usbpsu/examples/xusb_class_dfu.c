/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_dfu.c
 *
 * This file contains the implementation of the DFU specific class code for
 * the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.0	 vak  30/11/16 Added DFU support
 * 1.4	 BK   12/01/18 Renamed the file to be in sync with usb common code
 *		       changes for all USB IPs
 * 1.5	 vak  13/02/19 Added support for versal
 *
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"	/* XPAR parameters */
#include "xusb_class_dfu.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
static inline void Usb_DfuWaitForReset(struct dfu_if *DFU)
{

	/* This bit would be cleared when reset happens*/
	DFU->dfu_wait_for_interrupt = 1;
	while (DFU->dfu_wait_for_interrupt == 0) {
		/* Wait for an reset event */
		;
	}
}

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern u8 VirtFlash[];
u8 DetachCounter = 0;

/*****************************************************************************/
/**
* This function handles setting of DFU state.
*
* @param	DFU is a pointer to DFU instance of the controller
* @param	dfu_state is a value of the DFU state to be set
*
* @return
*		- XST_SUCCESS if the function is successful.
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
		/*
		 * Unsupported command. Stall the end point.
		 */
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
* @return
*		- XST_SUCCESS if the function is successful.
*
* @note		None.
*
******************************************************************************/
s32 USB_DfuGetStatus(struct dfu_if *DFU, u8 *status)
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

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function handles DFU disconnect, called from driver.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_DfuDisconnect(struct Usb_DevData *InstancePtr)
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
* This function handles DFU reset, called from driver.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_DfuReset(struct Usb_DevData *InstancePtr)
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
* This function handles DFU set interface.
*
* @param	InstancePtr is a pointer to Usb_DevData instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Usb_DfuSetIntf(struct Usb_DevData *InstancePtr,  SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);

	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct dfu_if *DFU = (struct dfu_if *)(ch9_ptr->data_ptr);


	/* Setting the alternate setting requested */
	DFU->current_inf = SetupData->wValue;
	if ((DFU->current_inf >= DFU_ALT_SETTING) ||
			(DFU->runtime_to_dfu == 1)) {

		/* Clear the flag , before entering into
		 * DFU mode from runtime mode
		 */
		if (DFU->runtime_to_dfu == 1)
			DFU->runtime_to_dfu = 0;

		/* Entering DFU_IDLE state */
		Usb_DfuSetState(DFU, STATE_DFU_IDLE);
	} else {
		/* Entering APP_IDLE state */
		Usb_DfuSetState(DFU, STATE_APP_IDLE);
	}
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
void Usb_DfuClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
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
	struct dfu_if *dfu = (struct dfu_if *)(ch9_ptr->data_ptr);

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
					&VirtFlash[dfu->
						total_bytes_dnloaded],
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
						&VirtFlash[dfu->
							total_bytes_uploaded],
							txBytesLeft);
				dfu->total_bytes_uploaded += txBytesLeft;
		} else {
				/* Send a short packet */
				result = EpBufferSend(InstancePtr->PrivateData, 0,
						&VirtFlash[dfu->
						total_bytes_uploaded], 0);
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
