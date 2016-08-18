/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xusbpsu_class_storage.h
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16  First release
*
* </pre>
*
*****************************************************************************/
#ifndef XUSBPSU_CLASS_STORAGE_H
#define XUSBPSU_CLASS_STORAGE_H


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xusbpsu.h"
#include "xusbpsu_ch9.h"


/************************** Constant Definitions *****************************/
//#define XUSBPSU_SUPER_SPEED

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


/*
 * Virtual Flash memory related definitions.
 */
#define VFLASH_SIZE			0x10000000	/* 256MB space */
#define VFLASH_BLOCK_SIZE	0x200
#define VFLASH_NUM_BLOCKS	(VFLASH_SIZE/VFLASH_BLOCK_SIZE)


/*
 * Class request opcodes.
 */
#define XUSBPSU_CLASSREQ_MASS_STORAGE_RESET	0xFF
#define XUSBPSU_CLASSREQ_GET_MAX_LUN		0xFE


/*
 * SCSI machine states
 */
#define USB_EP_STATE_COMMAND		0
#define USB_EP_STATE_DATA			1
#define USB_EP_STATE_STATUS			2

/**************************** Type Definitions ******************************/

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
} __attribute__((__packed__))USB_CBW;

typedef struct {
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8  bCSWStatus;
} __attribute__((__packed__))USB_CSW;

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
} __attribute__((__packed__))SCSI_INQUIRY;

typedef struct {
	u8  reserved[3];
	u8  listLength;
	u32 numBlocks;
	u8  descCode;
	u8  blockLengthMSB;
	u16 blockLength;
} __attribute__((__packed__))SCSI_CAP_LIST;

typedef struct {
	u32 numBlocks;
	u32 blockSize;
} __attribute__((__packed__))SCSI_READ_CAPACITY;

typedef struct {
	u8  opCode;
	u8  reserved1;
	u32 block;
	u8  reserved2;
	u16 length;
	u8  control;
} __attribute__((__packed__))SCSI_READ_WRITE;

typedef struct {
	u8  opCode;
	u8  immed;
	u8  reserved1;
	u8  reserved2;
	u8  start;
	u8  control;
} __attribute__((__packed__))SCSI_START_STOP;

#ifdef __cplusplus
}
#endif

#endif /* XUSBPSU_CLASS_STORAGE_H */
