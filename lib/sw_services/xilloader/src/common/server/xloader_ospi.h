/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ospi.h
*
* This is the header file which contains ospi declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv  08/23/2018 Initial release
* 1.01  bsv  09/10/2019 Added support to set OSPI to DDR mode
*       ma   02/03/2020 Change XPlmi_MeasurePerfTime to retrieve Performance
*                       time and print
*       bsv  02/04/2020 Reset qspi instance in init functions for LPD off
*						suspend and resume to work
*       bsv  04/09/2020 Code clean up of Xilloader
* 1.02  bsv  27/06/2020 Add dual stacked mode support
*       bsv  07/08/2020 APIs specific to this file made static
*       skd  07/14/2020 XLoader_OspiCopy prototype changed
*       skd  08/21/2020 Added GIGADEVICE and ISSI flash ID macros
*       bsv  10/13/2020 Code clean up
* 1.03  bsv  07/16/2021 Added Macronix flash support
*       bsv  08/31/2021 Code clean up
*       bm   01/11/2023 Added support for Gigadevice 512M, 1G, 2G parts
*       dd   02/08/2024 Added support for ISSI 512M
*       sk   02/26/2024 Added defines for Spansion flash part
*       ng   03/05/2024 Added support for Macronix OSPI 2G flash part
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XLOADER_OSPI_H
#define XLOADER_OSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifdef XLOADER_OSPI

/************************** Constant Definitions *****************************/
/*
 * Flash connection type as defined in Vivado
 */
#define READ_CMD_4B    			(0x13U)
#define READ_CMD_OCTAL_4B    		(0x7CU)
#define READ_ID				(0x9FU)
#define WRITE_DISABLE_CMD		(0x4U)
#define OSPI_WRITE_ENABLE_CMD		(0x6U)
#define ENTER_4B_ADDR_MODE      	(0xB7U)
#define EXIT_4B_ADDR_MODE       	(0xE9U)
#define READ_FLAG_STATUS_CMD		(0x70U)
#define WRITE_CONFIG_REG		(0x81U)
#define READ_CONFIG_REG			(0x85U)
#define WRITE_CONFIG2_REG_MX		(0x72U)
#define READ_CONFIG2_REG_MX		(0x71U)
#define READ_CMD_OPI_MX			(0xEEU)
#define READ_CMD_OPI_SPN		(0xEEU)
#define WRITE_CONFIG_REG_SPN		(0x71U)
#define READ_CONFIG_REG_SPN		(0x65U)
#define CONFIG_REG_4_ADDR_SPN		(0x800005U)
#define CONFIG_REG_5_ADDR_SPN		(0x800006U)

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x5B
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x1A; 1Gbit : 0x1B; 2Gbit : 0x1C
 * Gigadevice:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x40 or 0x47
 * Byte 2 is second byte of Device ID describes flash size:
 * 256Mbit : 0x19; 512Mbit : 0x1A; 1Gbit : 0x1B; 2Gbit : 0x1C
 * ISSI:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x5A or 0x5B
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x1A
 * Macronix:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x80
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x3A;  2Gbit : 0x3C
 */
#define MICRON_OCTAL_ID_BYTE0           (0x2CU) /* Micron manufacture id */
#define GIGADEVICE_OCTAL_ID_BYTE0       (0xC8U) /* Gigadevice manufacture id */
#define ISSI_OCTAL_ID_BYTE0             (0x9DU) /* ISSI manufacture id */
#define MACRONIX_OCTAL_ID_BYTE0         (0xC2U) /* Macronix manufacture id */
#define SPANSION_OCTAL_ID_BYTE0		(0x34U)  /* Spansion manufacture id */
#define SPANSION_OCTAL_ID_BYTE2_2G	(0x1CU)  /* Spansion 2Gbit flash size */
#define MICRON_OCTAL_ID_BYTE2_512       (0x1AU) /* Micron 512Mbit flash size */
#define MICRON_OCTAL_ID_BYTE2_1G        (0x1BU) /* Micron 1Gbit flash size */
#define MICRON_OCTAL_ID_BYTE2_2G        (0x1CU) /* Micron 2Gbit flash size */
#define MACRONIX_OCTAL_ID_BYTE2_512     (0x3AU) /* Macronix 512Mbit flash size */
#define MACRONIX_OCTAL_ID_BYTE2_2G      (0x3CU) /* Macronix 2Gbit flash size */
#define GIGADEVICE_OCTAL_ID_BYTE2_256	(0x19U) /* Gigadevice 256Mbit flash size */
#define GIGADEVICE_OCTAL_ID_BYTE2_512	(0x1AU) /* Gigadevice 512Mbit flash size */
#define GIGADEVICE_OCTAL_ID_BYTE2_1G	(0x1BU) /* Gigadevice 1Gbit flash size */
#define GIGADEVICE_OCTAL_ID_BYTE2_2G	(0x1CU) /* Gigadevice 2Gbit flash size */
#define ISSI_OCTAL_ID_BYTE2_512         (0x1AU) /* ISSI 512Mbit flash size */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_OSPI_MACRONIX_EXTENDED_OPCODE		(0xFFU)
#define XLOADER_MACRONIX_OSPI_DDR_DUMMY_CYCLES		(20U)
#define XLOADER_SPANSION_OSPI_DDR_DUMMY_CYCLES		(21U)
#define XLOADER_OSPI_DDR_DUMMY_CYCLES	(16U)
#define XLOADER_OSPI_SDR_DUMMY_CYCLES	(8U)
#define XLOADER_MACRONIX_OSPI_SET_DDR_DUMMY_CYCLES	(4U)
#define XLOADER_READ_ID_BYTES		(8U)
#define XLOADER_OSPI_READ_ADDR_SIZE	(4U)
#define XLOADER_OSPI_DDR_MODE_BYTE_CNT	(2U)
#define XLOADER_OSPI_SDR_MODE_BYTE_CNT	(1U)

#define XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_BYTE_CNT	(0U)
#define XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_ADDR_SIZE	(3U)
#define XLOADER_OSPI_READ_FLAG_STATUS_CMD_ADDR_SIZE	(0U)
#define XLOADER_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(3U)
#define XLOADER_MACRONIX_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(4U)
#define XLOADER_SPANSION_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(4U)
#define XLOADER_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT		(1U)
#define XLOADER_OSPI_READ_CFG_REG_CMD_ADDR_SIZE		(4U)
#define XLOADER_OSPI_READ_CFG_REG_CMD_BYTE_CNT		(2U)
#define XLOADER_WRITE_CFG_REG_VAL		(0xE7U)
#define XLOADER_MACRONIX_WRITE_CFG_REG_VAL		(0x02U)
#define XLOADER_SPANSION_WRITE_CFG_REG_VAL		(0x43U)
#define XLOADER_OSPI_WRITE_DONE_MASK	(0x80U)
#define XLOADER_WRITE_CFG_ECC_REG_VAL		(0xA0U)

/************************** Function Prototypes ******************************/
int XLoader_OspiInit(u32 DeviceFlags);
int XLoader_OspiCopy(u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_OspiRelease(void);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_OSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_OSPI_H */
