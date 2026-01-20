/******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xtpm_hw.h
*
* This file contains TPM library SPI specification definitions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  tri  03/13/25 Initial release
*       pre  09/23/25 Removed unused macros
* 1.2   pre  01/16/25 Updated comments
*
* </pre>
*
*******************************************************************************/
#ifndef XTPM_HW_H_
#define XTPM_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_TPM
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XTPM_SPI_SELECT					(0x00U) /**< Slave select 0 */
#define	XTPM_ACCESS_VALID				(0x80U) /**< Access register valid status mask */
#define	XTPM_ACCESS_ACT_LOCAL			(0x20U) /**< Access register active locality mask */
#define	XTPM_ACCESS_REQ_USE				(0x02U) /**< Access register request use mask */
#define	XTPM_STS_VALID 					(0x80U) /**< Status register valid status mask */
#define	XTPM_STS_CMD_READY				(0x40U) /**< Status register command ready mask */
#define	XTPM_STS_GO 					(0x20U) /**< Status register go mask */
#define	XTPM_STS_DATA_AVAIL				(0x10U) /**< Status register data available mask */
#define XTPM_TX_HEAD_SIZE				(4U) /**< TPM SPI transmit header size */
#define XTPM_RX_HEAD_SIZE				(10U) /**< TPM SPI receive header size */
#define XTPM_START_CMD_SIZE				(12U) /**< TPM startup command size */
#define XTPM_SELF_TEST_CMD_SIZE			(12U) /**< TPM self-test command size */
#define XTPM_SPI_MAX_SIZE				(64U) /**< Maximum SPI transfer size in bytes */
#define XTPM_PCR_MAX_EVENT_SIZE			(1024U) /**< Maximum PCR event size in bytes */
#define XTPM_REQ_MAX_SIZE			(1024U) /**< Maximum TPM request size in bytes */
#define XTPM_MAX_PCR_CNT        	(24U) /**< Maximum PCR count in TPM */
#define XTPM_RESP_MAX_SIZE			(4096U) /**< Maximum TPM response size in bytes */
#define	XTPM_ACCESS 				(0x00U) /**< TPM Access register address */
#define	XTPM_STS					(0x18U) /**< TPM Status register address */
#define	XTPM_DATA_FIFO				(0x24U) /**< TPM Data FIFO register address */

#ifndef SDT
	#define XTPM_SPI_DEVICE_ID	XPAR_XSPIPS_0_DEVICE_ID
#else
	#define XTPM_SPI_DEVICE_ID	XPAR_XSPIPS_0_BASEADDR
#endif

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif
#endif	/* PLM_TPM */
#endif	/* XTPM_HW_H_ */
