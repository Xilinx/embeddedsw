/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xusbps_class_storage.h
 *
 * This file contains definitions used in the chapter 9 code.
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

/**
 * @name Mass storage RBC/SCSI opcodes.
 * @{
 */
#define USB_RBC_FORMAT			0x04 /**< FORMAT UNIT command opcode. */
#define USB_RBC_INQUIRY			0x12 /**< INQUIRY command opcode. */
#define USB_RBC_MODE_SEL		0x15 /**< MODE SELECT command opcode. */
#define USB_RBC_MODE_SENSE		0x1a /**< MODE SENSE command opcode. */
#define USB_RBC_READ			0x28 /**< READ(10) command opcode. */
#define USB_RBC_READ_CAP		0x25 /**< READ CAPACITY command opcode. */
#define USB_RBC_VERIFY			0x2f /**< VERIFY command opcode. */
#define USB_RBC_WRITE			0x2a /**< WRITE(10) command opcode. */
#define USB_RBC_STARTSTOP_UNIT		0x1b /**< START STOP UNIT command opcode. */
#define USB_RBC_TEST_UNIT_READY		0x00 /**< TEST UNIT READY command opcode. */
#define USB_RBC_MEDIUM_REMOVAL		0x1e /**< PREVENT/ALLOW MEDIUM REMOVAL opcode. */
#define USB_UFI_GET_CAP_LIST		0x23 /**< UFI READ FORMAT CAPACITIES opcode. */
/** @} */


/**
 * @name Virtual Flash memory parameters.
 * @{
 */
#define VFLASH_SIZE		0x100000	/**< Virtual flash size: 1 MB. */
#define VFLASH_BLOCK_SIZE	0x200		/**< Block size in bytes (512 bytes). */
#define VFLASH_NUM_BLOCKS	(VFLASH_SIZE/VFLASH_BLOCK_SIZE) /**< Total number of blocks. */
/** @} */


/**
 * @name Mass storage class request opcodes.
 * @{
 */
#define XUSBPS_CLASSREQ_MASS_STORAGE_RESET	0xFF /**< Bulk-Only Mass Storage Reset request. */
#define XUSBPS_CLASSREQ_GET_MAX_LUN		0xFE /**< Get Maximum LUN request. */
/** @} */


/**
 * @name SCSI state machine states.
 * @{
 */
#define USB_EP_STATE_COMMAND		0 /**< Awaiting SCSI command block. */
#define USB_EP_STATE_DATA		1 /**< Data transfer phase in progress. */
#define USB_EP_STATE_STATUS		2 /**< Sending command status wrapper. */
/** @} */


/**************************** Type Definitions *******************************/

/* The following structures define USB storage class requests. The details of
 * the contents of those structures are not important in the context of this
 * example.
 */
#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
/**
 * @brief USB Command Block Wrapper (CBW) for Bulk-Only Transport.
 */
typedef struct {
	u32 dCBWSignature;            /**< CBW signature (0x43425355). */
	u32 dCBWTag;                  /**< Command tag for matching with CSW. */
	u32 dCBWDataTransferLength;   /**< Number of bytes of data to transfer. */
	u8  bmCBWFlags;               /**< Direction and other flags. */
	u8  cCBWLUN;                  /**< Logical Unit Number. */
	u8  bCBWCBLength;             /**< Length of the command block in CBWCB. */
	u8  CBWCB[16];                /**< Command block for the device to execute. */
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
/**
 * @brief SCSI INQUIRY response data structure.
 */
typedef	struct {
	u8 deviceType;       /**< Peripheral device type. */
	u8 rmb;              /**< Removable media bit. */
	u8 version;          /**< Version of the SCSI standard. */
	u8 blah;             /**< Response data format byte. */
	u8 additionalLength; /**< Additional data length. */
	u8 sccs;             /**< SCCS and reserved bits. */
	u8 info0;            /**< Additional information byte 0. */
	u8 info1;            /**< Additional information byte 1. */
	u8 vendorID[8];      /**< Vendor identification string. */
	u8 productID[16];    /**< Product identification string. */
	u8 revision[4];      /**< Product revision level string. */
#ifdef __ICCARM__
} SCSI_INQUIRY;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_INQUIRY;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
/**
 * @brief SCSI READ FORMAT CAPACITIES response (capacity list).
 */
typedef struct {
	u8  reserved[3];     /**< Reserved bytes. */
	u8  listLength;      /**< Capacity list data length. */
	u32 numBlocks;       /**< Number of addressable blocks. */
	u8  descCode;        /**< Descriptor code (formatted/unformatted). */
	u8  blockLengthMSB;  /**< Block length MSB (bits 23:16). */
	u16 blockLength;     /**< Block length (bits 15:0). */
#ifdef __ICCARM__
} SCSI_CAP_LIST;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_CAP_LIST;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
/**
 * @brief SCSI READ CAPACITY(10) response data.
 */
typedef struct {
	u32 numBlocks;  /**< Logical block address of the last logical block. */
	u32 blockSize;  /**< Block length in bytes. */
#ifdef __ICCARM__
} SCSI_READ_CAPACITY;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_READ_CAPACITY;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
/**
 * @brief SCSI READ(10)/WRITE(10) command block structure.
 */
typedef struct {
	u8  opCode;    /**< SCSI operation code. */
	u8  reserved1; /**< Reserved byte. */
	u32 block;     /**< Logical block address. */
	u8  reserved2; /**< Reserved byte. */
	u16 length;    /**< Transfer length in blocks. */
	u8  control;   /**< Control byte. */
#ifdef __ICCARM__
} SCSI_READ_WRITE;
#pragma pack(pop)
#else
} __attribute__((__packed__))SCSI_READ_WRITE;
#endif

#ifdef __ICCARM__
#pragma pack(push, 1)
#endif
/**
 * @brief SCSI START STOP UNIT command block structure.
 */
typedef struct {
	u8  opCode;    /**< SCSI operation code (START STOP UNIT = 0x1B). */
	u8  immed;     /**< Immediate bit: return status before completion. */
	u8  reserved1; /**< Reserved byte. */
	u8  reserved2; /**< Reserved byte. */
	u8  start;     /**< Start/stop and load/eject bits. */
	u8  control;   /**< Control byte. */
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
