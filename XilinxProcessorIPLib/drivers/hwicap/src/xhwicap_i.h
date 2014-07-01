/******************************************************************************
*
* Copyright (C) 2003 - 2014 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhwicap_i.h
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

#include "xhwicap_family.h"

/************************** Constant Definitions ****************************/

#define XHI_DEV_FAMILY_V4		2	/* Virtex4 */
#define XHI_DEV_FAMILY_V5		3	/* Virtex5 */
#define XHI_DEV_FAMILY_V6		4	/* Virtex6 */
#define XHI_DEV_FAMILY_S6		5	/* Spartan6 */
#define XHI_DEV_FAMILY_K7		7	/* Kintex7 */
#define XHI_DEV_FAMILY_V7		8	/* Virtex7 */
#define XHI_DEV_FAMILY_A7		9	/* Artix7 */
#define XHI_DEV_FAMILY_ZYNQ		10	/* Zynq */
#define XHI_DEV_FAMILY_K8		11	/* Zynq */


#if ((XHI_FPGA_FAMILY == XHI_DEV_FAMILY_K7) ||\
(XHI_FPGA_FAMILY == XHI_DEV_FAMILY_A7) ||\
(XHI_FPGA_FAMILY == XHI_DEV_FAMILY_V7) ||\
(XHI_FPGA_FAMILY == XHI_DEV_FAMILY_K8) ||\
(XHI_FPGA_FAMILY == XHI_DEV_FAMILY_ZYNQ))

#define XHI_DEV_FAMILY_7SERIES	6	/* 7 SERIES */

#define XHI_FAMILY XHI_DEV_FAMILY_7SERIES

#else

#define XHI_FAMILY XHI_FPGA_FAMILY


#endif



/**
 * @name Configuration Type1/Type2 packet headers masks
 * @{
 */
#define XHI_TYPE_MASK			0x7
#define XHI_REGISTER_MASK		0x1F
#define XHI_OP_MASK				0x3

#if XHI_FAMILY == XHI_DEV_FAMILY_S6

#define XHI_WORD_COUNT_MASK_TYPE_1	0x1F

#define XHI_TYPE_SHIFT			13
#define XHI_REGISTER_SHIFT		5
#define XHI_OP_SHIFT			11

#else

#define XHI_WORD_COUNT_MASK_TYPE_1	0x7FF
#define XHI_WORD_COUNT_MASK_TYPE_2	0x07FFFFFF

#define XHI_TYPE_SHIFT			29
#define XHI_REGISTER_SHIFT		13
#define XHI_OP_SHIFT			27

#endif

#define XHI_TYPE_1			1
#define XHI_TYPE_2			2
#define XHI_OP_WRITE			2
#define XHI_OP_READ			1

/* @} */

#if XHI_FAMILY == XHI_DEV_FAMILY_S6
/*
 * Addresses of the Configuration Registers
 */

#define XHI_CRC				0
#define XHI_FAR_MAJ			1
#define XHI_FAR_MIN			2
#define XHI_FDRI			3
#define XHI_FDRO			4
#define XHI_CMD				5
#define XHI_CTL				6
#define XHI_MASK			7
#define XHI_STAT			8
#define XHI_LOUT			9
#define XHI_COR1			10
#define XHI_COR2			11
#define XHI_PWRDN_REG			12
#define XHI_FLR				13
#define XHI_IDCODE			14
#define XHI_CWDT			15
#define XHI_HC_OPT_REG			16

#define XHI_CSBO			18
#define XHI_GENERAL1			19
#define XHI_GENERAL2			20
#define XHI_GENERAL3			21
#define XHI_GENERAL4			22
#define XHI_GENERAL5			23
#define XHI_MODE_REG			24
#define XHI_PU_GWE			25
#define XHI_PU_GTS			26
#define XHI_MFWR			27
#define XHI_CCLK_FREQ			28
#define XHI_SEU_OPT			29
#define XHI_EXP_SIGN			30
#define XHI_RDBK_SIGN			31
#define XHI_BOOTSTS			32
#define XHI_EYE_MASK			33
#define CBC_REG				34

#define XHI_NUM_REGISTERS		35 /* Note that there is skip at
					    * number 17. There is no register
					    * at with this number */

#else
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
#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */
#define XHI_NUM_REGISTERS           14
#elif ((XHI_FAMILY == XHI_DEV_FAMILY_V5) || (XHI_FAMILY == XHI_DEV_FAMILY_V6) || \
	(XHI_FAMILY == XHI_DEV_FAMILY_7SERIES))
	/* Virtex5 or Virtex6 or Kintex 7*/
#define XHI_C0R_1			14
#define XHI_CSOB			15
#define XHI_WBSTAR			16
#define XHI_TIMER			17
#define XHI_BOOTSTS			22
#define XHI_CTL_1			24
#define XHI_NUM_REGISTERS		25 /* Note that the register numbering
					    * is not sequential in V5/V6 and
					    *there are gaps */
#define XHI_COR_1			14
#endif
#endif
/**
 * @name Frame Address Register mask(s)
 * @{
 */
#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */
#define XHI_FAR_BOTTOM_MASK		0x1
#define XHI_FAR_BLOCK_MASK		0x7
#define XHI_FAR_MAJOR_FRAME_MASK	0xFF
#define XHI_FAR_HCLKROW_MASK		0x1F
#define XHI_FAR_MINOR_FRAME_MASK	0xFF
#define XHI_FAR_COLUMN_MASK		0xFF
#define XHI_FAR_MINOR_MASK		0x3F
#define XHI_FAR_BOTTOM_SHIFT		22
#define XHI_FAR_BLOCK_SHIFT		19
#define XHI_FAR_MAJOR_FRAME_SHIFT	17
#define XHI_FAR_HCLKROW_SHIFT		14
#define XHI_FAR_MINOR_FRAME_SHIFT	9
#define XHI_FAR_COLUMN_SHIFT		6
#define XHI_FAR_MINOR_SHIFT		0

/*
 * Address Block Types in the  Frame Address Register
 */
#define XHI_FAR_CLB_BLOCK		0 /**< CLB/IO/CLK Block */
#define XHI_FAR_BRAM_BLOCK		1 /**< Block RAM interconnect */
#define XHI_FAR_BRAM_INT_BLOCK		2 /**< Block RAM content */

#elif ((XHI_FAMILY == XHI_DEV_FAMILY_V5) || (XHI_FAMILY == XHI_DEV_FAMILY_V6) || \
	(XHI_FAMILY == XHI_DEV_FAMILY_7SERIES))
	/* Virtex5 or Virtex6 */
#define XHI_FAR_BLOCK_MASK		0x7
#define XHI_FAR_TOP_BOTTOM_MASK		0x1
#define XHI_FAR_MAJOR_FRAME_MASK	0xFF
#define XHI_FAR_ROW_ADDR_MASK		0x1F
#define XHI_FAR_MINOR_FRAME_MASK	0xFF
#define XHI_FAR_COLUMN_ADDR_MASK	0xFF
#define XHI_FAR_MINOR_ADDR_MASK		0x7F
#define XHI_FAR_BLOCK_SHIFT		21
#define XHI_FAR_TOP_BOTTOM_SHIFT	20
#define XHI_FAR_MAJOR_FRAME_SHIFT	17
#define XHI_FAR_ROW_ADDR_SHIFT		15
#define XHI_FAR_MINOR_FRAME_SHIFT	9
#define XHI_FAR_COLUMN_ADDR_SHIFT	7
#define XHI_FAR_MINOR_ADDR_SHIFT	0

/*
 * Address Block Types in the  Frame Address Register
 */
#define XHI_FAR_CLB_BLOCK		0 /**< CLB/IO/CLK Block */
#define XHI_FAR_BRAM_BLOCK		1 /**< Block RAM interconnect */

#endif
/* @} */

/*
 * Configuration Commands
 */
#define XHI_CMD_NULL			0
#define XHI_CMD_WCFG			1
#define XHI_CMD_MFW			2
#if XHI_FAMILY == XHI_DEV_FAMILY_S6
#define XHI_CMD_LFRM			3
#else
#define XHI_CMD_DGHIGH			3
#endif
#define XHI_CMD_RCFG			4
#define XHI_CMD_START			5
#define XHI_CMD_RCAP			6
#define XHI_CMD_RCRC			7
#define XHI_CMD_AGHIGH			8
#define XHI_CMD_SWITCH			9
#define XHI_CMD_GRESTORE		10
#define XHI_CMD_SHUTDOWN		11
#if (XHI_FAMILY == XHI_DEV_FAMILY_V4) || (XHI_FAMILY == XHI_DEV_FAMILY_V5) || \
	(XHI_FAMILY == XHI_DEV_FAMILY_V6)  || (XHI_FAMILY == XHI_DEV_FAMILY_7SERIES)
#define XHI_CMD_GCAPTURE		12
#endif
#define XHI_CMD_DESYNCH			13

#if (XHI_FAMILY == XHI_DEV_FAMILY_S6)
#define XHI_CMD_IPROG			14
#endif

#if ((XHI_FAMILY == XHI_DEV_FAMILY_V5) || (XHI_FAMILY == XHI_DEV_FAMILY_V6) || \
	(XHI_FAMILY == XHI_DEV_FAMILY_7SERIES))
	/* Virtex5 or Virtex6 */
#define XHI_CMD_IPROG			15
#define XHI_CMD_CRCC			16
#define XHI_CMD_LTIMER			17
#endif
#define XHI_TYPE_2_READ 		((XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
					(XHI_OP_READ << XHI_OP_SHIFT))

#define XHI_TYPE_2_WRITE 		( (XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
					(XHI_OP_WRITE << XHI_OP_SHIFT) )

#if (XHI_FAMILY == XHI_DEV_FAMILY_S6)

#define XHI_SYNC_PACKET1		0xAA99
#define XHI_SYNC_PACKET2		0x5566
#define XHI_DUMMY_PACKET		0xFFFF
#define XHI_NOOP_PACKET			0x2000
#define XHI_TYPE1_WRITE			(XHI_TYPE_1 << XHI_TYPE_SHIFT) | \
					((XHI_OP_WRITE << XHI_OP_SHIFT))

#define XHI_TYPE2_CNT_MASK		0x07FFFFFF

#define XHI_TYPE_1_PACKET_MAX_WORDS 	32

#define XHI_DEVICE_ID_READ		0x29C2
#define XHI_BLOCK_SHIFT			12
#define XHI_ROW_SHIFT			8
#define XHI_COR1_DEFAULT		0x3d10
#define XHI_COR2_DEFAULT		0x9EE
#else
/*
 * Packet constants
 */
#define XHI_SYNC_PACKET			0xAA995566
#define XHI_DUMMY_PACKET		0xFFFFFFFF
#define XHI_DEVICE_ID_READ		0x28018001
#define XHI_NOOP_PACKET			(XHI_TYPE_1 << XHI_TYPE_SHIFT)

#define XHI_TYPE_1_PACKET_MAX_WORDS 	2047
#define XHI_TYPE_1_HEADER_BYTES		4
#define XHI_TYPE_2_HEADER_BYTES		8


#if ((XHI_FAMILY == XHI_DEV_FAMILY_V4) || (XHI_FAMILY == XHI_DEV_FAMILY_V5))
#define XHI_NUM_FRAME_BYTES		164  /* Number of bytes in a frame */
#define XHI_NUM_FRAME_WORDS		41   /* Number of Words in a frame */
#define XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME  83 /* Num of Words in a frame read
						 * from the device including
						 * the NULL frame and an extra
						 * 32 bit word
						 */
#endif
#endif

/* Virtex6 or 7 SERIES */
#if ((XHI_FAMILY == XHI_DEV_FAMILY_V6) || (XHI_FAMILY == XHI_DEV_FAMILY_7SERIES))
#define XHI_NUM_FRAME_BYTES		324  /* Number of bytes in a frame */
#define XHI_NUM_FRAME_WORDS		81   /* Number of Words in a frame */
#define XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME  162 /* Num of Words in a frame read
						 * from the device including
						 * the NULL frame
						 */
#endif


#if XHI_FAMILY == XHI_DEV_FAMILY_S6 /* Spartan6 */
#define XHI_NUM_FRAME_BYTES		130  /* Number of bytes in a frame */
#define XHI_NUM_FRAME_WORDS		65   /* Number of Words in a frame */
#define XHI_NUM_WORDS_FRAME_INCL_NULL_FRAME  131 /* Num of Words in a frame read
						 * from the device including
						 * the NULL frame
						 */
#endif

#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */

#define XHI_GCLK_FRAMES             	3
#define XHI_IOB_FRAMES              	30
#define XHI_DSP_FRAMES              	21
#define XHI_CLB_FRAMES              	22
#define XHI_BRAM_FRAMES             	64
#define XHI_BRAM_INT_FRAMES         	20

#elif XHI_FAMILY == XHI_DEV_FAMILY_V5 /* Virtex5  */

#define XHI_GCLK_FRAMES             	4
#define XHI_IOB_FRAMES              	54
#define XHI_DSP_FRAMES              	28
#define XHI_CLB_FRAMES              	36
#define XHI_BRAM_FRAMES             	64
#define XHI_BRAM_INT_FRAMES         	30

#endif


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

#if XHI_FAMILY == XHI_DEV_FAMILY_V4 /* Virtex4 */
/****************************************************************************/
/**
*
* Generates a Type 1 packet header that is written to the Frame Address
* Register (FAR) for a Virtex4 device.
*
* @param	Top - top (0) or bottom (1) half of device
* @param	Block - Address Block Type (CLB or BRAM address space)
* @param	HClkRow - H Clock row
* @param	MajorAddress - CLB or BRAM column
* @param	MinorAdderss - Frame within column
*
* @return	Type 1 packet header to write the FAR
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_SetupFarV4(Top, Block, HClkRow, MajorAddress, MinorAddress)  \
	((Top << XHI_FAR_BOTTOM_SHIFT) | \
	(Block << XHI_FAR_BLOCK_SHIFT) | \
	(HClkRow << XHI_FAR_HCLKROW_SHIFT) | \
	(MajorAddress << XHI_FAR_COLUMN_SHIFT) | \
	(MinorAddress << XHI_FAR_MINOR_SHIFT))
#endif

#if ((XHI_FAMILY == XHI_DEV_FAMILY_V5) || (XHI_FAMILY == XHI_DEV_FAMILY_V6) || \
	(XHI_FAMILY == XHI_DEV_FAMILY_7SERIES))
/****************************************************************************/
/**
*
* Generates a Type 1 packet header that is written to the Frame Address
* Register (FAR) for a Virtex5 device.
*
* @param	Block - Address Block Type (CLB or BRAM address space)
* @param	Top - top (0) or bottom (1) half of device
* @param	Row - Row Address
* @param	ColumnAddress - CLB or BRAM column
* @param	MinorAddress - Frame within a column
*
* @return	Type 1 packet header to write the FAR
*
* @note		None.
*
*****************************************************************************/
#define XHwIcap_SetupFarV5(Top, Block, Row, ColumnAddress, MinorAddress)  \
	(Block << XHI_FAR_BLOCK_SHIFT) | \
	((Top << XHI_FAR_TOP_BOTTOM_SHIFT) | \
	(Row << XHI_FAR_ROW_ADDR_SHIFT) | \
	(ColumnAddress << XHI_FAR_COLUMN_ADDR_SHIFT) | \
	(MinorAddress << XHI_FAR_MINOR_ADDR_SHIFT))
#endif


/************************** Function Prototypes ******************************/


/************************** Variable Definitions ****************************/

#ifdef __cplusplus
}
#endif

#endif

