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
* 1.00  kc   02/21/2018 Initial release
* 1.01  bsv  09/15/2019 Added Read Config and Write Config registers
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
#include "xplmi_hw.h"

#ifdef XLOADER_OSPI
#include "xparameters.h"	/* SDK generated parameters */
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xplmi_status.h"	/* PLMI error codes */

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
#define	MICRON_OCTAL_ID_BYTE0		(0x2CU)
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

/************************** Function Prototypes ******************************/
int XLoader_OspiInit(u32 DeviceFlags);
int XLoader_OspiCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#endif /* end of XLOADER_OSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_OSPI_H */
