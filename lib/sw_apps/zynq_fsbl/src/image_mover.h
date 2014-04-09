/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file image_mover.h
*
* This file contains the interface for moving the image from FLASH to OCM

*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz	03/04/11	Initial release
* 2.00a jz	06/04/11	partition header expands to 12 words
* 5.00a kc	07/30/13	Added defines for image header information
* 8.00a kc	01/16/13	Added defines for partition owner attribute
* </pre>
*
* @note
*
******************************************************************************/
#ifndef ___IMAGE_MOVER_H___
#define ___IMAGE_MOVER_H___


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "fsbl.h"

/************************** Constant Definitions *****************************/
#define PARTITION_NUMBER_SHIFT	24
#define MAX_PARTITION_NUMBER	(0xE)

/* Boot Image Header defines */
#define IMAGE_HDR_OFFSET			0x098	/* Start of image header table */
#define IMAGE_PHDR_OFFSET			0x09C	/* Start of partition headers */
#define IMAGE_HEADER_SIZE			(64)
#define IMAGE_HEADER_TABLE_SIZE		(64)
#define TOTAL_PARTITION_HEADER_SIZE	(MAX_PARTITION_NUMBER * IMAGE_HEADER_SIZE)
#define TOTAL_IMAGE_HEADER_SIZE		(MAX_PARTITION_NUMBER * IMAGE_HEADER_SIZE)
#define TOTAL_HEADER_SIZE			(IMAGE_HEADER_TABLE_SIZE + \
									 TOTAL_IMAGE_HEADER_SIZE + \
									 TOTAL_PARTITION_HEADER_SIZE + 64)

/* Partition Header defines */
#define PARTITION_IMAGE_WORD_LEN_OFFSET	0x00	/* Word length of image */
#define PARTITION_DATA_WORD_LEN_OFFSET	0x04	/* Word length of data */
#define PARTITION_WORD_LEN_OFFSET		0x08	/* Word length of partition */
#define PARTITION_LOAD_ADDRESS_OFFSET	0x0C	/* Load addr in DDR	*/
#define PARTITION_EXEC_ADDRESS_OFFSET	0x10	/* Addr to start executing */
#define PARTITION_ADDR_OFFSET			0x14	/* Partition word offset */
#define PARTITION_ATTRIBUTE_OFFSET		0x18	/* Partition type */
#define PARTITION_HDR_CHECKSUM_OFFSET	0x3C	/* Header Checksum offset */
#define PARTITION_HDR_CHECKSUM_WORD_COUNT 0xF	/* Checksum word count */
#define PARTITION_HDR_WORD_COUNT		0x10	/* Header word len */
#define PARTITION_HDR_TOTAL_LEN			0x40	/* One partition hdr length*/

/* Attribute word defines */
#define ATTRIBUTE_IMAGE_TYPE_MASK		0xF0	/* Destination Device type */
#define ATTRIBUTE_PS_IMAGE_MASK			0x10	/* Code partition */
#define ATTRIBUTE_PL_IMAGE_MASK			0x20	/* Bit stream partition */
#define ATTRIBUTE_CHECKSUM_TYPE_MASK	0x7000	/* Checksum Type */
#define ATTRIBUTE_RSA_PRESENT_MASK		0x8000	/* RSA Signature Present */
#define ATTRIBUTE_PARTITION_OWNER_MASK	0x30000	/* Partition Owner */

#define ATTRIBUTE_PARTITION_OWNER_FSBL	0x00000	/* FSBL Partition Owner */


/**************************** Type Definitions *******************************/
typedef u32 (*ImageMoverType)( u32 SourceAddress,
				u32 DestinationAddress,
				u32 LengthBytes);

typedef struct StructPartHeader {
	u32 ImageWordLen;	/* 0x0 */
	u32 DataWordLen;	/* 0x4 */
	u32 PartitionWordLen;	/* 0x8 */
	u32 LoadAddr;		/* 0xC */
	u32 ExecAddr;		/* 0x10 */
	u32 PartitionStart;	/* 0x14 */
	u32 PartitionAttr;	/* 0x18 */
	u32 SectionCount;	/* 0x1C */
	u32 CheckSumOffset;	/* 0x20 */
	u32 Pads1[1];
	u32 ACOffset;	/* 0x28 */
	u32 Pads2[4];
	u32 CheckSum;		/* 0x3C */
}PartHeader;

struct HeaderArray {
	u32 Fields[16];
};


/***************** Macros (Inline Functions) Definitions *********************/
#define MoverIn32		Xil_In32
#define MoverOut32		Xil_Out32

/************************** Function Prototypes ******************************/
u32 LoadBootImage(void);
u32 GetPartitionHeaderInfo(u32 ImageBaseAddress);
u32 PartitionMove(u32 ImageBaseAddress, PartHeader *Header);
u32 ValidatePartitionHeaderChecksum(struct HeaderArray *H);
u32 GetPartitionHeaderStartAddr(u32 ImageAddress, u32 *Offset);
u32 GetImageHeaderAndSignature(u32 ImageAddress, u32 *Offset);
u32 GetFsblLength(u32 ImageAddress, u32 *FsblLength);
u32 LoadPartitionsHeaderInfo(u32 PartHeaderOffset,  PartHeader *Header);
u32 IsEmptyHeader(struct HeaderArray *H);
u32 IsLastPartition(struct HeaderArray *H);
void HeaderDump(PartHeader *Header);
u32 GetPartitionCount(PartHeader *Header);
u32 ValidateHeader(PartHeader *Header);
u32 DecryptPartition(u32 StartAddr, u32 DataLength, u32 ImageLength);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif


#endif /* ___IMAGE_MOVER_H___ */




