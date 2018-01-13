/******************************************************************************
 *
 * Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
 *
 * @file xusb_ch9.h
 *
 * This file contains definitions used chapter 9 specific code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   sg  06/06/16  First release
 * 1.1	vak  30/11/16  updated for adding ch9 function callbacks
 * 1.2   mn  01/20/17  fix to assign EP number and direction from wIndex field
 * 1.4   BK  12/01/18  Renamed the file and added changes to have a common
 *		       example for all USB IPs.
 *
 * </pre>
 *
 *****************************************************************************/

#ifndef XUSB_CH9_H
#define XUSB_CH9_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xusb_wrapper.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_printf.h"

#ifdef CH9_DEBUG
#include <stdio.h>
#define printf	xil_printf
#endif

#ifdef __ICCARM__
#define attribute(attr) attr
#else
#define attribute(attr) __attribute__((__packed__)) attr
#endif

/************************** Constant Definitions *****************************/

/************************** TypeDef Definitions *****************************/
/*
 * Standard USB structures as per 2.0 specification
 */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif

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
} attribute(USB_STD_DEV_DESC);

typedef struct {
	u8 bLength;
	u8 bType;
	u16 wTotalLength;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
} attribute(USB_STD_CFG_DESC);

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
} attribute(USB_STD_IF_DESC);


typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
} attribute(USB_STD_EP_DESC);

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
  u8 bLength;
  u8 bDescriptorType;
  u8 bMaxBurst;
  u8 bmAttributes;
  u16 wBytesPerInterval;
} attribute (USB_STD_EP_SS_COMP_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID[1];
} attribute(USB_STD_STRING_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumDeviceCaps;
} attribute(USB_STD_BOS_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u32 bmAttributes;
} attribute(USB_STD_DEVICE_CAP_7BYTE);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u8 bmAttributes;
	u16 wSpeedsSupported;
	u8 bFunctionalitySupport;
	u8 bU1DevExitLat;
	u16 wU2DevExitLat;
} attribute(USB_STD_DEVICE_CAP_10BYTE);

typedef struct{
	USB_STD_BOS_DESC	bos_desc;
	USB_STD_DEVICE_CAP_7BYTE dev_cap7;
	USB_STD_DEVICE_CAP_10BYTE dev_cap10;
} attribute(USB_BOS_DESC);

typedef struct {
    u32 (*Usb_Ch9SetupDevDescReply)(struct Usb_DevData *,u8 *, u32);
    u32 (*Usb_Ch9SetupCfgDescReply)(struct Usb_DevData *,u8 *, u32);
    u32 (*Usb_Ch9SetupBosDescReply)(u8 *, u32);
    u32 (*Usb_Ch9SetupStrDescReply)(struct Usb_DevData *,u8 *, u32, u8);
    s32 (*Usb_SetConfiguration)(struct Usb_DevData *, SetupPacket *);
    s32 (*Usb_SetConfigurationApp)(struct Usb_DevData *, SetupPacket *);
	void (*Usb_SetInterfaceHandler)(struct Usb_DevData *, SetupPacket *);
	void (*Usb_ClassReq)(struct Usb_DevData *, SetupPacket *);
	u32 (*Usb_GetDescReply)(struct Usb_DevData *, SetupPacket *,u8 *);
} attribute(CH9FUNC_CONTAINER);

typedef struct {
	CH9FUNC_CONTAINER ch9_func;
	void * data_ptr;
} attribute(USBCH9_DATA);

#ifdef __ICCARM__
#pragma pack(pop)
#endif

/************************** Constant Definitions *****************************/
/**
 * @name Request types
 * @{
 */
#define USB_REQ_TYPE_MASK		0x60	/**< Mask for request opcode */

#define USB_CMD_STDREQ			0x00	/**< Standard Request */
#define USB_CMD_CLASSREQ		0x20	/**< Class Request */
#define USB_CMD_VENDREQ			0x40	/**< Vendor Request */

#define USB_ENDPOINT_NUMBER_MASK	0x0f
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK		0x03
/* @} */

/**
 * @name Request Values
 * @{
 */
#define USB_REQ_GET_STATUS			0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE			0x03
#define USB_REQ_SET_ADDRESS			0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0a
#define USB_REQ_SET_INTERFACE		0x0b
#define USB_REQ_SYNC_FRAME			0x0c
#define USB_REQ_SET_SEL				0x30
#define USB_REQ_SET_ISOCH_DELAY		0x31
/* @} */

/**
 * @name Feature Selectors
 * @{
 */
#define USB_ENDPOINT_HALT			0x00
#define USB_DEVICE_REMOTE_WAKEUP	0x01
#define USB_TEST_MODE				0x02
#define USB_U1_ENABLE				0x30
#define USB_U2_ENABLE				0x31
#define USB_INTRF_FUNC_SUSPEND		0x00	/* function suspend */
/* @} */

/**
 * @name Descriptor Types
 * @{
 */
#define USB_TYPE_DEVICE_DESC			0x01
#define USB_TYPE_CONFIG_DESC			0x02
#define USB_TYPE_STRING_DESC			0x03
#define USB_TYPE_INTERFACE_DESC			0x04
#define USB_TYPE_ENDPOINT_CFG_DESC		0x05
#define USB_TYPE_DEVICE_QUALIFIER		0x06
#define OSD_TYPE_CONFIG_DESCR			0x07
#define USB_TYPE_INTERFACE_ASSOCIATION	0x0b
#define USB_TYPE_BOS_DESC				0x0F
#define USB_TYPE_HID_DESC				0x21	// Get descriptor: HID
#define USB_TYPE_REPORT_DESC			0x22	// Get descriptor:Report
#define USB_TYPE_DFUFUNC_DESC			0x21    /* DFU Functional Desc */
/* @} */

/**
 * @name Status type
 * @{
 */
#define USB_STATUS_MASK			0x3
#define USB_STATUS_DEVICE		0x0
#define USB_STATUS_INTERFACE	0x1
#define USB_STATUS_ENDPOINT		0x2
/* @} */

/**
 * @name EP Types
 * @{
 */
#define USB_EP_CONTROL			0
#define USB_EP_ISOCHRONOUS		1
#define USB_EP_BULK				2
#define USB_EP_INTERRUPT		3
/* @} */

/**
 * @name Device Classes
 * @{
 */
#define USB_CLASS_AUDIO			0x01
#define USB_CLASS_HID			0x03
#define USB_CLASS_STORAGE		0x08
#define USB_CLASS_MISC			0xEF
#define USB_CLASS_DFU			0xFE
#define USB_CLASS_VENDOR		0xFF
/* @} */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void Ch9Handler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_H */
