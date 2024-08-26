/*******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_utils.h
* @addtogroup xnvm_util_apis XilNvm Utils APIs
* @{
*
* @cond xnvm_internal
* This file contains NVM library utility functions APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kpt  08/14/2024 Initial release
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_UTILS_H
#define XNVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xnvm_efuse.h"

/*************************** Constant Definitions *****************************/
/**@cond xnvm_internal
 * @{
 */
 /**< Aes key lengths in different sizes*/
#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS
#define XNVM_AES_KEY_SIZE_IN_WORDS	(XNVM_256_BITS_AES_KEY_LEN_IN_BYTES / 4U)
#define XNVM_WORD_LEN			(4U)
/**
 * @}
 * @endcond
 */

/***************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/
u32 XNvm_AesCrcCalc(const u32 *Key);
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc);
int XNvm_ZeroizeAndVerify(u8 *DataPtr, const u32 Length);
u32 XNvm_EfuseReadReg(u32 BaseAddress, u32 RegOffset);
void XNvm_EfuseWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data);
int XNvm_EfuseLockController(void);
int XNvm_EfuseUnlockController(void);
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
			XNvm_EfuseRdMode RdMode);
int XNvm_EfuseResetReadMode(void);
int XNvm_EfuseDisableProgramming(void);

#ifdef __cplusplus
}
#endif

#endif		// XNVM_UTILS_H

/* @} */
