/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_err.h
*
* This file contains list of error codes generated across the application.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  bsv   01/19/21   First release
* 2.00  bsv   03/13/22   Added error codes for unrecognized Eeprom
*
* </pre>
*
******************************************************************************/

#ifndef __XBIR_ERR_H_
#define __XBIR_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#if defined(XPAR_XIICPS_NUM_INSTANCES)
#define	XBIR_ERROR_IIC_MUX			(0x2U)
#define	XBIR_ERROR_IIC_LKP_CONFIG	(0x3U)
#define	XBIR_ERROR_IIC_CONFIG		(0x4U)
#define	XBIR_ERROR_IIC_CONFIG_INIT	(0x5U)
#define	XBIR_ERROR_IIC_MASTER_SEND	(0x6U)
#define	XBIR_ERROR_IIC_MASTER_RECV	(0x7U)
#define	XBIR_ERROR_IIC_SET_SCLK		(0x8U)
#define XBIR_ERROR_I2C_WRITE_TIMEOUT	(0x9U)
#define XBIR_ERROR_I2C_READ_TIMEOUT	(0xAU)
#define XBIR_ERROR_IIC_SET_SCLK_TIMEOUT	(0xBU)
#endif

#define	XBIR_ERROR_QSPI_CONFIG		(0x20U)
#define	XBIR_ERROR_QSPI_CONFIG_INIT	(0x21U)
#define	XBIR_ERROR_QSPI_READ		(0x22U)
#define	XBIR_ERROR_QSPI_4BYTE_ENTER	(0x23U)
#define	XBIR_ERROR_INVALID_QSPI_CONN	(0x24U)
#define	XBIR_ERROR_QSPI_LENGTH		(0x25U)
#define	XBIR_ERROR_POLLED_TRANSFER	(0x26U)
#define	XBIR_ERROR_QSPI_MANUAL_START	(0x27U)
#define	XBIR_ERROR_QSPI_PRESCALER_CLK	(0x28U)
#define	XBIR_ERROR_QSPI_CONN_MODE	(0x29U)
#define XBIR_ERROR_QSPI_VENDOR		(0x2AU)
#define XBIR_ERR_SOM_EEPROM_CONTENTS	(0x2BU)
#define XBIR_ERR_CC_EEPROM_CONTENTS	(0x2CU)

#define XBIR_ERROR_BOOT_IMG_ID	(0x30U)
#define XBIR_ERROR_IMAGE_SIZE	(0x31U)
#define XBIR_ERROR_IMAGE_WRITE	(0x32U)
#define XBIR_ERROR_SECTOR_ERASE	(0x33U)
#define XBIR_ERROR_IMAGE_READ	(0x34U)
#define XBIR_ERROR_IMAGE_CHKSUM	(0x35U)
#define XBIR_ERROR_PERSISTENT_REG_ERASE	(0x36U)
#define XBIR_ERROR_PERSISTENT_REG_WRITE	(0x37U)
#define XBIR_ERROR_PERSISTENT_REG_READ	(0x38U)
#define XBIR_ERROR_PERSISTENT_REG_VAL_CHKSUM	(0x39U)

#define XBIR_ERROR_BOOT_IMG_STATUS_LEN	(0x40U)
#define XBIR_ERROR_INVALID_JSON_OBJ		(0x41U)
#define XBIR_ERROR_JSON_IMG_A_BOOTABLE_VAL	(0x42U)
#define XBIR_ERROR_JSON_IMG_B_BOOTABLE_VAL	(0x43U)
#define XBIR_ERROR_JSON_REQ_IMG_NAME		(0x44U)
#define XBIR_ERROR_JSON_OBJ_SEPARATOR	(0x45U)
#define XBIR_ERROR_IMG_A_UPLOAD		(0x46U)
#define XBIR_ERROR_IMG_B_UPLOAD		(0x47U)
#define XBIR_ERROR_SEND_SUCCESS_RESPONSE	(0x48U)
#define XBIR_ERROR_JSON_INCOMPLETE_IMG_CFG_REQ	(0x49U)

#define XBIR_ERROR_SD_CONFIG		(0x50U)
#define XBIR_ERROR_SD_CONFIG_INIT	(0x51U)
#define XBIR_ERROR_SD_WRITE		(0x52U)
#define XBIR_ERROR_SD_READ		(0x53U)
#define XBIR_ERROR_SD_ERASE		(0x54U)
#define XBIR_ERROR_SD_CARD_INIT		(0x55U)
#define XBIR_ERROR_MMC_PART_CONFIG	(0x56U)

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif	/* XBIR_ERR_H */
