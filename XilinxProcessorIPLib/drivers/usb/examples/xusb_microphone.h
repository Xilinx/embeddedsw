/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xusb_microphone.h
 *
 * This file contains the constants, type definitions, variables and function
 * prototypes used in the usb microphone application.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  12/20/10 First release
 * 4.02a bss  02/20/12 Modified to include Little Endian and
 * 			Big Endian descriptors.
 *
 * </pre>
 *****************************************************************************/

#ifndef  XUSB_HEADSET_H
#define  XUSB_HEADSET_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb_cp9.h"

/************************** Constant Definitions ******************************/

/*
 * Valid USB Status block.
 */
#define CMD_PASSED			0x00
#define CMD_FAILED			0x01
#define PHASE_ERROR			0x02

/*
 * USB Audio control class Specific constants
 */
#define USB_CLASS_AUDIO			0x01
#define USB_AUDIO_PROTOCOL		0x00

#define CS_INTERFACE			0x24
#define CS_ENDPOINT			0x25
#define INPUT_TERMINAL			0x02
#define OUTPUT_TERMINAL			0x03

/*
 * Audio Interface Subclass codes
 */
#define USB_AUDIO_CONTROL		0x01
#define USB_AUDIO_STREAMING		0x02

#define USB_ISOC_TRANSFER_TYPE_ADAPT	0x01
#define USB_ISOC_NOT_SHARED		0x01
#define FORMAT_TYPE_SUBTYPE		0x2
#define FORMAT_TYPE_I			0x1
/*
 * Error codes.
 */
#define NO_ERROR			0
#define ERR_NOTERASED			1
#define ERR_NOFREEBLOCKS		2
#define ERR_USBABORT			3
#define ERR_CMDFAILED			4
#define ERR_ECC				5
#define ERR_BADBZ			6
#define ERR_DSMM			7

/*
 * End point types.
 */
#define EP_CONTROL			0	/**< Control Endpoint */
#define EP_ISOCHRONOUS			1	/**< Isochronous Endpoint */
#define EP_BULK				2	/**< Bulk Endpoint */
#define EP_INTERRUPT			3	/**< Interrupt Endpoint */

/************************** Variable Definitions ******************************/
extern u16 MaxControlSize;
extern USB_CMD_BUF Ch9_CmdBuf;
extern IntChar UsbMemData;		/* Dual Port memory */
extern u32 AltSetting;
USB_STD_DEV_DESC DeviceDescriptor __attribute__ ((aligned(4))) = {
	sizeof(USB_STD_DEV_DESC),	/* Descriptor Size 18 bytes */
		DEVICE_DESCR,	/* This is a device descriptor */
#ifdef __LITTLE_ENDIAN__
		0x0200,		/* USB version */
#else
		0x02,		/* USB version */
#endif
		0,		/* Vendor Specific */
		00,		/* Unused */
		00,		/* Unused */
		0x40,		/* Ep0 Max Pkt Size 64 bytes */
#ifdef __LITTLE_ENDIAN__
		0x03FD,		/* Vendor Id */
		0x0200,		/* Product Id */
		0x0100,		/* BCD device */
#else
		0xFD03,		/* Vendor Id */
		0x0002,		/* Product Id */
		0x01,		/* BCD device */
#endif
		01,		/* String Index of manufacturer */
		02,		/* String Index of product */
		03,		/* String Index of serial number */
		01		/* Number of configurations */
};

USB_STD_QUAL_DESC QualifierDescriptor __attribute__ ((aligned(4))) = {
sizeof(USB_STD_QUAL_DESC),
		QUALIFIER_DESCR, 00, 02, 0, 00, 00, 0x40, 01, 0};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) HsUsbConfig = {
	{
		/*
		 * Configuration descriptor.
		 */
		sizeof(USB_STD_CFG_DESC),/* Size of config descriptor 9
						bytes */
		CONFIG_DESCR,	/* This is a conifig descriptor */
		sizeof(HsUsbConfig),	/* Total size of configuration
				LS */
		0x00,	/* Total size of configuration MS */
		0x02,	/* No. Of interfaces 2 */
		CONFIGURATION_ONE,	/* No of configuration values */
		0x00,	/* Configuration string */
		0xc0,	/* Self Powered */
		0x01	/* Uses 2mA from the USB bus */
	},
	{
		/*
		 * FPGA1 Standard interface.descriptor
		 */
		sizeof(USB_STD_IF_DESC),/* Interface Descriptor size 9
						bytes */
		INTERFACE_DESCR,	/* This is an interface
					descriptor */
		0x00,			/* Interface number 0 */
		0x00,			/* Alternate set 0 */
		0x00,			/* Number of end points 0 */
		USB_CLASS_AUDIO,	/* Audio device */
		USB_AUDIO_CONTROL,	/* Audio Control */
		USB_AUDIO_PROTOCOL,	/* Interface Protocol */
		0x00			/* Interface unused */
	},
	{ 	/*
		 * USB Audio Class-Specific AC Interface Header Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC),
					/* bLength */
		CS_INTERFACE,		/* bDescriptorType */
		0x01,			/* bDescriptorSubtype */
		0x00,			/* bcdADC - Audio Class 1.0 L*/
		0x01,			/* bcdADC - Audio Class 1.0 H*/
		(sizeof(USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC) +
		sizeof(USB_INPUT_TERMINAL_DESC) +
		sizeof(USB_OUTPUT_TERMINAL_DESC)),
		0x00,			/* wTotalLength */
		0x01,			/* bInCollection How many
					   Audio Streaming descriptors */
		0x01			/* baInterfaceNr Their interface
					   numbers */
	 },
	 {	/*
	 	 * USB Audio Input Terminal Descriptor
	 	 */
		sizeof(USB_INPUT_TERMINAL_DESC),	/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		INPUT_TERMINAL,			/* bDescriptorSubtype */
		0x01,				/* bTerminalId */
		0x01,				/* wTerminalType
						(USB Streaming) L*/
		0X02,				/* wTerminalType
						(USB Streaming) H*/
		0x00,				/* bAssocTerminal */
		0x01,				/* bNrChannels
					       (2 Channel Left/Right)*/
		0x00,				/* wChannelConfig L*/
		0x00,				/* wChannelConfig H*/
		0x00,				/* iChannelNames */
		0x00				/* iTerminal */
	},
	{	/*
		 * USB Audio Output Terminal Descriptor--
		 */
		sizeof(USB_OUTPUT_TERMINAL_DESC),/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		OUTPUT_TERMINAL,		/* bDescriptorSubtype*/
		0x02,				/* bTerminalId */
		0x01,				/* wTerminalType L*/
						/* USB streaming */
		0x01,				/* wTerminalType H*/
		0x00,				/* bAssocTerminal */
		0x01,				/* bSourceId */
		0x00				/* iTerminal */
	},
	{
		/*
		 *USB Standard Interface Descriptor (Alt. Set. 0)
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		INTERFACE_DESCR,		/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_AUDIO_STREAMING,		/* bInterfaceSubclass*/
		0x00,				/* bInterfaceProtocol*/
 		0x00				/* iInterface*/
	},
	{
	{
		/*
		 * USB Standard Interface Descriptor (Alt. Set. 1)
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		INTERFACE_DESCR,		/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_AUDIO_STREAMING,		/* bInterfaceSubclass*/
		0x00,				/* bInterfaceProtocol*/
		0x00				/* iInterface */
	},
	{
		/*
		 * USB Audio Class-Specific AS General Interface Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AS_IF_DESC),	/* bLength */
		CS_INTERFACE,			/* bDescriptorType  */
		0x01,				/* bDescriptorSubtype*/
		0x02,				/* bTerminalLink */
		0x01,				/* bDelay */
		0x01,				/* PCM wFormatTagL*/
		0x00				/* wFormatTagH*/
	},
	{
		/*
		 * USB Audio Type I Format Type Descriptor
		 */
		sizeof(USB_AUDIO_TYPE_I_FORMAT_DESC),/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		FORMAT_TYPE_SUBTYPE,		/* bDescriptorSubtype*/
		FORMAT_TYPE_I,				/* bFormatType */
		0x01,				/* bNrChannels */
		0x02,				/* bSubFrameSize */
		0x10,				/* bBitResolution */
		0x01,				/* bSamFreqType */
		0x40,				/*Sample freq 8000Hz*/
		0x1F,				/*Sample freq 8000Hz*/
		0x00				/*Sample freq 8000Hz*/
   	},
	{
		/*
		 * USB Audio Standard Endpoint Descriptor
		 */
		sizeof(USB_STD_AUDIO_EP_DESC),	/* bLength */
		ENDPOINT_DESCR,			/* bType */
		0x81,				/* bEndpoint IN endpoint
						   address 1*/
		USB_ISOC_TRANSFER_TYPE_ADAPT, 	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x14,				/* bMaxPacketSizeH */
		0x04,				/* bInterval */
		0x00,				/* bRefresh */
		0x00				/* bSyncAddress */
	},
	{
		/*
		 * USB Class-Specific Audio Data Endpoint Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC), /* bLength */
		CS_ENDPOINT,			/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
	}
};

FPGA1_CONFIGURATION __attribute__ ((aligned(4))) FsUsbConfig = {
	{
		/*
		 * Configuration descriptor.
		 */
		sizeof(USB_STD_CFG_DESC),/* Size of config descriptor 9
						bytes */
		CONFIG_DESCR,	/* This is a conifig descriptor */
		sizeof(FsUsbConfig),	/* Total size of configuration
				LS */
		0x00,	/* Total size of configuration MS */
		0x02,	/* No. Of interfaces 2 */
		CONFIGURATION_ONE,	/* No of configuration values */
		0x00,	/* Configuration string */
		0xC0,	/* Self Powered */
		0x01	/* Uses 2mA from the USB bus */
	},
	{
		/*
		 * FPGA1 Standard interface.descriptor
		 */
		sizeof(USB_STD_IF_DESC),/* Interface Descriptor size 9
						bytes */
		INTERFACE_DESCR,	/* This is an interface
					descriptor */
		0x00,			/* Interface number 0 */
		0x00,			/* Alternate set 0 */
		0x00,			/* Number of end points 0 */
		USB_CLASS_AUDIO,	/* Audio device */
		USB_AUDIO_CONTROL,	/* Audio Control */
		USB_AUDIO_PROTOCOL,	/* Interface Protocol */
		0x00			/* Interface unused */
	},
	{ 	/*
		 * USB Audio Class-Specific AC Interface Header Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC),
					/* bLength */
		CS_INTERFACE,		/* bDescriptorType */
		0x01,			/* bDescriptorSubtype */
		0x00,			/* bcdADC - Audio Class 1.0 L*/
		0x01,			/* bcdADC - Audio Class 1.0 H*/
		(sizeof(USB_CLASS_SPECIFIC_AC_IF_HEADER_DESC) +
		sizeof(USB_INPUT_TERMINAL_DESC) +
		sizeof(USB_OUTPUT_TERMINAL_DESC)),
		0x00,
					/* wTotalLength */
		0x01,			/* bInCollection How many
					   Audio Streaming descriptors
					   */
		0x01			/* baInterfaceNr Their interface
					   numbers */
	 },
	 {	/*
	 	 * USB Audio Input Terminal Descriptor
	 	 */
		sizeof(USB_INPUT_TERMINAL_DESC),	/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		INPUT_TERMINAL,			/* bDescriptorSubtype */
		0x01,				/* bTerminalId */
		0x01,				/* wTerminalType
						(USB Streaming) L*/
		0X02,				/* wTerminalType
						(USB Streaming) H*/
		0x00,				/* bAssocTerminal */
		0x01,				/* bNrChannels
					       (2 Channel Left/Right)*/
		0x00,				/* wChannelConfig L*/
		0x00,				/* wChannelConfig H*/
		0x00,				/* iChannelNames */
		0x00				/* iTerminal */
	},
	{	/*
		 * USB Audio Output Terminal Descriptor--
		 */
		sizeof(USB_OUTPUT_TERMINAL_DESC),/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		OUTPUT_TERMINAL,		/* bDescriptorSubtype*/
		0x02,				/* bTerminalId */
		0x01,				/* wTerminalType L*/
						/* USB streaming */
		0x01,				/* wTerminalType H*/
		0x00,				/* bAssocTerminal */
		0x01,				/* bSourceId */
		0x00				/* iTerminal */
	},
	{
		/*
		 *USB Standard Interface Descriptor (Alt. Set. 0)
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		INTERFACE_DESCR,		/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_AUDIO_STREAMING,		/* bInterfaceSubclass*/
		0x00,				/* bInterfaceProtocol*/
 		0x00				/* iInterface*/
	},
	{
	{
		/*
		 * USB Standard Interface Descriptor (Alt. Set. 1)
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		INTERFACE_DESCR,		/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_AUDIO_STREAMING,		/* bInterfaceSubclass*/
		0x00,				/* bInterfaceProtocol*/
		0x00				/* iInterface */
	},
	{
		/*
		 * USB Audio Class-Specific AS General Interface Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AS_IF_DESC),	/* bLength */
		CS_INTERFACE,			/* bDescriptorType  */
		0x01,				/* bDescriptorSubtype*/
		0x02,				/* bTerminalLink */
		0x01,				/* bDelay */
		0x00,				/* PCM wFormatTagL*/
		0x01				/* wFormatTagH*/
	},
	{
		/*
		 * USB Audio Type I Format Type Descriptor
		 */
		sizeof(USB_AUDIO_TYPE_I_FORMAT_DESC),/* bLength */
		CS_INTERFACE,			/* bDescriptorType */
		FORMAT_TYPE_SUBTYPE,		/* bDescriptorSubtype*/
		FORMAT_TYPE_I,				/* bFormatType */
		0x01,				/* bNrChannels */
		0x02,				/* bSubFrameSize */
		0x10,				/* bBitResolution */
		0x01,				/* bSamFreqType */
		0x00,				/*Sample freq 8000Hz*/
		0x1F,				/*Sample freq 8000Hz*/
		0x40				/*Sample freq 8000Hz*/
   	},
	{
		/*
		 * USB Audio Standard Endpoint Descriptor
		 */
		sizeof(USB_STD_AUDIO_EP_DESC),	/* bLength */
		ENDPOINT_DESCR,			/* bType */
		0x81,				/* bEndpoint IN endpoint
						   address 1*/
		USB_ISOC_TRANSFER_TYPE_ADAPT, 	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x10,				/* bMaxPacketSizeH */
		0x04,				/* bInterval */
		0x00,				/* bRefresh */
		0x00				/* bSyncAddress */
	},
	{
		/*
		 * USB Class-Specific Audio Data Endpoint Descriptor
		 */
		sizeof(USB_CLASS_SPECIFIC_AUDIO_DATA_EP_DESC), /* bLength */
		CS_ENDPOINT,			/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
	}
};



USB_STD_STRING_DESC LangId __attribute__ ((aligned(4))) = {
	/*
	 * Language ID codes.
	 */
	4, STRING_DESCR,
#ifdef __LITTLE_ENDIAN__
	{0x0409}
#else
	{0x0904}
#endif
};

USB_STD_STRING_MAN_DESC Manufacturer __attribute__ ((aligned(4))) = {
	/*
	 * Manufacturer String.
	 */
	sizeof(USB_STD_STRING_MAN_DESC), STRING_DESCR, {
	'X', 0, 'I', 0, 'L', 0, 'I', 0, 'N', 0, 'X', 0, ' ', 0}
};

USB_STD_STRING_PS_DESC ProductString __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_PS_DESC), STRING_DESCR, {
	'F', 0, 'P', 0, 'G', 0, 'A', 0, '4', 0}
};

USB_STD_STRING_SN_DESC SerialNumber __attribute__ ((aligned(4))) = {
	/*
	 * Product ID String.
	 */
	sizeof(USB_STD_STRING_SN_DESC), STRING_DESCR, {
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0,
			'7', 0, '1', 0, '5', 0, '0', 0, '4', 0, '2', 0,
			'6', 0, '2', 0, '0', 0, '0', 0, '5', 0, '7', 0, '4', 0}
};

/************************** Function Prototypes *******************************/

void InitUsbInterface(XUsb * InstancePtr);
void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
void EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_SPEAKER_H */


