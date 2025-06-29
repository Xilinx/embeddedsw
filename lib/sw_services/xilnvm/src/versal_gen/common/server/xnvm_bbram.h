/*******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_bbram.h
*
* This file contains NVM library BBRAM API's declaration
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
* 2.1	am   08/19/2020 Resolved MISRA C violations
*       am   10/13/2020 Resolved MISRA C violations
*       ana  10/15/2020 Updated doxygen comments
* 2.3   am   11/23/2020 Resolved MISRA C violation
* 3.1   skg  10/23/2022 Added In body comments for APIs
*       kal  03/08/2023 Added new error code XNVM_BBRAM_ERROR_IN_DMA_XFER
* 3.3   ng   11/22/2023 Fixed doxygen grouping
* 3.5   hj   03/26/2025 Added new error code XNVM_BBRAM_ERROR_GP_WRITE_IN_SB
*
* </pre>
*
*******************************************************************************/
#ifndef XNVM_BBRAM_H
#define XNVM_BBRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"

/*************************** Constant Definitions *****************************/
/**
 * @{ AES Key size
 */

/**< AES Key size in bytes */
#define XNVM_BBRAM_AES_KEY_SIZE          XNVM_256_BITS_AES_KEY_LEN_IN_BYTES
#define XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS (XNVM_BBRAM_AES_KEY_SIZE / sizeof(u32))

/**
 * @{ BBRAM error codes
 */
/**< BBRAM API error codes */
#define XNVM_EFUSE_BBRAM_TAG                       (u32)(0x8100U) /**< BBRAM ERRORs Offset */
#define XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT  (XNVM_EFUSE_BBRAM_TAG | 0x00U)
						/**< Error in enabling BBRAM Program mode */
#define XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT     (XNVM_EFUSE_BBRAM_TAG | 0x02U)
						/**< Error in AES CRC Done */
#define XNVM_BBRAM_ERROR_AES_CRC_MISMATCH         (XNVM_EFUSE_BBRAM_TAG | 0x03U)
						/**< Error in AES CRC verification */
#define XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE      (XNVM_EFUSE_BBRAM_TAG | 0x04U)
						/**< Error in BBRAM user data locking */
#define XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED    (XNVM_EFUSE_BBRAM_TAG | 0x05U)
						/**< Error in BBRAM user data write, as BBRAM_8 reg is locked */
#ifdef VERSAL_2VE_2VM
#define XNVM_BBRAM_ERROR_USER_DATA_WRITE_IN_SB    (XNVM_EFUSE_BBRAM_TAG | 0x06U)
						/**< Error, in BBRAM8 write in Secure Boot */
#endif

#define XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT          (XNVM_EFUSE_BBRAM_TAG | 0x10U)
						/**< Error in BBRAM_ZEROIZE */
#define XNVM_BBRAM_ERROR_IN_DMA_XFER              (XNVM_EFUSE_BBRAM_TAG | 0x20U)
						/**< Error in copying user key to PMC memory using DMA */
#define XNVM_BBRAM_INVALID_PARAM                  (XNVM_EFUSE_BBRAM_TAG | 0x30U)
						/**< Error, invalid parameter */
#define XNVM_BBRAM_ERROR_AES_INVALID_KEY_SIZE     (XNVM_EFUSE_BBRAM_TAG | 0x40U)
						/**< Error, invalid AES key size */
/***************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/

/* Writes AES key to BBRAM */
int XNvm_BbramWriteAesKey(const u8* Key, u16 KeyLen);

/* Locks user data and prevent writes */
int XNvm_BbramLockUsrDataWrite(void);

/* Write 32-bit general purpose user data */
int XNvm_BbramWriteUsrData(u32 GeneralPurposeData);

/* Read 32-bit user data */
u32 XNvm_BbramReadUsrData(void);

/* Zeroize BBRAM memory */
int XNvm_BbramZeroize(void);

#ifdef VERSAL_2VE_2VM
/* Write Configuration limiter parameters in BBRAM */
int XNvm_BbramWriteConfigLimiterParams(u32 ClEnFlag, u32 ClMode, u32 MaxNumOfConfigs);
#endif

#ifdef __cplusplus
}
#endif

#endif

/* @} */
