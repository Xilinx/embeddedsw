/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
#define MICRON_ID				(0x20U)
#define READ_CMD_OCTAL_4B    			(0x7CU)
#define READ_ID					(0x9FU)
#define MICRON_INDEX_START			(0x0U)
#define WRITE_DISABLE_CMD			(0x4U)
#define OSPI_WRITE_ENABLE_CMD		(0x6U)
#define ENTER_4B_ADDR_MODE      	(0xB7U)
#define EXIT_4B_ADDR_MODE       	(0xE9U)
#define READ_FLAG_STATUS_CMD		(0x70U)
#define WRITE_CONFIG_REG			(0x81U)
#define READ_CONFIG_REG				(0x85U)

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x5B
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x1A
 */
#define	MICRON_OCTAL_ID_BYTE0		  (0x2CU)
#define GIGADEVICE_OCTAL_ID_BYTE0     (0xC8U)
#define ISSI_OCTAL_ID_BYTE0           (0x9DU)
#define MICRON_OCTAL_ID_BYTE2_512	(0x1AU)
#define MICRON_OCTAL_ID_BYTE2_1G	(0x1BU)
#define MICRON_OCTAL_ID_BYTE2_2G	(0x1CU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XLOADER_OSPI_DDR_DUMMY_CYCLES	(16U)
#define XLOADER_OSPI_SDR_DUMMY_CYCLES	(8U)
#define XLOADER_READ_ID_BYTES		(8U)
#define XLOADER_OSPI_READ_ADDR_SIZE	(4U)
#define XLOADER_OSPI_DDR_MODE_BYTE_CNT	(2U)
#define XLOADER_OSPI_SDR_MODE_BYTE_CNT	(1U)
#define XLOADER_OSPI_DUMMY_CYCLES	(8U)

#define XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_BYTE_CNT	(0U)
#define XLOADER_OSPI_ENTER_4B_ADDR_MODE_CMD_ADDR_SIZE	(3U)
#define XLOADER_OSPI_READ_FLAG_STATUS_CMD_ADDR_SIZE	(0U)
#define XLOADER_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(3U)
#define XLOADER_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT		(1U)
#define XLOADER_OSPI_READ_CFG_REG_CMD_ADDR_SIZE		(4U)
#define XLOADER_OSPI_READ_CFG_REG_CMD_BYTE_CNT		(2U)
#define XLOADER_WRITE_CFG_REG_VAL		(0xE7U)
#define XLOADER_WRITE_CFG_REG_VAL		(0xE7U)
#define XLOADER_OSPI_WRITE_DONE_MASK	(0x80U)

/************************** Function Prototypes ******************************/
int XLoader_OspiInit(u32 DeviceFlags);
int XLoader_OspiCopy(u64 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_OSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_OSPI_H */
