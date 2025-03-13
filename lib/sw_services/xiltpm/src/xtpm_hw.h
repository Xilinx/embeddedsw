/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.00  tri   03/13/25 Initial release
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
#define XTPM_SPI_SELECT					(0x00U)
#define	XTPM_ACCESS_VALID				(0x80U)
#define	XTPM_ACCESS_ACT_LOCAL			(0x20U)
#define	XTPM_ACCESS_REQ_USE				(0x02U)
#define	XTPM_STS_VALID 					(0x80U)
#define	XTPM_STS_CMD_READY				(0x40U)
#define	XTPM_STS_GO 					(0x20U)
#define	XTPM_STS_DATA_AVAIL				(0x10U)
#define XTPM_TX_HEAD_SIZE				(4U)
#define XTPM_RX_HEAD_SIZE				(10U)
#define XTPM_PCR_EXT_CMD_SIZE			(33U)
#define XTPM_START_CMD_SIZE				(12U)
#define XTPM_SELF_TEST_CMD_SIZE			(12U)
#define XTPM_SPI_MAX_SIZE				(64U)
#define XTPM_PCR_EVENT_CMD_SIZE 		(29U)
#define XTPM_PCR_MAX_EVENT_SIZE			(1024U)
/* Maximum possible TPM request size in bytes */
#define XTPM_REQ_MAX_SIZE			(1024U)
#define XTPM_DIGEST_SIZE			(32U)
#define XTPM_MAX_PCR_CNT        	(24U)
/* Maximum possible TPM response size in bytes */
#define XTPM_RESP_MAX_SIZE			(4096U)
#define	XTPM_ACCESS 				(0x00U)
#define	XTPM_STS					(0x18U)
#define	XTPM_DATA_FIFO				(0x24U)

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
/* @} */
