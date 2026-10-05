/******************************************************************************
* Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/******************************************************************************/
/**
 * @file xusb_types.h
 *
 * This file contains the constants, type definitions, variables as used in the
 * USB chapter 9 and mass storage demo application.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 * 4.00a hvm  12/20/10 Updated with the Audio device definitions.
 * 5.6   pm   07/05/23 Added support for system device-tree flow.
 * 5.8   ka   01/29/26 Fixed codespell errors.
 *
 * </pre>
 *****************************************************************************/

#ifndef __XUSBTYPES_H__
#define __XUSBTYPES_H__
#ifdef __cplusplus
#include "xusb.h"
extern "C" {
#endif

/*
 * Application specific constant.
 */
#undef HID_DEVICES
#undef USB_KEYBOARD
#undef USB_MOUSE
#define MASS_STORAGE_DEVICE	/**< Define to select mass storage device example. */
#undef MICROPHONE
/*
 * Chapter 9 requests
 */
#define GET_STATUS				0x00	/**< USB Chapter 9 GET_STATUS request code. */
#define CLEAR_FEATURE				0x01	/**< USB Chapter 9 CLEAR_FEATURE request code. */
#define GET_STATE				0x02	/**< Reserved USB Chapter 9 request code. */
#define SET_FEATURE				0x03	/**< USB Chapter 9 SET_FEATURE request code. */
#define SET_ADDRESS				0x05	/**< USB Chapter 9 SET_ADDRESS request code. */
#define GET_DESCRIPTOR				0x06	/**< USB Chapter 9 GET_DESCRIPTOR request code. */
#define SET_DESCRIPTOR				0x07	/**< USB Chapter 9 SET_DESCRIPTOR request code. */
#define GET_CONFIGURATION			0x08	/**< USB Chapter 9 GET_CONFIGURATION request code. */
#define SET_CONFIGURATION		 	0x09	/**< USB Chapter 9 SET_CONFIGURATION request code. */
#define GET_INTERFACE				0x0A	/**< USB Chapter 9 GET_INTERFACE request code. */
#define SET_INTERFACE				0x0B	/**< USB Chapter 9 SET_INTERFACE request code. */
#define SYCH_FRAME				0x0C	/**< USB Chapter 9 SYNCH_FRAME request code. */

/*
 * Test Mode Options
 */
#define TEST_J					1	/**< USB test mode: Test_J state. */
#define TEST_K					2	/**< USB test mode: Test_K state. */
#define TEST_SE0_NAK				3	/**< USB test mode: Test_SE0_NAK state. */
#define TEST_PKT				4	/**< USB test mode: Test packet state. */

/*
 * USB Mass Storage requests
 */
#define MS_RESET				0xFF	/**< USB mass storage bulk-only reset command code. */
#define MS_GETMAXLUN		   		0xFE	/**< USB mass storage GET_MAX_LUN command code. */

/*
 * USB Human Interface Device constant.
 */
#define USB_CLASS_HID			0x03	/**< USB HID class code. */

/*
 * Request types used during USB enumeration.
 */
#define STANDARD_IN_DEVICE			0x80	/**< Standard IN request directed to device. */
#define STANDARD_IN_INTERFACE			0x81	/**< Standard IN request directed to interface. */
#define STANDARD_IN_ENDPOINT			0x82	/**< Standard IN request directed to endpoint. */
#define STANDARD_OUT_DEVICE			0x00	/**< Standard OUT request directed to device. */
#define STANDARD_OUT_INTERFACE			0x01	/**< Standard OUT request directed to interface. */
#define STANDARD_OUT_ENDPOINT			0x02	/**< Standard OUT request directed to endpoint. */
#define TYPE_MASK			 	0x60	/**< Mask to extract request type from bmRequestType. */
#define TYPE_STANDARD				0x00	/**< Standard request type value. */
#define TYPE_CLASS				0x20	/**< Class-specific request type value. */
#define TYPE_VENDOR				0x40	/**< Vendor-specific request type value. */
#define TYPE_RESERVED				0x60	/**< Reserved request type value. */

/*
 * DATA Transfer Direction
 */
#define DIR_DEVICE_TO_HOST			0x80	/**< Data transfer direction: device to host. */

/*
 * Descriptor Types
 */
#define DEVICE_DESCR				0x01	/**< USB device descriptor type code. */
#define CONFIG_DESCR				0x02	/**< USB configuration descriptor type code. */
#define STRING_DESCR				0x03	/**< USB string descriptor type code. */
#define INTERFACE_DESCR				0x04	/**< USB interface descriptor type code. */
#define ENDPOINT_DESCR				0x05	/**< USB endpoint descriptor type code. */
#define QUALIFIER_DESCR				0x06	/**< USB device qualifier descriptor type code. */
#define OSD_CONFIG_DESCR			0x07	/**< USB other-speed configuration descriptor type. */
#define HID_DESC			      	0x21	/**< USB HID descriptor type code. */
#define REPORT_DESC			        0x22	/**< USB HID report descriptor type code. */

/*
 * Feature Selectors
 */

#define DEVICE_REMOTE_WAKEUP	0x01	/**< Feature selector: device remote wakeup. */
#define TEST_MODE				0x02	/**< Feature selector: test mode. */

/*
 * Phase States
 */
#define SETUP_PHASE				0x0000	/**< Control transfer setup phase indicator. */
#define DATA_PHASE				0x0001	/**< Control transfer data phase indicator. */
#define STATUS_PHASE		   	0x0002	/**< Control transfer status phase indicator. */

/*
 * End point types.
 */
#define EP_CONTROL			0	/**< Control Endpoint */
#define EP_ISOCHRONOUS			1	/**< Isochronous Endpoint */
#define EP_BULK				2	/**< Bulk Endpoint */
#define EP_INTERRUPT			3	/**< Interrupt Endpoint */

/*
 * Maximum number of USB interfaces.
 */
#define MAX_INTERFACES			0x01	/**< Maximum number of USB interfaces. */

/*
 * FPGA Configuration Number
 */
#define CONFIGURATION_ONE	   	0x01	/**< USB configuration number one. */

/*
 * EP0 Setup data size.
 */
#define EP0_SETUP_DATA				64	/**< EP0 setup data buffer size in bytes. */

/**
 * @brief USB Chapter 9 command buffer structure for control transfers.
 */
typedef struct {
	union {
		u8 StandardDeviceRequest; /**< Standard device request type alias. */
		u8 bmRequestType;         /**< Bitmap of request type and direction. */
	} Byte0; /**< Byte 0: request type / direction field. */
	union {
		u8 FbRequest; /**< Firmware-specific request alias. */
		u8 bRequest;  /**< USB standard request identifier. */
	} Byte1; /**< Byte 1: request identifier field. */
	union {
		struct {
			u8 bDescriptorType;  /**< Descriptor type code. */
			u8 bDescriptorIndex; /**< Descriptor index value. */
		} Byte23; /**< Bytes 2-3: descriptor type and index. */
		u16 FwValue;          /**< Firmware-specific wValue alias. */
		u16 wValue;           /**< USB standard wValue field. */
		u16 wFeatureSelector; /**< Feature selector for SET/CLEAR_FEATURE. */
	} Word1; /**< Word 1 (bytes 2-3): wValue field. */
	union {
		struct {
			u8 Byteh; /**< Byte at offset 0; significance is platform-dependent. */
			u8 Bytel; /**< Byte at offset 1; significance is platform-dependent. */
		} Byte45; /**< Bytes 4-5: wIndex as individual bytes. */
		u16 wTargetSelector; /**< Target selector alias for wIndex. */
		u16 FwIndex;         /**< Firmware-specific wIndex alias. */
		u16 wIndex;          /**< USB standard wIndex field. */
	} Word2; /**< Word 2 (bytes 4-5): wIndex field. */
	union {
		struct {
			u8 Byteh; /**< Byte at offset 0; significance is platform-dependent. */
			u8 Bytel; /**< Byte at offset 1; significance is platform-dependent. */
		} Byte67; /**< Bytes 6-7: wLength as individual bytes. */
		u16 wLength; /**< USB standard wLength field. */
	} Word3; /**< Word 3 (bytes 6-7): wLength field. */
	u8 *ContReadPtr;                     /**< Pointer for continued read transfers. */
	u8 *ContWritePtr;                    /**< Pointer for continued write transfers. */
	u32 ContReadCount;                   /**< Remaining bytes for continued read. */
	u32 ContWriteCount;                  /**< Remaining bytes for continued write. */
	u32 SetupSeqTX;                      /**< Setup transaction transmit sequence counter. */
	u32 SetupSeqRX;                      /**< Setup transaction receive sequence counter. */
	u8 ContReadDataBuffer[EP0_SETUP_DATA]; /**< Continued read data buffer. */
} USB_CMD_BUF;

/** @brief Standard USB 2.0 device descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x01 for device). */
	u16 bcdUSB;	/**< USB specification release number (BCD). */
	u8 bDeviceClass;	/**< Device class code. */
	u8 bDeviceSubClass;	/**< Device subclass code. */
	u8 bDeviceProtocol;	/**< Device protocol code. */
	u8 bMaxPacketSize0;	/**< Maximum packet size for endpoint 0. */
	u16 idVendor;	/**< Vendor ID assigned by USB-IF. */
	u16 idProduct;	/**< Product ID assigned by manufacturer. */
	u16 bcdDevice;	/**< Device release number (BCD). */
	u8 iManufacturer;	/**< Index of manufacturer string descriptor. */
	u8 iProduct;	/**< Index of product string descriptor. */
	u8 iSerialNumber;	/**< Index of serial number string descriptor. */
	u8 bNumConfigurations;	/**< Number of possible configurations. */

} USB_STD_DEV_DESC;

/** @brief Standard USB 2.0 device qualifier descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type identifier. */
	u8 bVersionLow;	/**< USB version low byte. */
	u8 bVersionHigh;	/**< USB version high byte. */
	u8 bDeviceClass;	/**< Device class code. */
	u8 bDeviceSubClass;	/**< Device subclass code. */
	u8 bProtocol;	/**< Device protocol code. */
	u8 bMaxPkt0;	/**< Maximum EP0 packet size. */
	u8 bNumberConfigurations;	/**< Number of other-speed configurations. */
	u8 breserved;	/**< Reserved field. */
} USB_STD_QUAL_DESC;

/** @brief Standard USB 2.0 configuration descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type (0x02 for configuration). */
	u8 bTotalLength;	/**< Total configuration-data length low byte. */
	u8 bCorrection;	/**< Total configuration-data length high byte. */
	u8 bNumberInterfaces;	/**< Number of interfaces in this configuration. */
	u8 bConfigValue;	/**< Configuration value used by SET_CONFIGURATION. */
	u8 bIConfigString;	/**< Index of configuration string descriptor. */
	u8 bAttributes;	/**< Configuration attributes bitmask. */
	u8 bMaxPower;	/**< Maximum power consumption in 2mA units. */
} USB_STD_CFG_DESC;

/** @brief Standard USB 2.0 other-speed configuration descriptor. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type identifier. */
	u8 bTotalLength;	/**< Total configuration-data length low byte. */
	u8 bCorrection;	/**< Total configuration-data length high byte. */
	u8 bNumberInterfaces;	/**< Number of interfaces. */
	u8 bConfigValue;	/**< Configuration value. */
	u8 bIConfigString;	/**< Index of configuration string. */
	u8 bAttributes;	/**< Configuration attributes bitmask. */
	u8 bMaxPower;	/**< Maximum power consumption in 2mA units. */
} USB_STD_QUA_DESC;

/** @brief Standard USB 2.0 interface descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x04 for interface). */
	u8 bInterfaceNumber;	/**< Zero-based interface number. */
	u8 bAlternateSetting;	/**< Alternate setting selector. */
	u8 bNumEndPoints;	/**< Number of endpoints (excluding EP0). */
	u8 bInterfaceClass;	/**< Interface class code. */
	u8 bInterfaceSubClass;	/**< Interface subclass code. */
	u8 bInterfaceProtocol;	/**< Interface protocol code. */
	u8 iInterface;	/**< Index of interface string descriptor. */

} USB_STD_IF_DESC;

/** @brief Standard USB HID class descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x21 for HID). */
	u8 bcdHIDL;	/**< HID specification release number low byte. */
	u8 bcdHIDH;	/**< HID specification release number high byte. */
	u8 bCountryCode;	/**< Country code for localized hardware. */
	u8 bNumDescriptors;	/**< Number of subordinate class descriptors. */
	u8 bReportDescriptorType;	/**< Type of report descriptor. */
	u8 wDescriptorLengthL;	/**< Report descriptor length low byte. */
	u8 wDescriptorLengthH;	/**< Report descriptor length high byte. */

} USB_STD_HID_DESC;

/**
 * @brief USB audio class-specific AC interface header descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x01 header). */
	u8 bcdADCL;	/**< Audio device class specification low byte. */
	u8 bcdADCH;	/**< Audio device class specification high byte. */
	u8 wTotalLengthL;	/**< Total length of class-specific descriptors low byte. */
	u8 wTotalLengthH;	/**< Total length of class-specific descriptors high byte. */
	u8 bInCollection;	/**< Number of streaming interfaces. */
	u8 baInterfaceNr;	/**< Interface number of streaming interface. */

} USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC;

/**
 * @brief USB audio class-specific input terminal descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x02 input terminal). */
	u8 bTerminalId;	/**< Unique terminal identifier. */
	u8 wTerminalTypeL;	/**< Terminal type low byte. */
	u8 wTerminalTypeH;	/**< Terminal type high byte. */
	u8 bAssocTerminal;	/**< ID of associated output terminal. */
	u8 bNrChannels;	/**< Number of logical output channels. */
	u8 wChannelConfigL;	/**< Channel configuration low byte. */
	u8 wChannelConfigH;	/**< Channel configuration high byte. */
	u8 iChannelNames;	/**< Index of channel name string. */
	u8 iTerminal;	/**< Index of terminal string descriptor. */

} USB_INPUT_TERMINAL_DESC;

/**
 * @brief USB audio class-specific output terminal descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x03 output terminal). */
	u8 bTerminalId;	/**< Unique terminal identifier. */
	u8 wTerminalTypeL;	/**< Terminal type low byte. */
	u8 wTerminalTypeH;	/**< Terminal type high byte. */
	u8 bAssocTerminal;	/**< ID of associated input terminal. */
	u8 bSourceId;	/**< ID of upstream source unit. */
	u8 iTerminal;	/**< Index of terminal string descriptor. */

} USB_OUTPUT_TERMINAL_DESC;

/**
 * @brief USB audio class-specific feature unit descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x06 feature unit). */
	u8 bUnitID;	/**< Unique unit identifier. */
	u8 bSourceID;	/**< ID of upstream source unit. */
	u8 bControlsize;	/**< Size of bmaControls element in bytes. */
	u8 bmaControls0;	/**< Controls for master channel 0. */
	u8 bmaControls1;	/**< Controls for logical channel 1. */
	u8 bmaControls2;	/**< Controls for logical channel 2. */
	u8 iFeature;	/**< Index of feature unit string descriptor. */

} USB_FEATURE_UNIT_DESC;

/**
 * @brief USB audio class-specific AS general interface descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x01 AS general). */
	u8 bTerminalLink;	/**< ID of linked terminal. */
	u8 bDelay;	/**< Cumulative data delay introduced by interface. */
	u8 wFormatTagL;	/**< Audio data format tag low byte. */
	u8 wFormatTagH;	/**< Audio data format tag high byte. */

} USB_CLASS_SPECIFIC_AS_IF_DESC;

/**
 * @brief USB audio Type I format descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type (0x24 class-specific). */
	u8 bDescriptorSubtype;	/**< Descriptor subtype (0x02 format type). */
	u8 bFormatType;	/**< Format type identifier. */
	u8 bNrChannels;	/**< Number of audio channels. */
	u8 bSubFrameSize;	/**< Audio sample subframe size in bytes. */
	u8 bBitResolution;	/**< Audio sample bit resolution. */
	u8 bSamFreqType;	/**< Number of supported sample frequencies. */
	u8 tSamFreByteOneL; /**< Sample frequency byte 1 (low byte). */
	u8 tSamFreByteTwo;	/**< Sample frequency byte 2. */
	u8 tSamFreByteThreeH;/**< Sample frequency byte 3 (high byte). */

} USB_AUDIO_TYPE_I_FORMAT_DESC;

/**
 * @brief USB standard audio endpoint descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type identifier. */
	u8 bEndpointAddress;	/**< Endpoint address and direction. */
	u8 bmAttributes;	/**< Endpoint attributes bitmask. */
	u8 bMaxPacketSizeL;	/**< Maximum packet size low byte. */
	u8 bMaxPacketSizeH;	/**< Maximum packet size high byte. */
	u8 bInterval;	/**< Polling interval in frames. */
	u8 bRefresh;	/**< Refresh rate for synchronization. */
	u8 bSyncAddress;	/**< Synchronization endpoint address. */

} USB_STD_AUDIO_EP_DESC;

/**
 * @brief USB audio class-specific data endpoint descriptor.
 */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type identifier. */
	u8 bDescriptorSubtype;	/**< Descriptor subtype identifier. */
	u8 bmAttributes;	/**< Endpoint attributes bitmask. */
	u8 bLockDelayUnits;	/**< Lock delay units. */
	u8 wLockDelayL;	/**< Lock delay low byte. */
	u8 wLockDelayH;	/**< Lock delay high byte. */

} USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC;

/** @brief Standard USB 2.0 endpoint descriptor structure. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bDescriptorType;	/**< Descriptor type identifier. */
	u8 bEndpointAddress;	/**< Endpoint address and direction. */
	u8 bmAttributes;	/**< Endpoint transfer type attributes. */
	u8 bMaxPacketSizeL;	/**< Maximum packet size low byte. */
	u8 bMaxPacketSizeH;	/**< Maximum packet size high byte. */
	u8 bInterval;	/**< Polling interval in (micro)frames. */
} USB_STD_EP_DESC;

/** @brief Standard USB language ID string descriptor. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type (string = 0x03). */
	u16 wLANGID[1];	/**< Supported language ID array. */
} USB_STD_STRING_DESC;

/** @brief Standard USB manufacturer string descriptor. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type (string = 0x03). */
	u8 bString[14];	/**< Manufacturer name string in Unicode. */
} USB_STD_STRING_MAN_DESC;

/** @brief Standard USB product string descriptor. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type (string = 0x03). */
	u8 bString[10];	/**< Product name string in Unicode. */
} USB_STD_STRING_PS_DESC;

/** @brief Standard USB serial number string descriptor. */
typedef struct {
	u8 bLength;	/**< Descriptor length in bytes. */
	u8 bType;	/**< Descriptor type (string = 0x03). */
	u8 bString[42];	/**< Serial number string in Unicode. */
} USB_STD_STRING_SN_DESC;

#ifdef MASS_STORAGE_DEVICE
/** @brief USB mass storage device full configuration descriptor set. */
typedef struct {
	USB_STD_CFG_DESC stdCfg;	/**< Standard configuration descriptor. */
	USB_STD_IF_DESC ifCfg;	/**< Interface descriptor. */
	USB_STD_EP_DESC epCfg1;	/**< Endpoint 1 descriptor. */
	USB_STD_EP_DESC epCfg2;	/**< Endpoint 2 descriptor. */
} FPGA1_CONFIGURATION;

#endif

#ifdef HID_DEVICES
/** @brief USB HID device full configuration descriptor set. */
typedef struct {
	USB_STD_CFG_DESC	stdCfg;
	USB_STD_IF_DESC		ifCfg;
	USB_STD_HID_DESC	hidCfg;
	USB_STD_EP_DESC		epCfg1;

} FPGA1_CONFIGURATION;

#endif

#ifdef MICROPHONE
/** @brief USB microphone alternate configuration descriptor set. */
typedef struct {
	USB_STD_IF_DESC				ifCfg;
	USB_CLASS_SPECIFIC_AS_IF_DESC 		audifCfg;
	USB_AUDIO_TYPE_I_FORMAT_DESC		typeICfg;
	USB_STD_AUDIO_EP_DESC			audEpCfg;
	USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC 	audDataEpCfg;

} USB_ALT_CFG_SET;
/** @brief USB microphone full configuration descriptor set. */
typedef struct {
	USB_STD_CFG_DESC			stdCfg;
	USB_STD_IF_DESC				ifCfg;
	USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC	adIfHdCfg;
	USB_INPUT_TERMINAL_DESC			ipTermCfg;
	USB_OUTPUT_TERMINAL_DESC		opTermCfg0;
	USB_STD_IF_DESC				altIfCfg;
	USB_ALT_CFG_SET				altCfg;

} FPGA1_CONFIGURATION;

#endif
#ifdef __cplusplus
}
#endif
#endif /* __XUSBTYPES_H__ */

