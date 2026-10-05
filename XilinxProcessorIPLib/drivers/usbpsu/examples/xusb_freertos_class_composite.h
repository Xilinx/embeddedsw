/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_class_composite.h
 *
 * This file contains definitions used in the composite device class code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   28/03/18 First release
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
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "xusb_ch9.h"

/************************** Constant Definitions *****************************/
/* DFU states */
#define STATE_APP_IDLE                  0x00	/**< State App Idle. */
#define STATE_APP_DETACH                0x01	/**< State App Detach. */
#define STATE_DFU_IDLE                  0x02	/**< State Dfu Idle. */
#define STATE_DFU_DOWNLOAD_SYNC         0x03	/**< State Dfu Download Sync. */
#define STATE_DFU_DOWNLOAD_BUSY         0x04	/**< State Dfu Download Busy. */
#define STATE_DFU_DOWNLOAD_IDLE         0x05	/**< State Dfu Download Idle. */
#define STATE_DFU_MANIFEST_SYNC         0x06	/**< State Dfu Manifest Sync. */
#define STATE_DFU_MANIFEST              0x07	/**< State Dfu Manifest. */
#define STATE_DFU_MANIFEST_WAIT_RESET   0x08	/**< State Dfu Manifest Wait Reset. */
#define STATE_DFU_UPLOAD_IDLE           0x09	/**< State Dfu Upload Idle. */
#define STATE_DFU_ERROR                 0x0a	/**< State Dfu Error. */

/* DFU status */
#define DFU_STATUS_OK                   0x00	/**< DFU status: No error. */
#define DFU_STATUS_ERROR_TARGET         0x01	/**< Dfu Status Error Target. */
#define DFU_STATUS_ERROR_FILE           0x02	/**< Dfu Status Error File. */
#define DFU_STATUS_ERROR_WRITE          0x03	/**< Dfu Status Error Write. */
#define DFU_STATUS_ERROR_ERASE          0x04	/**< Dfu Status Error Erase. */
#define DFU_STATUS_ERROR_CHECK_ERASED   0x05	/**< Dfu Status Error Check Erased. */
#define DFU_STATUS_ERROR_PROG           0x06	/**< Dfu Status Error Prog. */
#define DFU_STATUS_ERROR_VERIFY         0x07	/**< Dfu Status Error Verify. */
#define DFU_STATUS_ERROR_ADDRESS        0x08	/**< Dfu Status Error Address. */
#define DFU_STATUS_ERROR_NOTDONE        0x09	/**< Dfu Status Error Notdone. */
#define DFU_STATUS_ERROR_FIRMWARE       0x0a	/**< Dfu Status Error Firmware. */
#define DFU_STATUS_ERROR_VENDOR         0x0b	/**< Dfu Status Error Vendor. */
#define DFU_STATUS_ERROR_USBR           0x0c	/**< Dfu Status Error Usbr. */
#define DFU_STATUS_ERROR_POR            0x0d	/**< Dfu Status Error Por. */
#define DFU_STATUS_ERROR_UNKNOWN        0x0e	/**< Dfu Status Error Unknown. */
#define DFU_STATUS_ERROR_STALLEDPKT     0x0f	/**< Dfu Status Error Stalledpkt. */

/* DFU commands */
#define DFU_DETACH      		0	/**< Dfu Detach. */
#define DFU_DNLOAD      		1	/**< Dfu Dnload. */
#define DFU_UPLOAD      		2	/**< Dfu Upload. */
#define DFU_GETSTATUS   		3	/**< Dfu Getstatus. */
#define DFU_CLRSTATUS   		4	/**< Dfu Clrstatus. */
#define DFU_GETSTATE    		5	/**< Dfu Getstate. */
#define DFU_ABORT       		6	/**< Dfu Abort. */

/* DFU alternate setting value when in run-time mode */
#define DFU_ALT_SETTING 		1	/**< Dfu Alt Setting. */

/* A.14 Audio Class-Specific Request Codes */
#define UAC2_CS_CUR			0x01	/**< Uac2 Cs Cur. */
#define UAC2_CS_RANGE			0x02	/**< Uac2 Cs Range. */

/* A.17 Control Selector Codes */
/* A.17.1 Clock Source Control Selectors */
#define UAC2_CS_CONTROL_UNDEFINED	0x00	/**< Uac2 Cs Control Undefined. */
#define UAC2_CS_CONTROL_SAM_FREQ	0x01	/**< Uac2 Cs Control Sam Freq. */
#define UAC2_CS_CONTROL_CLOCK_VALID	0x02	/**< Uac2 Cs Control Clock Valid. */

/* A.17.2 Clock Selector Control Selectors */
#define UAC2_CX_CONTROL_UNDEFINED	0x00	/**< Uac2 Cx Control Undefined. */
#define UAC2_CX_CONTROL_CLOCK_SEL	0x01	/**< Uac2 Cx Control Clock Sel. */

/* A.17.7 Feature Unit Control Selectors */
#define UAC2_FU_CONTROL_UNDEFINED	0x00	/**< Uac2 Fu Control Undefined. */
#define UAC2_FU_MUTE_CONTROL		0x01	/**< Uac2 Fu Mute Control. */
#define UAC2_FU_VOLUME_CONTROL		0x02	/**< Uac2 Fu Volume Control. */

#define ISO_EP				2	/**< Iso Ep. */
#define STORAGE_EP			3	/**< Mass storage endpoint number. */
#define KEYBOARD_EP			4	/**< Keyboard Ep. */
/** @brief Interval Per Second. */
#define INTERVAL_PER_SECOND		(u32)(8000)

/* Isoc interval for AUDIO */
#define AUDIO_INTERVAL			0x04	/**< Audio Interval. */

/* Array of supported sampling frequencies */
#define MAX_AUDIO_FREQ			0x04	/**< Max Audio Freq. */
#define CUR_AUDIO_FREQ			0x01	/**< Cur Audio Freq. */

/* Bytes per Audio transferred frame */
#define AUDIO_FRAME_SIZE		0x02	/**< Audio Frame Size. */

/* Bits per Audio transferred frame */
/** @brief Bit Resolution. */
#define BIT_RESOLUTION			((AUDIO_FRAME_SIZE) * (0x08))

/* Mass storage opcodes. */
#define USB_RBC_TEST_UNIT_READY		0x00	/**< Usb Rbc Test Unit Ready. */
#define USB_RBC_REQUEST_SENSE		0x03	/**< Usb Rbc Request Sense. */
#define USB_RBC_FORMAT			0x04	/**< Usb Rbc Format. */
#define USB_RBC_INQUIRY			0x12	/**< Usb Rbc Inquiry. */
#define USB_RBC_MODE_SELECT		0x15	/**< Usb Rbc Mode Select. */
#define USB_RBC_MODE_SENSE		0x1a	/**< Usb Rbc Mode Sense. */
#define USB_RBC_STARTSTOP_UNIT		0x1b	/**< Usb Rbc Startstop Unit. */
#define USB_RBC_MEDIUM_REMOVAL		0x1e	/**< Usb Rbc Medium Removal. */
#define USB_UFI_GET_CAP_LIST		0x23	/**< Usb Ufi Get Cap List. */
#define USB_RBC_READ_CAP		0x25	/**< Usb Rbc Read Cap. */
#define USB_RBC_READ			0x28	/**< Usb Rbc Read. */
#define USB_RBC_WRITE			0x2a	/**< Usb Rbc Write. */
#define USB_RBC_VERIFY			0x2f	/**< Usb Rbc Verify. */
#define USB_SYNC_SCSI			0x35	/**< Usb Sync Scsi. */

/* Virtual Flash memory related definitions */
#define STORAGE_SIZE			0x6400000	/**< Storage Size. */
#define STORAGE_BLOCK_SIZE		0x200	/**< Storage Block Size. */
/** @brief Storage Num Blocks. */
#define STORAGE_NUM_BLOCKS		(STORAGE_SIZE/STORAGE_BLOCK_SIZE)

/* Class request opcodes */
#define USB_CLASSREQ_MASS_STORAGE_RESET	0xFF	/**< Usb Classreq Mass Storage Reset. */
#define USB_CLASSREQ_GET_MAX_LUN	0xFE	/**< Usb Classreq Get Max Lun. */

/* SCSI machine states */
#define USB_EP_STATE_COMMAND		0	/**< Usb Ep State Command. */
#define USB_EP_STATE_DATA_IN		1	/**< Usb Ep State Data In. */
#define USB_EP_STATE_DATA_OUT		2	/**< Usb Ep State Data Out. */
#define USB_EP_STATE_STATUS		3	/**< Usb Ep State Status. */

/* HID class requests */
#define GET_REPORT_REQUEST              0x01	/**< Get Report Request. */
#define GET_IDLE_REQUEST                0x02	/**< Get Idle Request. */
#define GET_PROTOCOL_REQUEST            0x03	/**< Get Protocol Request. */
#define SET_REPORT_REQUEST              0x09	/**< Set Report Request. */
#define SET_IDLE_REQUEST                0x0A	/**< Set Idle Request. */
#define SET_PROTOCOL_REQUEST            0x0B	/**< Set Protocol Request. */

#define HID_KEYBOARD_PROTOCOL           0x00	/**< Hid Keyboard Protocol. */
#define REPORT_DESC_LENGTH              0x3F	/**< Report Desc Length. */

#define REPORT_LENGTH                   8	/**< Report Length. */

#define AUDIO_INTF			0	/**< Audio Intf. */
#define AUDIO_INTF_OUT			1	/**< Audio Intf Out. */
#define AUDIO_INTF_IN			2	/**< Audio Intf In. */
#define DFU_INTF			3	/**< Dfu Intf. */
#define STORAGE_INTF			4	/**< Storage Intf. */
#define KEYBOARD_INTF			5	/**< Keyboard Intf. */

/** @brief D I V_ R O U N D_ U P(n, d). */
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
}
__attribute__((__packed__))USB_CBW;
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
	u32			framesize;
	u32			interval;
	u32			packetsize;
	u32			packetresidue;
	u32			residue;
	u32			index;
	u32			firstpkt;
	u8			*disk;
	u32			disksize;
	u32			bytesRecv;	/**< Bytes Recv. */
	TaskHandle_t		xRecordTask;	/**< X Record Task. */
	TaskHandle_t		xPlayTask;	/**< X Play Task. */
	xSemaphoreHandle	xSemaphoreRecord;	/**< X Semaphore Record. */
	xSemaphoreHandle	xSemaphorePlay;	/**< X Semaphore Play. */
};

/** @brief USB transfer data structure. */
struct transfer {
	u8	*Ptr;	/**< Ptr. */
	u32	Length;	/**< Length. */
};

struct storage_if {
	u8			*disk;
	u8			*diskptr;
	u32			disksize;
	u32			bytesleft;
	u8			phase;
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
	USB_CBW			cbw;
#pragma data_alignment = 64
	USB_CSW			csw;
#else
#pragma data_alignment = 32
	USB_CBW			cbw;
#pragma data_alignment = 32
	USB_CSW			csw;
#endif
#else
	USB_CBW			cbw ALIGNMENT_CACHELINE;
	USB_CSW			csw ALIGNMENT_CACHELINE;
#endif
	TaskHandle_t		xSCSITask;	/**< X S C S I Task. */
	xSemaphoreHandle	xSemaphore;	/**< Curr Trans. */
	struct transfer		currTrans;	/**< Current transfer pointer. */
};

/** @brief USB HID keyboard interface structure. */
struct keyboard_if {
	TaskHandle_t		xKeyboardTask;	/**< X Keyboard Task. */
	xSemaphoreHandle	xSemaphore;	/**< X Semaphore. */
};

struct composite_dev {
	TaskHandle_t		xMainTask;	/**< F_keyboard. */
	struct audio_if		f_audio;
	struct dfu_if		f_dfu;
	struct storage_if	f_storage;
	struct keyboard_if	f_keyboard;	/**< Keyboard interface instance. */
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

void prvKeyboardTask(void *pvParameters);
void prvSCSITask( void *pvParameters);
void prvPlayBackTask( void *pvParameters);
void prvRecordTask( void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_DFU_H */
