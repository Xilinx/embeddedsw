/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_freertos_class_storage.h
 *
 * This file contains definitions used in the Mass Storage class code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   rb   22/03/18 First release
 * 1.5   vak  03/25/19 Fixed incorrect data_alignment pragma directive for IAR
 *
 * </pre>
 *
 *****************************************************************************/

#ifndef XUSB_CLASS_STORAGE_H
#define XUSB_CLASS_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xusb_ch9.h"
#include "task.h"
#include "semphr.h"

/************************** Constant Definitions *****************************/

/* Mass storage opcodes */
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
#ifdef AXI_USB
/* 16MB due to limited memory on AXIUSB platform. */
#define STORAGE_SIZE			0x1000000	/* 16MB space */
#else
#define STORAGE_SIZE			0x10000000	/**< Storage Size. */
#endif
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

#define STORAGE_EP			1	/**< Storage Ep. */

/**************************** Type Definitions ******************************/
/*
 * The following structures define USB storage class requests. The details of
 * the contents of those structures are not important in the context of this
 * example.
 */
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

typedef struct {
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8  bCSWStatus;
} __attribute__((__packed__))USB_CSW;

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

struct transfer {
	u8	*Ptr;
	u32	Length;
};

/** @brief USB mass storage device configuration structure. */
struct storage_dev {
	u8			*disk;	/**< Disk. */
	u8			*diskptr;	/**< Diskptr. */
	u32			disksize;	/**< Disksize. */
	u32			bytesleft;	/**< Bytesleft. */
	u8			phase;	/**< Phase. */
#ifdef __ICCARM__
#if defined (PLATFORM_ZYNQMP) || defined (versal)
#pragma data_alignment = 64
	USB_CBW cbw;
#pragma data_alignment = 64
	USB_CSW csw;
#else
#pragma data_alignment = 32
	USB_CBW cbw;
#pragma data_alignment = 32
	USB_CSW csw;
#endif
#else
	USB_CBW cbw		ALIGNMENT_CACHELINE;	/**< Alignment Cacheline. */
	USB_CSW csw		ALIGNMENT_CACHELINE;	/**< Alignment Cacheline. */
#endif
	TaskHandle_t		xSCSITask;	/**< X S C S I Task. */
	xSemaphoreHandle	xSemaphore;	/**< SCSI task semaphore. */
	struct transfer		currTrans;	/**< Current transfer state. */
};

extern TaskHandle_t xMainTask;

/************************** Function Prototypes ******************************/
void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void ParseCBW(struct Usb_DevData *InstancePtr, struct storage_dev *dev);	/**< Parse Command Block Wrapper. */
void SendCSW(struct Usb_DevData *InstancePtr, struct storage_dev *dev, u32 Length);
void prvSCSITask( void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_STORAGE_H */
