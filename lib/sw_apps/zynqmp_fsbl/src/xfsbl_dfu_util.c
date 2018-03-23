/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
*****************************************************************************/

#include "xfsbl_hw.h"

#ifdef XFSBL_USB
#include "xfsbl_dfu_util.h"
#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xusb_wrapper.h"
#include "xfsbl_usb.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/
#define DFUFUNC_DESCR				0x21U    /* DFU Functional Desc */
#define USB_MODES_NUM				2U
#define STRING_DESCRIPTORS_NUM		6U

static void XFsbl_DfuSetState(u32 DfuState );

/**************************** Type Definitions *******************************/
extern struct XUsbPsu UsbInstance;
struct XFsblPs_DfuIf DfuObj;
extern u32 DownloadDone;
extern u8* DfuVirtFlash;

/* Device Descriptors */
static XFsblPs_UsbStdDevDesc __attribute__ ((aligned(16))) DDesc[] = {
	{/* USB 2.0 */
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
	},
	{/* USB 3.0 */
		(u8)sizeof(XFsblPs_UsbStdDevDesc), /* bLength */
		USB_DEVICE_DESC, /* bDescriptorType */
		(0x0300U), /* bcdUSB 3.0 */
		0x00U, /* bDeviceClass */
		0x00U, /* bDeviceSubClass */
		0x00U, /* bDeviceProtocol */
		0x09U, /* bMaxPackedSize0 */
		(0x03FDU), /* idVendor */
		(0x0050U), /* idProduct */
		(0x0404U), /* bcdDevice */
		0x01U, /* iManufacturer */
		0x02U, /* iProduct */
		0x03U, /* iSerialNumber */
		0x01U /* bNumConfigurations */
	}};

/* String Descriptors */
static char* StringList[USB_MODES_NUM][STRING_DESCRIPTORS_NUM] = {
	{
		"UNUSED",
		"XILINX INC",
		"DFU 2.0 emulation v 1.1",
		"2A49876D9CC1AA4",
		"Xilinx DFU Downloader",
		"7ABC7ABC7ABC7ABC7ABC7ABC"
	},
	{
		"UNUSED",
		"XILINX INC",
		"DFU 3.0 emulation v 1.1",
		"2A49876D9CC1AA4",
		"Xilinx DFU Downloader",
		"7ABC7ABC7ABC7ABC7ABC7ABC"
	},
};

static XFsblPs_Usb30Config __attribute__ ((aligned(16))) Config3 = {
	/* Std Config */
	{	(u8)sizeof(XFsblPs_UsbStdCfgDesc), /* bLength */
		USB_CONFIG_DESC, /* bDescriptorType */
		(u16) sizeof(XFsblPs_Usb30Config), /* wTotalLength */
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
		0x04U, /* wMaxPacketSize - MSB */
		0x00U}, /* bInterval */

	/* SS Endpoint companion  */
	{	(u8)sizeof(XFsblPs_UsbStdEpSsCompDesc), /* bLength */
		0x30U, /* bDescriptorType */
		0x0FU, /* bMaxBurst */
		0x00U, /* bmAttributes */
		0x00U}, /* wBytesPerInterval */

	/* Bulk Out Endpoint Config */
	{	(u8)sizeof(XFsblPs_UsbStdEpDesc), /* bLength */
		USB_ENDPOINT_CFG_DESC, /* bDescriptorType */
		0x01U, /* bEndpointAddress */
		0x02U, /* bmAttribute  */
		0x00U, /* wMaxPacketSize - LSB */
		0x04U, /* wMaxPacketSize - MSB */
		0x00U}, /* bInterval */

	/* SS Endpoint companion  */
	{	(u8)sizeof(XFsblPs_UsbStdEpSsCompDesc), /* bLength */
		0x30U, /* bDescriptorType */
		0x0FU, /* bMaxBurst */
		0x00U, /* bmAttributes */
		0x00U}, /* wBytesPerInterval */

	/*** DFU Interface descriptor ***/
	{	(u8)sizeof(XFsblPs_UsbStdIfDesc), /* bLength */
		USB_INTERFACE_CFG_DESC, /* bDescriptorType */
		0x00U, /* bInterfaceNumber */
		0x01U, /* bAlternateSetting */
		0x00U, /* bNumEndPoints */
		0xFEU, /* bInterfaceClass DFU application specific class code */
		0x01U, /* bInterfaceSubClass DFU device firmware upgrade code*/
		0x02U, /* bInterfaceProtocol DFU mode protocol*/
		0x04U}, /* iInterface DFU string descriptor*/

	/**** DFU functional descriptor ****/
	{	(u8)sizeof(XFsblPs_UsbDfuFuncDesc), /* bLength*/
		DFUFUNC_DESCR, /* bDescriptorType DFU functional descriptor type */
		0x3U, /* bmAttributes Device is only download/upload capable */
		8192U, /* wDetatchTimeOut 8192 ms */
		DFU_MAX_TRANSFER, /*wTransferSize DFU block size 1024*/
		0x0110U /*bcdDfuVersion 1.1 */
	}
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
	s32 SStatus;
	XFsblPs_UsbStdStringDesc StringDesc;

	if (Index >= (sizeof(StringList) / sizeof(u8 *))) {
		DescLen = 0U;
		goto END;
	}

	if (BufPtr == NULL) {
		DescLen = 0U;
		goto END;
	}

	SStatus = IsSuperSpeed(UsbInstance.AppData);
	if(SStatus != XST_SUCCESS) {
		/* USB 2.0 */
		String = StringList[0][Index];

	} else {
		/* USB 3.0 */
		String = StringList[1][Index];
	}

	StringLen = strlen(String);

	/* Index 0 is LangId which is special as we can not represent
	 * the string required in the table above.Therefore we handle
	 * index 0 as a special case.*/

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
	s32 SStatus;
	u32 DevDescLength;

	/* Check buffer pointer is there and buffer is big enough. */
	if (BufPtr == NULL) {
		DevDescLength = 0U;
		goto END;
	}

	if (BufferLen < sizeof(XFsblPs_UsbStdDevDesc)) {
		DevDescLength = 0U;
		goto END;
	}

	DevDescLength = sizeof(XFsblPs_UsbStdDevDesc);

	SStatus = IsSuperSpeed(UsbInstance.AppData);
	if(SStatus != XST_SUCCESS) {
		/* USB 2.0 */
		(void)memcpy(BufPtr, &DDesc[0], DevDescLength);

	} else {
		/* USB 3.0 */
		(void)memcpy(BufPtr, &DDesc[1], DevDescLength);
	}

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
	s32 SStatus;
	u8 *Config;
	u32 CfgDescLen;

	/* Check buffer pointer is OK and buffer is big enough. */
	if (BufPtr == NULL) {
		CfgDescLen = 0U;
		goto END;
	}

	if (BufferLen < sizeof(XFsblPs_UsbStdCfgDesc)) {
		CfgDescLen = 0U;
		goto END;
	}

	SStatus = IsSuperSpeed(UsbInstance.AppData);
	if(SStatus != XST_SUCCESS) {
		/* USB 2.0 */
		Config = (u8 *)&Config2;
		CfgDescLen = sizeof(XFsblPs_UsbConfig);
	} else {
		/* USB 3.0 */
		Config = (u8 *)&Config3;
		CfgDescLen = sizeof(XFsblPs_Usb30Config);
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

	if (BufferLen < sizeof(XFsblPs_UsbStdBosDesc)) {
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

	UsbBosDescLen = sizeof(XFsblPs_UsbBosDesc);

	(void)XFsbl_MemCpy(BufPtr, &BosDesc, UsbBosDescLen);

END:
	return UsbBosDescLen;
}

/****************************************************************************/
/**
 * Changes State of Core to USB configured State.
 *
 * @param	Ctrl is a pointer to the Setup packet data.
 *
 * @return	XST_SUCCESS else XST_FAILURE
 *
 * @note		None.
 *
 *****************************************************************************/
s32 XFsbl_SetConfiguration(SetupPacket *Ctrl)
{
	s32 Ret;

	Xil_AssertNonvoid(Ctrl != NULL);

	UsbInstance.IsConfigDone = 0U;

	switch (UsbInstance.AppData->State) {
		case XUSBPSU_STATE_DEFAULT:
		{
			Ret = XST_FAILURE;
		}
			break;

		case XUSBPSU_STATE_ADDRESS:
		{
			UsbInstance.AppData->State = XUSBPSU_STATE_CONFIGURED;
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
 * @param	dfu_state is a value of the DFU state to be set
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
static void XFsbl_DfuSetState(u32 DfuState) {

	switch (DfuState) {

		case STATE_APP_IDLE:
		{
			DfuObj.CurrState = STATE_APP_IDLE;
			DfuObj.NextState = STATE_APP_DETACH;
			DfuObj.CurrStatus = DFU_STATUS_OK;
			DfuObj.RuntimeToDfu = 0U;
		}
		break;

		case STATE_APP_DETACH:
		{
			if (DfuObj.CurrState == STATE_APP_IDLE) {

				DfuObj.CurrState = STATE_APP_DETACH;
				DfuObj.NextState = STATE_DFU_IDLE;

				/* Wait For USB Reset to happen */
				XFsbl_DfuWaitForReset();

				/* Set this flag to indicate we are going from runtime to dfu mode */
				DfuObj.RuntimeToDfu = 1U;

				/* fall through */
			} else if (DfuObj.CurrState == STATE_DFU_IDLE) {
				/* Wait For USB Reset to happen */
				XFsbl_DfuWaitForReset();

				DfuObj.CurrState = STATE_APP_IDLE;
				DfuObj.NextState = STATE_APP_DETACH;
				DfuObj.CurrStatus = DFU_STATUS_OK;
				break;
			} else {
				goto stall;
			}
		}

		case STATE_DFU_IDLE:
		{
			DfuObj.CurrState = STATE_DFU_IDLE;
			DfuObj.NextState = STATE_DFU_DOWNLOAD_SYNC;
		}
		break;
		case STATE_DFU_DOWNLOAD_SYNC:
		{
			DfuObj.CurrState = STATE_DFU_DOWNLOAD_SYNC;
		}
		break;

		case STATE_DFU_DOWNLOAD_BUSY:
		case STATE_DFU_DOWNLOAD_IDLE:
		case STATE_DFU_ERROR:
		default:
		{
		stall:
			 /* Unsupported command. Stall the end point.*/

			DfuObj.CurrState = STATE_DFU_ERROR;
			XUsbPsu_EpSetStall(&UsbInstance, 0U, XUSBPSU_EP_DIR_IN);
		}
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
void XFsbl_DfuSetIntf(SetupPacket *SetupData)
{
	/* Setting the alternate setting requested */
	DfuObj.CurrentInf = SetupData->wValue;
	if ((DfuObj.CurrentInf >= DFU_ALT_SETTING) || (DfuObj.RuntimeToDfu == 1U)) {

		/* Clear the flag , before entering into DFU mode from runtime mode */
		if (DfuObj.RuntimeToDfu == 1U)
			DfuObj.RuntimeToDfu = 0U;

		/* Entering DFU_IDLE state */
		XFsbl_DfuSetState(STATE_DFU_IDLE);
	} else {
		/* Entering APP_IDLE state */
		XFsbl_DfuSetState(STATE_APP_IDLE);
	}
}

/*****************************************************************************
 * This function handles DFU heart and soul of DFU state machine.
 *
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return
 *				None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XFsbl_DfuClassReq(SetupPacket *SetupData)
{
	Xil_AssertVoid(SetupData != NULL);
	u32 RxBytesLeft;
	s32 Result;

	static u8 DfuReply[DFU_STATUS_SIZE]={0,};

	switch(SetupData->bRequest) {
		case DFU_DETACH:
		{
			XFsbl_DfuSetState(STATE_APP_DETACH);
		}
		break;

		case DFU_DNLOAD:
		{
			if (SetupData->wValue == 0U) {
				/* we are the start of the data, clear the download counter  */
				DfuObj.TotalBytesDnloaded = 0U;
			}

			RxBytesLeft = (u32)(SetupData->wLength);

			if(RxBytesLeft > 0U) {
				do {
					Result = XUsbPsu_EpBufferRecv(&UsbInstance, 0U, &DfuVirtFlash[DfuObj.TotalBytesDnloaded],
								RxBytesLeft);
				}while(Result != XST_SUCCESS);

				DfuObj.TotalBytesDnloaded += RxBytesLeft;
				DfuObj.CurrState = STATE_DFU_DOWNLOAD_IDLE;
				DfuObj.GotDnloadRqst = 0U;
			} else {/*if (RxBytesLeft == 0U)*/
				DfuObj.CurrState = STATE_DFU_IDLE;
				DfuObj.GotDnloadRqst = 0U;
				Result = XST_FAILURE;
			}
		}

		break;

		case DFU_GETSTATUS:
		{
			if(DfuObj.CurrState == STATE_DFU_IDLE )
			{
				DfuObj.CurrState = STATE_DFU_DOWNLOAD_SYNC;
				++DownloadDone;
			}
			else if (DfuObj.CurrState == STATE_DFU_DOWNLOAD_SYNC)
			{
				DfuObj.CurrState = STATE_DFU_DOWNLOAD_BUSY;
			}
			else
			{
				/*Misra C compliance*/
			}
			DfuReply[0] = DfuObj.CurrStatus;
			DfuReply[4] = DfuObj.CurrState;

			do {
					Result = XUsbPsu_EpBufferSend(&UsbInstance, 0U, DfuReply, (u32)SetupData->wLength);

			}while(Result != XST_SUCCESS);

		}
		break;

		default:
		{
			/* Unsupported command. Stall the end point.*/
			DfuObj.CurrState = STATE_DFU_ERROR;
			XUsbPsu_EpSetStall(&UsbInstance, 0U, XUSBPSU_EP_DIR_IN);
		}
		break;
	}
}

#endif
