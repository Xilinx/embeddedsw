/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* @file xfsbl_qspi.h
*
* This is the header file which contains qspi declarations for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.00  sg   12/03/15 Added GQSPI driver support
*                     32Bit boot mode support
* 3.0   ds   01/03/17 Add support for Micron QSPI 2G part
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_QSPI_H
#define XFSBL_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xfsbl_hw.h"

#ifdef XFSBL_QSPI

/************************** Constant Definitions *****************************/

/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define READ_ID_CMD				(0x9FU)
#define FAST_READ_CMD_24BIT		(0x0BU)
#define	DUAL_READ_CMD_24BIT		(0x3BU)
#define QUAD_READ_CMD_24BIT		(0x6BU)
#define FAST_READ_CMD_32BIT		(0x0CU)
#define DUAL_READ_CMD_32BIT		(0x3CU)
#define QUAD_READ_CMD_32BIT		(0x6CU)
#define QUAD_READ_CMD_32BIT2		(0xEBU)


#define WRITE_STATUS_CMD	(0x01U)
#define WRITE_ENABLE_CMD	(0x06U)
#define BANK_REG_RD_CMD		(0x16U)
#define BANK_REG_WR_CMD		(0x17U)
/* Bank register is called Extended Address Reg in Micron */
#define EXTADD_REG_RD_CMD	(0xC8U)
#define EXTADD_REG_WR_CMD	(0xC5U)

#define COMMAND_OFFSET		(0U) /* FLASH instruction */
#define ADDRESS_1_OFFSET	(1U) /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	(2U) /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	(3U) /* Middle byte of address to read or write */
#define ADDRESS_4_OFFSET	(4U) /* LSB byte of address to read or write */
#define DATA_OFFSET			(4U) /* Start of Data for Read/Write */
#define DUMMY_OFFSET		(4U) /* Dummy byte offset for fast, dual and quad
				     reads */
#define DUMMY_SIZE			(1U) /* Number of dummy bytes for fast, dual and
				     quad reads */
#define DUMMY_CLOCKS		8 /* Number of dummy bytes for fast, dual and
				     quad reads */
#define DUMMY_CLOCKS_MACRONIX	6 /* For 4-4-4 mode in Macronix dummy cycles are
					default to 6*/
#define RD_ID_SIZE			(4U) /* Read ID command + 3 bytes ID response */
#define BANK_SEL_SIZE		(2U) /* BRWR or EARWR command + 1 byte bank value */
#define WRITE_ENABLE_CMD_SIZE	(1U) /* WE command */
/*
 * The following constants specify the extra bytes which are sent to the
 * FLASH on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		(4U)

/*
 * Max limit of single DMA transfer is 512MB
 */
#define DMA_DATA_TRAN_SIZE		(0x20000000U)

/*
 * The following defines are for dual flash interface.
 */
#define LQSPI_CR_FAST_QUAD_READ		(0x0000006BU) /* Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		(0x00000100U) /* 1 Dummy Byte between
						     address and return data */

#define SINGLE_QSPI_IO_CONFIG_QUAD_READ	(LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

#define DUAL_QSPI_PARALLEL_IO_CONFIG_QUAD_READ	\
					(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQspiPsu_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)


#define DUAL_QSPI_STACK_IO_CONFIG_READ	(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

/**
 * Flash connection type as defined in PCW
 */
#define FLASH_SIZE_16MB			(0x1000000U)
#define BANKSIZE			(FLASH_SIZE_16MB)
#define SINGLEBANKSIZE			BANKSIZE

/*
 * Bank mask
 */
#define BANKMASK			(0xFFFFFFFFU & ~(BANKSIZE - 1))
#define SINGLEBANKMASK			BANKMASK

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0xBB or 0xBA
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 * Spansion:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is Device ID - Memory Interface type - 0x20 or 0x02
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 */

#define MICRON_ID		(0x20U)
#define SPANSION_ID		(0x01U)
#define WINBOND_ID		(0xEFU)
#define MACRONIX_ID		(0xC2U)
#define ISSI_ID			(0x9DU)

#define FLASH_SIZE_ID_8M		(0x14U)
#define FLASH_SIZE_ID_16M		(0x15U)
#define FLASH_SIZE_ID_32M		(0x16U)
#define FLASH_SIZE_ID_64M		(0x17U)
#define FLASH_SIZE_ID_128M		(0x18U)
#define FLASH_SIZE_ID_256M		(0x19U)
#define FLASH_SIZE_ID_512M		(0x20U)
#define FLASH_SIZE_ID_1G		(0x21U)
#define FLASH_SIZE_ID_2G		(0x22U)
/* Macronix size constants are different for 512M and 1G */
#define MACRONIX_FLASH_SIZE_ID_512M		(0x1AU)
#define MACRONIX_FLASH_SIZE_ID_1G		(0x1BU)
#define MACRONIX_FLASH_1_8_V_SIZE_ID_1G  	(0x3BU)
#define MACRONIX_FLASH_1_8_V_MX25_ID_256	(0x39U)
#define MACRONIX_FLASH_1_8_V_MX66_ID_512	(0x3AU)

/*
 * Size in bytes
 */
#define FLASH_SIZE_8M			(0x0100000U)
#define FLASH_SIZE_16M			(0x0200000U)
#define FLASH_SIZE_32M			(0x0400000U)
#define FLASH_SIZE_64M			(0x0800000U)
#define FLASH_SIZE_128M			(0x1000000U)
#define FLASH_SIZE_256M			(0x2000000U)
#define FLASH_SIZE_512M			(0x4000000U)
#define FLASH_SIZE_1G			(0x8000000U)
#define FLASH_SIZE_2G			(0x10000000U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

u32 XFsbl_Qspi24Init(u32 DeviceFlags);
u32 XFsbl_Qspi24Copy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
u32 XFsbl_Qspi24Release(void );
u32 XFsbl_Qspi32Init(u32 DeviceFlags);
u32 XFsbl_Qspi32Copy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
u32 XFsbl_Qspi32Release(void );

/************************** Variable Definitions *****************************/


#endif /* end of XFSBL_QSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_QSPI_H */
