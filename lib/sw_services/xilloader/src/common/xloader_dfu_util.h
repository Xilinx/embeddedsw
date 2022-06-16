/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xloader_dfu_util.h
*
* This file contains declarations of the DFU specific functions and structures to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00   bsv 02/10/2019 First release
*        bsv 04/09/2020 Code clean up
* 1.01   bsv 07/08/2020 Moved Ch9Handler APIs from xloader_usb.c
*        td  08/19/2020 Fixed MISRA C violations Rule 10.3
* 1.02   bsv 08/31/2021 Code clean up
*
* </pre>
*
*****************************************************************************/


#ifndef XLOADER_DFU_UTIL_H_
#define XLOADER_DFU_UTIL_H_
#include "xloader_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XLOADER_USB
/**************************** Type Definitions *******************************/
#define XLOADER_USB_DEVICE_DESC				(u8)(0x01U)
#define XLOADER_USB_CONFIG_DESC				(u8)(0x02U)
#define XLOADER_USB_INTERFACE_CFG_DESC			(u8)(0x04U)
#define XLOADER_USB_ENDPOINT_CFG_DESC			(u8)(0x05U)
#define XLOADER_STRING_SIZE				(128U)
#define XLOADER_STRING_DESC_LANG_ID_ZERO		(u16)(0x0409U)
#define XLOADER_STRING_DESC_ZERO_DESC_TYPE		(u8)(0x03U)
#define XLOADER_STRING_DESC_ZERO_SIZE			(u8)(0x04U)
#define XLOADER_STRING_DESC_TYPE			(u8)(0x03U)
#define XLOADER_DFUFUNC_DESCR				(u8)(0x21U)    /* DFU Functional Desc */
#define XLOADER_USB_MODES_NUM				(2U)
#define XLOADER_STRING_DESCRIPTORS_NUM			(6U)
#define XLOADER_MAX_STR_DESC_LEN			(25U)

/* DFU macros */
#define XLOADER_DFU_MAX_TRANSFER		(u16)(1024U)
/* DFU status */
#define XLOADER_DFU_STATUS_OK               	(0x0U)
/* DFU commands */
#define XLOADER_DFU_DETACH      		(0x0U)
#define XLOADER_DFU_DNLOAD      		(0x1U)
#define XLOADER_DFU_GETSTATUS   		(0x3U)
/* DFU alternate setting value when in run-time mode */
#define XLOADER_DFU_ALT_SETTING 		(0x1U)
/* DFU states */
#define XLOADER_STATE_APP_IDLE              	(0x00U)
#define XLOADER_STATE_APP_DETACH            	(0x01U)
#define XLOADER_STATE_DFU_IDLE              	(0x02U)
#define XLOADER_STATE_DFU_DOWNLOAD_SYNC     	(0x03U)
#define XLOADER_STATE_DFU_DOWNLOAD_BUSY     	(0x04U)
#define XLOADER_STATE_DFU_DOWNLOAD_IDLE     	(0x05U)
#define XLOADER_STATE_DFU_ERROR             	(0x0AU)

/* USB BOS macros */
#define XLOADER_USB_BOS_NUM_DEVICE_CAPS			(u8)(0x02U)
#define XLOADER_USB_BOS7_DESC_TYPE			(u8)(0x10U)
#define XLOADER_USB_BOS7_DEV_CAPABILITY_TYPE		(u8)(0x02U)
#define XLOADER_USB_BOS7_DISABLE_LPM			(0x00U)
#define XLOADER_USB_BOS10_DESC_TYPE			(u8)(0x10U)
#define XLOADER_USB_BOS10_DEV_CAPABILITY_TYPE		(u8)(0x03U)
#define XLOADER_USB_BOS10_ATTRB				(u8)(0x00U)
#define XLOADER_USB_BOS10_SPEEDS_SUPPORTED		(u16)(0x000FU)
#define XLOADER_USB_BOS10_FUNC_SUPPORTED		(u8)(0x01U)
#define XLOADER_USB_BOS10_DISABLE_LPM			(u8)(0x00U)

/* USB 2.0 macros */
#define XLOADER_USB2_BCD				(u16)(0x0200U)
#define XLOADER_USB2_BDEVICE_CLASS			(u8)(0x00U)
#define XLOADER_USB2_BDEVICE_SUBCLASS			(u8)(0x00U)
#define XLOADER_USB2_BDEVICE_PROTOCOL			(u8)(0x00U)
#define XLOADER_USB2_MAX_PACK_SIZE			(u8)(0x40U)
#define XLOADER_USB2_IDVENDOR				(u16)(0x03FDU)
#define XLOADER_USB2_IDPRODUCT				(u16)(0x0050U)
#define XLOADER_USB2_BDEVICE				(u16)(0x0100U)
#define XLOADER_USB2_MANUFACTURER			(u8)(0x01U)
#define XLOADER_USB2_IPRODUCT				(u8)(0x02U)
#define XLOADER_USB2_SERIAL_NUM				(u8)(0x03U)
#define XLOADER_USB2_NUM_CONFIG				(u8)(0x01U)

/* USB 2.0 Config macros */
#define XLOADER_USB2_CONFIG_NUM_INTF			(u8)(0x01U)
#define XLOADER_USB2_CONFIG_VAL				(u8)(0x01U)
#define XLOADER_USB2_CONFIGURATION			(u8)(0x00U)
#define XLOADER_USB2_CONFIG_ATTRB			(u8)(0xC0U)
#define XLOADER_USB2_CONFIG_MAX_PWR			(u8)(0x00U)

/* USB 2.0 Interface Config macros */
#define XLOADER_USB2_INTF_NUM				(u8)(0x00U)
#define XLOADER_USB2_INTF_ALT_SETTING			(u8)(0x00U)
#define XLOADER_USB2_INTF_NUM_ENDPOINTS			(u8)(0x02U)
#define XLOADER_USB2_INTF_CLASS				(u8)(0xFFU)
#define XLOADER_USB2_INTF_SUBCLASS			(u8)(0xFFU)
#define XLOADER_USB2_INTF_PROT				(u8)(0xFFU)
#define XLOADER_USB2_INTERFACE				(u8)(0x04U)

/* USB 2.0 Bulk In EndPoint macros */
#define XLOADER_USB2_BULK_IN_EP_ADDR			(u8)(0x81U)
#define XLOADER_USB2_BULK_IN_EP_ATTRB			(u8)(0x02U)
#define XLOADER_USB2_BULK_IN_EP_PKT_SIZE_LSB		(u8)(0x00U)
#define XLOADER_USB2_BULK_IN_EP_PKT_SIZE_MSB		(u8)(0x02U)
#define XLOADER_USB2_BULK_IN_EP_INTERVAL		(u8)(0x00U)

/* USB 2.0 Bulk Out EndPoint macros */
#define XLOADER_USB2_BULK_OUT_EP_ADDR			(u8)(0x01U)
#define XLOADER_USB2_BULK_OUT_EP_ATTRB			(u8)(0x02U)
#define XLOADER_USB2_BULK_OUT_EP_PKT_SIZE_LSB		(u8)(0x00U)
#define XLOADER_USB2_BULK_OUT_EP_PKT_SIZE_MSB		(u8)(0x02U)
#define XLOADER_USB2_BULK_OUT_EP_INTERVAL		(u8)(0x00U)

/* USB 2.0 DFU Interface Config macros */
#define XLOADER_USB2_DFU_INTF_NUM			(u8)(0x00U)
#define XLOADER_USB2_DFU_INTF_ALT_SETTING		(u8)(0x01U)
#define XLOADER_USB2_DFU_INTF_NUM_ENDPOINTS		(u8)(0x00U)
#define XLOADER_USB2_DFU_INTF_CLASS			(u8)(0xFEU)
#define XLOADER_USB2_DFU_INTF_SUBCLASS			(u8)(0x01U)
#define XLOADER_USB2_DFU_INTF_PROT			(u8)(0x02U)
#define XLOADER_USB2_DFU_INTERFACE			(u8)(0x04U)

/* USB 2.0 DFU functional descriptor */
#define XLOADER_USB2_DFUFUNC_ATTRB				(u8)(0x03U)
#define XLOADER_USB2_DFUFUNC_DETACH_TIMEOUT_MS			(u16)(8192U)
#define XLOADER_USB2_DFU_VERSION				(u16)(0x0110U)

/* USB 3.0 macros */
#define XLOADER_USB3_BCD				(u16)(0x0300U)
#define XLOADER_USB3_BDEVICE_CLASS			(u8)(0x00U)
#define XLOADER_USB3_BDEVICE_SUBCLASS			(u8)(0x00U)
#define XLOADER_USB3_BDEVICE_PROTOCOL			(u8)(0x00U)
#define XLOADER_USB3_MAX_PACK_SIZE			(u8)(0x09U)
#define XLOADER_USB3_IDVENDOR				(u16)(0x03FDU)
#define XLOADER_USB3_IDPRODUCT				(u16)(0x0050U)
#define XLOADER_USB3_BDEVICE				(u16)(0x0404U)
#define XLOADER_USB3_MANUFACTURER			(u8)(0x01U)
#define XLOADER_USB3_IPRODUCT				(u8)(0x02U)
#define XLOADER_USB3_SERIAL_NUM				(u8)(0x03U)
#define XLOADER_USB3_NUM_CONFIG				(u8)(0x01U)

/* USB 3.0 Config macros */
#define XLOADER_USB3_CONFIG_NUM_INTF			(u8)(0x01U)
#define XLOADER_USB3_CONFIG_VAL				(u8)(0x01U)
#define XLOADER_USB3_CONFIGURATION			(u8)(0x00U)
#define XLOADER_USB3_CONFIG_ATTRB			(u8)(0xC0U)
#define XLOADER_USB3_CONFIG_MAX_PWR			(u8)(0x00U)

/* USB 3.0 Interface Config macros */
#define XLOADER_USB3_INTF_NUM				(u8)(0x00U)
#define XLOADER_USB3_INTF_ALT_SETTING			(u8)(0x00U)
#define XLOADER_USB3_INTF_NUM_ENDPOINTS			(u8)(0x02U)
#define XLOADER_USB3_INTF_CLASS				(u8)(0xFFU)
#define XLOADER_USB3_INTF_SUBCLASS			(u8)(0xFFU)
#define XLOADER_USB3_INTF_PROT				(u8)(0xFFU)
#define XLOADER_USB3_INTERFACE				(u8)(0x04U)

/* USB 3.0 Bulk In EndPoint macros */
#define XLOADER_USB3_BULK_IN_EP_ADDR			(u8)(0x81U)
#define XLOADER_USB3_BULK_IN_EP_ATTRB			(u8)(0x02U)
#define XLOADER_USB3_BULK_IN_EP_PKT_SIZE_LSB		(u8)(0x00U)
#define XLOADER_USB3_BULK_IN_EP_PKT_SIZE_MSB		(u8)(0x04U)
#define XLOADER_USB3_BULK_IN_EP_INTERVAL		(u8)(0x00U)

/* USB 3.0 SS EndPoint macros */
#define XLOADER_USB3_SS_EP_DESC_TYPE			(u8)(0x30U)
#define XLOADER_USB3_SS_EP_MAX_BURST			(u8)(0x0FU)
#define XLOADER_USB3_SS_EP_ATTRB			(u8)(0x00U)
#define XLOADER_USB3_SS_EP_BYTES_PER_INTERVAL		(u16)(0x0000U)

/* USB 3.0 Bulk Out EndPoint macros */
#define XLOADER_USB3_BULK_OUT_EP_ADDR			(u8)(0x01U)
#define XLOADER_USB3_BULK_OUT_EP_ATTRB			(u8)(0x02U)
#define XLOADER_USB3_BULK_OUT_EP_PKT_SIZE_LSB		(u8)(0x00U)
#define XLOADER_USB3_BULK_OUT_EP_PKT_SIZE_MSB		(u8)(0x04U)
#define XLOADER_USB3_BULK_OUT_EP_INTERVAL		(u8)(0x00U)

/* USB 3.0 DFU Interface Config macros */
#define XLOADER_USB3_DFU_INTF_NUM			(u8)(0x00U)
#define XLOADER_USB3_DFU_INTF_ALT_SETTING		(u8)(0x01U)
#define XLOADER_USB3_DFU_INTF_NUM_ENDPOINTS		(u8)(0x00U)
#define XLOADER_USB3_DFU_INTF_CLASS			(u8)(0xFEU)
#define XLOADER_USB3_DFU_INTF_SUBCLASS			(u8)(0x01U)
#define XLOADER_USB3_DFU_INTF_PROT			(u8)(0x02U)
#define XLOADER_USB3_DFU_INTERFACE			(u8)(0x04U)

/* USB 3.0 DFU functional descriptor */
#define XLOADER_USB3_DFUFUNC_ATTRB			(u8)(0x03U)
#define XLOADER_USB3_DFUFUNC_DETACH_TIMEOUT_MS		(u16)(8192U)
#define XLOADER_USB3_DFU_VERSION			(u16)(0x0110U)

/* Macros to track DFU Download status */
#define XLOADER_DFU_DOWNLOAD_NOT_STARTED		(1U)
#define XLOADER_DFU_DOWNLOAD_IN_PROGRESS		(2U)
#define XLOADER_DOWNLOAD_COMPLETE	(3U)

/*
 * Standard USB structures as per 2.0 specification
 */
typedef struct {
	u8 Length;
	u8 DescriptorType;
	u16 BcdUsb;
	u8 DeviceClass;
	u8 DeviceSubClass;
	u8 DeviceProtocol;
	u8 MaxPacketSize0;
	u16 IdVendor;
	u16 IdProduct;
	u16 BcdDevice;
	u8 Manufacturer;
	u8 Product;
	u8 SerialNumber;
	u8 NumConfigurations;
} __attribute__((__packed__))XLoaderPs_UsbStdDevDesc;

typedef struct {
	u8 Length;
	u8 Type;
	u16 TotalLength;
	u8 NumberInterfaces;
	u8 ConfigValue;
	u8 IConfigString;
	u8 Attributes;
	u8 MaxPower;
}  __attribute__((__packed__))XLoaderPs_UsbStdCfgDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 InterfaceNumber;
	u8 AlternateSetting;
	u8 NumEndPoints;
	u8 InterfaceClass;
	u8 InterfaceSubClass;
	u8 InterfaceProtocol;
	u8 Interface;
}  __attribute__((__packed__))XLoaderPs_UsbStdIfDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 Attributes;
	u16 DetachTimeOut;
	u16 TransferSize;
	u16 BcdDfuVersion;
}  __attribute__((__packed__))XLoaderPs_UsbDfuFuncDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 EndpointAddress;
	u8 Attributes;
	u8 MaxPacketSizeL;
	u8 MaxPacketSizeH;
	u8 Interval;
}  __attribute__((__packed__))XLoaderPs_UsbStdEpDesc;

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
  u8 Length;
  u8 DescriptorType;
  u8 MaxBurst;
  u8 Attributes;
  u16 BytesPerInterval;
} __attribute__((__packed__))XLoaderPs_UsbStdEpSsCompDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u16 LangId[XLOADER_STRING_SIZE];
}  __attribute__((__packed__))XLoaderPs_UsbStdStringDesc;

typedef struct {
	XLoaderPs_UsbStdCfgDesc StdCfg;
	XLoaderPs_UsbStdIfDesc IfCfg;
	XLoaderPs_UsbStdEpDesc Epin;
	XLoaderPs_UsbStdEpDesc Epout;
	XLoaderPs_UsbStdIfDesc IfCfgAltDfu;
	XLoaderPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XLoaderPs_UsbConfig;

typedef struct {
	XLoaderPs_UsbStdCfgDesc StdCfg;
	XLoaderPs_UsbStdIfDesc IfCfg;
	XLoaderPs_UsbStdEpDesc Epin;
	XLoaderPs_UsbStdEpSsCompDesc Epssin;
	XLoaderPs_UsbStdEpDesc Epout;
	XLoaderPs_UsbStdEpSsCompDesc Epssout;
	XLoaderPs_UsbStdIfDesc IfCfgAltDfu;
	XLoaderPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XLoaderPs_Usb30Config;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u16 TotalLength;
	u8 NumDeviceCaps;
} __attribute__((__packed__))XLoaderPs_UsbStdBosDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 DevCapabiltyType;
	u32 Attributes;
} __attribute__((__packed__))XLoaderPs_UsbStdDeviceCap7Byte;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 DevCapabiltyType;
	u8 Attributes;
	u16 SpeedsSupported;
	u8 FunctionalitySupport;
	u8 U1DevExitLat;
	u8 U2DevExitLat;
} __attribute__((__packed__))XLoaderPs_UsbStdDeviceCap10Byte;

typedef struct{
	XLoaderPs_UsbStdBosDesc	BosDesc;
	XLoaderPs_UsbStdDeviceCap7Byte DevCap7;
	XLoaderPs_UsbStdDeviceCap10Byte DevCap10;
} __attribute__((__packed__))XLoaderPs_UsbBosDesc;

struct XLoaderPs_DfuIf {
	struct Usb_DevData* InstancePtr;
	u8 CurrState;
	u8 NextState;
	u8 CurrStatus;
	u8 GotReset;
	u32 CurrentInf; /**< Current interface */
	u8 GotDnloadRqst;
	u32 TotalTransfers;
	u32 TotalBytesDnloaded;
	volatile u8 DfuWaitForInterrupt;
	u8 IsDfu;
	u8 RuntimeToDfu;
};

typedef struct {
	u8 (*XLoaderPs_Ch9SetupDevDescReply)(const struct Usb_DevData* InstancePtr,
		u8 *BufPtr, u32 BufferLen);
	u8 (*XLoaderPs_Ch9SetupCfgDescReply)(const struct Usb_DevData* InstancePtr,
		u8 *BufPtr, const u32 BufferLen);
	u8 (*XLoaderPs_Ch9SetupBosDescReply)(const struct Usb_DevData* InstancePtr,
		u8 *BufPtr, u32 BufferLen);
	u8 (*XLoaderPs_Ch9SetupStrDescReply)(const struct Usb_DevData* InstancePtr,
		u8 *BufPtr, const u32 BufferLen, u8 Index);
	int (*XLoaderPs_SetConfiguration)(struct Usb_DevData* InstancePtr,
		const SetupPacket *SetupData);
	int (*XLoaderPs_SetConfigurationApp)(struct Usb_DevData* InstancePtr,
		SetupPacket *SetupData);
	void (*XLoaderPs_SetInterfaceHandler)(const struct Usb_DevData* InstancePtr,
		const SetupPacket *SetupData);
	void (*XLoaderPs_ClassReq)(const struct Usb_DevData* InstancePtr,
		const SetupPacket *SetupData);
	u32 (*XLoaderPs_GetDescReply)(struct Usb_DevData* InstancePtr,
                SetupPacket *SetupData, u8 *BufPtr);
}XLoader_Ch9Func_Container;

typedef struct {
	XLoader_Ch9Func_Container Ch9_func;
	void * Data_ptr;
}XLoader_UsbCh9_Data;

extern u8 DownloadDone;
extern u8* DfuVirtFlash;
extern XLoader_UsbCh9_Data Dfu_data;
extern struct XLoaderPs_DfuIf DfuObj;

/************************** Function Prototypes ******************************/
void XLoader_DfuReset(struct Usb_DevData* UsbInstancePtr);
void XLoader_DfuSetState(const struct Usb_DevData* InstancePtr, u32 DfuState);

#endif/*XLOADER_USB*/

#ifdef __cplusplus
}
#endif

#endif /* SRC_XLOADER_DFU_UTIL_H_ */
