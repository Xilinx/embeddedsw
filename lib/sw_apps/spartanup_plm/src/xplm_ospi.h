/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_ospi.h
 *
 * This is the header file which contains ospi declarations for the PLM.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/
#ifndef XPLM_OSPI_H
#define XPLM_OSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <xil_types.h>

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
#define MACRONIX_OCTAL_ID_BYTE0           (0xC2U)
#define MICRON_OCTAL_ID_BYTE2_512	(0x1AU)
#define MICRON_OCTAL_ID_BYTE2_1G	(0x1BU)
#define MICRON_OCTAL_ID_BYTE2_2G	(0x1CU)
#define MACRONIX_OCTAL_ID_BYTE2_512	(0x3AU)
#define GIGADEVICE_OCTAL_ID_BYTE2_256	(0x19U)
#define GIGADEVICE_OCTAL_ID_BYTE2_512	(0x1AU)
#define GIGADEVICE_OCTAL_ID_BYTE2_1G	(0x1BU)
#define GIGADEVICE_OCTAL_ID_BYTE2_2G	(0x1CU)

#define XPLM_OSPI_MACRONIX_EXTENDED_OPCODE		(0xFFU)
#define XPLM_MACRONIX_OSPI_DDR_DUMMY_CYCLES		(20U)
#define XPLM_OSPI_DDR_DUMMY_CYCLES	(16U)
#define XPLM_OSPI_SDR_DUMMY_CYCLES	(8U)
#define XPLM_MACRONIX_OSPI_SET_DDR_DUMMY_CYCLES	(4U)
#define XPLM_READ_ID_BYTES		(8U)
#define XPLM_OSPI_READ_ADDR_SIZE	(4U)
#define XPLM_OSPI_DDR_MODE_BYTE_CNT	(2U)
#define XPLM_OSPI_SDR_MODE_BYTE_CNT	(1U)

#define XPLM_OSPI_ENTER_4B_ADDR_MODE_CMD_BYTE_CNT	(0U)
#define XPLM_OSPI_ENTER_4B_ADDR_MODE_CMD_ADDR_SIZE	(3U)
#define XPLM_OSPI_READ_FLAG_STATUS_CMD_ADDR_SIZE	(0U)
#define XPLM_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(3U)
#define XPLM_MACRONIX_OSPI_WRITE_CFG_REG_CMD_ADDR_SIZE	(4U)
#define XPLM_OSPI_WRITE_CFG_REG_CMD_BYTE_CNT		(1U)
#define XPLM_OSPI_READ_CFG_REG_CMD_ADDR_SIZE		(4U)
#define XPLM_OSPI_READ_CFG_REG_CMD_BYTE_CNT		(2U)
#define XPLM_WRITE_CFG_REG_VAL		(0xE7U)
#define XPLM_MACRONIX_WRITE_CFG_REG_VAL		(0x02U)
#define XPLM_OSPI_WRITE_DONE_MASK	(0x80U)

#define XPLM_FLASH_SIZE_512M		(0x4000000U)
#define XPLM_FLASH_SIZE_1G		(0x8000000U)
#define XPLM_FLASH_SIZE_2G		(0x10000000U)
#define XPLM_FLASH_SIZE_256M		(0x2000000U)
#define XPLM_FLASH_SIZE_ID_256M		(0x19U)
#define XPLM_FLASH_SIZE_ID_512M		(0x20U)
#define XPLM_FLASH_SIZE_ID_1G		(0x21U)
#define XPLM_FLASH_SIZE_ID_2G		(0x22U)

/**
 * Macros to select RX and TX transfer flags.
 */
#define XOSPIPSV_OSPIDMA_SRC_CTRL    0X0000100CU
#define XOSPIPSV_OSPIDMA_SRC_CTRL_APB_ERR_RESP_MASK    0X01000000U

#define XOSPIPSV_OSPIDMA_DST_CTRL    0X0000180CU
#define XOSPIPSV_OSPIDMA_DST_CTRL_APB_ERR_RESP_MASK    0X01000000U

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_OspiInit(void);
u32 XPlm_OspiCopy(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_OSPI_H */
