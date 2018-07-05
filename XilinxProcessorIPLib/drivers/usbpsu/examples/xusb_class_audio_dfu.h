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
 * @file xusb_class_audio_dfu.h
 *
 * This file contains definitions used in the AUDIO-DFU composite class code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/02/18 First release
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XUSB_CLASS_DFU_H
#define XUSB_CLASS_DFU_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xusb_ch9.h"

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
#define DFU_DETACH      		0
#define DFU_DNLOAD      		1
#define DFU_UPLOAD      		2
#define DFU_GETSTATUS   		3
#define DFU_CLRSTATUS   		4
#define DFU_GETSTATE    		5
#define DFU_ABORT       		6

/* DFU alternate setting value when in run-time mode */
#define DFU_ALT_SETTING 		1

/* A.14 Audio Class-Specific Request Codes */
#define UAC2_CS_CUR			0x01
#define UAC2_CS_RANGE			0x02

/* A.17 Control Selector Codes */
/* A.17.1 Clock Source Control Selectors */
#define UAC2_CS_CONTROL_UNDEFINED	0x00
#define UAC2_CS_CONTROL_SAM_FREQ	0x01
#define UAC2_CS_CONTROL_CLOCK_VALID	0x02

/* A.17.2 Clock Selector Control Selectors */
#define UAC2_CX_CONTROL_UNDEFINED	0x00
#define UAC2_CX_CONTROL_CLOCK_SEL	0x01

/* A.17.7 Feature Unit Control Selectors */
#define UAC2_FU_CONTROL_UNDEFINED	0x00
#define UAC2_FU_MUTE_CONTROL		0x01
#define UAC2_FU_VOLUME_CONTROL		0x02

#define ISO_EP				2
#define INTERVAL_PER_SECOND		(u32)(8000)

/* Isoc interval for AUDIO */
#define AUDIO_INTERVAL			0x04

/* Array of supported sampling frequencies */
#define MAX_AUDIO_FREQ			0x04
#define CUR_AUDIO_FREQ			0x01

/* Bytes per Audio transferred frame */
#define AUDIO_FRAME_SIZE		0x02

/* Bits per Audio transferred frame */
#define BIT_RESOLUTION			((AUDIO_FRAME_SIZE) * (0x08))

#define AUDIO_INTF			0
#define AUDIO_INTF_OUT			1
#define AUDIO_INTF_IN			2
#define DFU_INTF			3

#define DIV_ROUND_UP(n, d)		(((n) + (d) - 1) / (d))

/**************************** Type Definitions *******************************/
struct dfu_if {
	struct Usb_DevData *InstancePtr;
	u8	curr_state;
	u8	next_state;
	u8	status;
	u8	got_reset;
	u32	current_inf;
	u8	got_dnload_rqst;
	u32	total_transfers;
	u32	total_bytes_dnloaded;
	u32	total_bytes_uploaded;
	u8	dfu_wait_for_interrupt;
	u8	is_dfu;
	u8	runtime_to_dfu;
};

struct audio_if {
	u32	framesize;
	u32	interval;
	u32	packetsize;
	u32	packetresidue;
	u32	residue;
	u32	index;
	u32	firstpkt;
};

struct audio_dfu_if {
	u8			*virtualdisk;
	u32			disksize;
	struct audio_if		f_audio;
	struct dfu_if		f_dfu;
};

/************************** Function Prototypes ******************************/

s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state );
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status);
void USB_DfuGetStatus(struct dfu_if *DFU, u8 *status);
void Usb_DisconnectHandler(struct Usb_DevData *InstancePtr);
void Usb_ResetHandler(struct Usb_DevData *InstancePtr);
void Usb_SetIntf(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
