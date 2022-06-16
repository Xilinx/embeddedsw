/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00   bsv 02/10/2019 First release
*        bsv 04/09/2020 Code clean up
* 1.01   bsv 07/08/2020 Moved Ch9Handler APIs from xloader_usb.c
*        td  08/19/2020 Fixed MISRA C violations Rule 10.3
*        bsv 10/13/2020 Code clean up
*        td	 10/19/2020	MISRA C Fixes
* 1.02   bsv 08/31/2021 Code clean up
* 1.03   kpt 12/13/2021 Replaced Xil_SecureMemCpy with Xil_SMemCpy
*
* </pre>
*
*****************************************************************************/
#include "xplmi_hw.h"
#include "xloader_dfu_util.h"
#ifdef XLOADER_USB
#include "xparameters.h"	/* XPAR parameters */
#include "xusbpsu.h"		/* USB controller driver */
#include "xloader_usb.h"
#include "xloader.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/***************** Function Prototypes ***************************************/
static u8 XLoader_Ch9SetupDevDescReply(const struct Usb_DevData* InstancePtr,
	u8 *BufPtr, u32 BufferLen);
static u8 XLoader_Ch9SetupCfgDescReply(const struct Usb_DevData* InstancePtr,
	u8 *BufPtr, const u32 BufferLen);
static u8 XLoader_Ch9SetupStrDescReply(const struct Usb_DevData* InstancePtr,
	u8 *BufPtr, const u32 BufferLen, u8 Index);
static u8 XLoader_Ch9SetupBosDescReply(const struct Usb_DevData* InstancePtr,
	u8 *BufPtr, u32 BufferLen);
static int XLoader_UsbReqGetStatus(const struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData);
static int XLoader_UsbReqSetFeature(const struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData);
static void XLoader_StdDevReq(struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData);
static int XLoader_UsbReqGetDescriptor(const struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData);
static void XLoader_DfuClassReq(const struct Usb_DevData* InstancePtr,
	const SetupPacket *SetupData);
static int XLoader_SetConfiguration(struct Usb_DevData* InstancePtr,
	const SetupPacket *Ctrl);
static void XLoader_DfuSetIntf(const struct Usb_DevData* InstancePtr,
	const SetupPacket *SetupData);

/**************************** Type Definitions *******************************/
struct XLoaderPs_DfuIf DfuObj;

/* Initialize a DFU data structure */
XLoader_UsbCh9_Data Dfu_data = {
        .Ch9_func = {
                .XLoaderPs_Ch9SetupDevDescReply = XLoader_Ch9SetupDevDescReply,
                .XLoaderPs_Ch9SetupCfgDescReply = XLoader_Ch9SetupCfgDescReply,
                .XLoaderPs_Ch9SetupBosDescReply = XLoader_Ch9SetupBosDescReply,
                .XLoaderPs_Ch9SetupStrDescReply = XLoader_Ch9SetupStrDescReply,
                .XLoaderPs_SetConfiguration = XLoader_SetConfiguration,
                /* Hook the set interface handler */
                .XLoaderPs_SetInterfaceHandler = XLoader_DfuSetIntf,
                /* Hook up storage class handler */
                .XLoaderPs_ClassReq = XLoader_DfuClassReq,
                /* Set the DFU address for call back */
        },
        .Data_ptr = (void *)&DfuObj,
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
	DfuObj.DfuWaitForInterrupt = (u8)TRUE;
	while (DfuObj.DfuWaitForInterrupt == (u8)FALSE) {
		;
	}
}

/******************************************************************************/
/**
 * @brief	This function returns a string descriptor for the given index.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	BufPtr is a  pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 * @param	Index is the index of the string for which the descriptor
 *			is requested.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
static u8 XLoader_Ch9SetupStrDescReply(const struct Usb_DevData* InstancePtr, u8 *BufPtr,
	const u32 BufferLen, u8 Index)
{
	int Status = XST_FAILURE;
	u32 LoopVar;
	const char* String;
	u32 StringLen;
	u8 DescLen = 0U;
	XLoaderPs_UsbStdStringDesc StringDesc;
	u8 StrIndex;
	/* String Descriptors */
	static const char* const StringList[XLOADER_USB_MODES_NUM]
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

	StringLen = Xil_Strnlen(String, XLOADER_MAX_STR_DESC_LEN);

	/*
	 * Index 0 is LangId which is special as we can not represent
	 * the string required in the table above.Therefore we handle
	 * index 0 as a special case.
	 */
	if (0U == Index) {
		StringDesc.Length = XLOADER_STRING_DESC_ZERO_SIZE;
		StringDesc.DescriptorType = XLOADER_STRING_DESC_ZERO_DESC_TYPE;
		StringDesc.LangId[0U] = XLOADER_STRING_DESC_LANG_ID_ZERO;
		for (StrIndex = 1U; StrIndex < XLOADER_STRING_SIZE; ++StrIndex) {
			StringDesc.LangId[StrIndex] = 0U;
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
	if (DescLen > BufferLen) {
		DescLen = 0U;
		goto END;
	}

	Status = Xil_SMemCpy(BufPtr, DescLen, &StringDesc, DescLen, DescLen);
	if (Status != XST_SUCCESS) {
		DescLen = 0U;
	}

END:
	return DescLen;
}

/*****************************************************************************/
/**
 * @brief	This function returns the device descriptor for the device.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	BufPtr is pointer to the buffer that is to be filled
 *			with the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
static u8 XLoader_Ch9SetupDevDescReply(const struct Usb_DevData* InstancePtr, u8 *BufPtr,
	u32 BufferLen)
{
	int Status = XST_FAILURE;
	u8 DevDescLength = 0U;
	/* Device Descriptors */
	const XLoaderPs_UsbStdDevDesc __attribute__ ((aligned(16U))) DDesc[] = {
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


	/* Check buffer pointer is there and buffer is big enough. */
	if (BufPtr == NULL) {
		goto END;
	}
	DevDescLength = sizeof(XLoaderPs_UsbStdDevDesc);
	if (BufferLen < DevDescLength) {
		DevDescLength = 0U;
		goto END;
	}

	Status = XUsbPsu_IsSuperSpeed((struct XUsbPsu*)InstancePtr->PrivateData);
	if(Status != XST_SUCCESS) {
		/* USB 2.0 */
		Status = Xil_SMemCpy(BufPtr, DevDescLength, &DDesc[0U], DevDescLength,
				DevDescLength);
		if (Status != XST_SUCCESS) {
			DevDescLength = 0U;
		}
	}
	else {
		/* USB 3.0 */
		Status = Xil_SMemCpy(BufPtr, DevDescLength, &DDesc[1U], DevDescLength,
				DevDescLength);
		if (Status != XST_SUCCESS) {
			DevDescLength = 0U;
		}
	}

END:
	return DevDescLength;
}

/*****************************************************************************/
/**
 * @brief	This function returns the configuration descriptor for the device.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
static u8 XLoader_Ch9SetupCfgDescReply(const struct Usb_DevData* InstancePtr, u8 *BufPtr,
		const u32 BufferLen)
{
	int Status = XST_FAILURE;
	const u8 *Config;
	u8 CfgDescLen = 0U;
	XLoaderPs_UsbConfig __attribute__ ((aligned(16U))) Config2 = {
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
		}
	};
	XLoaderPs_Usb30Config __attribute__ ((aligned(16U))) Config3 = {
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

	Status = Xil_SMemCpy(BufPtr, CfgDescLen, Config, CfgDescLen,
			CfgDescLen);
	if (Status != XST_SUCCESS) {
		CfgDescLen = 0U;
	}

END:
	return CfgDescLen;
}

/*****************************************************************************/
/**
 * @brief	This function returns the BOS descriptor for the device.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufferLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success and 0 on error.
 *
 ******************************************************************************/
static u8 XLoader_Ch9SetupBosDescReply(const struct Usb_DevData* InstancePtr, u8 *BufPtr,
	u32 BufferLen)
{
	int Status = XST_FAILURE;
	u8 UsbBosDescLen = 0U;
	const XLoaderPs_UsbBosDesc __attribute__ ((aligned(16U))) BosDesc = {
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

	(void)(InstancePtr);

	UsbBosDescLen = sizeof(XLoaderPs_UsbBosDesc);

	Status = Xil_SMemCpy(BufPtr, BufferLen, &BosDesc, UsbBosDescLen,
			UsbBosDescLen);
	if (Status != XST_SUCCESS) {
		UsbBosDescLen = 0U;
	}

	return UsbBosDescLen;
}

/*****************************************************************************/
/**
 * @brief	This function changes State of Core to USB configured State.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	Ctrl is a pointer to the Setup packet data
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XLoader_SetConfiguration(struct Usb_DevData* InstancePtr, const SetupPacket *Ctrl)
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
			Status = XST_FAILURE;
			break;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles setting of DFU state.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	DfuState is a value of the DFU state to be set
 *
 * @return	None
 *
 ******************************************************************************/
void XLoader_DfuSetState(const struct Usb_DevData* InstancePtr, u32 DfuState)
{
	int Status = XST_FAILURE;
	(void) InstancePtr;

	switch (DfuState) {
		case XLOADER_STATE_APP_IDLE:
			DfuObj.CurrState = XLOADER_STATE_APP_IDLE;
			DfuObj.NextState = XLOADER_STATE_APP_DETACH;
			DfuObj.CurrStatus = XLOADER_DFU_STATUS_OK;
			/* Set to runtime mode by default */
			DfuObj.IsDfu = (u8)FALSE;
			DfuObj.RuntimeToDfu = (u8)FALSE;
			if (DownloadDone == 0U) {
				DownloadDone = XLOADER_DFU_DOWNLOAD_NOT_STARTED;
			}
			else if (DownloadDone ==
				XLOADER_DFU_DOWNLOAD_IN_PROGRESS) {
				++DownloadDone;
			}
			Status = XST_SUCCESS;
			break;
		case XLOADER_STATE_APP_DETACH:
			if (DfuObj.CurrState == XLOADER_STATE_APP_IDLE) {
				DfuObj.CurrState = XLOADER_STATE_APP_DETACH;
				DfuObj.NextState = XLOADER_STATE_DFU_IDLE;
				/* Wait For USB Reset to happen */
				XLoader_DfuWaitForReset();
				/* Setting Dfu Mode */
				DfuObj.IsDfu = (u8)TRUE;
				/*
				 * Set this flag to indicate we are going
				 * from runtime to dfu mode
				 */
				DfuObj.RuntimeToDfu = (u8)TRUE;
				DfuObj.CurrState = XLOADER_STATE_DFU_IDLE;
				DfuObj.NextState = XLOADER_STATE_DFU_DOWNLOAD_SYNC;
				DfuObj.IsDfu = (u8)TRUE;
				Status = XST_SUCCESS;
			}
			else if (DfuObj.CurrState == XLOADER_STATE_DFU_IDLE) {
				/* Wait For USB Reset to happen */
				XLoader_DfuWaitForReset();
				DfuObj.CurrState = XLOADER_STATE_APP_IDLE;
				DfuObj.NextState = XLOADER_STATE_APP_DETACH;
				DfuObj.CurrStatus = XLOADER_DFU_STATUS_OK;
				DfuObj.IsDfu = (u8)FALSE;
				Status = XST_SUCCESS;
			}
			else {
				/* Error */
			}
			break;
		case XLOADER_STATE_DFU_IDLE:
			DfuObj.CurrState = XLOADER_STATE_DFU_IDLE;
			DfuObj.NextState = XLOADER_STATE_DFU_DOWNLOAD_SYNC;
			DfuObj.IsDfu = (u8)TRUE;
			++DownloadDone;
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
			Status = XST_FAILURE;
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
void XLoader_DfuReset(struct Usb_DevData* UsbInstancePtr)
{
	(void)(UsbInstancePtr);
	if (DfuObj.DfuWaitForInterrupt == (u8)TRUE) {
		/* Tell DFU that we got reset signal */
		DfuObj.DfuWaitForInterrupt = (u8)FALSE;
	}
}

/*****************************************************************************/
/**
 * @brief	This function handles DFU set interface.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return	None
 *
 ******************************************************************************/
static void XLoader_DfuSetIntf(const struct Usb_DevData* InstancePtr, const SetupPacket *SetupData)
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
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is a pointer to setup token of control transfer
 *
 * @return	None
 *
 ******************************************************************************/
static void XLoader_DfuClassReq(const struct Usb_DevData* InstancePtr, const SetupPacket *SetupData)
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
						XLOADER_STATE_DFU_IDLE;
					DfuObj.GotDnloadRqst = (u8)FALSE;
					DfuObj.TotalTransfers = 0U;
				}
			}

			if((DfuObj.GotDnloadRqst == (u8)TRUE) &&
				(Result == XST_SUCCESS)) {
				DfuObj.CurrState =
					XLOADER_STATE_DFU_DOWNLOAD_IDLE;
				DfuObj.GotDnloadRqst = (u8)FALSE;
			}
			break;
		case XLOADER_DFU_GETSTATUS:
			if (DfuObj.GotDnloadRqst == (u8)TRUE) {
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

	return;
}

/*****************************************************************************/
/**
 * @brief	This function handles a standard Get Descriptor request.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is the structure containing the setup request
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
static int XLoader_UsbReqGetDescriptor(const struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData)
{
	int Status = XST_FAILURE;
	u8 ReplyLen;
	u8 Reply[XLOADER_REQ_REPLY_LEN] = {0U};

	switch ((SetupData->wValue >> 8U) & 0xFFU) {
		case XLOADER_TYPE_DEVICE_DESC:
		case XLOADER_TYPE_DEVICE_QUALIFIER:
			ReplyLen = XLoader_Ch9SetupDevDescReply(InstancePtr,
					Reply, XLOADER_REQ_REPLY_LEN);
			if (ReplyLen > (u8)SetupData->wLength) {
				ReplyLen = (u8)SetupData->wLength;
			}

			if (ReplyLen != 0U) {
				if(((SetupData->wValue >> 8U) & 0xFFU) ==
					XLOADER_TYPE_DEVICE_QUALIFIER) {
					Reply[0U] = ReplyLen;
					Reply[1U] = 0x6U;
					Reply[2U] = 0x0U;
					Reply[3U] = 0x2U;
					Reply[4U] = 0xFFU;
					Reply[5U] = 0x00U;
					Reply[6U] = 0x0U;
					Reply[7U] = 0x10U;
					Reply[8U] = 0x0U;
					Reply[9U] = 0x0U;
				}
				Status = XUsbPsu_EpBufferSend(
					(struct XUsbPsu*)InstancePtr->PrivateData,
					0U, Reply, ReplyLen);
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		case XLOADER_TYPE_CONFIG_DESC:
			ReplyLen = XLoader_Ch9SetupCfgDescReply(InstancePtr,
					Reply, XLOADER_REQ_REPLY_LEN);

			if(ReplyLen > (u8)SetupData->wLength){
				ReplyLen = (u8)SetupData->wLength;
			}

			if (ReplyLen != 0U) {
				Status = XUsbPsu_EpBufferSend(
					(struct XUsbPsu*)InstancePtr->PrivateData,
					0U, Reply, ReplyLen);
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		case XLOADER_TYPE_STRING_DESC:
			ReplyLen = XLoader_Ch9SetupStrDescReply(InstancePtr, Reply,
					XLOADER_STRING_SIZE, (u8)(SetupData->wValue & 0xFFU));

			if(ReplyLen > (u8)SetupData->wLength){
				ReplyLen = (u8)SetupData->wLength;
			}

			if(ReplyLen != 0U) {
				Status = XUsbPsu_EpBufferSend(
					(struct XUsbPsu*)InstancePtr->PrivateData,
					0U, Reply, ReplyLen);
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		case XLOADER_TYPE_BOS_DESC:
			ReplyLen = XLoader_Ch9SetupBosDescReply(InstancePtr,
					Reply, XLOADER_REQ_REPLY_LEN);

			if (ReplyLen > (u8)SetupData->wLength) {
				ReplyLen = (u8)SetupData->wLength;
			}

			if (ReplyLen != 0U) {
				Status = XUsbPsu_EpBufferSend(
					(struct XUsbPsu*)InstancePtr->PrivateData,
					0U, Reply, ReplyLen);
			}
			else {
				Status = XST_SUCCESS;
			}
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function handles a Setup Data packet from the host.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is the structure containing the setup request
 *
 * @return	None
 *
******************************************************************************/
void XLoader_Ch9Handler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData)
{
	switch (SetupData->bRequestType & XLOADER_REQ_TYPE_MASK) {
		case XLOADER_CMD_STDREQ:
			XLoader_StdDevReq(InstancePtr, SetupData);
			break;
		case XLOADER_CMD_CLASSREQ:
			XLoader_DfuClassReq(InstancePtr, SetupData);
			break;
		default:
			/* Stall on Endpoint 0 */
			XLoader_Printf(DEBUG_INFO,
			"\nUnknown class req, stalling at %s\n\r", __func__);
			XUsbPsu_Ep0StallRestart(
				(struct XUsbPsu*)InstancePtr->PrivateData);
			break;
	}
}

/*****************************************************************************/
/**
 * @brief	This function handles a standard device request.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is the structure containing the setup request
 *
 * @return	None
 *
******************************************************************************/
static void XLoader_StdDevReq(struct Usb_DevData *InstancePtr, const SetupPacket *SetupData)
{
	int Status = XST_FAILURE;
	u8 TmpBuffer[XLOADER_DFU_STATUS_SIZE] = {0U};
	u8 EpNum = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_NUMBER_MASK);
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_DIR_MASK);

	/* Check that the requested reply length is not bigger than our reply
	 * buffer. This should never happen.*/
	if (SetupData->wLength > XLOADER_REQ_REPLY_LEN) {
		Status = XST_SUCCESS;
		goto END;
	}

	switch (SetupData->bRequest) {
		case XLOADER_REQ_GET_STATUS:
			Status = XLoader_UsbReqGetStatus(InstancePtr,
							SetupData);
			break;
		case XLOADER_REQ_SET_ADDRESS:
			Status = XUsbPsu_SetDeviceAddress(
				(struct XUsbPsu*)InstancePtr->PrivateData,
				SetupData->wValue);
			break;
		case XLOADER_REQ_GET_DESCRIPTOR:
			Status = XLoader_UsbReqGetDescriptor(InstancePtr,
							SetupData);
			break;
		case XLOADER_REQ_SET_CONFIGURATION:
			if ((SetupData->wValue & 0xFFU) == 1U) {
				Status = XLoader_SetConfiguration(InstancePtr,
								SetupData);
			}
			break;
		case XLOADER_REQ_GET_CONFIGURATION:
			Status = XST_SUCCESS;
			break;
		case XLOADER_REQ_SET_FEATURE:
			Status = XLoader_UsbReqSetFeature(InstancePtr,
							SetupData);
			break;
		case XLOADER_REQ_SET_INTERFACE:
			XLoader_DfuSetIntf(InstancePtr, SetupData);
			Status = XST_SUCCESS;
			break;
		case XLOADER_REQ_SET_SEL:
			Status = XUsbPsu_EpBufferRecv(
				(struct XUsbPsu*)InstancePtr->PrivateData, 0U,
				TmpBuffer, XLOADER_DFU_STATUS_SIZE);
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

END:
	if (Status != XST_SUCCESS) {
		/* Set the send stall bit if there was an error */
		XLoader_Printf(DEBUG_INFO, "Std dev req %d/%d error, stall 0"
				" in out\n\r", SetupData->bRequest,
				(SetupData->wValue >> 8U) & 0xFFU);
		if (EpNum == (u8)FALSE) {
			XUsbPsu_Ep0StallRestart(
				(struct XUsbPsu *)InstancePtr->PrivateData);
		} else {
			XUsbPsu_EpSetStall(
				(struct XUsbPsu *)InstancePtr->PrivateData,
				EpNum, Direction);
		}
	}
}

/*****************************************************************************/
/**
 * @brief	This function handles a standard Get Status request.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is the structure containing the setup request
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
static int XLoader_UsbReqGetStatus(const struct Usb_DevData *InstancePtr,
		const SetupPacket *SetupData)
{
	int Status = XST_FAILURE;
	u16 ShortVar;
	u8 Reply[XLOADER_REQ_REPLY_LEN] = {0U};
	u8 EpNum = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_NUMBER_MASK);
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_DIR_MASK);

	switch(SetupData->bRequestType & XLOADER_STATUS_MASK) {

		case XLOADER_STATUS_DEVICE:
			ShortVar = XLOADER_ENDPOINT_SELF_PWRD_STATUS;
			Status = Xil_SMemCpy(&Reply[0U], sizeof(u16), &ShortVar, sizeof(u16),
					sizeof(u16));/* Self powered */
			break;
		case XLOADER_STATUS_ENDPOINT:
			ShortVar = (u16)XUsbPsu_IsEpStalled(
				(struct XUsbPsu*)InstancePtr->PrivateData,
				EpNum, Direction);
			Status = Xil_SMemCpy(&Reply[0U], sizeof(u16), &ShortVar, sizeof(u16),
					 sizeof(u16));
			break;
		case XLOADER_STATUS_INTERFACE:
			Status = XST_SUCCESS;
			/* Need to send all zeroes as reply*/
			break;
		default:
			Status = XST_SUCCESS;
			break;
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SetupData->wLength != 0U) {
		Status = XUsbPsu_EpBufferSend(
			(struct XUsbPsu*)InstancePtr->PrivateData,
			0U, Reply, SetupData->wLength);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles a standard Set Feature request.
 *
 * @param	InstancePtr is a pointer to XUsbPsu instance of the controller
 * @param	SetupData is the structure containing the setup request
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
******************************************************************************/
static int XLoader_UsbReqSetFeature(const struct Usb_DevData *InstancePtr,
	const SetupPacket *SetupData)
{
	int Status = XST_FAILURE;
	u8 EpNum = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_NUMBER_MASK);
	/*
	 * Direction - 1 -- XUSBPSU_EP_DIR_IN
	 * Direction - 0 -- XUSBPSU_EP_DIR_OUT
	 */
	u8 Direction = (u8)(SetupData->wIndex & XLOADER_USB_ENDPOINT_DIR_MASK);

	switch(SetupData->bRequestType & XLOADER_STATUS_MASK) {
		case XLOADER_STATUS_ENDPOINT:
			if(SetupData->wValue == XLOADER_ENDPOINT_HALT) {
				if (EpNum == (u8)FALSE) {
					XUsbPsu_Ep0StallRestart(
					(struct XUsbPsu *)InstancePtr->PrivateData);
				}
				else {
					XUsbPsu_EpSetStall(
					(struct XUsbPsu *)InstancePtr->PrivateData,
					EpNum, Direction);
				}
			}
			Status = XST_SUCCESS;
			break;
		case XLOADER_STATUS_DEVICE:
			Status = XST_SUCCESS;
			break;
		default:
			break;
	}

	return Status;
}
#endif
