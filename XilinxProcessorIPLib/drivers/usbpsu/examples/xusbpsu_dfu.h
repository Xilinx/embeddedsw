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
 *
 * @file xusbpsu_dfu.h
 *
 * This file contains definitions used in the chapter 9 code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0	 vak  30/11/16 Addded DFU support
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef DFU_H
#define DFU_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xusbpsu_ch9.h"

/************************** Constant Definitions *****************************/

/* DFU states */
#define STATE_APP_IDLE                  0x00
#define STATE_APP_DETACH                0x01
#define STATE_DFU_IDLE                  0x02
#define STATE_DFU_DOWNLOAD_SYNC         0x03
#define STATE_DFU_DOWNLOAD_BUSY         0x04
#define STATE_DFU_DOWNLOAD_IDLE         0x05
#define STATE_DFU_MANIFEST_SYNC         0x06
#define STATE_DFU_MANIFEST              0x07
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08
#define STATE_DFU_UPLOAD_IDLE           0x09
#define STATE_DFU_ERROR                 0x0a


/* DFU status */
#define DFU_STATUS_OK                   0x00
#define DFU_STATUS_ERROR_TARGET         0x01
#define DFU_STATUS_ERROR_FILE           0x02
#define DFU_STATUS_ERROR_WRITE          0x03
#define DFU_STATUS_ERROR_ERASE          0x04
#define DFU_STATUS_ERROR_CHECK_ERASED   0x05
#define DFU_STATUS_ERROR_PROG           0x06
#define DFU_STATUS_ERROR_VERIFY         0x07
#define DFU_STATUS_ERROR_ADDRESS        0x08
#define DFU_STATUS_ERROR_NOTDONE        0x09
#define DFU_STATUS_ERROR_FIRMWARE       0x0a
#define DFU_STATUS_ERROR_VENDOR         0x0b
#define DFU_STATUS_ERROR_USBR           0x0c
#define DFU_STATUS_ERROR_POR            0x0d
#define DFU_STATUS_ERROR_UNKNOWN        0x0e
#define DFU_STATUS_ERROR_STALLEDPKT     0x0f

/* DFU commands */
#define DFU_DETACH      0
#define DFU_DNLOAD      1
#define DFU_UPLOAD      2
#define DFU_GETSTATUS   3
#define DFU_CLRSTATUS   4
#define DFU_GETSTATE    5
#define DFU_ABORT       6

/* DFU alternate setting value when in run-time mode */
#define DFU_ALT_SETTING 		1

#define USB_DEVICE_DESC			0x01
#define USB_CONFIG_DESC			0x02
#define USB_STRING_DESC			0x03
#define USB_INTERFACE_CFG_DESC		0x04
#define USB_ENDPOINT_CFG_DESC		0x05

#define DFUFUNC_DESCR				0x21  /* DFU Functional Desc */
#define DFU_MAX_TRANSFER			1024

/**************************** Type Definitions *******************************/

/*
 * Standard USB structures as per 2.0 specification
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} __attribute__((__packed__))USB_STD_DEV_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u16 wTotalLength;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
}  __attribute__((__packed__))USB_STD_CFG_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndPoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
}  __attribute__((__packed__))USB_STD_IF_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;
	u16 wTransferSize;
	u16 bcdDFUVersion;
}  __attribute__((__packed__))USB_DFU_FUNC_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
}  __attribute__((__packed__))USB_STD_EP_DESC;

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
  u8 bLength;
  u8 bDescriptorType;
  u8 bMaxBurst;
  u8 bmAttributes;
  u16 wBytesPerInterval;
} __attribute__((__packed__))USB_STD_EP_SS_COMP_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID[1];
}  __attribute__((__packed__))USB_STD_STRING_DESC;

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_DESC epout;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} __attribute__((__packed__))USB_CONFIG;

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epin;
	USB_STD_EP_SS_COMP_DESC epssin;
	USB_STD_EP_DESC epout;
	USB_STD_EP_SS_COMP_DESC epssout;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} __attribute__((__packed__))USB30_CONFIG;


typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} __attribute__((__packed__))DFU_USB_CONFIG;

typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg_alt_dfu;
	USB_DFU_FUNC_DESC dfu_func_desc;
} __attribute__((__packed__))DFU_USB30_CONFIG;


typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumDeviceCaps;
} __attribute__((__packed__))USB_STD_BOS_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u32 bmAttributes;
} __attribute__((__packed__))USB_STD_DEVICE_CAP_7BYTE;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u8 bmAttributes;
	u16 wSpeedsSupported;
	u8 bFunctionalitySupport;
	u8 bU1DevExitLat;
	u16 wU2DevExitLat;
} __attribute__((__packed__))USB_STD_DEVICE_CAP_10BYTE;

typedef struct{
	USB_STD_BOS_DESC	bos_desc;
	USB_STD_DEVICE_CAP_7BYTE dev_cap7;
	USB_STD_DEVICE_CAP_10BYTE dev_cap10;
} __attribute__((__packed__))USB_BOS_DESC;

struct dfu_status {
    unsigned char bStatus;
    unsigned int  bwPollTimeout;
    unsigned char bState;
    unsigned char iString;
};

struct dfu_if {
	USBCH9_DATA dfu_data;
	char curr_state;
	char next_state;
	struct XUsbPsu *InstancePtr;
	char status;
	char got_reset;
	u32 current_inf; /* current interface */
	USB_STD_DEV_DESC *DFUdeviceDesc;
	DFU_USB30_CONFIG *DFUconfig3;
	DFU_USB_CONFIG *DFUconfig2;
	char *DFUStringList2;
	char *DFUStringList3;
	char got_dnload_rqst;
	u32 total_transfers;
	u32 total_bytes_dnloaded;
	u32 total_bytes_uploaded;
	u8 dfu_wait_for_interrupt;
	u8 is_dfu;
	u8 runtime_to_dfu;
};

/************************** Function Prototypes ******************************/

int dfu_set_state(struct dfu_if *DFU, int dfu_state );
int dfu_if_init (struct XUsbPsu *UsbInstance);

#ifdef __cplusplus
}
#endif

#endif /* end of DFU_H*/
