/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits,
* goodwill, or any type of  loss or damage suffered as a result of any
* action brought  by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the  possibility
* of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-  safe, or for use
* in any application requiring fail-safe  performance, such as life-support
* or safety devices or  systems, Class III medical devices, nuclear
* facilities,  applications related to the deployment of airbags, or any
* other applications that could lead to death, personal  injury, or severe
* property or environmental damage  (individually and collectively,
* "Critical  Applications"). Customer assumes the sole risk and  liability
* of any use of Xilinx products in Critical  Applications, subject only to
* applicable laws and  regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
* OF THIS FILE AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps8_bbm.h
*
* This file implements the Bad Block Management(BBM) functionality. This is
* similar to the Bad Block Management which is a part of the MTD subsystem in
* Linux.  The factory marked bad blocks are scanned initially and a Bad Block
* Table(BBT) is created in the memory.  This table is also written to the flash
* so that upon reboot, the BBT is read back from the flash and loaded into the
* memory instead of scanning every time. The Bad Block Table(BBT) is written
* into one of the the last four blocks in the flash memory. The last four
* blocks are marked as Reserved so that user can't erase/program those blocks.
*
* There are two bad block tables, a primary table and a mirror table. The
* tables are versioned and incrementing version number is used to detect and
* recover from interrupted updates. Each table is stored in a separate block,
* beginning in the first page of that block. Only two blocks would be necessary
* in the absence of bad blocks within the last four; the range of four provides
* a little slack in case one or two of those blocks is bad. These blocks are
* marked as reserved and cannot be programmed by the user. A NAND Flash device
* with 3 or more factory bad blocks in the last 4 cannot be used. The bad block
* table signature is written into the spare data area of the pages containing
* bad block table so that upon rebooting the bad block table signature is
* searched and the bad block table is loaded into RAM. The signature is "Bbt0"
* for primary Bad Block Table and "1tbB" for Mirror Bad Block Table. The
* version offset follows the signature offset in the spare data area. The
* version number increments on every update to the bad block table and the
* version wraps at 0xff.
*
* Each block in the Bad Block Table(BBT) is represented by 2 bits.
* The two bits are encoded as follows in RAM BBT.
* 0'b00 -> Good Block
* 0'b01 -> Block is bad due to wear
* 0'b10 -> Reserved block
* 0'b11 -> Factory marked bad block
*
* While writing to the flash the two bits are encoded as follows.
* 0'b00 -> Factory marked bad block
* 0'b01 -> Reserved block
* 0'b10 -> Block is bad due to wear
* 0'b11 -> Good Block
*
* The user can check for the validity of the block using the API
* XNandPs8_IsBlockBad and take the action based on the return value. Also user
* can update the bad block table using XNandPs8_MarkBlockBad API.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date        Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 2.0   sb     11/04/2014  Added support for writing BBT signature and version
*			   in page section by enabling XNANDPS8_BBT_NO_OOB.
* </pre>
*
******************************************************************************/
#ifndef XNANDPS8_BBM_H		/* prevent circular inclusions */
#define XNANDPS8_BBM_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnandps8.h"

/************************** Constant Definitions *****************************/
/*
 * Block definitions for RAM based Bad Block Table (BBT)
 */
#define XNANDPS8_BLOCK_GOOD			0x0U	/**< Block is good */
#define XNANDPS8_BLOCK_BAD			0x1U	/**< Block is bad */
#define XNANDPS8_BLOCK_RESERVED			0x2U	/**< Reserved block */
#define XNANDPS8_BLOCK_FACTORY_BAD		0x3U	/**< Factory marked bad
							  block */
/*
 * Block definitions for FLASH based Bad Block Table (BBT)
 */
#define XNANDPS8_FLASH_BLOCK_GOOD		0x3U	/**< Block is good */
#define XNANDPS8_FLASH_BLOCK_BAD		0x2U	/**< Block is bad */
#define XNANDPS8_FLASH_BLOCK_RESERVED		0x1U	/**< Reserved block */
#define XNANDPS8_FLASH_BLOCK_FACTORY_BAD	0x0U	/**< Factory marked bad
							  block */

#define XNANDPS8_BBT_SCAN_2ND_PAGE		0x00000001U	/**< Scan the
								  second page
								  for bad block
								  information
								  */
#define XNANDPS8_BBT_DESC_PAGE_OFFSET		0U	/**< Page offset of Bad
							  Block Table Desc */
#define XNANDPS8_BBT_DESC_SIG_OFFSET		2U	/**< Bad Block Table
							  signature offset */
#define XNANDPS8_BBT_DESC_VER_OFFSET		6U	/**< Bad block Table
							  version offset */
#define XNANDPS8_BBT_DESC_SIG_LEN		4U	/**< Bad block Table
							  signature length */
#define XNANDPS8_BBT_DESC_MAX_BLOCKS		64U	/**< Bad block Table
							  max blocks */

#define XNANDPS8_BBT_BLOCK_SHIFT		2U	/**< Block shift value
							  for a block in BBT */
#define XNANDPS8_BBT_ENTRY_NUM_BLOCKS		4U	/**< Num of blocks in
							  one BBT entry */
#define XNANDPS8_BB_PATTERN_OFFSET_SMALL_PAGE	5U	/**< Bad block pattern
							  offset in a page */
#define XNANDPS8_BB_PATTERN_LENGTH_SMALL_PAGE	1U	/**< Bad block pattern
							  length */
#define XNANDPS8_BB_PATTERN_OFFSET_LARGE_PAGE	0U	/**< Bad block pattern
							  offset in a large
							  page */
#define XNANDPS8_BB_PATTERN_LENGTH_LARGE_PAGE	2U	/**< Bad block pattern
							  length */
#define XNANDPS8_BB_PATTERN			0xFFU	/**< Bad block pattern
							  to search in a page
							  */
#define XNANDPS8_BLOCK_TYPE_MASK		0x03U	/**< Block type mask */
#define XNANDPS8_BLOCK_SHIFT_MASK		0x06U	/**< Block shift mask
							  for a Bad Block Table
							  entry byte */

#define XNANDPS8_ONDIE_SIG_OFFSET		0x4U
#define XNANDPS8_ONDIE_VER_OFFSET		0x14U

#define XNANDPS8_BBT_VERSION_LENGTH	1U
#define XNANDPS8_BBT_SIG_LENGTH		4U

#define XNANDPS8_BBT_BUF_LENGTH		((XNANDPS8_MAX_BLOCKS >> 		\
					 XNANDPS8_BBT_BLOCK_SHIFT) +	\
					(XNANDPS8_BBT_DESC_SIG_OFFSET +	\
					 XNANDPS8_BBT_SIG_LENGTH +	\
					 XNANDPS8_BBT_VERSION_LENGTH))
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* This macro returns the Block shift value corresponding to a Block.
*
* @param        Block is the block number.
*
* @return       Block shift value
*
* @note         None.
*
*****************************************************************************/
#define XNandPs8_BbtBlockShift(Block) \
			((u8)(((Block) * 2U) & XNANDPS8_BLOCK_SHIFT_MASK))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

void XNandPs8_InitBbtDesc(XNandPs8 *InstancePtr);

s32 XNandPs8_ScanBbt(XNandPs8 *InstancePtr);

s32 XNandPs8_IsBlockBad(XNandPs8 *InstancePtr, u32 Block);

s32 XNandPs8_MarkBlockBad(XNandPs8 *InstancePtr, u32 Block);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
