/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_class_storage.h
 *
 * This file contains definitions used in the chapter 9 code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a wgr  10/10/10 First release
 *       ms   04/10/17 Modified filename tag to include the file in doxygen
 *                     examples.
 * </pre>
 *
 ******************************************************************************/

#ifndef XUSBPS_CLASS_STORAGE_H
#define XUSBPS_CLASS_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xusbps_ch9_storage.h"

/************************** Constant Definitions *****************************/

/* Mass storage opcodes.
 */
#define USB_RBC_FORMAT			0x04
#define USB_RBC_INQUIRY			0x12
#define USB_RBC_MODE_SEL		0x15
#define USB_RBC_MODE_SENSE		0x1a
#define USB_RBC_READ			0x28
#define USB_RBC_READ_CAP		0x25
#define USB_RBC_VERIFY			0x2f
#define USB_RBC_WRITE			0x2a
#define USB_RBC_STARTSTOP_UNIT		0x1b
#define USB_RBC_TEST_UNIT_READY		0x00
#define USB_RBC_MEDIUM_REMOVAL		0x1e
#define USB_UFI_GET_CAP_LIST		0x23


/* Virtual Flash memory related definitions.
 */
#define VFLASH_SIZE		0x100000	/* 1MB space */
#define VFLASH_BLOCK_SIZE	0x200
#define VFLASH_NUM_BLOCKS	(VFLASH_SIZE/VFLASH_BLOCK_SIZE)


/* Class request opcodes.
 */
#define XUSBPS_CLASSREQ_MASS_STORAGE_RESET	0xFF
#define XUSBPS_CLASSREQ_GET_MAX_LUN		0xFE


/* SCSI machine states
 */
#define USB_EP_STATE_COMMAND		0
#define USB_EP_STATE_DATA		1
#define USB_EP_STATE_STATUS		2


/**************************** Type Definitions *******************************/

/* The following structures define USB storage class requests. The details of
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

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void XUsbPs_HandleStorageReq(XUsbPs *InstancePtr, u8 EpNum,
				u8 *BufferPtr, u32 BufferLen);
void XUsbPs_ClassReq(XUsbPs *InstancePtr, XUsbPs_SetupData *SetupData);

#ifdef __cplusplus
}
#endif

#endif /* XUSBPS_CLASS_STORAGE_H */
