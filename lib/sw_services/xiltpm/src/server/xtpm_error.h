/******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpm_error.h
*
* This file contains the error codes related to TPM module
* platform including Versal device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   tri  03/13/25 Initial release
*       pre  09/23/25 Fixed misrac violations
* 1.2   pre  01/16/25 Fixed a Doxygen warning
*       pre  03/12/26 Added error code for invalid PCR index
*       pre  03/16/26 Added error code for invalid hash algorithm
*       pre  03/21/26 Added error codes related to GetPcrLog feature
*       pre  04/22/26 Added error code for TPM get access timeout
*
* </pre>
*
******************************************************************************/
#ifndef XTPM_ERROR_H_
#define XTPM_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup xiltpm_error_codes XilTPM Error Codes
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"

/************************** Constant Definitions ****************************/

/**
 *  The following table lists the Versal TPM library error codes.
 */
#ifdef PLM_TPM
enum {
	XTPM_ERR_SPIPS_INIT = 0x2,/**< 0x2 - SPI initialization failure */
	XTPM_ERR_SET_ACCESS,/**< 0x3 - TPM access set failure */
	XTPM_ERR_GET_ACCESS,/**< 0x4 - TPM access get failure.*/
	XTPM_ERR_START_UP,/**< 0x5 - TPM module startup failure.*/
	XTPM_ERR_SELF_TEST,/**< 0x6 - TPM module self test failure.*/
	XTPM_ERR_MEASURE_ROM,/**< 0x7 - Error while ROM digest transfer to TPM */
	XTPM_ERR_MEASURE_PLM,/**<0x8 - Error while PLM digest transfer to TPM */
	XTPM_ERR_MEASURE_PARTITION,/**<0x9 - Error while PLM digest transfer to TPM */
	XTPM_ERR_RESP_POLLING,/**<0xA - Error in polling response buffer */
	XTPM_ERR_DATA_TRANSFER,/**<0xB - Failure at TPM module data transfer */
	XTPM_ERR_SPIPS_CONFIG,/**<0xC - Failure at SPIPS configuration */
	XTPM_ERR_SPIPS_CFG_INIT,/**<0xD - SPIPS configuration initialization failure */
	XTPM_ERR_SPIPS_SELF_TEST,/**<0xE - SPIPS self-test failure */
	XTPM_ERR_SPIPS_FIFO_WRITE,/**<0xF - SPIPS write failure to FIFO buffer */
	XTPM_ERR_SPIPS_FIFO_READ,/**<0x10 - SPIPS read failure from FIFO buffer */
	XTPM_ERR_DATA_TX_LENGTH_LIMIT,/**<0x11 - TPM module data transfer length limit error */
	XTPM_ERR_SPIPS_POLLING_TRANSFER,/**<0x12 - Polling failure at SPIPS */
	XTPM_ERR_SPIPS_SET_OPTIONS,/**<0x13 - SPIPS set options failure */
	XTPM_ERR_SPIPS_SET_CLK_PRESCALER,/**<0x14 - SPIPS set clock prescaler failure */
	XTPM_ERR_SPIPS_TRANSFER,/**<0x15 - Data transfer failure to TPM */
	XTPM_ERR_PCR_INDEX_INVALID,/**<0x16 - Invalid PCR index */
	XTPM_ERR_HASH_ALGO_INVALID, /**<0x17 - Invalid hash algorithm */
	XTPM_ERR_DATA_SIZE_INVALID, /**<0x18 - Invalid data size for PCR event */
	XTPM_ERR_PCR_LOG_UPDATE, /**<0x19 - Error while updating PCR log */
	XTPM_PCR_ERR_INVALID_LOG_READ_REQUEST, /**<0x1A - Invalid PCR log read request */
	XTPM_ERR_INVALID_ADDR_RANGE, /**<0x1B - Invalid address range for PCR log read */
	XTPM_ERR_ACCESS_TIMEOUT, /**<0x1C - Timeout while waiting for TPM access */
};
#endif	/* PLM_TPM */

/** @} End of xiltpm_error_codes group */

#ifdef __cplusplus
}
#endif
#endif	/* XTPM_ERROR_H_ */