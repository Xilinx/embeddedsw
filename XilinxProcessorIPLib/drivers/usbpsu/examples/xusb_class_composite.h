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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xusb_class_composite.h
 *
 * This file contains definitions used in the composite device class code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   05/03/18 First release
 * 1.5   vak  13/02/19 Added support for versal
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 *
 * </pre>
 *
 *****************************************************************************/
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
#define STORAGE_EP			3
#define KEYBOARD_EP			4
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

/* Mass storage opcodes */
#define USB_RBC_TEST_UNIT_READY		0x00
#define USB_RBC_REQUEST_SENSE		0x03
#define USB_RBC_FORMAT			0x04
#define USB_RBC_INQUIRY			0x12
#define USB_RBC_MODE_SELECT		0x15
#define USB_RBC_MODE_SENSE		0x1a
#define USB_RBC_STARTSTOP_UNIT		0x1b
#define USB_RBC_MEDIUM_REMOVAL		0x1e
#define USB_UFI_GET_CAP_LIST		0x23
#define USB_RBC_READ_CAP		0x25
#define USB_RBC_READ			0x28
#define USB_RBC_WRITE			0x2a
#define USB_RBC_VERIFY			0x2f
#define USB_SYNC_SCSI			0x35

/* Virtual Flash memory related definitions */
#define STORAGE_SIZE			0x6400000	/* 100MB space */
#define STORAGE_BLOCK_SIZE		0x200
#define STORAGE_NUM_BLOCKS		(STORAGE_SIZE/STORAGE_BLOCK_SIZE)

/* Class request opcodes */
#define USB_CLASSREQ_MASS_STORAGE_RESET	0xFF
#define USB_CLASSREQ_GET_MAX_LUN	0xFE

/* SCSI machine states */
#define USB_EP_STATE_COMMAND		0
#define USB_EP_STATE_DATA_IN		1
#define USB_EP_STATE_DATA_OUT		2
#define USB_EP_STATE_STATUS		3

/* HID class requests */
#define GET_REPORT_REQUEST              0x01
#define GET_IDLE_REQUEST                0x02
#define GET_PROTOCOL_REQUEST            0x03
#define SET_REPORT_REQUEST              0x09
#define SET_IDLE_REQUEST                0x0A
#define SET_PROTOCOL_REQUEST            0x0B

#define HID_KEYBOARD_PROTOCOL           0x00
#define REPORT_DESC_LENGTH              0x3F

#define REPORT_LENGTH                   8

#define AUDIO_INTF			0
#define AUDIO_INTF_OUT			1
#define AUDIO_INTF_IN			2
#define DFU_INTF			3
#define STORAGE_INTF			4
#define KEYBOARD_INTF			5

#define DIV_ROUND_UP(n, d)		(((n) + (d) - 1) / (d))

/**************************** Type Definitions *******************************/

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8  bmCBWFlags;
	u8  cCBWLUN;
	u8  bCBWCBLength;
	u8  CBWCB[16];
#ifdef __ICCARM__
} USB_CBW;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_CBW;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8  bCSWStatus;
#ifdef __ICCARM__
} USB_CSW;
#pragma pack(pop)
#else
} __attribute__((__packed__))USB_CSW;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef	struct {
	u8 deviceType;
	u8 rmb;
	u8 version;
	u8 blah;
	u8 additionalLength;
	u8 sccs;
	u8 info0;
	u8 info1;
	u8 vendorID[8];
	u8 productID[16];
	u8 revision[4];
#ifdef __ICCARM__
} SCSI_INQUIRY;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_INQUIRY;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  reserved[3];
	u8  listLength;
	u32 numBlocks;
	u8  descCode;
	u8  blockLengthMSB;
	u16 blockLength;
#ifdef __ICCARM__
} SCSI_CAP_LIST;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_CAP_LIST;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u32 numBlocks;
	u32 blockSize;
#ifdef __ICCARM__
} SCSI_READ_CAPACITY;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_READ_CAPACITY;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  opCode;
	u8  reserved1;
	u32 block;
	u8  reserved2;
	u16 length;
	u8  control;
#ifdef __ICCARM__
} SCSI_READ_WRITE;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_READ_WRITE;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
typedef struct {
	u8  opCode;
	u8  immed;
	u8  reserved1;
	u8  reserved2;
	u8  start;
	u8  control;
#ifdef __ICCARM__
} SCSI_START_STOP;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_START_STOP;
#endif

struct dfu_if {
	struct Usb_DevData *InstancePtr;
	u8	curr_state;
	u8	next_state;
	u8	status;
	u8	got_reset;
	u8	got_dnload_rqst;
	u8	dfu_wait_for_interrupt;
	u8	is_dfu;
	u8	runtime_to_dfu;
	u8	*disk;
	u32	disksize;
	u32	current_inf;
	u32	total_transfers;
	u32	total_bytes_dnloaded;
	u32	total_bytes_uploaded;
};

struct audio_if {
	u32	framesize;
	u32	interval;
	u32	packetsize;
	u32	packetresidue;
	u32	residue;
	u32	index;
	u32	firstpkt;
	u8	*disk;
	u32	disksize;
};

struct storage_if {
	u8	*disk;
	u8	*diskptr;
	u32	disksize;
	u32	bytesleft;
	u8	phase;
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
	USB_CBW	cbw;
#pragma data_alignment = 64
	USB_CSW	csw;
#else
#pragma data_alignment = 32
	USB_CBW	cbw;
#pragma data_alignment = 32
	USB_CSW	csw;
#endif
#else
	USB_CBW	cbw	ALIGNMENT_CACHELINE;
	USB_CSW	csw	ALIGNMENT_CACHELINE;
#endif
};

struct composite_dev {
	struct audio_if		f_audio;
	struct dfu_if		f_dfu;
	struct storage_if	f_storage;
};

/************************** Function Prototypes ******************************/

s32 Usb_DfuSetState(struct dfu_if *DFU, u8 dfu_state );
void USB_DfuSetDwloadState(struct dfu_if *DFU, u8 *status);
void USB_DfuGetStatus(struct dfu_if *DFU, u8 *status);
void Usb_DisconnectHandler(struct Usb_DevData *InstancePtr);
void Usb_ResetHandler(struct Usb_DevData *InstancePtr);
void Usb_SetIntf(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);

void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void ParseCBW(struct Usb_DevData *InstancePtr, struct storage_if *f);
void SendCSW(struct Usb_DevData *InstancePtr, struct storage_if *f, u32 Length);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
