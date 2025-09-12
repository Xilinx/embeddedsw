/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_ch9_audio.c
 *
 * This file contains the implementation of chapter 9 specific code for
 * the example.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who	Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0  pm 	20/02/20 First release
 * 2.10  ka     21/08/25 Fixed GCC warnings
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xusbps_ch9_audio.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
static s32 EpEnable(XUsbPs *InstancePtr, u8 EpNum, u8 Dir,
		    u16 Maxsize, u8 Type);
static void XUsbPs_SetEpInterval(XUsbPs *InstancePtr,
				 u8 UsbEpNum, u8 Dir, u32 Interval);
static void XUsbPs_StreamOn(XUsbPs *InstancePtr, u8 EpNum,
			    u8 Dir, u8 *BufferPtr);
static void XUsbPs_StreamOff(XUsbPs *InstancePtr, u8 EpNum,
			     u8 Dir);

/************************** Variable Definitions *****************************/
extern u8 AudioFreq[MAX_AUDIO_FREQ + 1][3];
extern u8 BufferPtrTemp[1024];
extern s32 Index;
extern s32 Residue;
extern u8 FirstPktFrame;
extern u8 *WrRamDiskPtr;

/*
 * Device Descriptors
 */
USB_STD_DEV_DESC __attribute__ ((aligned(16))) deviceDesc = {
	/*
	 * USB 2.0
	 */
	sizeof(USB_STD_DEV_DESC),	/* bLength */
	XUSBPS_TYPE_DEVICE_DESC,		/* bDescriptorType */
	0x0200,					/* bcdUSB 2.0 */
#ifdef XUSBPS_UAC1
	0x00,					/* bDeviceClass */
	0x00,					/* bDeviceSubClass */
	0x00,					/* bDeviceProtocol */
#else	/*	UAC2 */
	XUSBPS_CLASS_MISC,			/* bDeviceClass */
	0x02,					/* bDeviceSubClass */
	0x01,					/* bDeviceProtocol */
#endif
	0x40,					/* bMaxPackedSize0 */
	0x03Fd,					/* idVendor */
	0x0200,					/* idProduct */
	0x0100,					/* bcdDevice */
	0x01,					/* iManufacturer */
	0x02,					/* iProduct */
	0x03,					/* iSerialNumber */
	0x01					/* bNumConfigurations */

};

#ifdef XUSBPS_UAC1

USB_CONFIG __attribute__ ((aligned(16))) config2 = {
	{/*
		 * Std Config
		 */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		XUSBPS_TYPE_CONFIG_DESC,	/* bDescriptorType */
		sizeof(USB_CONFIG),		/* wTotalLength */
		0x02,				/* No. Of interfaces 2 */
		0x01,				/* No of configuration values */
		0x00,				/* Configuration string */
		0xC0,				/* bmAttribute */
		0x01				/* bMaxPower  */
	},
	{/*
		 * UAC1.0 Standard Interface Descriptor
		 */
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9 bytes */
		XUSBPS_TYPE_IF_CFG_DESC,	/* This is an interface descriptor */
		0x00,				/* Interface number 0 */
		0x00,				/* Alternate set 0 */
		0x00,				/* Number of end points 0 */
		XUSBPS_CLASS_AUDIO,		/* Audio device */
		USB_SUBCLASS_AUDIOCONTROL,	/* Audio Control */
		UAC_VERSION,			/* Interface Protocol */
		0x00					/* iInterface */
	},
	{/*
		 * USB Audio Class-Specific AC Interface Header Descriptor
		 */
		sizeof(UAC1_AC_HEADER_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bcdADC - Audio Class 1.0 L */
		0x01,				/* bcdADC - Audio Class 1.0 H */
		(sizeof(UAC1_AC_HEADER_DESC) +
		 sizeof(UAC1_INPUT_TERMINAL_DESC) +
		 sizeof(UAC1_FEATURE_UNIT_DESC) +
		 sizeof(UAC1_OUTPUT_TERMINAL_DESC)),
		0x00,					/* wTotalLength */
		0x01,			/* bInCollection How many
					   Audio Streaming descriptors */
		0x01			/* baInterfaceNr Their interface
					   numbers */
	},
	{/*
		 * Input Terminal Descriptor
		 */
		sizeof(UAC1_INPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_INPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_IT_ID,				/* bTerminalId */
		0x01,					/* wTerminalType L */
#ifdef XUSBPS_MICROPHONE
		0x02,					/* wTerminalType H */
#else	/* SPEAKER */
		0x01,					/* wTerminalType H */
#endif
		0x00,					/* bAssocTerminal */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		0x00,					/* wChannelConfig L*/
		0x00,					/* wChannelConfig H*/
		0x00,					/* iChannelNames */
		0x00					/* iTerminal */
	},
	{/*
		 * Feature Unit Descriptor
		 */
		sizeof(UAC1_FEATURE_UNIT_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype*/
		FETR_UNT_ID,			/* bUnitId */
		USB_IT_ID,				/* bSourceId */
		0x02,					/* bControlSize */
		{
			0x0003,				/* bmaControls(0) */
			/* Mute and Volume Control */
			0x0003,				/* bmaControls(1) */
			/* Mute and Volume Control */
		},
		0x00					/* iFeature */
	},
	{/*
		 * Output Terminal Descriptor
		 */
		sizeof(UAC1_OUTPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,	/* bDescriptorSubtype */
		USB_OT_ID,				/* bTerminalId */
		0x01,					/* wTerminalType L */
#ifdef XUSBPS_MICROPHONE
		0x01,					/* wTerminalType H */
#else	/* SPEAKER */
		0x03,					/* wTerminalType H */
#endif
		0x00,					/* bAssocTerminal */
		FETR_UNT_ID,			/* bSourceId */
		0x00					/* iTerminal */
	},
	{/*
		 * Audio Streaming Interface - Alt0
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x01,					/* bInterfaceNumber */
		0x00,					/* bAlternateSetting */
		0x00,					/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x00					/* iInterface */
	},
	{/*
		 * Audio Streaming Interface - Alt1
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x01,					/* bInterfaceNumber */
		0x01,					/* bAlternateSetting */
		0x01,					/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x00					/* iInterface */
	},
	{/*
		 * Audio Stream Interface Descriptor
		 */
		sizeof(UAC1_AS_HEADER_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType  */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
#ifdef XUSBPS_MICROPHONE
		0x02,					/* bTerminalLink */
#else	/* SPEAKER */
		0x01,					/* bTerminalLink */
#endif
		0x01,					/* bDelay */
		0x01,					/* PCM wFormatTagL*/
		0x00					/* wFormatTagH*/
	},
	{/*
		 * Audio Type I Format Type Descriptor
		 */
		sizeof(UAC1_FORMAT_TYPE_I_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		AUDIO_FRAME_SIZE,		/* bSubFrameSize */
		BIT_RESOLUTION,			/* bBitResolution */
		0x01,					/* bSamFreqType */
		0x40,					/* Sample freq 8000Hz */
		0x1F,					/* Sample freq 8000Hz */
		0x00					/* Sample freq 8000Hz */
	},
	{/*
		 * STD Endpoint Descriptor
		 */
		sizeof(USB_EP_DESC),	/* bLength */
		XUSBPS_TYPE_ENDPOINT_CFG_DESC,		/* bType */
#ifdef XUSBPS_MICROPHONE
		USB_EP1_IN,				/* bEndpoint IN */
#else	/* SPEAKER */
		USB_EP1_OUT,			/* bEndpoint OUT */
#endif
		0x01,					/* Isochronous, adaptive */
		0x00,					/* bMaxPacketSizeL */
		0x04,					/* bMaxPacketSizeH */
		AUDIO_INTERVAL,			/* bInterval */
		0x00,					/* bRefresh */
		0x00					/* bSyncAddress */
	},
	{/*
		 * CS AS ISO Endpoint
		 */
		sizeof(UAC1_ISO_EP_DESC),	/* bLength */
		XUSBPS_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
};

/*
 * String Descriptors
 */
static u8 StringList[1][6][128] = {
	{
		"UNUSED",
		"XILINX",
		"USB2.0 AUDIO",
		"000000017150426200574",
	}
};

#else	//	UAC2

/*
 * Configuration Descriptors
 */
USB_CONFIG __attribute__ ((aligned(16))) config2 = {
	{/*
		 * Std Config
		 */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		XUSBPS_TYPE_CONFIG_DESC,	/* bDescriptorType */
		sizeof(USB_CONFIG),		/* wTotalLength */
		0x03,				/* No. Of interfaces 3 */
		0x01,				/* No of configuration values */
		0x00,				/* Configuration string */
		0xC0,				/* bmAttribute */
		0x01				/* bMaxPower  */
	},
	{/*
		 * Class-Specific Interface Association Descriptor
		 */
		sizeof(USB_IF_ASSOC_DESC),		/* bLength */
		XUSBPS_TYPE_INTERFACE_ASSOCIATION,	/* bDescriptorType */
		0x00,					/* bFirstInterface */
		0x03,					/* bInterfaceCount */
		XUSBPS_CLASS_AUDIO,			/* bFunctionclass */
		UAC2_FUNCTION_SUBCLASS_UNDEFINED,	/* bFunctionSubClass */
		UAC_VERSION,				/* bFunctionProtocol */
		0x04					/* iFunction */
	},
	{/*
		 * UAC2.0 Standard Interface Descriptor
		 */
		sizeof(USB_STD_IF_DESC),	/* Interface Descriptor size 9 bytes */
		XUSBPS_TYPE_IF_CFG_DESC,	/* This is an interface descriptor */
		0x00,				/* Interface number 0 */
		0x00,				/* Alternate set 0 */
		0x00,				/* Number of end points 0 */
		XUSBPS_CLASS_AUDIO,		/* Audio device */
		USB_SUBCLASS_AUDIOCONTROL,	/* Audio Control */
		UAC_VERSION,			/* Interface Protocol */
		0x05				/* iInterface */
	},
	{/*
		 * USB Audio Class-Specific AC Interface Header Descriptor
		 */
		sizeof(UAC2_AC_HEADER_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,		/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bcdADC - Audio Class 1.0 L */
		0x02,				/* bcdADC - Audio Class 1.0 H */
		UAC2_FUNCTION_IO_BOX,	/* bCategory */
		(sizeof(UAC2_AC_HEADER_DESC) +
		 sizeof(UAC2_CLOCK_SOURCE_DESC) +
		 sizeof(UAC2_CLOCK_SELECTOR_DESC) +
		 (sizeof(UAC2_INPUT_TERMINAL_DESC) +
		  sizeof(UAC2_FEATURE_UNIT_DESC) +
		  sizeof(UAC2_OUTPUT_TERMINAL_DESC)) * 2),	/* wTotalLength */
		0x01				/* bmControls */
	},
	{/*
		 * Clock Source Descriptor
		 */
		sizeof(UAC2_CLOCK_SOURCE_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SOURCE,		/* bDescriptorSubtype */
		USB_CLK_SRC_ID,			/* bClockId */
		UAC2_CLOCK_SOURCE_TYPE_INT_FIXED,	/* bmAttributes */
		(CONTROL_RDONLY << CLK_FREQ_CTRL),	/* bmControls */
		0x00,					/* bAssocTerminal */
		0x06					/* iClockSource */
	},
	{/*
		 * Clock Selector Descriptor
		 */
		sizeof(UAC2_CLOCK_SELECTOR_DESC),/* bLength */
		XUSBPS_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SELECTOR,		/* bDescriptorSubtype */
		USB_CLK_SEL_ID,			/* bClockId */
		0x01,				/* bNrPins */
		{
			USB_CLK_SRC_ID		/* baCSourceID(0) */
		},
		0x03,				/* bmControl */
		0x00				/* iClockSelector */
	},
	{/*
		 * USB_OUT Input terminal
		 */
		sizeof(UAC2_INPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_INPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_OUT_IT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x01,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		USB_CLK_SEL_ID,			/* bCSourceId */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		0x03,				/* bmChannelConfigL1 */
		0x00,				/* bmChannelConfigL2 */
		0x00,				/* bmChannelConfigL3 */
		0x00,				/* bmChannelConfigL4 */
		0x00,				/* iChannelNames */
		(CONTROL_RDWR << COPY_CTRL),	/* bmContols */
		0x07				/* iTerminal */
	},
	{/*
		 * USB_OUT Feature Unit Descriptor
		 */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		OUT_FETR_UNT_ID,		/* bUnitID */
		USB_OUT_IT_ID,			/* bSourceID */
		{
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
		},
		0x00				/* iFeature */
	},
	{/*
		 * USB_OUT Output Terminal
		 */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,	/* bDescriptorSubtype */
		USB_OUT_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x03,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		OUT_FETR_UNT_ID,		/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x08				/* iTerminal */
	},
	{/*
		 * USB_IN Input Terminal
		 */
		sizeof(UAC2_INPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_INPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_IN_IT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x02,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		USB_CLK_SEL_ID,			/* bCSourceId */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		0x03,				/* bmChannelConfigL1 */
		0x00,				/* bmChannelConfigL2 */
		0x00,				/* bmChannelConfigL3 */
		0x00,				/* bmChannelConfigL4 */
		0x00,				/* iChannelNames */
		(CONTROL_RDWR << COPY_CTRL),	/* bmContols */
		0x09				/* iTerminal */
	},
	{/*
		 * USB_IN Feature Unit Descriptor
		 */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		IN_FETR_UNT_ID,			/* bUnitID */
		USB_IN_IT_ID,			/* bSourceID */
		{
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
		},
		0x00				/* iFeature */
	},
	{/*
		 * USB_IN Output Terminal
		 */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,	/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x01,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		IN_FETR_UNT_ID,			/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x0a				/* iTerminal */
	},
	{/*
		 * Audio Streaming OUT Interface - Alt0
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0b				/* iInterface */
	},
	{/*
		 * Audio Streaming OUT Interface - Alt1
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0c				/* iInterface */
	},
	{/*
		 * Audio Stream OUT Interface Descriptor
		 */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType  */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_OUT_IT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,	/* bmFormatsL1 */
		0x00,				/* bmFormatsL2 */
		0x00,				/* bmFormatsL3 */
		0x00,  				/* bmFormatsL4 */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		0x03,				/* bmChannelConfigL1 */
		0x00,				/* bmChannelConfigL2 */
		0x00,				/* bmChannelConfigL3 */
		0x00,				/* bmChannelConfigL4 */
		0x00				/* iChannelNames */
	},
	{/*
		 * Audio USB_OUT Format
		 */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{/*
		 * STD OUT Endpoint
		 */
		sizeof(USB_EP_DESC),	/* bLength */
		XUSBPS_TYPE_ENDPOINT_CFG_DESC,		/* bType */
		USB_EP1_OUT,			/* bEndpoint OUT endpoint address 0 */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{/*
		 * CS AS ISO OUT Endpoint
		 */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		XUSBPS_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bmControls */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	},
	{/*
		 * Audio Streaming IN Interface - Alt0
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0d				/* iInterface */
	},
	{/*
		 * Audio Streaming IN Interface - Alt1
		 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		XUSBPS_TYPE_IF_CFG_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		XUSBPS_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0e					/* iInterface */
	},
	{/*
		 * Audio Stream IN Interface Descriptor
		 */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,    	/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,	/* bmFormatsL1 */
		0x00,				/* bmFormatsL2 */
		0x00,				/* bmFormatsL3 */
		0x00,				/* bmFormatsL4 */
		AUDIO_CHANNEL_NUM,		/* bNrChannels */
		0x03,				/* bmChannelConfigL1 */
		0x00,				/* bmChannelConfigL2 */
		0x00,				/* bmChannelConfigL3 */
		0x00,				/* bmChannelConfigL4 */
		0x00				/* iChannelNames */
	},
	{/*
		 * Audio USB_IN Format
		 */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),	/* bLength */
		XUSBPS_DT_CS_INTERFACE,	/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{/*
		 * STD AS ISO IN Endpoint
		 */
		sizeof(USB_EP_DESC),		/* bLength */
		XUSBPS_TYPE_ENDPOINT_CFG_DESC,	/* bType */
		USB_EP1_IN,			/* bEndpoint IN endpoint address 0 */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{/*
		 * CS AS ISO IN Endpoint
		 */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		XUSBPS_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
};

/*
 * String Descriptors
 */
static u8 StringList[1][20][128] = {
	{
		"UNUSED",
		"XILINX",
		"USB2.0 AUDIO",
		"000000017150426200574",
		"Source/Sink",
		"Topology Control",
		"44100Hz",
		"USBH Out",
		"USBH In",
		"USBD Out",
		"USBD In",
		"Playback Inactive",
		"Playback Active",
		"Capture Inactive",
		"Capture Active",
	}
};

#endif

/*****************************************************************************/
/**
 *
 * This function returns the device descriptor for the device.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	BufPtr is pointer to the buffer that is to be filled
 *			with the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *			0 on error.
 *
 ******************************************************************************/
u32 XUsbPs_Ch9SetupDevDescReply(u8 *BufPtr, u32 BufLen)
{

	/* Check buffer pointer is there and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_DEV_DESC)) {
		return 0;
	}
	memcpy(BufPtr, &deviceDesc, sizeof(USB_STD_DEV_DESC));

	return sizeof(USB_STD_DEV_DESC);
}

/*****************************************************************************/
/**
 *
 * This function returns the configuration descriptor for the device.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *			0 on error.
 *
 ******************************************************************************/
u32 XUsbPs_Ch9SetupCfgDescReply(u8 *BufPtr, u32 BufLen)
{
	u8 *config;
	u32 CfgDescLen;

	/* Check buffer pointer is OK and buffer is big enough. */
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_CFG_DESC)) {
		return 0;
	}

	/*
	 * USB 2.0
	 */
	config = (u8 *)&config2;
	CfgDescLen  = sizeof(USB_CONFIG);


	memcpy(BufPtr, config, CfgDescLen);

	return CfgDescLen;
}


/*****************************************************************************/
/**
 *
 * This function returns a string descriptor for the given index.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	BufPtr is a  pointer to the buffer that is to be filled with
 *			the descriptor.
 * @param	BufLen is the size of the provided buffer.
 * @param	Index is the index of the string for which the descriptor
 *			is requested.
 *
 * @return 	Length of the descriptor in the buffer on success.
 *			0 on error.
 *
 ******************************************************************************/
u32 XUsbPs_Ch9SetupStrDescReply(u8 *BufPtr, u32 BufLen, u8 Index)
{
	u32 i;
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];
	u8 StrArray;

	USB_STD_STRING_DESC *StringDesc;

	if (!BufPtr) {
		return 0;
	}

	if (Index >= sizeof(StringList) / sizeof(u8 *)) {
		return 0;
	}

	/* USB 2.0 */
	StrArray = 0;

	String = (char *)&StringList[StrArray][Index];

	StringLen = strlen(String);

	StringDesc = (USB_STD_STRING_DESC *) TmpBuf;

	/* Index 0 is special as we can not represent the string required in
	 * the table above. Therefore we handle index 0 as a special case.
	 */
	if (0 == Index) {
		StringDesc->bLength = 4;
		StringDesc->bDescriptorType = 0x03;
		StringDesc->wLANGID[0] = 0x0409;
	}
	/* All other strings can be pulled from the table above. */
	else {
		StringDesc->bLength = StringLen * 2 + 2;
		StringDesc->bDescriptorType = 0x03;

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = (u16) String[i];
		}
	}
	DescLen = StringDesc->bLength;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufLen) {
		return 0;
	}

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}

/****************************************************************************/
/**
 * Changes State of Core to USB configured State.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	Ctrl is a pointer to the Setup packet data.
 *
 * @return	XST_SUCCESS else XST_FAILURE
 *
 * @note	None.
 *
 *****************************************************************************/
void XUsbPs_SetConfiguration(XUsbPs *InstancePtr, int ConfigIdx)
{
	u8 State;

	(void)ConfigIdx;

	State = InstancePtr->AppData->State;
	XUsbPs_SetConfigDone(InstancePtr, 0U);

	switch (State) {
		case XUSBPS_STATE_DEFAULT:
			break;

		case XUSBPS_STATE_ADDRESS:
			InstancePtr->AppData->State = XUSBPS_STATE_CONFIGURED;
			break;

		case XUSBPS_STATE_CONFIGURED:
			break;

		default:
			break;
	}

}

/****************************************************************************/
/**
 * This function is called by Chapter9 handler when SET_CONFIGURATION command
 * is received from Host.
 *
 * @param	InstancePtr is pointer to Usb_DevData instance.
 * @param	SetupData is the setup packet received from Host.
 *
 * @return
 *		- XST_SUCCESS if successful,
 *		- XST_FAILURE if unsuccessful.
 *
 * @note
 *		Non control endpoints must be enabled after SET_CONFIGURATION
 *		command since hardware clears all previously enabled endpoints
 *		except control endpoints when this command is received.
 *
 *****************************************************************************/
void XUsbPs_SetConfigurationApp(XUsbPs *InstancePtr,
				XUsbPs_SetupData *SetupData)
{

	XUsbPs_SetConfigDone((XUsbPs *)InstancePtr, 1U);
	if ((SetupData->wValue & 0xff) == 0) {
		/* Endpoint disables - not needed for Control EP */
		XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
				 XUSBPS_EP_DIRECTION_OUT);

		XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
				 XUSBPS_EP_DIRECTION_IN);

		XUsbPs_SetConfigDone((XUsbPs *)InstancePtr, 0U);
	}

}

/****************************************************************************/
/**
 * This function is called by Chapter9 handler when SET_CONFIGURATION command
 * or SET_INTERFACE command is received from Host.
 *
 * @param	InstancePtr is pointer to Usb_DevData instance.
 * @param	SetupData is the setup packet received from Host.
 *
 * @note
 *		Non control endpoints must be enabled after SET_INTERFACE
 *		command since hardware clears all previously enabled endpoints
 *		except control endpoints when this command is received.
 *
 *****************************************************************************/
void XUsbPs_SetInterfaceHandler(XUsbPs *InstancePtr,
				XUsbPs_SetupData *SetupData)
{
	s32 RetVal;
	u16 MaxPktSize = 1024;

#ifdef XUSBPS_UAC1

	if ((SetupData->wIndex & 0xff) != 1) {
		return;
	}

#ifdef XUSBPS_MICROPHONE

	if ((SetupData->wValue & 0xff) == 1) {
		XUsbPs_SetEpInterval(InstancePtr, ISO_EP,
				     XUSBPS_EP_DIRECTION_IN,
				     AUDIO_INTERVAL);
		Index = 0;
		Residue = 0;
		FirstPktFrame = 1;
		/* Endpoint enables - not needed for Control EP */
		RetVal = EpEnable((XUsbPs *)InstancePtr, ISO_EP,
				  XUSBPS_EP_DIRECTION_IN,
				  MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);
		if (RetVal != XST_SUCCESS) {
			xil_printf("failed to enable ISOC IN Ep\r\n");
			return;
		}
		XUsbPs_StreamOn(InstancePtr, ISO_EP, XUSBPS_EP_DIRECTION_IN,
				BufferPtrTemp);
	} else {
		XUsbPs_StreamOff(InstancePtr, ISO_EP, XUSBPS_EP_DIRECTION_IN);
		/* Endpoint disables - not needed for Control EP */
		XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
				 XUSBPS_EP_DIRECTION_IN);
	}

#else	/* SPEAKER */

	if ((SetupData->wValue & 0xff) == 1) {
		XUsbPs_SetEpInterval(InstancePtr, ISO_EP,
				     XUSBPS_EP_DIRECTION_OUT,
				     AUDIO_INTERVAL);
		Index = 0;
		Residue = 0;
		FirstPktFrame = 1;
		/* Endpoint enables - not needed for Control EP */
		RetVal = EpEnable(InstancePtr,
				  ISO_EP, XUSBPS_EP_DIRECTION_OUT,
				  MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);
		if (RetVal != XST_SUCCESS) {
			xil_printf("failed to enable ISOC OUT Ep\r\n");
			return;
		}
		XUsbPs_StreamOn(InstancePtr, ISO_EP, XUSBPS_EP_DIRECTION_OUT,
				BufferPtrTemp);
	} else {
		XUsbPs_StreamOff(InstancePtr, ISO_EP, XUSBPS_EP_DIRECTION_OUT);
		/* Endpoint disables - not needed for Control EP */
		XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
				 XUSBPS_EP_DIRECTION_OUT);
	}

#endif  /* end of SPEAKER */

#else	/*	UAC2 */

	if ((SetupData->wIndex & 0xff) == 1) {
		if ((SetupData->wValue & 0xff) == 1) {
			XUsbPs_SetEpInterval(InstancePtr,
					     ISO_EP, XUSBPS_EP_DIRECTION_OUT,
					     AUDIO_INTERVAL);
			Index = 0;
			Residue = 0;
			FirstPktFrame = 1;
			/* Endpoint enables - not needed for Control EP */
			RetVal = EpEnable((XUsbPs *)InstancePtr, ISO_EP,
					  XUSBPS_EP_DIRECTION_OUT,
					  MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);
			if (RetVal != XST_SUCCESS) {
				xil_printf("failed to enable ISOC OUT Ep\r\n");
				return;
			}
			XUsbPs_StreamOn(InstancePtr, ISO_EP,
					XUSBPS_EP_DIRECTION_OUT,
					BufferPtrTemp);
		} else {
			XUsbPs_StreamOff(InstancePtr, ISO_EP,
					 XUSBPS_EP_DIRECTION_OUT);
			/* Endpoint disables - not needed for Control EP */
			XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
					 XUSBPS_EP_DIRECTION_OUT);
		}
	}

	if ((SetupData->wIndex & 0xff) == 2) {
		if ((SetupData->wValue & 0xff) == 1) {
			XUsbPs_SetEpInterval(InstancePtr, ISO_EP,
					     XUSBPS_EP_DIRECTION_IN, AUDIO_INTERVAL);
			Index = 0;
			Residue = 0;
			FirstPktFrame = 1;
			/* Endpoint enables - not needed for Control EP */
			RetVal = EpEnable((XUsbPs *)InstancePtr, ISO_EP,
					  XUSBPS_EP_DIRECTION_IN,
					  MaxPktSize, USB_EP_TYPE_ISOCHRONOUS);
			if (RetVal != XST_SUCCESS) {
				xil_printf("failed to enable ISOC IN Ep\r\n");
				return;
			}
			XUsbPs_StreamOn(InstancePtr, ISO_EP,
					XUSBPS_EP_DIRECTION_IN,
					BufferPtrTemp);
		} else {
			XUsbPs_StreamOff(InstancePtr, ISO_EP,
					 XUSBPS_EP_DIRECTION_IN);
			/* Endpoint disables - not needed for Control EP */
			XUsbPs_EpDisable((XUsbPs *)InstancePtr, ISO_EP,
					 XUSBPS_EP_DIRECTION_IN);
		}
	}
#endif	// end of UAC2
}

/******************************************************************************/
/**
 * This function sets Endpoint Interval.
 *
 * @param	InstancePtr is a private member of Usb_DevData instance.
 * @param	UsbEpnum is Endpoint Number.
 * @param	Dir is Endpoint Direction(In/Out).
 * @param	Interval is the data transfer service interval
 *
 * @return 	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void XUsbPs_SetEpInterval(XUsbPs *InstancePtr, u8 UsbEpNum, u8 Dir,
				 u32 Interval)
{

	if (Dir == XUSBPS_EP_DIRECTION_OUT)
		((XUsbPs *)InstancePtr)->DeviceConfig.Ep[UsbEpNum].Out.Interval
			= Interval;
	else
		((XUsbPs *)InstancePtr)->DeviceConfig.Ep[UsbEpNum].In.Interval
			= Interval;
}

/******************************************************************************/
/**
 * This function start stream.
 *
 * @param	InstancePtr is a private member of Usb_DevData instance.
 * @param	Epnum is Endpoint Number.
 * @param	Dir is Endpoint Direction(In/Out).
 * @param	BufferPtr is buffer to sent/receive
 *
 * @return 	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void XUsbPs_StreamOn(XUsbPs *InstancePtr, u8 EpNum, u8 Dir,
			    u8 *BufferPtr)
{
	if (Dir == XUSBPS_EP_DIRECTION_OUT)
		XUsbPs_EpDataBufferReceive((XUsbPs *)InstancePtr, EpNum,
					   BufferPtr, 0);
	else {
		XUsbPs_EpBufferSend((XUsbPs *)InstancePtr, EpNum, BufferPtr, 0);
	}
}

/******************************************************************************/
/**
 * This function stop stream.
 *
 * @param	InstancePtr is a private member of Usb_DevData instance.
 * @param	Epnum is Endpoint Number.
 * @param	Dir is Endpoint Direction(In/Out).
 *
 * @return 	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static void XUsbPs_StreamOff(XUsbPs *InstancePtr, u8 EpNum, u8 Dir)
{
	(void)InstancePtr;
	(void)EpNum;
	(void)Dir;
}

/****************************************************************************/
/**
 * Enable the respective Endpoint
 *
 * @param	InstancePtr is a private member of Usb_DevData instance.
 * @param	EpNum is the Endpoint Number
 * @param	Dir is the Endpoint Direction
 * @param	Maxsize is the Endpoint Max Packet size
 * @param 	Type is the Endpoint type (Control / Bulk / Intr / ISO)
 *
 * @return	XST_SUCCESS else XST_FAILURE
 *
 * @note		None.
 *
 *****************************************************************************/
static s32 EpEnable(XUsbPs *InstancePtr, u8 EpNum, u8 Dir, u16 Maxsize, u8 Type)
{
	(void)Maxsize;
	XUsbPs_EpEnable((XUsbPs *)InstancePtr, EpNum, Dir);
	/* Set BULK mode for both directions.  */
	if (Dir == XUSBPS_EP_DIRECTION_OUT) {
		XUsbPs_ClrBits((XUsbPs *)InstancePtr,
			       XUSBPS_EPCRn_OFFSET(EpNum),
			       XUSBPS_EPCR_RXT_TYPE_MASK |
			       XUSBPS_EPCR_RXR_MASK |
			       XUSBPS_EPCR_RXS_MASK);
		XUsbPs_SetBits((XUsbPs *)InstancePtr,
			       XUSBPS_EPCRn_OFFSET(EpNum),
			       ((Type - 1) << XUSBPS_EPCR_RXT_TYPE_SHIFT) |
			       XUSBPS_EPCR_RXR_MASK);

		/* Prime the OUT endpoint. */
		XUsbPs_EpPrime((XUsbPs *)InstancePtr, EpNum, Dir);

		((XUsbPs *)InstancePtr)->DeviceConfig.Ep[EpNum].Out.MemAlloted
			= 0;
		((XUsbPs *)InstancePtr)->DeviceConfig.Ep[EpNum].Out.BufferPtr
			= NULL;
		((XUsbPs *)InstancePtr)->DeviceConfig.Ep[EpNum].Out.BytesTxed
			= 0;
		((XUsbPs *)InstancePtr)->
		DeviceConfig.Ep[EpNum].Out.RequestedBytes = 0;
	} else {
		XUsbPs_ClrBits((XUsbPs *)InstancePtr,
			       XUSBPS_EPCRn_OFFSET(EpNum),
			       XUSBPS_EPCR_TXT_TYPE_MASK |
			       XUSBPS_EPCR_TXR_MASK |
			       XUSBPS_EPCR_TXS_MASK);
		XUsbPs_SetBits((XUsbPs *)InstancePtr,
			       XUSBPS_EPCRn_OFFSET(EpNum),
			       ((Type - 1) << XUSBPS_EPCR_TXT_TYPE_SHIFT) |
			       XUSBPS_EPCR_TXR_MASK);

		((XUsbPs *)InstancePtr)->
		DeviceConfig.Ep[EpNum].In.BufferPtr = NULL;
		((XUsbPs *)InstancePtr)->
		DeviceConfig.Ep[EpNum].In.BytesTxed = 0;
		((XUsbPs *)InstancePtr)->
		DeviceConfig.Ep[EpNum].In.RequestedBytes = 0;
	}
	return XST_SUCCESS;
}
