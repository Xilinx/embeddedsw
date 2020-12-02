/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusb_class_storage.h
 *
 * This file contains definitions used in the Mass Storage class code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   sg  06/06/16  First release
 * 1.4   BK  12/01/18 Renamed the file and added changes to have a common
 *		      example for all USB IPs.
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

/************************** Constant Definitions *****************************/
/*
 * Mass storage opcodes.
 */
#define USB_RBC_TEST_UNIT_READY		0x00
#define USB_RBC_REQUEST_SENSE		0x03
#define USB_RBC_FORMAT				0x04
#define USB_RBC_INQUIRY				0x12
#define USB_RBC_MODE_SELECT			0x15
#define USB_RBC_MODE_SENSE			0x1a
#define USB_RBC_STARTSTOP_UNIT		0x1b
#define USB_RBC_MEDIUM_REMOVAL		0x1e
#define USB_UFI_GET_CAP_LIST		0x23
#define USB_RBC_READ_CAP			0x25
#define USB_RBC_READ				0x28
#define USB_RBC_WRITE				0x2a
#define USB_RBC_VERIFY				0x2f
#define USB_SYNC_SCSI				0x35

/* Virtual Flash memory related definitions.
 */
#ifdef __MICROBLAZE__
/* 16MB due to limited memory on AXIUSB platform. */
#define VFLASH_SIZE			0x1000000	/* 16MB space */
#else
#define VFLASH_SIZE			0x10000000	/* 256MB space */
#endif
#define VFLASH_BLOCK_SIZE	0x200
#define VFLASH_NUM_BLOCKS	(VFLASH_SIZE/VFLASH_BLOCK_SIZE)

/* Class request opcodes.
 */
#define USB_CLASSREQ_MASS_STORAGE_RESET	0xFF
#define USB_CLASSREQ_GET_MAX_LUN		0xFE

/* SCSI machine states
 */
#define USB_EP_STATE_COMMAND		0
#define USB_EP_STATE_DATA_IN		1
#define USB_EP_STATE_DATA_OUT		2
#define USB_EP_STATE_STATUS			3

/**************************** Type Definitions ******************************/

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif

/*
 * The following structures define USB storage class requests. The details of
 * the contents of those structures are not important in the context of this
 * example.
 */
typedef struct {
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8  bmCBWFlags;
	u8  cCBWLUN;
	u8  bCBWCBLength;
	u8  CBWCB[16];
} attribute(USB_CBW);

typedef struct {
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8  bCSWStatus;
} attribute(USB_CSW);

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
} attribute(SCSI_INQUIRY);

typedef struct {
	u8  reserved[3];
	u8  listLength;
	u32 numBlocks;
	u8  descCode;
	u8  blockLengthMSB;
	u16 blockLength;
} attribute(SCSI_CAP_LIST);

typedef struct {
	u32 numBlocks;
	u32 blockSize;
} attribute(SCSI_READ_CAPACITY);

typedef struct {
	u8  opCode;
	u8  reserved1;
	u32 block;
	u8  reserved2;
	u16 length;
	u8  control;
} attribute(SCSI_READ_WRITE);

typedef struct {
	u8  opCode;
	u8  immed;
	u8  reserved1;
	u8  reserved2;
	u8  start;
	u8  control;
} attribute(SCSI_START_STOP);

#ifdef __ICCARM__
#pragma pack(pop)
#endif

/************************** Function Prototypes ******************************/
void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void ParseCBW(struct Usb_DevData *InstancePtr);
void SendCSW(struct Usb_DevData *InstancePtr, u32 Length);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_STORAGE_H */
