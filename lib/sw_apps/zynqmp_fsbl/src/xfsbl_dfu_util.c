/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_dfu_util.c
*
* This file contains definitions of the DFU specific functions to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/01/17 First release
* 2.0   bvikram  09/30/20 Fix USB boot mode
* 3.0   bvikram  03/24/21 Fix compilation warnings
* 4.0   bvikram  06/09/21 Add support for delayed enumeration of DFU device
*
* </pre>
*
*****************************************************************************/

#include "xfsbl_hw.h"

#ifdef XFSBL_USB
#include "xfsbl_dfu_util.h"
#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xfsbl_usb.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/
#define DFUFUNC_DESCR				0x21U    /* DFU Functional Desc */
#define USB_MODES_NUM				2U
#define STRING_DESCRIPTORS_NUM		6U

/**************************** Type Definitions *******************************/
struct XFsblPs_DfuIf DfuObj;
extern u32 DownloadDone;
extern u8* DfuVirtFlash;
extern struct Usb_DevData UsbInstance;

/* Initialize a DFU data structure */
XFsbl_UsbCh9_Data Dfu_data = {
        .Ch9_func = {
                .XFsblPs_Ch9SetupDevDescReply = XFsbl_Ch9SetupDevDescReply,
                .XFsblPs_Ch9SetupCfgDescReply = XFsbl_Ch9SetupCfgDescReply,
                .XFsblPs_Ch9SetupBosDescReply = XFsbl_Ch9SetupBosDescReply,
                .XFsblPs_Ch9SetupStrDescReply = XFsbl_Ch9SetupStrDescReply,
                .XFsblPs_SetConfiguration = XFsbl_SetConfiguration,
                /* Hook the set interface handler */
                .XFsblPs_SetInterfaceHandler = XFsbl_DfuSetIntf,
                /* Hook up storage class handler */
                .XFsblPs_ClassReq = XFsbl_DfuClassReq,
                /* Set the DFU address for call back */
        },
        .Data_ptr = (void *)&DfuObj,
};

/* Device Descriptors */
static XFsblPs_UsbStdDevDesc __attribute__ ((aligned(16))) DDesc = {
	/* USB 2.0 */
		(u8)sizeof(XFsblPs_UsbStdDevDesc), /* bLength */
		USB_DEVICE_DESC, /* bDescriptorType */
		(0x0200U), /* bcdUSB 2.0 */
		0x00U, /* bDeviceClass */
		0x00U, /* bDeviceSubClass */
		0x00U, /* bDeviceProtocol */
		0x40U, /* bMaxPackedSize0 */
		(0x03FDU), /* idVendor */
		(0x0050U), /* idProduct */
		(0x0100U), /* bcdDevice */
		0x01U, /* iManufacturer */
		0x02U, /* iProduct */
		0x03U, /* iSerialNumber */
		0x01U /* bNumConfigurations */
	};

/* String Descriptors */
static char* StringList[STRING_DESCRIPTORS_NUM] = {
		"UNUSED",
		"XILINX INC",
		"DFU 2.0 emulation v 1.1",
		"2A49876D9CC1AA4",
		"Xilinx DFU Downloader",
		"7ABC7ABC7ABC7ABC7ABC7ABC"
	};

static XFsblPs_UsbConfig __attribute__ ((aligned(16))) Config2 = {
	/* Std Config */
	{	(u8)sizeof(XFsblPs_UsbStdCfgDesc), /* bLength */
		USB_CONFIG_DESC, /* bDescriptorType */
		(u16)sizeof(XFsblPs_UsbConfig), /* wTotalLength */
		0x01U, /* bNumInterfaces */
		0x01U, /* bConfigurationValue */
		0x00U, /* iConfiguration */
		0xC0U, /* bmAttribute */
		0x00U}, /* bMaxPower  */

	/* Interface Config */
	{	(u8)sizeof(XFsblPs_UsbStdIfDesc), /* bLength */
		USB_INTERFACE_CFG_DESC, /* bDescriptorType */
		0x00U, /* bInterfaceNumber */
		0x00U, /* bAlternateSetting */
		0x02U, /* bNumEndPoints */
		0xFFU, /* bInterfaceClass */
		0xFFU, /* bInterfaceSubClass */
		0xFFU, /* bInterfaceProtocol */
		0x04U}, /* iInterface */

	/* Bulk In Endpoint Config */
	{	(u8)sizeof(XFsblPs_UsbStdEpDesc), /* bLength */
		USB_ENDPOINT_CFG_DESC, /* bDescriptorType */
		0x81U, /* bEndpointAddress */
		0x02U, /* bmAttribute  */
		0x00U, /* wMaxPacketSize - LSB */
		0x02U, /* wMaxPacketSize - MSB */
		0x00U}, /* bInterval */

	/* Bulk Out Endpoint Config */
	{	(u8)sizeof(XFsblPs_UsbStdEpDesc), /* bLength */
		USB_ENDPOINT_CFG_DESC, /* bDescriptorType */
		0x01U, /* bEndpointAddress */
		0x02U, /* bmAttribute  */
		0x00U, /* wMaxPacketSize - LSB */
		0x02U, /* wMaxPacketSize - MSB */
		0x00U}, /* bInterval */

	/*** DFU Interface Descriptor ***/
	{	(u8)sizeof(XFsblPs_UsbStdIfDesc), /* bLength */
		USB_INTERFACE_CFG_DESC, /* bDescriptorType */
		0x00U, /* bInterfaceNumber */
		0x01U, /* bAlternateSetting */
		0x00U, /* bNumEndPoints */
		0xFEU, /* bInterfaceClass DFU application specific class code */
		0x01U, /* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02U, /* bInterfaceProtocol DFU mode protocol*/
		0x04U}, /* iInterface DFU string descriptor*/

	/*****DFU functional descriptor*****/
	{	(u8)sizeof(XFsblPs_UsbDfuFuncDesc), /* bLength*/
		DFUFUNC_DESCR, /* bDescriptorType DFU functional descriptor type */
		0x3U, /* bmAttributes Device is only download capable bitCanDnload */
		8192U, /*wDetatchTimeOut 8192 ms*/
		DFU_MAX_TRANSFER, /*wTransferSize DFU block size 1024*/
		0x0110U /*bcdDfuVersion 1.1 */
	}
};

/*******************************************************************************
 *
 * This function waits for DFU reset.
 *
 * @param  None
 *
 * @return None
 *
 * @note   None.
 *
 ******************************************************************************/
static void XFsbl_DfuWaitForReset(void)
{

	/* This bit would be cleared when reset happens*/
	DfuObj.DfuWaitForInterrupt = 1U;
	dmb();
	while (DfuObj.DfuWaitForInterrupt == 0U) {
		;
	}
}

/*********************************************************************************
 *
 * This function returns a string descriptor for the given index.
 *
 * @param	BufPtr is a  pointer to the buffer that is to be filled with
 *		the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 * @param	Index is the index of the string for which the descriptor
 *		is requested.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *		0 on error.
 * @note		None.
 *
 ******************************************************************************/
u32 XFsbl_Ch9SetupStrDescReply(u8 *BufPtr, u32 BufferLen, u8 Index)
{
	u32 LoopVar;
	char* String;
	u32 StringLen;
	u32 DescLen;
	XFsblPs_UsbStdStringDesc StringDesc;

	if (Index >= STRING_DESCRIPTORS_NUM) {
		DescLen = 0U;
		goto END;
	}

	if (BufPtr == NULL) {
		DescLen = 0U;
		goto END;
	}

	String = StringList[Index];
	StringLen = strlen(String);

	/* Index 0 is LangId which is special as we can not represent
	 * the string required in the table above.Therefore we handle
	 * index 0 as a special case. */
	if (0U == Index) {
		StringDesc.Length = 4U;
		StringDesc.DescriptorType = 0x03U;
		StringDesc.LangId[0] = 0x0409U;
		for(Index=1U; Index < STRING_SIZE; ++Index) {
			StringDesc.LangId[Index] = 0U;
		}
	}
	/* All other strings can be pulled from the table above.*/
	else {
		StringDesc.Length = (u8)((StringLen * 2U) + 2U);
		StringDesc.DescriptorType = 0x03U;

		for(LoopVar = 0U; LoopVar < StringLen; ++LoopVar) {
			StringDesc.LangId[LoopVar] = (u16) String[LoopVar];
		}
		for(;LoopVar < STRING_SIZE; ++LoopVar) {
			StringDesc.LangId[LoopVar] = 0U;
		}
	}
	DescLen = StringDesc.Length;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufferLen) {
		DescLen = 0U;
		goto END;
	}

	(void)XFsbl_MemCpy(BufPtr, &StringDesc, DescLen);

END:
	return DescLen;
}

/*****************************************************************************
 *
 * This function returns the device descriptor for the device.
 *
 * @param	BufPtr is pointer to the buffer that is to be filled
 *		with the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *		0 on error.
 * @note		None.
 *
 ******************************************************************************/
u32 XFsbl_Ch9SetupDevDescReply(u8 *BufPtr, u32 BufferLen)
{
	u32 DevDescLength;

	/* Check buffer pointer is there and buffer is big enough. */
	if (BufPtr == NULL) {
		DevDescLength = 0U;
		goto END;
	}

	DevDescLength = sizeof(XFsblPs_UsbStdDevDesc);

	if (BufferLen < DevDescLength) {
		DevDescLength = 0U;
		goto END;
	}

	(void)memcpy(BufPtr, &DDesc, DevDescLength);

END:
	return DevDescLength;
}

/*****************************************************************************
 *
 * This function returns the configuration descriptor for the device.
 *
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *		the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *		0 on error.
 * @note		None.
 *
 ******************************************************************************/
u32 XFsbl_Ch9SetupCfgDescReply(u8 *BufPtr, u32 BufferLen)
{
	u8 *Config;
	u32 CfgDescLen;

	/* Check buffer pointer is OK and buffer is big enough. */
	if (BufPtr == NULL) {
		CfgDescLen = 0U;
		goto END;
	}

	Config = (u8 *)&Config2;
	CfgDescLen = sizeof(XFsblPs_UsbConfig);
	if (BufferLen < CfgDescLen) {
		CfgDescLen = 0U;
		goto END;
	}

	(void)XFsbl_MemCpy(BufPtr, Config, CfgDescLen);
END:
	return CfgDescLen;
}

/*****************************************************************************/
/**
 *
 * This function returns the BOS descriptor for the device.
 *
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *		the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *		0 on error.
 * @note		None.
 *
 ******************************************************************************/
u32 XFsbl_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufferLen)
{
	u32 UsbBosDescLen;
	/* Check buffer pointer is OK and buffer is big enough. */
	if (BufPtr == NULL) {
		UsbBosDescLen = 0U;
		goto END;
	}

	UsbBosDescLen = sizeof(XFsblPs_UsbBosDesc);

	if (BufferLen < UsbBosDescLen) {
		UsbBosDescLen = 0U;
		goto END;
	}

	XFsblPs_UsbBosDesc __attribute__ ((aligned(16))) BosDesc = {
		/* BOS descriptor */
		{	(u8)sizeof(XFsblPs_UsbStdBosDesc), /* bLength */
			XFSBL_TYPE_BOS_DESC, /* DescriptorType */
			(u16)sizeof(XFsblPs_UsbBosDesc), /* wTotalLength */
			0x02U}, /* bNumDeviceCaps */

		{	(u8)sizeof(XFsblPs_UsbStdDeviceCap7Byte), /* bLength */
			0x10U, /* bDescriptorType */
			0x02U, /* bDevCapabiltyType */
			0x00U}, /* Disable LPM/BESL for USB 2.0*/

		{	(u8)sizeof(XFsblPs_UsbStdDeviceCap10Byte), /* bLength */
			0x10U, /* bDescriptorType */
			0x03U, /* bDevCapabiltyType */
			0x00U, /* bmAttributes */
			0x000FU, /* wSpeedsSupported */
			0x01U, /* bFunctionalitySupport */
			0x00U, /* Disable LPM for USB 3.0 */
			0x00U} /* Disable LPM for USB 3.0 */

	};

	(void)XFsbl_MemCpy(BufPtr, &BosDesc, UsbBosDescLen);

END:
	return UsbBosDescLen;
}

/****************************************************************************/
/**
 * @brief	This function changes State of Core to USB configured State.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	Ctrl is a pointer to the Setup packet data
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 * @note		None.
 *
 *****************************************************************************/
s32 XFsbl_SetConfiguration(struct Usb_DevData* InstancePtr, SetupPacket *Ctrl)
{
	s32 Ret;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	((struct XUsbPsu*)(InstancePtr->PrivateData))->IsConfigDone = 0U;

	switch (InstancePtr->State) {
		case XUSBPSU_STATE_DEFAULT:
		{
			Ret = XST_FAILURE;
		}
			break;

		case XUSBPSU_STATE_ADDRESS:
		{
			InstancePtr->State = XUSBPSU_STATE_CONFIGURED;
			Ret = XST_SUCCESS;
		}
			break;

		case XUSBPSU_STATE_CONFIGURED:
		{
			Ret = XST_SUCCESS;
		}
			break;

		default:
		{
			Ret = XST_FAILURE;
		}
		break;
	}
	return Ret;
}


/*****************************************************************************
 * This function handles setting of DFU state.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	DfuState is a value of the DFU state to be set
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XFsbl_DfuSetState(struct Usb_DevData* InstancePtr, u32 DfuState) {
	int Status = XST_FAILURE;

	switch (DfuState) {

		case STATE_APP_IDLE:
		{
			DfuObj.CurrState = STATE_APP_IDLE;
			DfuObj.NextState = STATE_APP_DETACH;
			DfuObj.CurrStatus = DFU_STATUS_OK;
			/* Set to runtime mode by default */
			DfuObj.IsDfu = (u8)FALSE;
			DfuObj.RuntimeToDfu = (u8)FALSE;
			if (DownloadDone == 0U) {
				DownloadDone = 1U;
			}
			else if (DownloadDone == 2U) {
				++DownloadDone;
			}
			Status = XST_SUCCESS;
		}
			break;

		case STATE_APP_DETACH:
		{
			if (DfuObj.CurrState == STATE_APP_IDLE) {

				DfuObj.CurrState = STATE_APP_DETACH;
				DfuObj.NextState = STATE_DFU_IDLE;

				/* Wait For USB Reset to happen */
				XFsbl_DfuWaitForReset();
				/* Setting Dfu Mode */
				DfuObj.IsDfu = (u8)TRUE;
				/*
				 * Set this flag to indicate we are going
				 * from runtime to dfu mode
				 */
				DfuObj.RuntimeToDfu = (u8)TRUE;
				DfuObj.CurrState = STATE_DFU_IDLE;
				DfuObj.NextState = STATE_DFU_DOWNLOAD_SYNC;
				DfuObj.IsDfu = (u8)TRUE;
				Status = XST_SUCCESS;
			} else if (DfuObj.CurrState == STATE_DFU_IDLE) {
				/* Wait For USB Reset to happen */
				XFsbl_DfuWaitForReset();

				DfuObj.CurrState = STATE_APP_IDLE;
				DfuObj.NextState = STATE_APP_DETACH;
				DfuObj.CurrStatus = DFU_STATUS_OK;
				DfuObj.IsDfu = (u8)FALSE;
				Status = XST_SUCCESS;
			}
			else {
				/* Error */
			}
		}
			break;
		case STATE_DFU_IDLE:
		{
			DfuObj.CurrState = STATE_DFU_IDLE;
			DfuObj.NextState = STATE_DFU_DOWNLOAD_SYNC;
			DfuObj.IsDfu = (u8)TRUE;
			++DownloadDone;
			Status = XST_SUCCESS;
		}
		break;
		case STATE_DFU_DOWNLOAD_SYNC:
		{
			DfuObj.CurrState = STATE_DFU_DOWNLOAD_SYNC;
			Status = XST_SUCCESS;
		}
		break;

		case STATE_DFU_DOWNLOAD_BUSY:
		case STATE_DFU_DOWNLOAD_IDLE:
		case STATE_DFU_ERROR:
		default:
			break;
	}

	if (Status != XST_SUCCESS) {
		/* Unsupported command. Stall the end point. */
		DfuObj.CurrState = STATE_DFU_ERROR;
		XUsbPsu_Ep0StallRestart(
			(struct XUsbPsu*)InstancePtr->PrivateData);
	}

}

/*****************************************************************************
 * This function handles DFU reset, called from driver.
 *
 * @param	InstancePtr is a pointer to USB instance of the controller
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XFsbl_DfuReset(struct Usb_DevData* InstancePtr)
{
	if (DfuObj.DfuWaitForInterrupt == 1U) {
		/* Tell DFU that we got reset signal */
		DfuObj.DfuWaitForInterrupt = 0U;
	}
}

/*****************************************************************************
 * This function handles DFU set interface.
 *
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XFsbl_DfuSetIntf(struct Usb_DevData* InstancePtr, SetupPacket *SetupData)
{
	/* Setting the alternate setting requested */
	DfuObj.CurrentInf = SetupData->wValue;
	if (DfuObj.RuntimeToDfu == (u8)TRUE) {
		/*
		 * Clear the flag, before entering into DFU
		 * mode from runtime mode.
		 */
		DfuObj.RuntimeToDfu = (u8)FALSE;
		/* Entering DFU_IDLE state */
		XFsbl_DfuSetState(InstancePtr, STATE_DFU_IDLE);
	}
	else if (DfuObj.CurrentInf >= DFU_ALT_SETTING) {
		/* Entering DFU_IDLE state */
		XFsbl_DfuSetState(InstancePtr, STATE_DFU_IDLE);
	} else {
		/* Entering APP_IDLE state */
		XFsbl_DfuSetState(InstancePtr, STATE_APP_IDLE);
	}
}

/*****************************************************************************
 * This function handles DFU heart and soul of DFU state machine.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XFsbl_DfuClassReq(struct Usb_DevData* InstancePtr, SetupPacket *SetupData)
{
	int Result = XST_FAILURE;
	u32 RxBytesLeft;
	static u8 DfuReply[DFU_STATUS_SIZE] = {0,};

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData != NULL);

	switch(SetupData->bRequest) {
		case DFU_DETACH:
		{
			XFsbl_DfuSetState(InstancePtr, STATE_APP_DETACH);
		}
		break;

		case DFU_DNLOAD:
		{
			if(DfuObj.GotDnloadRqst == (u8)FALSE) {
				DfuObj.GotDnloadRqst = (u8)TRUE;
			}
			if ((DfuObj.TotalTransfers == 0U) &&
				(SetupData->wValue == 0U)) {
				/* We are at the start of the data,
				 * clear the download counter
				 */
				DfuObj.TotalBytesDnloaded = 0U;
			}

			RxBytesLeft = SetupData->wLength;

			if(RxBytesLeft > 0U) {
				Result = XUsbPsu_EpBufferRecv(
					(struct XUsbPsu*)InstancePtr->PrivateData,
					0U, &DfuVirtFlash[DfuObj.TotalBytesDnloaded],
					RxBytesLeft);
				DfuObj.TotalBytesDnloaded += RxBytesLeft;
			}
			else {
				if (DfuObj.GotDnloadRqst == (u8)TRUE) {
					DfuObj.CurrState =
						STATE_DFU_IDLE;
					DfuObj.GotDnloadRqst = (u8)FALSE;
					DfuObj.TotalTransfers = 0U;
				}
			}

			if((DfuObj.GotDnloadRqst == (u8)TRUE) &&
				(Result == XST_SUCCESS)) {
				DfuObj.CurrState =
					STATE_DFU_DOWNLOAD_IDLE;
				DfuObj.GotDnloadRqst = (u8)FALSE;
			}
		}
			break;
		case DFU_GETSTATUS:
		{
			if (DfuObj.GotDnloadRqst == (u8)TRUE) {
				if (DfuObj.CurrState == STATE_DFU_IDLE ) {
					DfuObj.CurrState =
						STATE_DFU_DOWNLOAD_SYNC;
				}
				else if (DfuObj.CurrState ==
						STATE_DFU_DOWNLOAD_SYNC) {
					DfuObj.CurrState =
						STATE_DFU_DOWNLOAD_BUSY;
				}
				else {
					/*Do nothing */
				}
			}
			DfuReply[0] = DfuObj.CurrStatus;
			DfuReply[4] = DfuObj.CurrState;
			if (SetupData->wLength > 0U) {
				Result = XUsbPsu_EpBufferSend(
				(struct XUsbPsu*)InstancePtr->PrivateData,
				0U, DfuReply, (u32)SetupData->wLength);
				if (Result != XST_SUCCESS) {
					goto END;
				}
			}
		}
			break;
		default:
			/* Unsupported command. Stall the end point. */
			DfuObj.CurrState = STATE_DFU_ERROR;
			XUsbPsu_Ep0StallRestart(
				(struct XUsbPsu*)InstancePtr->PrivateData);
			break;
	}

END:
	return;
}

#endif
