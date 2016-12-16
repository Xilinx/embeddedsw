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
/*****************************************************************************/
/**
 * @file xusbpsu_dfu_class.c
 *
 * This file contains the implementation of the DFU class specific code
 * for the example.
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ---------------------------------------------------------
 * 1.0	 vak  30/11/16 Addded DFU support
 *</pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xusbpsu_ch9.h"
#include "xusbpsu_ch9_dfu.h"
#include "xusbpsu_dfu.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define MAX_STRLEN	128

static inline void XUsbPsu_DfuWaitForReset(struct dfu_if *DFU)
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

u8 DFUVirtFlash[0x10000000];
u8 DetachCounter = 0;

void XUsbPsu_DfuDisconnect(struct XUsbPsu *InstancePtr);
void XUsbPsu_DfuReset(struct XUsbPsu *InstancePtr);


/* Create a DFU instance structure*/
struct dfu_if DFU;

/* Device Descriptors */
USB_STD_DEV_DESC __attribute__ ((aligned(16)))
				DFUdeviceDesc[] = {
	{/* USB 2.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		(0x0200),			/* bcdUSB 2.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x40,				/* bMaxPackedSize0 */
		(0x03FD),			/* idVendor */
		(0x0500),			/* idProduct */
		(0x0100),			/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	},
	{/* USB 3.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		(0x0300),			/* bcdUSB 3.0 */
		0x00,				/* bDeviceClass */
		0x00,				/* bDeviceSubClass */
		0x00,				/* bDeviceProtocol */
		0x09,				/* bMaxPackedSize0 */
		(0x0525),			/* idVendor */
		(0xA4A5),			/* idProduct */
		(0x0404),			/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	}
};

/* Configuration Descriptors */
DFU_USB30_CONFIG __attribute__ ((aligned(16)))
					DFUconfig3 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(DFU_USB30_CONFIG),	/* wTotalLength */
		 0x01,				/* bNumInterfaces */
		 0x01,				/* bConfigurationValue */
		 0x00,				/* iConfiguration */
		 0xc0,				/* bmAttribute */
		 0x00},				/* bMaxPower  */


		/*** DFU INTERFACE DESCRIPTOR ***/
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_INTERFACE_CFG_DESC,		/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndPoints */
		0xFE,				/* bInterfaceClass DFU application specific class code */
		0x01,				/* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02,				/* bInterfaceProtocol DFU mode protocol*/
		0x04},				/* iInterface DFU string descriptor*/

		/**** DFU functional discriptor ****/
		{sizeof(USB_DFU_FUNC_DESC),	/* bLength*/
		DFUFUNC_DESCR,			/* bDescriptorType DFU functional descriptor type */
		0x3,				/* bmAttributes Device is only download/upload capable */
		8192,				/*wDetatchTimeOut 8192 ms*/
		DFU_MAX_TRANSFER,		/*wTransferSize DFU block size 512*/
		0x0110				/*bcdDfuVersion 1.1 */
		}
};

DFU_USB_CONFIG __attribute__ ((aligned(16))) DFUconfig2 = {
		/* Std Config */
		{sizeof(USB_STD_CFG_DESC),	/* bLength */
		 USB_CONFIG_DESC,		/* bDescriptorType */
		 sizeof(DFU_USB_CONFIG),	/* wTotalLength */
		 0x01,				/* bNumInterfaces */
		 0x01,				/* bConfigurationValue */
		 0x00,				/* iConfiguration */
		 0xc0,				/* bmAttribute */
		 0x00},				/* bMaxPower  */


		/*** DFU INTERFACE DESCRIPTOR ***/
		{sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_INTERFACE_CFG_DESC,		/* bDescriptorType */
		0x00,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndPoints */
		0xFE,				/* bInterfaceClass DFU application specific class code */
		0x01,				/* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02,				/* bInterfaceProtocol DFU mode protocol*/
		0x03},				/* iInterface DFU string descriptor*/

		/**** DFU functional discriptor ****/
		{sizeof(USB_DFU_FUNC_DESC), /* bLength*/
		DFUFUNC_DESCR, /* bDescriptorType DFU functional descriptor type */
		0x3, /* bmAttributes Device is only download/upload capable */
		8192, /*wDetatchTimeOut 8192 ms*/
		DFU_MAX_TRANSFER, /*wTransferSize DFU block size 512*/
		0x0110 /*bcdDfuVersion 1.1 */
		}
	};

/* String Descriptors */
static char DFUStringList[2][6][MAX_STRLEN] = {
			{
				"UNUSED",
				"XILINX INC",
				"DFU 2.0 emulation v 1.1",
				"2A49876D9CC1AA4",
				"DEFAULT DFU ITERFACE",
				"7ABC7ABC7ABC7ABC7ABC7ABC"
			},
			{
				"UNUSED",
				"XILINX INC",
				"DFU 3.0 emulation v 1.1",
				"2A49876D9CC1AA4",
				"DEFAULT DFU ITERFACE",
				"7ABC7ABC7ABC7ABC7ABC7ABC"
			},
	};

/*****************************************************************************/
/**
* This function initializes CH9 data in DFU interface.
*
* @param	Pointer to the DFU data that has to be initialized.
*
* @return
*		- XST_SUCCESS if the function is successful.
*
* @note		None.
*
******************************************************************************/
void dfu_init_data(USBCH9_DATA * data)
{
	/* hook the ch9 function call backs */
	data->ch9_func.XUsbPsu_Ch9SetupDevDescReply =
							XUsbPsu_Ch9SetupDevDescReply;
	data->ch9_func.XUsbPsu_Ch9SetupCfgDescReply =
							XUsbPsu_Ch9SetupCfgDescReply;
	data->ch9_func.XUsbPsu_Ch9SetupBosDescReply =
							XUsbPsu_Ch9SetupBosDescReply;
	data->ch9_func.XUsbPsu_Ch9SetupStrDescReply =
							XUsbPsu_Ch9SetupStrDescReply;
	data->ch9_func.XUsbPsu_SetConfiguration =
							XUsbPsu_SetConfiguration;
	data->ch9_func.XUsbPsu_SetConfigurationApp =
							XUsbPsu_SetConfigurationApp;
	/* hook the set interface handler */
	data->ch9_func.XUsbPsu_SetInterfaceHandler =
							XUsbPsu_DfuSetIntf;
	/* hook up storage class handler */
	data->ch9_func.XUsbPsu_ClassReq = XUsbPsu_DfuClassReq;

	/* Set the DFU address for call back */
	data->data_ptr = (void *)&DFU;

};

/*****************************************************************************/
/**
* This function handles a DFU interface initialization part .
*
* @param	DFU is a pointer to DFU instance of the controller
* @param	UsbInstancePtr is a pointer to XUsbPsu instance of the controller.
*
* @return
*		- XST_SUCCESS if the function is successful.
*
* @note		None.
*
******************************************************************************/
int dfu_if_init(struct XUsbPsu *UsbInstance)
{

	/* hook up chapter9 handler */
	XUsbPsu_set_ch9handler(UsbInstance, XUsbPsu_Ch9Handler);

	/* Set the disconnect event handler */
	XUsbPsu_set_disconnect(UsbInstance, XUsbPsu_DfuDisconnect);

	/* Set the reset event handler */
	XUsbPsu_set_rsthandler(UsbInstance, XUsbPsu_DfuReset);

	/* Initialize a DFU data structure */
	dfu_init_data(&DFU.dfu_data);

	/* Set driver data */
	XUsbPsu_set_drvdata(UsbInstance, &DFU.dfu_data);

	/* Initialize the DFU to appIdle */
	DFU.InstancePtr = UsbInstance;

	/* Set DFU state to appIdle */
	dfu_set_state(&DFU, STATE_APP_IDLE);

	/* Set the DFU descriptor pointers, so we can use it when in DFU mode */
	DFU.DFUdeviceDesc = DFUdeviceDesc;
	DFU.DFUconfig2 = &DFUconfig2;
	DFU.DFUconfig3 = &DFUconfig3;
	DFU.DFUStringList2 = (char *)DFUStringList;
	DFU.DFUStringList3 = (char *)(DFUStringList + 1);
	DFU.total_transfers = 0;
	DFU.total_bytes_dnloaded = 0;
	DFU.total_bytes_uploaded = 0;

	return XST_SUCCESS;
}

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
int dfu_set_state(struct dfu_if *DFU, int dfu_state)
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
			XUsbPsu_DfuWaitForReset(DFU);

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
			XUsbPsu_DfuWaitForReset(DFU);

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
		XUsbPsu_EpSetStall(DFU->InstancePtr, 0, XUSBPSU_EP_DIR_IN);
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
void dfu_set_dwloadstate(struct dfu_if *DFU, char *status)
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
int dfu_get_status(struct dfu_if *DFU, char *status)
{

	/* update the download state */
	dfu_set_dwloadstate(DFU, status);

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
* @param	InstancePtr is a pointer to USB instance of the controller
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_DfuDisconnect(struct XUsbPsu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
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
* @param	InstancePtr is a pointer to USB instance of the controller
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_DfuReset(struct XUsbPsu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
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
* @param	InstancePtr is a pointer to USB instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_DfuSetIntf(struct XUsbPsu *InstancePtr,  SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);

	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
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
		dfu_set_state(DFU, STATE_DFU_IDLE);
	} else {
		/* Entering APP_IDLE state */
		dfu_set_state(DFU, STATE_APP_IDLE);
	}
}

/*****************************************************************************/
/**
* This function handles DFU heart and soul of DFU state machine.
*
* @param	InstancePtr is a pointer to USB instance of the controller
* @param	SetupData is a pointer to setup token of control transfer
*
* @return
*		None.
*
* @note		None.
*
******************************************************************************/
void XUsbPsu_DfuClassReq(struct XUsbPsu *InstancePtr, SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);
	int txBytesLeft;
	int rxBytesLeft;
	int result = -1;

	static u8 DFUReply[6] ALIGNMENT_CACHELINE;
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)XUsbPsu_get_drvdata(InstancePtr);
	struct dfu_if *dfu = (struct dfu_if *)(ch9_ptr->data_ptr);

	switch (SetupData->bRequest) {

	case DFU_DETACH:
#ifdef DFU_DEBUG
		xil_printf("Got DFU_DETACH\n");
#endif
		dfu_set_state(dfu, STATE_APP_DETACH);
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
			result = XUsbPsu_EpBufferRecv(InstancePtr, 0,
					&DFUVirtFlash[dfu->
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
				result = XUsbPsu_EpBufferSend(InstancePtr, 0,
						&DFUVirtFlash[dfu->
							total_bytes_uploaded],
							txBytesLeft);
				dfu->total_bytes_uploaded += txBytesLeft;
		} else {
				/* Send a short packet */
				result = XUsbPsu_EpBufferSend(InstancePtr, 0,
						&DFUVirtFlash[dfu->
						total_bytes_uploaded], 0);
				dfu->total_bytes_uploaded = 0;
				dfu->total_transfers = 0;
		}
		dfu->curr_state = STATE_DFU_IDLE;
		break;
	case DFU_GETSTATUS:
		dfu_get_status(dfu, (char *)DFUReply);
		XUsbPsu_EpBufferSend(InstancePtr,
				0, DFUReply, SetupData->wLength);
		break;
	case DFU_CLRSTATUS:
		if (dfu->curr_state == STATE_DFU_ERROR) {
			XUsbPsu_EpClearStall(InstancePtr, 0, XUSBPSU_EP_DIR_IN);
			dfu_set_state(dfu, STATE_DFU_IDLE);
		}
		break;
	case DFU_GETSTATE:
		DFUReply[0] = (u8)dfu->curr_state;
		XUsbPsu_EpBufferSend(InstancePtr,
				0, DFUReply, SetupData->wLength);
		break;

	case DFU_ABORT:
		/* Stop current download transfer */
		if (dfu->got_dnload_rqst == 1) {
			XUsbPsu_StopTransfer(InstancePtr,
						0, XUSBPSU_EP_DIR_OUT);
			dfu_set_state(dfu, STATE_DFU_IDLE);
		}
	default:

		/* Unsupported command. Stall the end point. */
		dfu->curr_state = STATE_DFU_ERROR;
		XUsbPsu_EpSetStall(InstancePtr, 0, XUSBPSU_EP_DIR_IN);
		break;
	}
}
