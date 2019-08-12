/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xloader_qspi.h
*
* This is the header file which contains qspi declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XLOADER_QSPI_H
#define XLOADER_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xplmi_status.h"
#ifdef XLOADER_QSPI
#include "xqspipsu.h"
#include "xplmi_debug.h"
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


#define WRITE_ENABLE_CMD	(0x06U)
#define BANK_REG_RD_CMD		(0x16U)
#define BANK_REG_WR_CMD		(0x17U)
/* Bank register is called Extended Addr Reg in Micron */
#define EXTADD_REG_RD_CMD	(0xC8U)
#define EXTADD_REG_WR_CMD	(0xC5U)

#define COMMAND_OFST		(0U) /* FLASH instruction */
#define ADDR_1_OFST	(1U) /* MSB byte of address to read or write */
#define ADDR_2_OFST	(2U) /* Middle byte of address to read or write */
#define ADDR_3_OFST	(3U) /* Middle byte of address to read or write */
#define ADDR_4_OFST	(4U) /* LSB byte of address to read or write */
#define DATA_OFST			(4U) /* Start of Data for Read/Write */
#define DUMMY_OFST		(4U) /* Dummy byte offset for fast, dual and quad
				     reads */
#define DUMMY_SIZE			(1U) /* Number of dummy bytes for fast, dual and
				     quad reads */
#define DUMMY_CLOCKS		8 /* Number of dummy bytes for fast, dual and
				     quad reads */
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

#define FLASH_SIZE_ID_64M		(0x17U)
#define FLASH_SIZE_ID_128M		(0x18U)
#define FLASH_SIZE_ID_256M		(0x19U)
#define FLASH_SIZE_ID_512M		(0x20U)
#define FLASH_SIZE_ID_1G		(0x21U)
#define FLASH_SIZE_ID_2G		(0x22U)
/* Macronix size constants are different for 512M and 1G */
#define MACRONIX_FLASH_SIZE_ID_512M		(0x1AU)
#define MACRONIX_FLASH_SIZE_ID_1G		(0x1BU)

/*
 * Size in bytes
 */
#define FLASH_SIZE_64M			(0x0800000U)
#define FLASH_SIZE_128M			(0x1000000U)
#define FLASH_SIZE_256M			(0x2000000U)
#define FLASH_SIZE_512M			(0x4000000U)
#define FLASH_SIZE_1G			(0x8000000U)
#define FLASH_SIZE_2G			(0x10000000U)

/* TODO change to QSPI driver API */
#define XLOADER_QSPIDMA_DST_CTRL	(0xF103080CU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XLoader_Qspi24Init(u32 DeviceFlags);
XStatus XLoader_Qspi24Copy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_Qspi24Release(void );
int XLoader_Qspi32Init(u32 DeviceFlags);
XStatus XLoader_Qspi32Copy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_Qspi32Release(void );
/************************** Variable Definitions *****************************/


#endif /* end of XLOADER_QSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_QSPI_H */
