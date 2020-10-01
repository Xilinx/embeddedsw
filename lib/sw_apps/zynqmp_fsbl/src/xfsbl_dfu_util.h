/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xfsbl_dfu_util.h
*
* This file contains declarations of the DFU specific functions and structures to be used
* in USB boot mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   bvikram  02/01/17 First release
* 2.0   bvikram  09/20/20 Fix USB boot issue
*
* </pre>
*
*****************************************************************************/


#ifndef XFSBL_DFU_UTIL_H_
#define XFSBL_DFU_UTIL_H_
#include "xfsbl_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XFSBL_USB
/**************************** Type Definitions *******************************/

#define USB_DEVICE_DESC				0x01U
#define USB_CONFIG_DESC				0x02U
#define USB_INTERFACE_CFG_DESC		0x04U
#define USB_ENDPOINT_CFG_DESC		0x05U
#define STRING_SIZE					128U
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
} __attribute__((__packed__))XFsblPs_UsbStdDevDesc;

typedef struct {
	u8 Length;
	u8 Type;
	u16 TotalLength;
	u8 NumberInterfaces;
	u8 ConfigValue;
	u8 IConfigString;
	u8 Attributes;
	u8 MaxPower;
}  __attribute__((__packed__))XFsblPs_UsbStdCfgDesc;

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
}  __attribute__((__packed__))XFsblPs_UsbStdIfDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 Attributes;
	u16 DetachTimeOut;
	u16 TransferSize;
	u16 BcdDfuVersion;
}  __attribute__((__packed__))XFsblPs_UsbDfuFuncDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 EndpointAddress;
	u8 Attributes;
	u8 MaxPacketSizeL;
	u8 MaxPacketSizeH;
	u8 Interval;
}  __attribute__((__packed__))XFsblPs_UsbStdEpDesc;

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
  u8 Length;
  u8 DescriptorType;
  u8 MaxBurst;
  u8 Attributes;
  u16 BytesPerInterval;
} __attribute__((__packed__))XFsblPs_UsbStdEpSsCompDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u16 LangId[STRING_SIZE];
}  __attribute__((__packed__))XFsblPs_UsbStdStringDesc;

typedef struct {
	XFsblPs_UsbStdCfgDesc StdCfg;
	XFsblPs_UsbStdIfDesc IfCfg;
	XFsblPs_UsbStdEpDesc Epin;
	XFsblPs_UsbStdEpDesc Epout;
	XFsblPs_UsbStdIfDesc IfCfgAltDfu;
	XFsblPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XFsblPs_UsbConfig;

typedef struct {
	XFsblPs_UsbStdCfgDesc StdCfg;
	XFsblPs_UsbStdIfDesc IfCfg;
	XFsblPs_UsbStdEpDesc Epin;
	XFsblPs_UsbStdEpSsCompDesc Epssin;
	XFsblPs_UsbStdEpDesc Epout;
	XFsblPs_UsbStdEpSsCompDesc Epssout;
	XFsblPs_UsbStdIfDesc IfCfgAltDfu;
	XFsblPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XFsblPs_Usb30Config;


typedef struct {
	XFsblPs_UsbStdCfgDesc StdCfg;
	XFsblPs_UsbStdIfDesc IfCfgAltDfu;
	XFsblPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XFsblPs_DfuUsbConfig;



typedef struct {
	u8 Length;
	u8 DescriptorType;
	u16 TotalLength;
	u8 NumDeviceCaps;
} __attribute__((__packed__))XFsblPs_UsbStdBosDesc;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 DevCapabiltyType;
	u32 Attributes;
} __attribute__((__packed__))XFsblPs_UsbStdDeviceCap7Byte;

typedef struct {
	u8 Length;
	u8 DescriptorType;
	u8 DevCapabiltyType;
	u8 Attributes;
	u16 SpeedsSupported;
	u8 FunctionalitySupport;
	u8 U1DevExitLat;
	u16 U2DevExitLat;
} __attribute__((__packed__))XFsblPs_UsbStdDeviceCap10Byte;

typedef struct{
	XFsblPs_UsbStdBosDesc	BosDesc;
	XFsblPs_UsbStdDeviceCap7Byte DevCap7;
	XFsblPs_UsbStdDeviceCap10Byte DevCap10;
} __attribute__((__packed__))XFsblPs_UsbBosDesc;

struct XFsblPs_DfuIf {
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
	u32 (*XFsblPs_Ch9SetupDevDescReply)(u8 *BufPtr, u32 BufferLen);
	u32 (*XFsblPs_Ch9SetupCfgDescReply)(u8 *BufPtr, const u32 BufferLen);
	u32 (*XFsblPs_Ch9SetupBosDescReply)(u8 *BufPtr, u32 BufferLen);
	u32 (*XFsblPs_Ch9SetupStrDescReply)(u8 *BufPtr, const u32 BufferLen, u8 Index);
	int (*XFsblPs_SetConfiguration)(struct Usb_DevData* InstancePtr,
		SetupPacket *SetupData);
	int (*XFsblPs_SetConfigurationApp)(const struct Usb_DevData* InstancePtr,
		SetupPacket *SetupData);
	void (*XFsblPs_SetInterfaceHandler)(struct Usb_DevData* InstancePtr,
		SetupPacket *SetupData);
	void (*XFsblPs_ClassReq)(struct Usb_DevData* InstancePtr,
		SetupPacket *SetupData);
	u32 (*XFsblPs_GetDescReply)(const struct Usb_DevData* InstancePtr,
                SetupPacket *SetupData, u8 *BufPtr);
}XFsbl_Ch9Func_Container;

typedef struct {
	XFsbl_Ch9Func_Container Ch9_func;
	void * Data_ptr;
}XFsbl_UsbCh9_Data;

#define DFU_MAX_TRANSFER			1024U
/* DFU status */
#define DFU_STATUS_OK               0x00U
/* DFU commands */
#define DFU_DETACH      			0x0U
#define DFU_DNLOAD      			0x1U
#define DFU_GETSTATUS   			0x3U

/* DFU alternate setting value when in run-time mode */
#define DFU_ALT_SETTING 			0x1U
/* DFU states */
#define STATE_APP_IDLE              0x00U
#define STATE_APP_DETACH            0x01U
#define STATE_DFU_IDLE              0x02U
#define STATE_DFU_DOWNLOAD_SYNC     0x03U
#define STATE_DFU_DOWNLOAD_BUSY     0x04U
#define STATE_DFU_DOWNLOAD_IDLE     0x05U
#define STATE_DFU_ERROR             0x0AU
/************************** Function Prototypes ******************************/

#endif/*XFSBL_USB*/

#ifdef __cplusplus
}
#endif


#endif /* SRC_XFSBL_DFU_UTIL_H_ */
