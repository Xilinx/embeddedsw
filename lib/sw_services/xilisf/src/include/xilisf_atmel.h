/******************************************************************************
*
* Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xilisf_atmel.h
*
* This file contains the definitions to be used when accessing the Atmel
* AT45XXXD Serial Flash.
* If any new definitions are added to this file, check if they need to be
* added to the xilisf_intelstm.h file too.
*
* @note		None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- -----------------------------------------------
* 1.00a ksu/sdm  03/03/08 First release
*
* </pre>
*
******************************************************************************/
#ifndef XILISF_ATMEL_H /* prevent circular inclusions */
#define XILISF_ATMEL_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

/**
 * The following definitions specify the Device Id for the different Atmel
 * AT45XXXD Serial Flash Devices.
 */
#define XISF_ATMEL_DEV_AT45DB011D	0x22	/**< Device ID of AT45DB011D */
#define XISF_ATMEL_DEV_AT45DB021D	0x23	/**< Device ID of AT45DB027D */
#define XISF_ATMEL_DEV_AT45DB041D	0x24	/**< Device ID of AT45DB041D */
#define XISF_ATMEL_DEV_AT45DB081D	0x25	/**< Device ID of AT45DB081D */
#define XISF_ATMEL_DEV_AT45DB161D	0x26	/**< Device ID of AT45DB161D */
#define XISF_ATMEL_DEV_AT45DB321D	0x27	/**< Device ID of AT45DB321D */
#define XISF_ATMEL_DEV_AT45DB642D	0x28	/**< Device ID of AT45DB642D */


/**
 * Definitions of Atmel Serial Flash Device geometry.
 */
#define XISF_BYTES256_PER_PAGE		256	/**< 256 Bytes per Page */
#define XISF_BYTES264_PER_PAGE		264	/**< 264 Bytes per Page */
#define XISF_BYTES512_PER_PAGE		512	/**< 512 Bytes per Page */
#define XISF_BYTES528_PER_PAGE		528	/**< 528 Bytes per Page */
#define XISF_BYTES1024_PER_PAGE		1024	/**< 1024 Bytes per Page */
#define XISF_BYTES1056_PER_PAGE		1056	/**< 1056 Bytes per Page */
#define XISF_PAGES8_PER_BLOCK		8	/**< Pages per Block */
#define XISF_BLOCKS16_PER_SECTOR	16	/**< 16 Blocks per Sector */
#define XISF_BLOCKS32_PER_SECTOR	32	/**< 32 Blocks per Sector */
#define XISF_NUM_OF_SECTORS4		4	/**< 4 Sectors */
#define XISF_NUM_OF_SECTORS8		8	/**< 8 Sectors */
#define XISF_NUM_OF_SECTORS16		16	/**< 16 Sectors */
#define XISF_NUM_OF_SECTORS32		32	/**< 32 Sectors */
#define XISF_NUM_OF_SECTORS64		64	/**< 64 Sectors */

/**
 * Definitions of Bit masks used for calculating device address from a
 * linear address in the case of Default Addressing mode .
 */
#define XISF_BYTES256_PER_PAGE_MASK	0x0FF	/**< Byte mask for devices with
						   * 256 or less bytes per page
					 	   */
#define XISF_BYTES512_PER_PAGE_MASK	0x1FF	/**< Byte mask for devices with
						   * more than 256 bytes per
						   * page */
#define XISF_BYTES1024_PER_PAGE_MASK	0x3FF	/**< Byte mask for devices with
						   * more than 512 bytes per
						   * page */
#define XISF_BYTES2048_PER_PAGE_MASK	0x7FF	/**< Byte mask for devices with
						   * more than 1024 bytes per
						   * page */

/**
 * Definitions of Read commands.
 */
#define XISF_CMD_RANDOM_READ		0x03	/**< Random Read command */
#define XISF_CMD_FAST_READ		0x0B	/**< Fast Read command */
#define XISF_CMD_ISFINFO_READ		0x9F	/**< Device Info command */
#define XISF_CMD_PAGETOBUF1_TRANS	0x53	/**< Transfer contents of a Page
						  *  to the Buffer 1 command */
#define XISF_CMD_PAGETOBUF2_TRANS	0x55	/**< Transfer contents of a Page
						  *  to the Buffer 2 command  */
#define XISF_CMD_BUF1_READ		0xD1	/**< Buffer 1 Read command */
#define XISF_CMD_BUF2_READ		0xD3	/**< Buffer 2 Read command */
#define XISF_CMD_FAST_BUF1_READ		0xD4	/**< Fast Buffer 1 Read Cmd */
#define XISF_CMD_FAST_BUF2_READ		0xD6	/**< Fast Buffer 2 Read Cmd */
#define XISF_CMD_STATUSREG_READ		0xD7	/**< Status Greg Read Cmd */

/**
 * Definitions of Write commands.
 */
#define XISF_CMD_PAGEPROG_WRITE		0x82	/**< Page Program command */
#define XISF_CMD_AUTOPAGE_WRITE		0x58	/**< Auto write command */
#define XISF_CMD_BUFFER1_WRITE		0x84	/**< Buffer1 write command */
#define XISF_CMD_BUFFER2_WRITE		0x87	/**< Buffer2 write command */
#define XISF_CMD_ERASE_BUF1TOPAGE_WRITE	0x83	/**< Erase page first then write
						  *  buffer 1 to the page
						  *  command */
#define XISF_CMD_ERASE_BUF2TOPAGE_WRITE	0x86	/**< Erase page first then write
						  *  buffer2 to the page
						  *  command */
#define XISF_CMD_BUF1TOPAGE_WRITE	0x88	/**< Write buffer1 to the page
						  *  command without Erase */
#define XISF_CMD_BUF2TOPAGE_WRITE	0x89	/**< Write buffer2 to the page
						  *  command without Erase */

/**
 * Definitions of Erase commands.
 */
#define XISF_CMD_PAGE_ERASE		0x81	/**< Page Erase command */
#define XISF_CMD_BLOCK_ERASE		0x50	/**< Block erase command */
#define XISF_CMD_SECTOR_ERASE		0x7C	/**< Sector Erase command */

/**
 * Definitions of commands used for
 *  - Erasing SPR
 *  - Programming SPR
 *  - Enabling/Disabling SPR.
 */
#define XISF_CMD_SPR_BYTE1		0x3D 	/**< SPR command byte1 */
#define XISF_CMD_SPR_BYTE2		0x2A 	/**< SPR command byte2 */
#define XISF_CMD_SPR_BYTE3		0x7F 	/**< SPR command byte3 */
#define XISF_CMD_SPR_BYTE4_ERASE	0xCF 	/**< SPR erase command */
#define XISF_CMD_SPR_BYTE4_PROGRAM	0xFC 	/**< SPR program command */
#define XISF_CMD_SPR_BYTE4_ENABLE	0xA9 	/**< SPR enable command */
#define XISF_CMD_SPR_BYTE4_DISABLE	0x9A 	/**< SPR disable command */

/**
 * Definitions of command used for reading SPR.
 */
#define XISF_CMD_SPR_READ		0x32 	/**< SPR read command */

/**
 * The following definitions specify the buffer number of Atmel Serial Flash.
 */
#define XISF_PAGE_BUFFER1		1 	/**< Buffer 1 */
#define XISF_PAGE_BUFFER2		2 	/**< Buffer 2 */


/**
 * The following definitions specify the Status Register bit definitions of
 * Atmel Serial Flash.
 */
#define XISF_SR_ADDR_MODE_MASK		0x01	/**< Address mode mask */
#define XISF_SR_COMPARE_MASK		0x40	/**< Compare mask */
#define XISF_SR_IS_READY_MASK		0x80	/**< Ready mask */
#define XISF_SR_DEVID_MASK		0x2C	/**< Device ID mask */
#define XISF_SR_DEVID_SHIFT_MASK	0x02	/**< Device ID shift mask */

/**
 * The following definitions determine the addressing mode of the Atmel Serial
 * Flash.
 */
#define XISF_POWEROFTWO_ADDRESS		0x01	/**< Pow-Of-2 address mask */
#define XISF_DEFAULT_ADDRESS		0x00	/**< Default address mask */

/**
 * This definitions specify the extra bytes in each of the Write command, Read
 * command, Erase command, commands operating on SPR, auto page write, page to
 * buffer and buffer to page transfer commands. This count includes
 * Command byte, address bytes and any don't care bytes needed.
 */
#define XISF_CMD_SEND_EXTRA_BYTES	4 	/**< Command extra bytes */

/**
 * This definition specifies the extra bytes in each of the Write/Read/Erase
 * commands, commands operating on SPR, auto page write, page to
 * buffer and buffer to page transfer commands in 4 bytes addressing mode.
 * This count includes Command byte, Address bytes and any
 * don't care bytes needed.
 */
#define XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE	5 /**< Command extra bytes */

/**
 * This definitions specify the extra bytes in Fast read Fast buffer read
 * commands. This count includes Command byte, address bytes and any don't care
 * bytes needed.
 */
#define XISF_CMD_FAST_READ_EXTRA_BYTES	5	/**< Fast read and Fast buffer
						  *  read extra bytes */

/**
 * The following definitions specify the total bytes in some of the commands.
 * This count includes Command byte and any don't care bytes needed.
 */
#define XISF_STATUS_RDWR_BYTES		2	/**< Status Read/Write bytes
						  *  count */
#define XISF_INFO_READ_BYTES		5	/**< Flash Info Read bytes
						  *  count */
#define XISF_INFO_EXTRA_BYTES		1	/**< Flash Info Read Extra
						  *  bytes */
#define XISF_CMD_MAX_EXTRA_BYTES	5	/**< Max extra bytes for
						  *  all commands  */
#define XISF_DUMMYBYTE			0xFF	/**< Dummy byte to fill */


/**
 * Address Shift Masks.
 */
#define XISF_ADDR_SHIFT16		16 /**< 16 bit Shift */
#define XISF_ADDR_SHIFT8		8  /**< 8 bit Shift */


/**
 * Byte Positions.
 */
#define BYTE1				0 /**< Byte 1 position */
#define BYTE2				1 /**< Byte 2 position */
#define BYTE3				2 /**< Byte 3 position */
#define BYTE4				3 /**< Byte 4 position */
#define BYTE5				4 /**< Byte 5 position */


/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* end of protection macro */

