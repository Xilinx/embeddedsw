/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_dfu_util.c
*
* This file contains definitions of the DFU specific functions to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/10/19 First release
*
* </pre>
*
*****************************************************************************/
#include "xloader_dfu_util.h"
#ifdef XLOADER_USB
#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xloader_usb.h"
#include "xplmi_util.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/
extern struct XUsbPsu UsbInstance;
struct XLoaderPs_DfuIf DfuObj;
extern u32 DownloadDone;
extern u8* DfuVirtFlash;

/* Device Descriptors */
static XLoaderPs_UsbStdDevDesc __attribute__ ((aligned(16U))) DDesc[] = {
	{/* USB 2.0 */
		(u8)sizeof(XLoaderPs_UsbStdDevDesc), /**< Length */
		XLOADER_USB_DEVICE_DESC, /**< DescriptorType */
		XLOADER_USB2_BCD, /**< BcdUSB 2.0 */
		XLOADER_USB2_BDEVICE_CLASS, /**< DeviceClass */
		XLOADER_USB2_BDEVICE_SUBCLASS, /**< DeviceSubClass */
		XLOADER_USB2_BDEVICE_PROTOCOL, /**< DeviceProtocol */
		XLOADER_USB2_MAX_PACK_SIZE, /**< MaxPackedSize0 */
		XLOADER_USB2_IDVENDOR, /**< IdVendor */
		XLOADER_USB2_IDPRODUCT, /**< IdProduct */
		XLOADER_USB2_BDEVICE, /**< BcdDevice */
		XLOADER_USB2_MANUFACTURER, /**< Manufacturer */
		XLOADER_USB2_IPRODUCT, /**< Product */
		XLOADER_USB2_SERIAL_NUM, /**< SerialNumber */
		XLOADER_USB2_NUM_CONFIG, /**< NumConfigurations */
	},
	{
		/* USB 3.0 */
		(u8)sizeof(XLoaderPs_UsbStdDevDesc), /**< Length */
		XLOADER_USB_DEVICE_DESC, /**< DescriptorType */
		XLOADER_USB3_BCD, /**< BcdUSB 3.0 */
		XLOADER_USB3_BDEVICE_CLASS, /**< DeviceClass */
		XLOADER_USB3_BDEVICE_SUBCLASS, /**< DeviceSubClass */
		XLOADER_USB3_BDEVICE_PROTOCOL, /**< DeviceProtocol */
		XLOADER_USB3_MAX_PACK_SIZE, /**< MaxPackedSize0 */
		XLOADER_USB3_IDVENDOR, /**< IdVendor */
		XLOADER_USB3_IDPRODUCT, /**< IdProduct */
		XLOADER_USB3_BDEVICE, /**< BcdDevice */
		XLOADER_USB3_MANUFACTURER, /**< Manufacturer */
		XLOADER_USB3_IPRODUCT, /**< Product */
		XLOADER_USB3_SERIAL_NUM, /**< SerialNumber */
		XLOADER_USB3_NUM_CONFIG, /**< NumConfigurations */
	},
};

static XLoaderPs_Usb30Config __attribute__ ((aligned(16U))) Config3 = {
	/* Std Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdCfgDesc), /**< Length */
		XLOADER_USB_CONFIG_DESC, /**< DescriptorType */
		(u16) sizeof(XLoaderPs_Usb30Config), /**< TotalLength */
		XLOADER_USB3_CONFIG_NUM_INTF, /**< NumInterfaces */
		XLOADER_USB3_CONFIG_VAL, /**< ConfigurationValue */
		XLOADER_USB3_CONFIGURATION, /**< Configuration */
		XLOADER_USB3_CONFIG_ATTRB, /**< Attribute */
		XLOADER_USB3_CONFIG_MAX_PWR, /**< MaxPower  */
	},
	/* Interface Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdIfDesc), /**< Length */
		XLOADER_USB_INTERFACE_CFG_DESC, /**< DescriptorType */
		XLOADER_USB3_INTF_NUM, /**< InterfaceNumber */
		XLOADER_USB3_INTF_ALT_SETTING, /**< AlternateSetting */
		XLOADER_USB3_INTF_NUM_ENDPOINTS, /**< NumEndPoints */
		XLOADER_USB3_INTF_CLASS, /**< InterfaceClass */
		XLOADER_USB3_INTF_SUBCLASS, /**< InterfaceSubClass */
		XLOADER_USB3_INTF_PROT, /**< InterfaceProtocol */
		XLOADER_USB3_INTERFACE, /**< Interface */
	},
	/* Bulk In Endpoint Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpDesc), /**< Length */
		XLOADER_USB_ENDPOINT_CFG_DESC, /**< DescriptorType */
		XLOADER_USB3_BULK_IN_EP_ADDR, /**< EndpointAddress */
		XLOADER_USB3_BULK_IN_EP_ATTRB, /**< Attribute */
		XLOADER_USB3_BULK_IN_EP_PKT_SIZE_LSB, /**< MaxPacketSize - LSB */
		XLOADER_USB3_BULK_IN_EP_PKT_SIZE_MSB, /**< MaxPacketSize - MSB */
		XLOADER_USB3_BULK_IN_EP_INTERVAL, /**< Interval */
	},
	/* SS Endpoint companion */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpSsCompDesc), /**< Length */
		XLOADER_USB3_SS_EP_DESC_TYPE, /**< DescriptorType */
		XLOADER_USB3_SS_EP_MAX_BURST, /**< MaxBurst */
		XLOADER_USB3_SS_EP_ATTRB, /**< Attributes */
		XLOADER_USB3_SS_EP_BYTES_PER_INTERVAL, /**< BytesPerInterval */
	},
	/* Bulk Out Endpoint Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpDesc), /**< Length */
		XLOADER_USB_ENDPOINT_CFG_DESC, /**< DescriptorType */
		XLOADER_USB3_BULK_OUT_EP_ADDR, /**< EndpointAddress */
		XLOADER_USB3_BULK_OUT_EP_ATTRB, /**< Attribute  */
		XLOADER_USB3_BULK_OUT_EP_PKT_SIZE_LSB, /**< MaxPacketSize - LSB */
		XLOADER_USB3_BULK_OUT_EP_PKT_SIZE_MSB, /**< MaxPacketSize - MSB */
		XLOADER_USB3_BULK_OUT_EP_INTERVAL, /**< Interval */
	},
	/* SS Endpoint companion */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpSsCompDesc), /**< Length */
		XLOADER_USB3_SS_EP_DESC_TYPE, /**< DescriptorType */
		XLOADER_USB3_SS_EP_MAX_BURST, /**< MaxBurst */
		XLOADER_USB3_SS_EP_ATTRB, /**< Attributes */
		XLOADER_USB3_SS_EP_BYTES_PER_INTERVAL, /**< BytesPerInterval */
	},
	/* DFU Interface descriptor */
	{
		(u8)sizeof(XLoaderPs_UsbStdIfDesc), /**< Length */
		XLOADER_USB_INTERFACE_CFG_DESC, /**< DescriptorType */
		XLOADER_USB3_DFU_INTF_NUM, /**< InterfaceNumber */
		XLOADER_USB3_DFU_INTF_ALT_SETTING, /**< AlternateSetting */
		XLOADER_USB3_DFU_INTF_NUM_ENDPOINTS, /**< NumEndPoints */
		XLOADER_USB3_DFU_INTF_CLASS, /**< InterfaceClass DFU application specific class code */
		XLOADER_USB3_DFU_INTF_SUBCLASS, /**< InterfaceSubClass DFU device firmware upgrade code */
		XLOADER_USB3_DFU_INTF_PROT, /**< InterfaceProtocol DFU mode protocol */
		XLOADER_USB3_DFU_INTERFACE, /**< Interface DFU string descriptor */
	},
	/* DFU functional descriptor */
	{
		(u8)sizeof(XLoaderPs_UsbDfuFuncDesc), /**< Length*/
		XLOADER_DFUFUNC_DESCR, /**< DescriptorType DFU functional descriptor type */
		XLOADER_USB3_DFUFUNC_ATTRB, /**< Attributes Device is only download/upload capable */
		XLOADER_USB3_DFUFUNC_DETACH_TIMEOUT_MS, /**< DetatchTimeOut 8192 ms */
		XLOADER_DFU_MAX_TRANSFER, /**< TransferSize DFU block size 1024 */
		XLOADER_USB3_DFU_VERSION, /**< DfuVersion 1.1 */
	},
};

static XLoaderPs_UsbConfig __attribute__ ((aligned(16U))) Config2 = {
	/* Std Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdCfgDesc), /**< Length */
		XLOADER_USB_CONFIG_DESC, /**< DescriptorType */
		(u16)sizeof(XLoaderPs_UsbConfig), /**< TotalLength */
		XLOADER_USB2_CONFIG_NUM_INTF, /**< NumInterfaces */
		XLOADER_USB2_CONFIG_VAL, /**< ConfigurationValue */
		XLOADER_USB2_CONFIGURATION, /**< Configuration */
		XLOADER_USB2_CONFIG_ATTRB, /**< Attribute */
		XLOADER_USB2_CONFIG_MAX_PWR, /**< MaxPower  */
	},
	/* Interface Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdIfDesc), /**< Length */
		XLOADER_USB_INTERFACE_CFG_DESC, /**< DescriptorType */
		XLOADER_USB2_INTF_NUM, /**< InterfaceNumber */
		XLOADER_USB2_INTF_ALT_SETTING, /**< AlternateSetting */
		XLOADER_USB2_INTF_NUM_ENDPOINTS, /**< NumEndPoints */
		XLOADER_USB2_INTF_CLASS, /**< InterfaceClass */
		XLOADER_USB2_INTF_SUBCLASS, /**< InterfaceSubClass */
		XLOADER_USB2_INTF_PROT, /**< InterfaceProtocol */
		XLOADER_USB2_INTERFACE, /**< Interface */
	},
	/* Bulk In Endpoint Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpDesc), /**< Length */
		XLOADER_USB_ENDPOINT_CFG_DESC, /**< DescriptorType */
		XLOADER_USB2_BULK_IN_EP_ADDR, /**< EndpointAddress */
		XLOADER_USB2_BULK_IN_EP_ATTRB, /**< Attribute  */
		XLOADER_USB2_BULK_IN_EP_PKT_SIZE_LSB, /**< MaxPacketSize - LSB */
		XLOADER_USB2_BULK_IN_EP_PKT_SIZE_MSB, /**< MaxPacketSize - MSB */
		XLOADER_USB2_BULK_IN_EP_INTERVAL, /**< Interval */
	},
	/* Bulk Out Endpoint Config */
	{
		(u8)sizeof(XLoaderPs_UsbStdEpDesc), /**< Length */
		XLOADER_USB_ENDPOINT_CFG_DESC, /**< DescriptorType */
		XLOADER_USB2_BULK_OUT_EP_ADDR, /**< EndpointAddress */
		XLOADER_USB2_BULK_OUT_EP_ATTRB, /**< Attribute  */
		XLOADER_USB2_BULK_OUT_EP_PKT_SIZE_LSB, /**< MaxPacketSize - LSB */
		XLOADER_USB2_BULK_OUT_EP_PKT_SIZE_MSB, /**< MaxPacketSize - MSB */
		XLOADER_USB2_BULK_OUT_EP_INTERVAL, /**< Interval */
	},
	/* DFU Interface Descriptor */
	{
		(u8)sizeof(XLoaderPs_UsbStdIfDesc), /**< Length */
		XLOADER_USB_INTERFACE_CFG_DESC, /**< DescriptorType */
		XLOADER_USB2_DFU_INTF_NUM, /**< InterfaceNumber */
		XLOADER_USB2_DFU_INTF_ALT_SETTING, /**< AlternateSetting */
		XLOADER_USB2_DFU_INTF_NUM_ENDPOINTS, /**< NumEndPoints */
		XLOADER_USB2_DFU_INTF_CLASS, /**< InterfaceClass DFU application specific class code */
		XLOADER_USB2_DFU_INTF_SUBCLASS, /**< InterfaceSubClass DFU device firmware upgrade code */
		XLOADER_USB2_DFU_INTF_PROT, /**< InterfaceProtocol DFU mode protocol */
		XLOADER_USB2_DFU_INTERFACE, /**< Interface DFU string descriptor */
	},
	/* DFU functional descriptor */
	{
		(u8)sizeof(XLoaderPs_UsbDfuFuncDesc), /**< Length*/
		XLOADER_DFUFUNC_DESCR, /**< DescriptorType DFU functional descriptor type */
		XLOADER_USB2_DFUFUNC_ATTRB, /**< Attributes Device is only download/upload capable */
		XLOADER_USB2_DFUFUNC_DETACH_TIMEOUT_MS, /**< DetatchTimeOut 8192 ms */
		XLOADER_DFU_MAX_TRANSFER, /**< TransferSize DFU block size 1024 */
		XLOADER_USB2_DFU_VERSION, /**< DfuVersion 1.1 */
	},
};

/******************************************************************************/
/**
 * @brief	This function waits for DFU reset.
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
static void XLoader_DfuWaitForReset(void)
{
	/* This bit would be cleared when reset happens. */
	DfuObj.DfuWaitForInterrupt = TRUE;
	while (DfuObj.DfuWaitForInterrupt == FALSE) {
		;
	}
}

/******************************************************************************/
/**
 * @brief	This function returns a string descriptor for the given index.
 *
 * @param	BufPtr is a  pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 * @param	Index is the index of the string for which the descriptor
 *			is requested.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
u32 XLoader_Ch9SetupStrDescReply(struct Usb_DevData* InstancePtr, u8 *BufPtr,
	u32 BufLen, u8 Index)
{
	int Status = XST_FAILURE;
	u32 LoopVar;
	char* String;
	u32 StringLen;
	u32 DescLen = 0U;
	XLoaderPs_UsbStdStringDesc StringDesc;
	/* String Descriptors */
	static char* StringList[XLOADER_USB_MODES_NUM]
			[XLOADER_STRING_DESCRIPTORS_NUM] = {
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

	if (Index >= XLOADER_STRING_DESCRIPTORS_NUM) {
		goto END;
	}

	if (BufPtr == NULL) {
		goto END;
	}

	Status = XUsbPsu_IsSuperSpeed(
			(struct XUsbPsu*)InstancePtr->PrivateData);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		String = StringList[0U][Index];

	} else {
		/* USB 3.0 */
		String = StringList[1U][Index];
	}

	StringLen = strlen(String);

	/*
	 * Index 0 is LangId which is special as we can not represent
	 * the string required in the table above.Therefore we handle
	 * index 0 as a special case.
	 */
	if (0U == Index) {
		StringDesc.Length = XLOADER_STRING_DESC_ZERO_SIZE;
		StringDesc.DescriptorType = XLOADER_STRING_DESC_ZERO_DESC_TYPE;
		StringDesc.LangId[0U] = XLOADER_STRING_DESC_LANG_ID_ZERO;
		for (Index = 1U; Index < XLOADER_STRING_SIZE; ++Index) {
			StringDesc.LangId[Index] = 0U;
		}
	}
	else {
		/* All other strings can be pulled from the table above. */
		StringDesc.Length = (u8)((StringLen * 2U) + 2U);
		StringDesc.DescriptorType = XLOADER_STRING_DESC_TYPE;

		for (LoopVar = 0U; LoopVar < StringLen; ++LoopVar) {
			StringDesc.LangId[LoopVar] = (u16) String[LoopVar];
		}
		for (; LoopVar < XLOADER_STRING_SIZE; ++LoopVar) {
			StringDesc.LangId[LoopVar] = (u16)0U;
		}
	}
	DescLen = StringDesc.Length;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufLen) {
		DescLen = 0U;
		goto END;
	}

	(void)XPlmi_MemCpy(BufPtr, &StringDesc, DescLen);

END:
	return DescLen;
}

/*****************************************************************************/
/**
 * @brief	This function returns the device descriptor for the device.
 *
 * @param	BufPtr is pointer to the buffer that is to be filled
 *			with the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
u32 XLoader_Ch9SetupDevDescReply(struct Usb_DevData* InstancePtr, u8 *BufPtr,
	u32 BufLen)
{
	int Status = XST_FAILURE;
	u32 DevDescLength = 0U;

	/* Check buffer pointer is there and buffer is big enough. */
	if (BufPtr == NULL) {
		goto END;
	}
	DevDescLength = sizeof(XLoaderPs_UsbStdDevDesc);
	if (BufLen < DevDescLength) {
		DevDescLength = 0U;
		goto END;
	}

	Status = XUsbPsu_IsSuperSpeed((struct XUsbPsu*)InstancePtr->PrivateData);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		(void)XPlmi_MemCpy(BufPtr, &DDesc[0U], DevDescLength);
	}
	else {
		/* USB 3.0 */
		(void)XPlmi_MemCpy(BufPtr, &DDesc[1U], DevDescLength);
	}

END:
	return DevDescLength;
}

/*****************************************************************************/
/**
 * @brief	This function returns the configuration descriptor for the device.
 *
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
u32 XLoader_Ch9SetupCfgDescReply(struct Usb_DevData* InstancePtr, u8 *BufPtr,
																u32 BufferLen)
{
	int Status = XST_FAILURE;
	u8 *Config;
	u32 CfgDescLen = 0U;

	/* Check buffer pointer is OK and buffer is big enough. */
	if (BufPtr == NULL) {
		goto END;
	}

	Status = XUsbPsu_IsSuperSpeed((struct XUsbPsu*)InstancePtr->PrivateData);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		Config = (u8 *)&Config2;
		CfgDescLen = sizeof(XLoaderPs_UsbConfig);
	}
	else {
		/* USB 3.0 */
		Config = (u8 *)&Config3;
		CfgDescLen = sizeof(XLoaderPs_Usb30Config);
	}

	if (BufferLen < CfgDescLen) {
		CfgDescLen = 0U;
		goto END;
	}

	(void)XPlmi_MemCpy(BufPtr, Config, CfgDescLen);

END:
	return CfgDescLen;
}

/*****************************************************************************/
/**
 * @brief	This function returns the BOS descriptor for the device.
 *
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
u32 XLoader_Ch9SetupBosDescReply(struct Usb_DevData* InstancePtr, u8 *BufPtr,
	u32 BufferLen)
{
	u32 UsbBosDescLen = 0U;
	(void)(InstancePtr);

	/* Check buffer pointer is OK and buffer is big enough. */
	if (BufPtr == NULL) {
		goto END;
	}

	UsbBosDescLen = sizeof(XLoaderPs_UsbBosDesc);
	if (BufferLen < UsbBosDescLen) {
		UsbBosDescLen = 0U;
		goto END;
	}

	XLoaderPs_UsbBosDesc __attribute__ ((aligned(16U))) BosDesc = {
		/* BOS descriptor */
		{	(u8)sizeof(XLoaderPs_UsbStdBosDesc), /**< Length */
			XLOADER_TYPE_BOS_DESC, /**< DescriptorType */
			(u16)sizeof(XLoaderPs_UsbBosDesc), /**< TotalLength */
			XLOADER_USB_BOS_NUM_DEVICE_CAPS, /**< NumDeviceCaps */
		},
		{	(u8)sizeof(XLoaderPs_UsbStdDeviceCap7Byte), /**< Length */
			XLOADER_USB_BOS7_DESC_TYPE, /**< DescriptorType */
			XLOADER_USB_BOS7_DEV_CAPABILITY_TYPE, /**< DevCapabiltyType */
			XLOADER_USB_BOS7_DISABLE_LPM, /**< Disable LPM/BESL for USB 2.0 */
		},
		{	(u8)sizeof(XLoaderPs_UsbStdDeviceCap10Byte), /**< Length */
			XLOADER_USB_BOS10_DESC_TYPE, /**< DescriptorType */
			XLOADER_USB_BOS10_DEV_CAPABILITY_TYPE, /**< DevCapabiltyType */
			XLOADER_USB_BOS10_ATTRB, /**< Attributes */
			XLOADER_USB_BOS10_SPEEDS_SUPPORTED, /**< SpeedsSupported */
			XLOADER_USB_BOS10_FUNC_SUPPORTED, /**< FunctionalitySupport */
			XLOADER_USB_BOS10_DISABLE_LPM, /**< Disable LPM for USB 3.0 */
			XLOADER_USB_BOS10_DISABLE_LPM, /**< Disable LPM for USB 3.0 */
		},
	};

	(void)XPlmi_MemCpy(BufPtr, &BosDesc, UsbBosDescLen);

END:
	return UsbBosDescLen;
}

/*****************************************************************************/
/**
 * @brief	This function changes State of Core to USB configured State.
 *
 * @param	Ctrl is a pointer to the Setup packet data
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XLoader_SetConfiguration(struct Usb_DevData* InstancePtr, SetupPacket *Ctrl)
{
	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	((struct XUsbPsu*)(InstancePtr->PrivateData))->IsConfigDone = 0U;

	switch (InstancePtr->State) {
		case XUSBPSU_STATE_ADDRESS:
			InstancePtr->State = XUSBPSU_STATE_CONFIGURED;
			Status = XST_SUCCESS;
			break;
		case XUSBPSU_STATE_CONFIGURED:
			Status = XST_SUCCESS;
			break;
		case XUSBPSU_STATE_DEFAULT:
			break;
		default:
			break;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles setting of DFU state.
 *
 * @param	Dfu_state is a value of the DFU state to be set
 *
 * @return	None
 *
 ******************************************************************************/
void XLoader_DfuSetState(struct Usb_DevData* InstancePtr, u32 DfuState)
{
	int Status = XST_FAILURE;
	(void) InstancePtr;

	switch (DfuState) {
		case XLOADER_STATE_APP_IDLE:
			DfuObj.CurrState = XLOADER_STATE_APP_IDLE;
			DfuObj.NextState = XLOADER_STATE_APP_DETACH;
			DfuObj.CurrStatus = XLOADER_DFU_STATUS_OK;
			/* Set to runtime mode by default */
			DfuObj.IsDfu = FALSE;
			DfuObj.RuntimeToDfu = FALSE;
			++DownloadDone;
			Status = XST_SUCCESS;
			break;
		case XLOADER_STATE_APP_DETACH:
			if (DfuObj.CurrState == XLOADER_STATE_APP_IDLE) {
				DfuObj.CurrState = XLOADER_STATE_APP_DETACH;
				DfuObj.NextState = XLOADER_STATE_DFU_IDLE;
				/* Wait For USB Reset to happen */
				XLoader_DfuWaitForReset();
				/* Setting Dfu Mode */
				DfuObj.IsDfu = TRUE;
				/*
				 * Set this flag to indicate we are going
				 * from runtime to dfu mode
				 */
				DfuObj.RuntimeToDfu = TRUE;
				DfuObj.CurrState = XLOADER_STATE_DFU_IDLE;
				DfuObj.NextState = XLOADER_STATE_DFU_DOWNLOAD_SYNC;
				DfuObj.IsDfu = TRUE;
				Status = XST_SUCCESS;
			}
			else if (DfuObj.CurrState == XLOADER_STATE_DFU_IDLE) {
				/* Wait For USB Reset to happen */
				XLoader_DfuWaitForReset();
				DfuObj.CurrState = XLOADER_STATE_APP_IDLE;
				DfuObj.NextState = XLOADER_STATE_APP_DETACH;
				DfuObj.CurrStatus = XLOADER_DFU_STATUS_OK;
				DfuObj.IsDfu = FALSE;
				Status = XST_SUCCESS;
			}
			else {
				/* Error */
			}
			break;
		case XLOADER_STATE_DFU_IDLE:
			DfuObj.CurrState = XLOADER_STATE_DFU_IDLE;
			DfuObj.NextState = XLOADER_STATE_DFU_DOWNLOAD_SYNC;
			DfuObj.IsDfu = TRUE;
			Status = XST_SUCCESS;
			break;
		case XLOADER_STATE_DFU_DOWNLOAD_SYNC:
			DfuObj.CurrState = XLOADER_STATE_DFU_DOWNLOAD_SYNC;
			Status = XST_SUCCESS;
			break;
		case XLOADER_STATE_DFU_DOWNLOAD_BUSY:
		case XLOADER_STATE_DFU_DOWNLOAD_IDLE:
		case XLOADER_STATE_DFU_ERROR:
			break;
		default:
			break;
	}

	if (Status != XST_SUCCESS) {
		/* Unsupported command. Stall the end point. */
		DfuObj.CurrState = XLOADER_STATE_DFU_ERROR;
		XUsbPsu_Ep0StallRestart(
			(struct XUsbPsu*)InstancePtr->PrivateData);
	}

}

/*****************************************************************************/
/**
 * @brief	This function handles DFU reset, called from driver.
 *
 * @param	InstancePtr is a pointer to USB instance of the controller
 *
 * @return	None
 *
 ******************************************************************************/
void XLoader_DfuReset(struct Usb_DevData* InstancePtr)
{
	(void)(InstancePtr);
	if (DfuObj.DfuWaitForInterrupt == TRUE) {
		/* Tell DFU that we got reset signal */
		DfuObj.DfuWaitForInterrupt = FALSE;
	}
}

/*****************************************************************************/
/**
 * @brief	This function handles DFU set interface.
 *
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return	None
 *
 ******************************************************************************/
void XLoader_DfuSetIntf(struct Usb_DevData* InstancePtr, SetupPacket *SetupData)
{
	/* Setting the alternate setting requested */
	DfuObj.CurrentInf = SetupData->wValue;
	if (DfuObj.RuntimeToDfu == TRUE) {
		/*
		 * Clear the flag, before entering into DFU
		 * mode from runtime mode.
		 */
		DfuObj.RuntimeToDfu = FALSE;
		/* Entering DFU_IDLE state */
		XLoader_DfuSetState(InstancePtr, XLOADER_STATE_DFU_IDLE);
	}
	else if (DfuObj.CurrentInf >= XLOADER_DFU_ALT_SETTING) {
		/* Entering DFU_IDLE state */
		XLoader_DfuSetState(InstancePtr, XLOADER_STATE_DFU_IDLE);
	}
	else {
		/* Entering APP_IDLE state */
		XLoader_DfuSetState(InstancePtr, XLOADER_STATE_APP_IDLE);
	}
}

/*****************************************************************************/
/**
 * @brief	This function handles DFU heart and soul of DFU state machine.
 *
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return	None
 *
 ******************************************************************************/
void XLoader_DfuClassReq(struct Usb_DevData* InstancePtr, SetupPacket *SetupData)
{
	int Result;
	u32 RxBytesLeft;
	static u8 DfuReply[XLOADER_DFU_STATUS_SIZE] = {0U,};

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData != NULL);

	switch(SetupData->bRequest) {
		case XLOADER_DFU_DETACH:
			XLoader_DfuSetState(
				InstancePtr, XLOADER_STATE_APP_DETACH);
			break;
		case XLOADER_DFU_DNLOAD:
			if(DfuObj.GotDnloadRqst == FALSE) {
				DfuObj.GotDnloadRqst = TRUE;
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
				if (DfuObj.GotDnloadRqst == TRUE) {
					DfuObj.CurrState =
						XLOADER_STATE_DFU_IDLE;
					DfuObj.GotDnloadRqst = FALSE;
					DfuObj.TotalTransfers = 0U;
				}
			}

			if((DfuObj.GotDnloadRqst == TRUE) &&
				(Result == XST_SUCCESS)) {
				DfuObj.CurrState =
					XLOADER_STATE_DFU_DOWNLOAD_IDLE;
				DfuObj.GotDnloadRqst = FALSE;
			}
			break;
		case XLOADER_DFU_GETSTATUS:
			if (DfuObj.GotDnloadRqst == TRUE) {
				if (DfuObj.CurrState == XLOADER_STATE_DFU_IDLE ) {
					DfuObj.CurrState =
						XLOADER_STATE_DFU_DOWNLOAD_SYNC;
				}
				else if (DfuObj.CurrState ==
						XLOADER_STATE_DFU_DOWNLOAD_SYNC) {
					DfuObj.CurrState =
						XLOADER_STATE_DFU_DOWNLOAD_BUSY;
				}
				else {
					/*Do nothing */
				}
			}
			DfuReply[0U] = DfuObj.CurrStatus;
			DfuReply[4U] = DfuObj.CurrState;
			if (SetupData->wLength > 0U) {
				Result = XUsbPsu_EpBufferSend(
				(struct XUsbPsu*)InstancePtr->PrivateData,
				0U, DfuReply, (u32)SetupData->wLength);
			}
			break;
		default:
			/* Unsupported command. Stall the end point. */
			DfuObj.CurrState = XLOADER_STATE_DFU_ERROR;
			XUsbPsu_Ep0StallRestart(
				(struct XUsbPsu*)InstancePtr->PrivateData);
			break;
	}
}
#endif
