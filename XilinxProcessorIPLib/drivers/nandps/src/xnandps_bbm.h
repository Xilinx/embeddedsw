/******************************************************************************
*
* Copyright (C) 2009 - 2018 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xnandps_bbm.h
* @addtogroup nandps_v2_4
* @{
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
* XNandPs_IsBlockBad and take the action based on the return value. Also user
* can update the bad block table using XNandPs_MarkBlockBad API.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* </pre>
*
******************************************************************************/
#ifndef BBM_H		/* prevent circular inclusions */
#define BBM_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xnandps.h"

/************************** Constant Definitions *****************************/
/*
 * Block definitions for RAM based Bad Block Table (BBT)
 */
#define XNANDPS_BLOCK_GOOD			0x0	/**< Block is good */
#define XNANDPS_BLOCK_BAD			0x1	/**< Block is bad */
#define XNANDPS_BLOCK_RESERVED			0x2	/**< Reserved block */
#define XNANDPS_BLOCK_FACTORY_BAD		0x3	/**< Factory marked bad
							  block */
/*
 * Block definitions for FLASH based Bad Block Table (BBT)
 */
#define XNANDPS_FLASH_BLOCK_GOOD		0x3	/**< Block is good */
#define XNANDPS_FLASH_BLOCK_BAD		0x2	/**< Block is bad */
#define XNANDPS_FLASH_BLOCK_RESERVED		0x1	/**< Reserved block */
#define XNANDPS_FLASH_BLOCK_FACTORY_BAD	0x0	/**< Factory marked bad
							  block */

#define XNANDPS_BBT_SCAN_2ND_PAGE		0x00000001	/**< Scan the
								  second page
								  for bad block
								  information
								  */
#define XNANDPS_BBT_DESC_PAGE_OFFSET		0 	/**< Page offset of Bad
							  Block Table Desc */
#define XNANDPS_BBT_DESC_SIG_OFFSET		8 	/**< Bad Block Table
							  signature offset */
#define XNANDPS_BBT_DESC_VER_OFFSET		12	/**< Bad block Table
							  version offset */
#define XNANDPS_BBT_DESC_SIG_LEN		4	/**< Bad block Table
							  signature length */
#define XNANDPS_BBT_DESC_MAX_BLOCKS		4	/**< Bad block Table
							  max blocks */

#define XNANDPS_BBT_BLOCK_SHIFT		2	/**< Block shift value
							  for a block in BBT */
#define XNANDPS_BBT_ENTRY_NUM_BLOCKS		4	/**< Num of blocks in
							  one BBT entry */
#define XNANDPS_BB_PATTERN_OFFSET_SMALL_PAGE	5	/**< Bad block pattern
							  offset in a page */
#define XNANDPS_BB_PATTERN_LENGTH_SMALL_PAGE	1	/**< Bad block pattern
							  length */
#define XNANDPS_BB_PATTERN_OFFSET_LARGE_PAGE	0	/**< Bad block pattern
							  offset in a large
							  page */
#define XNANDPS_BB_PATTERN_LENGTH_LARGE_PAGE	2	/**< Bad block pattern
							  length */
#define XNANDPS_BB_PATTERN			0xFF	/**< Bad block pattern
							  to search in a page
							  */
#define XNANDPS_BLOCK_TYPE_MASK		0x03	/**< Block type mask */
#define XNANDPS_BLOCK_SHIFT_MASK		0x06	/**< Block shift mask
							  for a Bad Block Table
							  entry byte */

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
#define XNandPs_BbtBlockShift(Block) \
		((Block * 2) & XNANDPS_BLOCK_SHIFT_MASK)

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

int XNandPs_IsBlockBad(XNandPs *InstancePtr, u32 Block);
int XNandPs_MarkBlockBad(XNandPs *InstancePtr, u32 Block);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
