/******************************************************************************
* Copyright (C) 2003 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_i.h
* @addtogroup hwicap_v11_3
* @{
*
* This head file contains internal identifiers, which are those shared
* between the files of the driver.  It is intended for internal use
* only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/14/03 First release
* 1.00b nps  02/09/05 V4 changes
* 1.01a tjb  10/14/05 V4 Updates
* 2.00a sv   10/14/07 V5 Updates
* 3.01a sv   10/19/09 Corrected the V5 BOOTSTS and CTL_1 Register definitions
*                     as they were wrongly defined
* 4.00a hvm  11/13/09 Updated with V6 changes
* 5.00a hvm  2/25/10  Added changes to support S6
* 6.00a hvm  08/01/11 Added support for K7
* 7.00a bss  03/14/12 Added Virtex 7, Artix 7 and Zynq Device families
* 8.01a bss  05/14/12 Added the define XHI_COR_1 for CR718042
* 9.0   bss  02/20/14 Added XHI_DEV_FAMILY_K8 for Kintex 8 devices. #CR764668
* 10.0  bss  6/24/14  Removed support for families older than 7 series.
*		      Removed IDCODE macros.
* 10.1  nsk  01/06/15 Removed defines XHI_FAR_MAJOR_FRAME_MASK
*                     XHI_FAR_MINOR_FRAME_MASK
*                     XHI_FAR_MAJOR_FRAME_SHIFT
*                     XHI_FAR_MINOR_FRAME_SHIFT
*                     XHI_C0R_1
*                     Updated XHI_FAR_COLUMN_ADDR_MASK to 0x3FF
*                     Updated XHI_FAR_BLOCK_SHIFT to 23
*                     Updated XHI_FAR_TOP_BOTTOM_SHIFT to 22
*                     Updated XHI_FAR_ROW_ADDR_SHIFT to 17
*                     Updated XHI_NUM_FRAME_BYTES to 404
*                     Updated XHI_NUM_FRAME_WORDS to 101
*                     Updated XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME to 202
*
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_I_H_ /* prevent circular inclusions */
#define XHWICAP_I_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

/************************** Constant Definitions ****************************/

/**
 * @name Configuration Type1/Type2 packet headers masks
 * @{
 */
#define XHI_TYPE_MASK			0x7
#define XHI_REGISTER_MASK		0x1F
#define XHI_OP_MASK			0x3

#define XHI_WORD_COUNT_MASK_TYPE_1	0x7FF
#define XHI_WORD_COUNT_MASK_TYPE_2	0x07FFFFFF

#define XHI_TYPE_SHIFT			29
#define XHI_REGISTER_SHIFT		13
#define XHI_OP_SHIFT			27

#define XHI_TYPE_1			1
#define XHI_TYPE_2			2
#define XHI_OP_WRITE			2
#define XHI_OP_READ			1

/* @} */

/*
 * Addresses of the Configuration Registers
 */
#define XHI_CRC				0
#define XHI_FAR				1
#define XHI_FDRI			2
#define XHI_FDRO			3
#define XHI_CMD				4
#define XHI_CTL				5
#define XHI_MASK			6
#define XHI_STAT			7
#define XHI_LOUT			8
#define XHI_COR				9
#define XHI_MFWR			10
#define XHI_CBC				11
#define XHI_IDCODE			12
#define XHI_AXSS			13
#define XHI_COR_1			14
#define XHI_CSOB			15
#define XHI_WBSTAR			16
#define XHI_TIMER			17
#define XHI_BOOTSTS			22
#define XHI_CTL_1			24
#define XHI_BSPI			31
#define XHI_NUM_REGISTERS		25 /* Note that the register numbering
                                            * is not sequential, there are gaps
                                            */

/**
 * @name Frame Address Register mask(s)
 * @{
 */
#define XHI_FAR_BLOCK_MASK		0x7
#define XHI_FAR_TOP_BOTTOM_MASK		0x1
#define XHI_FAR_ROW_ADDR_MASK		0x1F
#define XHI_FAR_COLUMN_ADDR_MASK	0x3FF
#define XHI_FAR_MINOR_ADDR_MASK		0x7F
#define XHI_FAR_BLOCK_SHIFT		23
#define XHI_FAR_TOP_BOTTOM_SHIFT	22
#define XHI_FAR_ROW_ADDR_SHIFT		17
#define XHI_FAR_COLUMN_ADDR_SHIFT	7
#define XHI_FAR_MINOR_ADDR_SHIFT	0

/*
 * Address Block Types in the  Frame Address Register
 */
#define XHI_FAR_CLB_BLOCK		0 /**< CLB/IO/CLK Block */
#define XHI_FAR_BRAM_BLOCK		1 /**< Block RAM interconnect */
#define XHI_FAR_CFG_CLB_BLOCK		2 /**< CFG CLB Block */

/* @} */

/*
 * Configuration Commands
 */
#define XHI_CMD_NULL			0
#define XHI_CMD_WCFG			1
#define XHI_CMD_MFW			2
#define XHI_CMD_DGHIGH			3
#define XHI_CMD_RCFG			4
#define XHI_CMD_START			5
#define XHI_CMD_RCAP			6
#define XHI_CMD_RCRC			7
#define XHI_CMD_AGHIGH			8
#define XHI_CMD_SWITCH			9
#define XHI_CMD_GRESTORE		10
#define XHI_CMD_SHUTDOWN		11
#define XHI_CMD_GCAPTURE		12
#define XHI_CMD_DESYNCH			13
#define XHI_CMD_IPROG			15
#define XHI_CMD_CRCC			16
#define XHI_CMD_LTIMER			17
#define XHI_TYPE_2_READ 		((XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
					(XHI_OP_READ << XHI_OP_SHIFT))

#define XHI_TYPE_2_WRITE 		( (XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
					(XHI_OP_WRITE << XHI_OP_SHIFT) )

/*
 * Packet constants
 */
#define XHI_SYNC_PACKET			0xAA995566
#define XHI_DUMMY_PACKET		0xFFFFFFFF
#define XHI_BUS_WTH_PACKET		0x000000BB
#define	XHI_BUS_DET_PACKET		0x11220044
#define XHI_SHUTDOWN_PACKET		0x0000000B
#define XHI_DEVICE_ID_READ		0x28018001
#define XHI_NOOP_PACKET			(XHI_TYPE_1 << XHI_TYPE_SHIFT)

#define XHI_TYPE_1_PACKET_MAX_WORDS 	2047
#define XHI_TYPE_1_HEADER_BYTES		4
#define XHI_TYPE_2_HEADER_BYTES		8

#define XHI_NUM_FRAME_BYTES		404  /* Number of bytes in a frame */
#define XHI_NUM_FRAME_WORDS		101  /* Number of Words in a frame */
#define XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME  202 /* Num of Words in a frame read
						 * from the device including
						 * the NULL frame
						 */
/* Device Resources */
#define CLB				0
#define DSP				1
#define BRAM				2
#define BRAM_INT			3
#define IOB				4
#define IOI				5
#define CLK				6
#define MGT				7

/*
 * The number of words reserved for the header
 */
#define XHI_HEADER_BUFFER_WORDS		20
#define XHI_HEADER_BUFFER_BYTES		(XHI_HEADER_BUFFER_WORDS << 2)

/*
 * CLB major frames start at 3 for the first column (since we are using
 * column numbers that start at 1, when the column is added to this offset,
 * that first one will be 3 as required.
 */
#define XHI_CLB_MAJOR_FRAME_OFFSET	2

/*
 * Constant to use for CRC check when CRC has been disabled
 */
#define XHI_DISABLED_AUTO_CRC		0x0000DEFC
#define XHI_DISABLED_AUTO_CRC_ONE	0x9876
#define XHI_DISABLED_AUTO_CRC_TWO	0xDEFC

/*
 * Major Row Offset
 */
#define XHI_CLB_MAJOR_ROW_OFFSET 	96+(32*XHI_HEADER_BUFFER_WORDS)-1

/*
 * Number of times to poll the Status Register
 */
#define XHI_MAX_RETRIES			1000

/*
 * Mask for the Device ID read from the ID code Register
 */
#define XHI_DEVICE_ID_CODE_MASK		0x0FFFFFFF


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Generates a Type 1 packet header that reads back the requested Configuration
* register.
*
* @param	Register is the address of the register to be read back.
*
* @return	Type 1 packet header to read the specified register
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_Type1Read(Register) \
	( (XHI_TYPE_1 << XHI_TYPE_SHIFT) | (Register << XHI_REGISTER_SHIFT) | \
	(XHI_OP_READ << XHI_OP_SHIFT) )

/****************************************************************************/
/**
*
* Generates a Type 2 packet header that reads back the requested Configuration
* register.
*
* @param	Register is the address of the register to be read back.
*
* @return	Type 1 packet header to read the specified register
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_Type2Read(Register) \
	( XHI_TYPE_2_READ | (Register << XHI_REGISTER_SHIFT))

/****************************************************************************/
/**
*
* Generates a Type 1 packet header that writes to the requested Configuration
* register.
*
* @param	Register is the address of the register to be written to.
*
* @return	Type 1 packet header to write the specified register
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_Type1Write(Register) \
	( (XHI_TYPE_1 << XHI_TYPE_SHIFT) | (Register << XHI_REGISTER_SHIFT) | \
	(XHI_OP_WRITE << XHI_OP_SHIFT) )

/****************************************************************************/
/**
*
* Generates a Type 2 packet header that writes to the requested Configuration
* register.
*
* @param	Register is the address of the register to be written to.
*
* @return	Type 1 packet header to write the specified register
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_Type2Write(Register) \
	( (XHI_TYPE_2 << XHI_TYPE_SHIFT) | (Register << XHI_REGISTER_SHIFT) | \
	(XHI_OP_WRITE << XHI_OP_SHIFT) )

/****************************************************************************/
/**
*
* Generates a Type 1 packet header that is written to the Frame Address
* Register (FAR).
*
* @param	Block - Address Block Type (CLB or BRAM address space)
* @param	Top - top (0) or bottom (1) half of device
* @param	Row - Row Address
* @param	ColumnAddress - CLB or BRAM column
* @param	MinorAddress - Frame within a column
*
* @return	Type 1 packet header to write the FAR
*
* @note         We are retaining this Macro for Backwards compatiblity
*               Use the XHwIcap_SetupFar macro.
*
*****************************************************************************/
#define XHwIcap_SetupFarV5(Top, Block, Row, ColumnAddress, MinorAddress)  \
	(Block << XHI_FAR_BLOCK_SHIFT) | \
	((Top << XHI_FAR_TOP_BOTTOM_SHIFT) | \
	(Row << XHI_FAR_ROW_ADDR_SHIFT) | \
	(ColumnAddress << XHI_FAR_COLUMN_ADDR_SHIFT) | \
	(MinorAddress << XHI_FAR_MINOR_ADDR_SHIFT))

#define XHwIcap_SetupFar XHwIcap_SetupFarV5

/************************** Function Prototypes ******************************/


/************************** Variable Definitions ****************************/

#ifdef __cplusplus
}
#endif

#endif

/** @} */
