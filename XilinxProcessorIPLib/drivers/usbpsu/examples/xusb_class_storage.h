/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
#define USB_RBC_TEST_UNIT_READY		0x00	/**< Usb Rbc Test Unit Ready. */
#define USB_RBC_REQUEST_SENSE		0x03	/**< Usb Rbc Request Sense. */
#define USB_RBC_FORMAT				0x04	/**< Usb Rbc Format. */
#define USB_RBC_INQUIRY				0x12	/**< Usb Rbc Inquiry. */
#define USB_RBC_MODE_SELECT			0x15	/**< Usb Rbc Mode Select. */
#define USB_RBC_MODE_SENSE			0x1a	/**< Usb Rbc Mode Sense. */
#define USB_RBC_STARTSTOP_UNIT		0x1b	/**< Usb Rbc Startstop Unit. */
#define USB_RBC_MEDIUM_REMOVAL		0x1e	/**< Usb Rbc Medium Removal. */
#define USB_UFI_GET_CAP_LIST		0x23	/**< Usb Ufi Get Cap List. */
#define USB_RBC_READ_CAP			0x25	/**< Usb Rbc Read Cap. */
#define USB_RBC_READ				0x28	/**< Usb Rbc Read. */
#define USB_RBC_WRITE				0x2a	/**< Usb Rbc Write. */
#define USB_RBC_VERIFY				0x2f	/**< Usb Rbc Verify. */
#define USB_SYNC_SCSI				0x35	/**< Usb Sync Scsi. */

/* Virtual Flash memory related definitions.
 */
#ifdef __MICROBLAZE__
/* 16MB due to limited memory on AXIUSB platform. */
#define VFLASH_SIZE			0x1000000	/* 16MB space */
#else
#define VFLASH_SIZE			0x10000000	/**< Vflash Size. */
#endif
#define VFLASH_BLOCK_SIZE	0x200	/**< Vflash Block Size. */
/** @brief Vflash Num Blocks. */
#define VFLASH_NUM_BLOCKS	(VFLASH_SIZE/VFLASH_BLOCK_SIZE)

/* Class request opcodes.
 */
#define USB_CLASSREQ_MASS_STORAGE_RESET	0xFF	/**< Usb Classreq Mass Storage Reset. */
#define USB_CLASSREQ_GET_MAX_LUN		0xFE	/**< Usb Classreq Get Max Lun. */

/* SCSI machine states
 */
#define USB_EP_STATE_COMMAND		0	/**< Usb Ep State Command. */
#define USB_EP_STATE_DATA_IN		1	/**< Usb Ep State Data In. */
#define USB_EP_STATE_DATA_OUT		2	/**< Usb Ep State Data Out. */
#define USB_EP_STATE_STATUS			3	/**< Usb Ep State Status. */

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
	u32 dCBWSignature;	/**< D C B W Signature. */
	u32 dCBWTag;	/**< D C B W Tag. */
	u32 dCBWDataTransferLength;	/**< D C B W Data Transfer Length. */
	u8  bmCBWFlags;	/**< Bm C B W Flags. */
	u8  cCBWLUN;	/**< C C B W L U N. */
	u8  bCBWCBLength;	/**< B C B W C B Length. */
	u8  CBWCB[16];	/**< Cbwcb. */
} attribute(USB_CBW);

typedef struct {
	u32 dCSWSignature;	/**< D C S W Signature. */
	u32 dCSWTag;	/**< D C S W Tag. */
	u32 dCSWDataResidue;	/**< D C S W Data Residue. */
	u8  bCSWStatus;	/**< B C S W Status. */
} attribute(USB_CSW);

typedef	struct {
	u8 deviceType;	/**< Device Type. */
	u8 rmb;	/**< Rmb. */
	u8 version;	/**< Version. */
	u8 blah;	/**< Blah. */
	u8 additionalLength;	/**< Additional Length. */
	u8 sccs;	/**< Sccs. */
	u8 info0;	/**< Info0. */
	u8 info1;	/**< Info1. */
	u8 vendorID[8];	/**< Vendor I D. */
	u8 productID[16];	/**< Product I D. */
	u8 revision[4];	/**< Revision. */
} attribute(SCSI_INQUIRY);

typedef struct {
	u8  reserved[3];	/**< Reserved. */
	u8  listLength;	/**< List Length. */
	u32 numBlocks;	/**< Num Blocks. */
	u8  descCode;	/**< Desc Code. */
	u8  blockLengthMSB;	/**< Block Length M S B. */
	u16 blockLength;	/**< Block Length. */
} attribute(SCSI_CAP_LIST);

typedef struct {
	u32 numBlocks;
	u32 blockSize;	/**< Block Size. */
} attribute(SCSI_READ_CAPACITY);

typedef struct {
	u8  opCode;	/**< Op Code. */
	u8  reserved1;	/**< Reserved1. */
	u32 block;	/**< Block. */
	u8  reserved2;	/**< Reserved2. */
	u16 length;	/**< Length. */
	u8  control;	/**< Control. */
} attribute(SCSI_READ_WRITE);

typedef struct {
	u8  opCode;
	u8  immed;	/**< Immed. */
	u8  reserved1;
	u8  reserved2;
	u8  start;	/**< Start. */
	u8  control;
} attribute(SCSI_START_STOP);

#ifdef __ICCARM__
#pragma pack(pop)
#endif

/************************** Function Prototypes ******************************/
void ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void ParseCBW(struct Usb_DevData *InstancePtr);	/**< Parse Command Block Wrapper. */
void SendCSW(struct Usb_DevData *InstancePtr, u32 Length);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CLASS_STORAGE_H */
