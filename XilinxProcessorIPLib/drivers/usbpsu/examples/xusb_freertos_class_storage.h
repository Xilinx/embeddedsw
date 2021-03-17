/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_storage_class_storage.h
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
#ifdef AXI_USB
/* 16MB due to limited memory on AXIUSB platform. */
#define STORAGE_SIZE			0x1000000	/* 16MB space */
#else
#define STORAGE_SIZE			0x10000000	/* 256MB space */
#endif
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

#define STORAGE_EP			1

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
} __attribute__((__packed__))USB_CBW;
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

struct storage_dev {
	u8			*disk;
	u8			*diskptr;
	u32			disksize;
	u32			bytesleft;
	u8			phase;
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
	USB_CBW cbw		ALIGNMENT_CACHELINE;
	USB_CSW csw		ALIGNMENT_CACHELINE;
#endif
	TaskHandle_t		xSCSITask;
	xSemaphoreHandle	xSemaphore;
	struct transfer		currTrans;
};

extern TaskHandle_t xMainTask;

/************************** Function Prototypes ******************************/
void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void ParseCBW(struct Usb_DevData *InstancePtr, struct storage_dev *dev);
void SendCSW(struct Usb_DevData *InstancePtr, struct storage_dev *dev, u32 Length);
void prvSCSITask( void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_STORAGE_H */
