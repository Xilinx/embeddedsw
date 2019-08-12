/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xusb_freertos_ch9_audio.c
*
* This file contains the implementation of the audio specific chapter 9 code
* for the example.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rb   26/03/18 First release
* 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xusb_freertos_ch9_audio.h"
#include "xusb_freertos_class_audio.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/* Device Descriptors */
#ifdef __ICCARM__
#pragma data_alignment = 16
#endif

#ifdef __ICCARM__
USB_STD_DEV_DESC deviceDesc[] = {
#else
USB_STD_DEV_DESC __attribute__ ((aligned(16))) deviceDesc[] = {
#endif
	{
		/* USB 2.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_TYPE_DEVICE_DESC,		/* bDescriptorType */
		0x0200,				/* bcdUSB 2.0 */
		USB_CLASS_MISC,			/* bDeviceClass */
		0x02,				/* bDeviceSubClass */
		0x01,				/* bDeviceProtocol */
		0x40,				/* bMaxPackedSize0 */
		0x03Fd,				/* idVendor */
		0x0200,				/* idProduct */
		0x0100,				/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	},
	{
		/* USB 3.0 */
		sizeof(USB_STD_DEV_DESC),	/* bLength */
		USB_TYPE_DEVICE_DESC,		/* bDescriptorType */
		0x0300,				/* bcdUSB 3.0 */
		USB_CLASS_MISC,			/* bDeviceClass */
		0x02,				/* bDeviceSubClass */
		0x01,				/* bDeviceProtocol */
		0x09,				/* bMaxPackedSize0 */
		0x03Fd,				/* idVendor */
		0x0200,				/* idProduct */
		0x0100,				/* bcdDevice */
		0x01,				/* iManufacturer */
		0x02,				/* iProduct */
		0x03,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	}
};

/* Configuration Descriptors */
#ifdef __ICCARM__
USB30_CONFIG config3 = {
#else
USB30_CONFIG __attribute__ ((aligned(16))) config3 = {
#endif
	{
		/* Std Config */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		USB_TYPE_CONFIG_DESC,		/* bDescriptorType */
		sizeof(USB30_CONFIG),		/* wTotalLength */
		0x03,				/* No. Of interfaces 3 */
		0x01,				/* No of configuration values */
		0x00,				/* Configuration string */
		0xC0,				/* bmAttribute */
		0x01				/* bMaxPower  */
	},
	{
		/* Class-Specific Interface Association Descriptor */
		sizeof(USB_IF_ASSOC_DESC),	/* bLength */
		USB_TYPE_INTERFACE_ASSOCIATION,	/* bDescriptorType */
		0x00,				/* bFirstInterface */
		0x03,				/* bInterfaceCount */
		USB_CLASS_AUDIO,		/* bFunctionclass */
		UAC2_FUNCTION_SUBCLASS_UNDEFINED,/* bFunctionSubClass */
		UAC_VERSION,			/* bFunctionProtocol */
		0x04				/* iFunction */
	},
	{
		/* UAC2.0 Standard Interface Descriptor */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x00,				/* Interface number 0 */
		0x00,				/* Alternate set 0 */
		0x00,				/* Number of end points 0 */
		USB_CLASS_AUDIO,		/* Audio device */
		USB_SUBCLASS_AUDIOCONTROL,	/* Audio Control */
		UAC_VERSION,			/* Interface Protocol */
		0x05				/* iInterface */
	},
	{
		/* USB Audio Class-Specific AC Interface Header Descriptor */
		sizeof(UAC2_AC_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bcdADC - Audio Class 1.0 L */
		0x02,				/* bcdADC - Audio Class 1.0 H */
		UAC2_FUNCTION_IO_BOX,		/* bCategory */
		(sizeof(UAC2_AC_HEADER_DESC) +
		 sizeof(UAC2_CLOCK_SOURCE_DESC) +
		 sizeof(UAC2_CLOCK_SELECTOR_DESC) +
		 (sizeof(UAC2_INPUT_TERMINAL_DESC) +
		  sizeof(UAC2_FEATURE_UNIT_DESC) +
		  sizeof(UAC2_OUTPUT_TERMINAL_DESC)) * 2),/* wTotalLength */
		0x01				/* bmControls */
	},
	{
		/* Clock Source Descriptor */
		sizeof(UAC2_CLOCK_SOURCE_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SOURCE,		/* bDescriptorSubtype */
		USB_CLK_SRC_ID,			/* bClockId */
		UAC2_CLOCK_SOURCE_TYPE_INT_FIXED,	/* bmAttributes */
		(CONTROL_RDONLY << CLK_FREQ_CTRL),	/* bmControls */
		0x00,				/* bAssocTerminal */
		0x06				/* iClockSource */
	},
	{
		/* Clock Selector Descriptor */
		sizeof(UAC2_CLOCK_SELECTOR_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SELECTOR,		/* bDescriptorSubtype */
		USB_CLK_SEL_ID,			/* bClockId */
		0x01,				/* bNrPins */
		{
			USB_CLK_SRC_ID		/* baCSourceID(0) */
		},
		0x03,				/* bmControl */
		0x00				/* iClockSelector */
	},
	{
		/* USB_OUT Input terminal */
		sizeof(UAC2_INPUT_TERMINAL_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
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
	{
		/* USB_OUT Feature Unit Descriptor */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		OUT_FETR_UNT_ID,		/* bUnitID */
		USB_OUT_IT_ID,			/* bSourceID */
		{
#ifdef AXI_USB
			/*
			 * In AXI_USB, we are not getting any SET class-request
			 * interrupt for Volume Feature SET request.
			 */
			0x00000000,		/* bmaControls(0) */
			0x00000000,		/* bmaControls(1) */
			0x00000000		/* bmaControls(2) */
#else	/* For ZYNQ and ZYNQMP */
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
#endif
		},
		0x00				/* iFeature */
	},
	{
		/* USB_OUT Output Terminal */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_OUT_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x03,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		OUT_FETR_UNT_ID,		/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x08				/* iTerminal */
	},
	{
		/* USB_IN Input Terminal */
		sizeof(UAC2_INPUT_TERMINAL_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
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
	{
		/* USB_IN Feature Unit Descriptor */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		IN_FETR_UNT_ID,			/* bUnitID */
		USB_IN_IT_ID,			/* bSourceID */
		{
#ifdef AXI_USB
			/*
			 * In AXI_USB, we are not getting any SET class-request
			 * interrupt for Volume Feature SET request.
			 */
			0x00000000,		/* bmaControls(0) */
			0x00000000,		/* bmaControls(1) */
			0x00000000		/* bmaControls(2) */
#else	/* For ZYNQ and ZYNQMP */
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
#endif
		},
		0x00				/* iFeature */
	},
	{
		/* USB_IN Output Terminal */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x01,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		IN_FETR_UNT_ID,			/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x0a				/* iTerminal */
	},
	{
		/* Audio Streaming OUT Interface - Alt0 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0b				/* iInterface */
	},
	{
		/* Audio Streaming OUT Interface - Alt1 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0c				/* iInterface */
	},
	{
		/* Audio Stream OUT Interface Descriptor */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType  */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_OUT_IT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,		/* bmFormatsL1 */
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
	{
		/* Audio USB_OUT Format */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{
		/* STD OUT Endpoint */
		sizeof(USB_EP_DESC),		/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bType */
		USB_EP1_OUT,			/* bEndpointAddress */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{
		/* SS Endpoint companion */
		sizeof(USB_STD_EP_SS_COMP_DESC),/* bLength */
		0x30,				/* bDescriptorType */
		0x0F,				/* bMaxBurst */
		0x00,				/* bmAttributes */
		0x0400				/* wBytesPerInterval */
	},
	{
		/* CS AS ISO OUT Endpoint */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		USB_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bmControls */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	},
	{
		/* Audio Streaming IN Interface - Alt0 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0d				/* iInterface */
	},
	{
		/* Audio Streaming IN Interface - Alt1 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0e				/* iInterface */
	},
	{
		/* Audio Stream IN Interface Descriptor */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,		/* bmFormatsL1 */
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
	{
		/* Audio USB_IN Format */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{
		/* STD AS ISO IN Endpoint */
		sizeof(USB_EP_DESC),		/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bType */
		USB_EP1_IN,			/* bEndpointAddress */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{
		/* SS Endpoint companion */
		sizeof(USB_STD_EP_SS_COMP_DESC),/* bLength */
		0x30,				/* bDescriptorType */
		0x0F,				/* bMaxBurst */
		0x00,				/* bmAttributes */
		0x0400				/* wBytesPerInterval */
	},
	{
		/* CS AS ISO IN Endpoint */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		USB_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
};

#ifdef __ICCARM__
USB_CONFIG config2 = {
#else
USB_CONFIG __attribute__ ((aligned(16))) config2 = {
#endif
	{
		/* Std Config */
		sizeof(USB_STD_CFG_DESC),	/* bLength */
		USB_TYPE_CONFIG_DESC,		/* bDescriptorType */
		sizeof(USB_CONFIG),		/* wTotalLength */
		0x03,				/* No. Of interfaces 3 */
		0x01,				/* No of configuration values */
		0x00,				/* Configuration string */
		0xC0,				/* bmAttribute */
		0x01				/* bMaxPower  */
	},
	{
		/* Class-Specific Interface Association Descriptor */
		sizeof(USB_IF_ASSOC_DESC),	/* bLength */
		USB_TYPE_INTERFACE_ASSOCIATION,	/* bDescriptorType */
		0x00,				/* bFirstInterface */
		0x03,				/* bInterfaceCount */
		USB_CLASS_AUDIO,		/* bFunctionclass */
		UAC2_FUNCTION_SUBCLASS_UNDEFINED,/* bFunctionSubClass */
		UAC_VERSION,			/* bFunctionProtocol */
		0x04				/* iFunction */
	},
	{
		/* UAC2.0 Standard Interface Descriptor */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x00,				/* Interface number 0 */
		0x00,				/* Alternate set 0 */
		0x00,				/* Number of end points 0 */
		USB_CLASS_AUDIO,		/* Audio device */
		USB_SUBCLASS_AUDIOCONTROL,	/* Audio Control */
		UAC_VERSION,			/* Interface Protocol */
		0x05				/* iInterface */
	},
	{
		/* USB Audio Class-Specific AC Interface Header Descriptor */
		sizeof(UAC2_AC_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		0x01,				/* bDescriptorSubtype */
		0x00,				/* bcdADC - Audio Class 1.0 L */
		0x02,				/* bcdADC - Audio Class 1.0 H */
		UAC2_FUNCTION_IO_BOX,		/* bCategory */
		(sizeof(UAC2_AC_HEADER_DESC) +
			sizeof(UAC2_CLOCK_SOURCE_DESC) +
			sizeof(UAC2_CLOCK_SELECTOR_DESC) +
			(sizeof(UAC2_INPUT_TERMINAL_DESC) +
			sizeof(UAC2_FEATURE_UNIT_DESC) +
			sizeof(UAC2_OUTPUT_TERMINAL_DESC)) * 2),/* wTotalLength */
		0x01				/* bmControls */
	},
	{
		/* Clock Source Descriptor */
		sizeof(UAC2_CLOCK_SOURCE_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SOURCE,		/* bDescriptorSubtype */
		USB_CLK_SRC_ID,			/* bClockId */
		UAC2_CLOCK_SOURCE_TYPE_INT_FIXED,/* bmAttributes */
		(CONTROL_RDONLY << CLK_FREQ_CTRL),/* bmControls */
		0x00,				/* bAssocTerminal */
		0x06				/* iClockSource */
	},
	{
		/* Clock Selector Descriptor */
		sizeof(UAC2_CLOCK_SELECTOR_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC2_CLOCK_SELECTOR,		/* bDescriptorSubtype */
		USB_CLK_SEL_ID,			/* bClockId */
		0x01,				/* bNrPins */
		{
			USB_CLK_SRC_ID		/* baCSourceID(0) */
		},
		0x03,				/* bmControl */
		0x00				/* iClockSelector */
	},
	{
		/* USB_OUT Input terminal */
		sizeof(UAC2_INPUT_TERMINAL_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
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
	{
		/* USB_OUT Feature Unit Descriptor */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		OUT_FETR_UNT_ID,		/* bUnitID */
		USB_OUT_IT_ID,			/* bSourceID */
		{
#ifdef AXI_USB
			/*
			 * In AXI_USB, we are not getting any SET class-request
			 * interrupt for Volume Feature SET request.
			 */
			0x00000000,		/* bmaControls(0) */
			0x00000000,		/* bmaControls(1) */
			0x00000000		/* bmaControls(2) */
#else	/* For ZYNQ and ZYNQMP */
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
#endif
		},
		0x00				/* iFeature */
	},
	{
		/* USB_OUT Output Terminal */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_OUT_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x03,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		OUT_FETR_UNT_ID,		/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x08				/* iTerminal */
	},
	{
		/* USB_IN Input Terminal */
		sizeof(UAC2_INPUT_TERMINAL_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
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
	{
		/* USB_IN Feature Unit Descriptor */
		sizeof(UAC2_FEATURE_UNIT_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FEATURE_UNIT,		/* bDescriptorSubtype */
		IN_FETR_UNT_ID,			/* bUnitID */
		USB_IN_IT_ID,			/* bSourceID */
		{
#ifdef AXI_USB
			/*
			 * In AXI_USB, we are not getting any SET class-request
			 * interrupt for Volume Feature SET request.
			 */
			0x00000000,		/* bmaControls(0) */
			0x00000000,		/* bmaControls(1) */
			0x00000000		/* bmaControls(2) */
#else	/* For ZYNQ and ZYNQMP */
			0x0000000F,		/* bmaControls(0) */
			/* Mute and Volume host read and writable */
			0x0000000F,		/* bmaControls(1) */
			/* Mute and Volume host read and writable */
			0x0000000F		/* bmaControls(2) */
#endif
		},
		0x00				/* iFeature */
	},
	{
		/* USB_IN Output Terminal */
		sizeof(UAC2_OUTPUT_TERMINAL_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_OUTPUT_TERMINAL,		/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalId */
		0x01,				/* wTerminalType L */
		0x01,				/* wTerminalType H */
		0x00,				/* bAssocTerminal */
		IN_FETR_UNT_ID,			/* bSourceId */
		USB_CLK_SEL_ID,			/* bCSourceId */
		(CONTROL_RDWR << COPY_CTRL),	/* bmControls */
		0x0a				/* iTerminal */
	},
	{
		/* Audio Streaming OUT Interface - Alt0 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0b				/* iInterface */
	},
	{
		/* Audio Streaming OUT Interface - Alt1 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x01,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0c				/* iInterface */
	},
	{
		/* Audio Stream OUT Interface Descriptor */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType  */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_OUT_IT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,		/* bmFormatsL1 */
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
	{
		/* Audio USB_OUT Format */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{
		/* STD OUT Endpoint */
		sizeof(USB_EP_DESC),		/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bType */
		USB_EP1_OUT,			/* bEndpointAddress */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{
		/* CS AS ISO OUT Endpoint */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		USB_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bmControls */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	},
	{
		/* Audio Streaming IN Interface - Alt0 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x00,				/* bAlternateSetting */
		0x00,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0d				/* iInterface */
	},
	{
		/* Audio Streaming IN Interface - Alt1 */
		sizeof(USB_STD_IF_DESC),	/* bLength */
		USB_TYPE_INTERFACE_DESC,	/* bDescriptorType */
		0x02,				/* bInterfaceNumber */
		0x01,				/* bAlternateSetting */
		0x01,				/* bNumEndpoints */
		USB_CLASS_AUDIO,		/* bInterfaceClass */
		USB_SUBCLASS_AUDIOSTREAMING,	/* bInterfaceSubclass */
		UAC_VERSION,			/* bInterfaceProtocol */
		0x0e				/* iInterface */
	},
	{
		/* Audio Stream IN Interface Descriptor */
		sizeof(UAC2_AS_HEADER_DESC),	/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_AS_GENERAL,			/* bDescriptorSubtype */
		USB_IN_OT_ID,			/* bTerminalLink */
		0x00,				/* bmControls */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		UAC2_FORMAT_TYPE_I_PCM,		/* bmFormatsL1 */
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
	{
		/* Audio USB_IN Format */
		sizeof(UAC2_FORMAT_TYPE_I_DESC),/* bLength */
		USB_DT_CS_INTERFACE,		/* bDescriptorType */
		UAC_FORMAT_TYPE_SUBTYPE,	/* bDescriptorSubtype */
		UAC_FORMAT_TYPE_I,		/* bFormatType */
		AUDIO_FRAME_SIZE,		/* bSubSlotSize */
		BIT_RESOLUTION			/* bBitResolution */
	},
	{
		/* STD AS ISO IN Endpoint */
		sizeof(USB_EP_DESC),		/* bLength */
		USB_TYPE_ENDPOINT_CFG_DESC,	/* bType */
		USB_EP1_IN,			/* bEndpointAddress */
		0x01 | USB_ENDPOINT_SYNC_ASYNC,	/* Isochronous, adaptive */
		0x00,				/* bMaxPacketSizeL */
		0x04,				/* bMaxPacketSizeH */
		AUDIO_INTERVAL			/* bInterval */
	},
	{
		/* CS AS ISO IN Endpoint */
		sizeof(UAC2_ISO_EP_DESC),	/* bLength */
		USB_DT_CS_ENDPOINT,		/* bDescriptorType */
		UAC_EP_GENERAL,			/* bDescriptorSubtype */
		0x00,				/* bmAttributes */
		0x00,				/* bLockDelayUnits */
		0x00,				/* wLockDelayL */
		0x00				/* wLockDelayH */
	}
};

/* String Descriptors */
static u8 StringList[2][20][128] = {
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
	},
	{
		"UNUSED",
		"XILINX",
		"USB3.0 AUDIO",
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

/*****************************************************************************/
/**
 *
 * This function returns the device descriptor for the device.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	BufPtr is pointer to the buffer that is to be filled
 *		with the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return	Length of the descriptor in the buffer on success.
 *		0 on error.
 *
 ******************************************************************************/
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	u8 Index;
	s32 Status;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		Index = 0;
	} else {
		/* USB 3.0 */
		Index = 1;
	}

	if (!BufPtr || BufLen < sizeof(USB_STD_DEV_DESC))
		return 0;

	memcpy(BufPtr, &deviceDesc[Index], sizeof(USB_STD_DEV_DESC));

	return sizeof(USB_STD_DEV_DESC);
}


/*****************************************************************************/
/**
 *
 * This function returns the configuration descriptor for the device.
 *
 * @param	InstancePtr is a pointer to the Usb_DevData instance.
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *		the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return	Length of the descriptor in the buffer on success.
 *		0 on error.
 *
 ******************************************************************************/
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr,
		u8 *BufPtr, u32 BufLen)
{
	s32 Status;
	u8 *config;
	u32 CfgDescLen;

	if (!BufPtr || BufLen < sizeof(USB_STD_CFG_DESC))
		return 0;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		config = (u8 *)&config2;
		CfgDescLen  = sizeof(USB_CONFIG);

	} else {
		/* USB 3.0 */
		config = (u8 *)&config3;
		CfgDescLen  = sizeof(USB30_CONFIG);

	}

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
 *		the descriptor.
 * @param	BufLen is the size of the provided buffer.
 * @param	Index is the index of the string for which the descriptor
 *		is requested.
 *
 * @return	Length of the descriptor in the buffer on success.
 *		0 on error.
 *
 ******************************************************************************/
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr,
		u32 BufLen, u8 Index)
{
	s32 i;
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];
	s32 Status;
	u8 StrArray;

	USB_STD_STRING_DESC *StringDesc;

	if (!BufPtr)
		return 0;

	/* check the validity of index */
	if (Index >= sizeof(StringList) / sizeof(u8 *))
		return 0;

	Status = IsSuperSpeed(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* USB 2.0 */
		StrArray = 0;
	} else {
		/* USB 3.0 */
		StrArray = 1;
	}

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

		for (i = 0; i < StringLen; i++)
			StringDesc->wLANGID[i] = (u16) String[i];
	}
	DescLen = StringDesc->bLength;

	/* Check if the provided buffer is big enough to hold the descriptor. */
	if (DescLen > BufLen)
		return 0;

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}


/*****************************************************************************/
/**
 *
 * This function returns the BOS descriptor for the device.
 *
 * @param	BufPtr is the pointer to the buffer that is to be filled with
 *		the descriptor.
 * @param	BufLen is the size of the provided buffer.
 *
 * @return	Length of the descriptor in the buffer on success.
 *		0 on error.
 *
 ******************************************************************************/
u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen)
{
#ifdef __ICCARM__
#pragma data_alignment = 16
	static USB_BOS_DESC bosDesc = {
#else
	static USB_BOS_DESC __attribute__ ((aligned(16))) bosDesc = {
#endif
		/* BOS descriptor */
		{
			sizeof(USB_STD_BOS_DESC),	/* bLength */
			USB_TYPE_BOS_DESC,		/* DescriptorType */
			sizeof(USB_BOS_DESC),		/* wTotalLength */
			0x02
		}, /* bNumDeviceCaps */

		{
			sizeof(USB_STD_DEVICE_CAP_7BYTE), /* bLength */
			0x10,				/* bDescriptorType */
			0x02,				/* bDevCapabiltyType */
			0x06
		}, /* bmAttributes */

		{
			sizeof(USB_STD_DEVICE_CAP_10BYTE), /* bLength */
			0x10,				/* bDescriptorType */
			0x03,				/* bDevCapabiltyType */
			0x00,				/* bmAttributes */
			(0x000F),			/* wSpeedsSupported */
			0x01,				/* bFunctionalitySup */
			0x01,				/* bU1DevExitLat */
			(0x01F4)}			/* wU2DevExitLat */
	};

	if (!BufPtr || BufLen < sizeof(USB_STD_BOS_DESC))
		return 0;

	memcpy(BufPtr, &bosDesc, sizeof(USB_BOS_DESC));

	return sizeof(USB_BOS_DESC);
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
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl)
{
	u8 State;
	s32 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	State = InstancePtr->State;
	SetConfigDone(InstancePtr->PrivateData, 0U);

	switch (State) {
		case USB_STATE_DEFAULT:
			Ret = XST_FAILURE;
			break;

		case USB_STATE_ADDRESS:
			InstancePtr->State = USB_STATE_CONFIGURED;
			break;

		case USB_STATE_CONFIGURED:
			break;

		default:
			Ret = XST_FAILURE;
			break;
	}

	return Ret;
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
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr,
		SetupPacket *SetupData)
{
	s32 RetVal;

	SetConfigDone(InstancePtr->PrivateData, 1U);
	if ((SetupData->wValue & 0xff) == 0) {

		/* Endpoint disables - not needed for Control EP */
		RetVal = EpDisable(InstancePtr->PrivateData, ISO_EP,
				USB_EP_DIR_OUT);
		if (RetVal != XST_SUCCESS) {
			xil_printf("failed to disable ISOC OUT Ep\r\n");
			return XST_FAILURE;
		}

		RetVal = EpDisable(InstancePtr->PrivateData, ISO_EP,
				USB_EP_DIR_IN);
		if (RetVal != XST_SUCCESS) {
			xil_printf("failed to disable ISOC IN Ep\r\n");
			return XST_FAILURE;
		}

		SetConfigDone(InstancePtr->PrivateData, 0U);
	}

	return XST_SUCCESS;
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
void Usb_SetInterfaceHandler(struct Usb_DevData *InstancePtr,
		SetupPacket *SetupData)
{
	USBCH9_DATA *ch9_ptr =
		(USBCH9_DATA *)Get_DrvData(InstancePtr->PrivateData);
	struct audio_dev *dev = (struct audio_dev *)(ch9_ptr->data_ptr);
	BaseType_t xHigherPriorityTaskWoken;

	if ((SetupData->wIndex & 0xff) == 1) {
		if ((SetupData->wValue & 0xff) == 1) {

			xTaskNotifyFromISR(dev->xMainTask, PLAY_START,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		} else {

			xTaskNotifyFromISR(dev->xMainTask, PLAY_STOP,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}

	if ((SetupData->wIndex & 0xff) == 2) {
		if ((SetupData->wValue & 0xff) == 1) {

			xTaskNotifyFromISR(dev->xMainTask, RECORD_START,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		} else {

			xTaskNotifyFromISR(dev->xMainTask, RECORD_STOP,
					eSetValueWithOverwrite,
					&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}
