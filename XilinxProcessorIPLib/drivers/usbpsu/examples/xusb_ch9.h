/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
/** @brief Packed attribute macro for cross-compiler compatibility. */
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
	u8 bLength;	/**< B Length. */
	u8 bDescriptorType;	/**< B Descriptor Type. */
	u16 bcdUSB;	/**< Bcd U S B. */
	u8 bDeviceClass;	/**< B Device Class. */
	u8 bDeviceSubClass;	/**< B Device Sub Class. */
	u8 bDeviceProtocol;	/**< B Device Protocol. */
	u8 bMaxPacketSize0;	/**< B Max Packet Size0. */
	u16 idVendor;	/**< Id Vendor. */
	u16 idProduct;	/**< Id Product. */
	u16 bcdDevice;	/**< Bcd Device. */
	u8 iManufacturer;	/**< I Manufacturer. */
	u8 iProduct;	/**< I Product. */
	u8 iSerialNumber;	/**< I Serial Number. */
	u8 bNumConfigurations;	/**< B Num Configurations. */
} attribute(USB_STD_DEV_DESC);

typedef struct {
	u8 bLength;
	u8 bType;	/**< B Type. */
	u16 wTotalLength;	/**< W Total Length. */
	u8 bNumberInterfaces;	/**< B Number Interfaces. */
	u8 bConfigValue;	/**< B Config Value. */
	u8 bIConfigString;	/**< B I Config String. */
	u8 bAttributes;	/**< B Attributes. */
	u8 bMaxPower;	/**< B Max Power. */
} attribute(USB_STD_CFG_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;	/**< B Interface Number. */
	u8 bAlternateSetting;	/**< B Alternate Setting. */
	u8 bNumEndPoints;	/**< B Num End Points. */
	u8 bInterfaceClass;	/**< B Interface Class. */
	u8 bInterfaceSubClass;	/**< B Interface Sub Class. */
	u8 bInterfaceProtocol;	/**< B Interface Protocol. */
	u8 iInterface;	/**< I Interface. */
} attribute(USB_STD_IF_DESC);


typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;	/**< B Endpoint Address. */
	u8 bmAttributes;	/**< Bm Attributes. */
	u8 bMaxPacketSizeL;	/**< B Max Packet Size L. */
	u8 bMaxPacketSizeH;	/**< B Max Packet Size H. */
	u8 bInterval;	/**< B Interval. */
} attribute(USB_STD_EP_DESC);

/*
 * SUPERSPEED USB ENDPOINT COMPANION descriptor structure
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bMaxBurst;	/**< B Max Burst. */
	u8 bmAttributes;
	u16 wBytesPerInterval;	/**< W Bytes Per Interval. */
} attribute (USB_STD_EP_SS_COMP_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wLANGID[1];	/**< W L A N G I D. */
} attribute(USB_STD_STRING_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumDeviceCaps;	/**< B Num Device Caps. */
} attribute(USB_STD_BOS_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;	/**< B Dev Capability Type. */
	u32 bmAttributes;	/**< Bm Attributes. */
} attribute(USB_STD_DEVICE_CAP_7BYTE);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDevCapabiltyType;
	u8 bmAttributes;
	u16 wSpeedsSupported;	/**< W Speeds Supported. */
	u8 bFunctionalitySupport;	/**< B Functionality Support. */
	u8 bU1DevExitLat;	/**< B U1 Dev Exit Lat. */
	u16 wU2DevExitLat;	/**< W U2 Dev Exit Lat. */
} attribute(USB_STD_DEVICE_CAP_10BYTE);

typedef struct {
	USB_STD_BOS_DESC	bos_desc;	/**< Bos_desc. */
	USB_STD_DEVICE_CAP_7BYTE dev_cap7;	/**< Dev_cap7. */
	USB_STD_DEVICE_CAP_10BYTE dev_cap10;	/**< Dev_cap10. */
} attribute(USB_BOS_DESC);

typedef struct {
	u32 (*Usb_Ch9SetupDevDescReply)(struct Usb_DevData *, u8 *, u32);	/**< Usb_ Ch9 Setup Dev Desc Reply)(struct  Usb_ Dev Data *, u... */
	u32 (*Usb_Ch9SetupCfgDescReply)(struct Usb_DevData *, u8 *, u32);	/**< Usb_ Ch9 Setup Cfg Desc Reply)(struct  Usb_ Dev Data *, u... */
	u32 (*Usb_Ch9SetupBosDescReply)(u8 *, u32);	/**< Usb_ Ch9 Setup Bos Desc Reply)(u8 *, u32). */
	u32 (*Usb_Ch9SetupStrDescReply)(struct Usb_DevData *, u8 *, u32, u8);	/**< Usb_ Ch9 Setup Str Desc Reply)(struct  Usb_ Dev Data *, u... */
	s32 (*Usb_SetConfiguration)(struct Usb_DevData *, SetupPacket *);	/**< Usb_ Set Configuration)(struct  Usb_ Dev Data *,  Setup P... */
	s32 (*Usb_SetConfigurationApp)(struct Usb_DevData *, SetupPacket *);	/**< Usb_ Set Configuration App)(struct  Usb_ Dev Data *,  Set... */
	void (*Usb_SetInterfaceHandler)(struct Usb_DevData *, SetupPacket *);	/**< Usb_ Set Interface Handler)(struct  Usb_ Dev Data *,  Set... */
	void (*Usb_ClassReq)(struct Usb_DevData *, SetupPacket *);	/**< Usb_ Class Req)(struct  Usb_ Dev Data *,  Setup Packet *). */
	u32 (*Usb_GetDescReply)(struct Usb_DevData *, SetupPacket *, u8 *);	/**< Usb_ Get Desc Reply)(struct  Usb_ Dev Data *,  Setup Pack... */
} attribute(CH9FUNC_CONTAINER);

typedef struct {
	CH9FUNC_CONTAINER ch9_func;	/**< Ch9_func. */
	void *data_ptr;	/**< Data_ptr. */
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

#define USB_ENDPOINT_NUMBER_MASK	0x0f	/**< Mask to extract endpoint number from address. */
#define USB_ENDPOINT_DIR_MASK		0x80	/**< Mask to extract endpoint direction from address. */

#define USB_ENDPOINT_XFERTYPE_MASK		0x03	/**< Mask to extract endpoint transfer type. */
/** @} */

/**
 * @name Request Values
 * @{
 */
#define USB_REQ_GET_STATUS			0x00	/**< USB Get Status request code. */
#define USB_REQ_CLEAR_FEATURE		0x01	/**< USB Clear Feature request code. */
#define USB_REQ_SET_FEATURE			0x03	/**< USB Set Feature request code. */
#define USB_REQ_SET_ADDRESS			0x05	/**< USB Set Address request code. */
#define USB_REQ_GET_DESCRIPTOR		0x06	/**< USB Get Descriptor request code. */
#define USB_REQ_SET_DESCRIPTOR		0x07	/**< USB Set Descriptor request code. */
#define USB_REQ_GET_CONFIGURATION	0x08	/**< USB Get Configuration request code. */
#define USB_REQ_SET_CONFIGURATION	0x09	/**< USB Set Configuration request code. */
#define USB_REQ_GET_INTERFACE		0x0a	/**< USB Get Interface request code. */
#define USB_REQ_SET_INTERFACE		0x0b	/**< USB Set Interface request code. */
#define USB_REQ_SYNC_FRAME			0x0c	/**< USB Sync Frame request code. */
#define USB_REQ_SET_SEL				0x30	/**< USB Set SEL request code. */
#define USB_REQ_SET_ISOCH_DELAY		0x31	/**< USB Set Isochronous Delay request code. */
/** @} */

/**
 * @name Feature Selectors
 * @{
 */
#define USB_ENDPOINT_HALT			0x00	/**< Endpoint halt feature selector. */
#define USB_DEVICE_REMOTE_WAKEUP	0x01	/**< Remote wakeup feature selector. */
#define USB_TEST_MODE				0x02	/**< Test mode feature selector. */
#define USB_U1_ENABLE				0x30	/**< U1 enable feature selector. */
#define USB_U2_ENABLE				0x31	/**< U2 enable feature selector. */
#define USB_INTRF_FUNC_SUSPEND		0x00	/**< Interface function suspend feature selector. */
/** @} */

/**
 * @name Descriptor Types
 * @{
 */
#define USB_TYPE_DEVICE_DESC			0x01	/**< USB Device Descriptor type. */
#define USB_TYPE_CONFIG_DESC			0x02	/**< USB Configuration Descriptor type. */
#define USB_TYPE_STRING_DESC			0x03	/**< USB String Descriptor type. */
#define USB_TYPE_INTERFACE_DESC			0x04	/**< USB Interface Descriptor type. */
#define USB_TYPE_ENDPOINT_CFG_DESC		0x05	/**< USB Endpoint Descriptor type. */
#define USB_TYPE_DEVICE_QUALIFIER		0x06	/**< USB Device Qualifier Descriptor type. */
#define OSD_TYPE_CONFIG_DESCR			0x07	/**< OS-specific Configuration Descriptor type. */
#define USB_TYPE_INTERFACE_ASSOCIATION	0x0b	/**< USB Interface Association Descriptor type. */
#define USB_TYPE_BOS_DESC				0x0F	/**< USB BOS Descriptor type. */
#define USB_TYPE_HID_DESC				0x21	/**< USB HID Descriptor type. */
#define USB_TYPE_REPORT_DESC			0x22	/**< USB HID Report Descriptor type. */
#define USB_TYPE_DFUFUNC_DESC			0x21	/**< USB DFU Functional Descriptor type. */
/** @} */

/**
 * @name Status type
 * @{
 */
#define USB_STATUS_MASK			0x3	/**< Mask for USB status recipient type. */
#define USB_STATUS_DEVICE		0x0	/**< USB status: Device status. */
#define USB_STATUS_INTERFACE	0x1	/**< USB status: Interface status. */
#define USB_STATUS_ENDPOINT		0x2	/**< USB status: Endpoint status. */
/** @} */

/**
 * @name EP Types
 * @{
 */
#define USB_EP_CONTROL			0	/**< Endpoint type: Control. */
#define USB_EP_ISOCHRONOUS		1	/**< Endpoint type: Isochronous. */
#define USB_EP_BULK				2	/**< Endpoint type: Bulk. */
#define USB_EP_INTERRUPT		3	/**< Endpoint type: Interrupt. */
/** @} */

/**
 * @name Device Classes
 * @{
 */
#define USB_CLASS_AUDIO			0x01	/**< USB class code: Audio. */
#define USB_CLASS_HID			0x03	/**< USB class code: HID. */
#define USB_CLASS_STORAGE		0x08	/**< USB class code: Mass Storage. */
#define USB_CLASS_MISC			0xEF	/**< USB class code: Miscellaneous. */
#define USB_CLASS_DFU			0xFE	/**< USB class code: DFU. */
#define USB_CLASS_VENDOR		0xFF	/**< USB class code: Vendor-specific. */
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void Ch9Handler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_H */
