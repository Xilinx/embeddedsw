/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_qspi.h
*
* This is the header file which contains qspi declarations for XilLoader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
* 1.01  sb   08/23/2019 Added QSPI buswidth detection logic
*       har  08/28/2019 Fixed MISRA C violations
*       bsv  08/30/2019 Added fallback and multiboot support in PLM
*       bsv  09/12/2019 Added support for Macronix 1.8V flash parts
* 1.02  bsv  02/04/2020 Reset qspi instance in init functions for LPD off
*						suspend and resume to work
*       bsv  04/09/2020 Code clean up
* 1.03  bsv  07/03/2020 Added support for macronix part P/N:MX25U12835F
*       skd  07/14/2020 XLoader_QspiCopy prototype changed
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       skd  08/21/2020 Removed flash size macros
*       bsv  10/13/2020 Code clean up
* 1.04  bsv  07/22/2021 Added support for Winbond flash part
*       bsv  08/31/2021 Code clean up
*       ng   08/09/2023 Removed redundant windbond flash size macro
*
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
#include "xplmi_status.h"
#ifdef XLOADER_QSPI

/************************** Constant Definitions *****************************/
/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define XLOADER_READ_ID_CMD			(0x9FU)
#define XLOADER_FAST_READ_CMD_24BIT		(0x0BU)
#define	XLOADER_DUAL_READ_CMD_24BIT		(0x3BU)
#define XLOADER_QUAD_READ_CMD_24BIT		(0x6BU)
#define XLOADER_FAST_READ_CMD_32BIT		(0x0CU)
#define XLOADER_DUAL_READ_CMD_32BIT		(0x3CU)
#define XLOADER_QUAD_READ_CMD_32BIT		(0x6CU)
#define XLOADER_QUAD_READ_CMD_24BIT2		(0xEBU)

#define XLOADER_WRITE_ENABLE_CMD	(0x06U)
#define XLOADER_BANK_REG_RD_CMD		(0x16U)
#define XLOADER_BANK_REG_WR_CMD		(0x17U)
/* Bank register is called Extended Addr Reg in Micron */
#define XLOADER_EXTADD_REG_RD_CMD	(0xC8U)
#define XLOADER_EXTADD_REG_WR_CMD	(0xC5U)

#define XLOADER_COMMAND_OFST	(0U) /* FLASH instruction */
#define XLOADER_ADDR_1_OFST	(1U) /* MSB byte of address to read or write */
#define XLOADER_ADDR_2_OFST	(2U) /* Middle byte of address to read or write */
#define XLOADER_ADDR_3_OFST	(3U) /* Middle byte of address to read or write */
#define XLOADER_ADDR_4_OFST	(4U) /* LSB byte of address to read or write */
#define XLOADER_DUMMY_CLOCKS	(8U) /* Number of dummy bytes for fast, dual and
				     quad reads */

/*
 * Max limit of single DMA transfer is 512MB
 */
#define XLOADER_DMA_DATA_TRAN_SIZE	(0x20000000U)

/*
 * Macros related to Qspi Bank Size
 */
#define XLOADER_FLASH_SIZE_16MB			(0x1000000U)
#define XLOADER_BANKSIZE			XLOADER_FLASH_SIZE_16MB
#define XLOADER_FLASH_SIZE_64MB			(0x4000000U)
#define XLOADER_WINBOND_BANKSIZE		XLOADER_FLASH_SIZE_64MB
/*
 * Bank mask
 */
#define XLOADER_BANKMASK		(~(XLOADER_BANKSIZE - 1U))
#define XLOADER_WINBOND_BANKMASK		(~(XLOADER_WINBOND_BANKSIZE - 1U))

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
#define XLOADER_MICRON_ID		(0x20U)
#define XLOADER_SPANSION_ID		(0x01U)
#define XLOADER_WINBOND_ID		(0xEFU)
#define XLOADER_MACRONIX_ID		(0xC2U)
#define XLOADER_ISSI_ID			(0x9DU)

/* Macronix size constants for 1.8V and 3.3 parts. */
#define XLOADER_MACRONIX_FLASH_SIZE_ID_512M		(0x1AU)
#define XLOADER_MACRONIX_FLASH_SIZE_ID_1G		(0x1BU)
#define XLOADER_MACRONIX_FLASH_SIZE_ID_2G		(0x1CU)
#define XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_128M       (0x38U)
#define XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_512M       (0x3AU)
#define XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_1G         (0x3BU)
#define XLOADER_MACRONIX_FLASH_1_8_V_SIZE_ID_2G		(0x3CU)

/*Qspi width detection macros*/
#define XLOADER_QSPI_BUSWIDTH_DETECT_VALUE	(0xAA995566U)
#define XLOADER_QSPI_BUSWIDTH_PDI_OFFSET	(0x10U)
#define XLOADER_QSPI_BUSWIDTH_LENGTH		(0x10U)
#define XLOADER_QSPI_BUSWIDTH_ONE		(0U)
#define XLOADER_QSPI_BUSWIDTH_TWO		(1U)
#define XLOADER_QSPI_BUSWIDTH_FOUR		(2U)

#define XLOADER_READ_ID_CMD_TX_BYTE_CNT		(1U)
#define XLOADER_READ_ID_CMD_RX_BYTE_CNT		(4U)
#define XLOADER_QSPI24_COPY_DISCARD_BYTE_CNT	(4U)
#define XLOADER_QSPI32_COPY_DISCARD_BYTE_CNT	(5U)
#define XLOADER_QSPI_WRITE_ENABLE_CMD_BYTE_CNT	(1U)
#define XLOADER_EXTADD_REG_WR_CMD_BYTE_CNT	(2U)
#define XLOADER_EXTADD_REG_RD_CMD_BYTE_CNT	(1U)
#define XLOADER_BANK_REG_WR_CMD_BYTE_CNT	(2U)
#define XLOADER_BANK_REG_RD_CMD_BYTE_CNT	(1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XLoader_QspiInit(u32 DeviceFlags);
int XLoader_QspiCopy(u64 SrcAddr, u64 DestAddr, u32 Length, u32 Flags);
int XLoader_QspiGetBusWidth(u64 ImageOffsetAddress);
int XLoader_QspiRelease(void);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_QSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_QSPI_H */
