/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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
* 1.0   bvikram  02/10/19 First release
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
	u16 LangId[STRING_SIZE];
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
	XLoaderPs_UsbStdCfgDesc StdCfg;
	XLoaderPs_UsbStdIfDesc IfCfgAltDfu;
	XLoaderPs_UsbDfuFuncDesc DfuFuncDesc;
} __attribute__((__packed__))XLoaderPs_DfuUsbConfig;



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
	u16 U2DevExitLat;
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
	u32 CurrentInf; /* current interface */
	u8 GotDnloadRqst;
	u32 TotalTransfers;
	u32 TotalBytesDnloaded;
	volatile u8 DfuWaitForInterrupt;
	u8 IsDfu;
	u8 RuntimeToDfu;
};

typedef struct {
    u32 (*XLoaderPs_Ch9SetupDevDescReply)(struct Usb_DevData *, u8 *, u32);
    u32 (*XLoaderPs_Ch9SetupCfgDescReply)(struct Usb_DevData *, u8 *, u32);
    u32 (*XLoaderPs_Ch9SetupBosDescReply)(struct Usb_DevData *, u8 *, u32);
    u32 (*XLoaderPs_Ch9SetupStrDescReply)(struct Usb_DevData *, u8 *, u32, u8);
    int (*XLoaderPs_SetConfiguration)(struct Usb_DevData *, SetupPacket *);
	int (*XLoaderPs_SetConfigurationApp)(struct Usb_DevData *, SetupPacket *);
	void (*XLoaderPs_SetInterfaceHandler)(struct Usb_DevData *, SetupPacket *);
	void (*XLoaderPs_ClassReq)(struct Usb_DevData *, SetupPacket *);
	u32 (*XLoaderPs_GetDescReply)(struct Usb_DevData *, SetupPacket *,u8 *);
}XLoader_Ch9Func_Container;

typedef struct {
	XLoader_Ch9Func_Container ch9_func;
	void * data_ptr;
}XLoader_UsbCh9_Data;

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

#endif/*XLOADER_USB*/

#ifdef __cplusplus
}
#endif


#endif /* SRC_XLOADER_DFU_UTIL_H_ */
