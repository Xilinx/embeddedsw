/*******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 3.6   aa   07/24/2025 Remove unused macros
* 3.7   mb   03/18/2026 Add support for temperature and voltage checks
*                       before efuse programming
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

#define XNVM_AES_KEY_SIZE_IN_WORDS	(XNVM_256_BITS_AES_KEY_LEN_IN_BYTES / 4U)
#define XNVM_WORD_LEN			(4U)
/**
 * @}
 * @endcond
 */

 /**
 * @name Timeout in terms of number of times status register polled to check eFuse Crc check id done.
 * @{
 */
#define XNVM_POLL_TIMEOUT				(0x400U) /**< Poll timeout during CRC verification  */
/** @} */

/***************************** Type Definitions *******************************/

/**
 * eFuse operation mode
 */
typedef enum {
	XNVM_EFUSE_MODE_RD, /**< eFuse read mode */
	XNVM_EFUSE_MODE_PGM /**< eFuse program mode */
} XNvm_EfuseOpMode;

/**
 * eFuse read mode
 */
typedef enum {
	XNVM_EFUSE_NORMAL_RD, /**< eFuse normal read */
	XNVM_EFUSE_MARGIN_RD /**< eFuse margin read */
} XNvm_EfuseRdMode;

/*************************** Function Prototypes ******************************/
u32 XNvm_AesCrcCalc(const u32 *Key);
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc);
int XNvm_ZeroizeAndVerify(u8 *DataPtr, const u32 Length);
u32 XNvm_EfuseReadReg(u32 BaseAddress, u32 RegOffset);
void XNvm_EfuseWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data);
int XNvm_EfuseLockController(void);
int XNvm_EfuseUnlockController(void);
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op, XNvm_EfuseRdMode RdMode, u32 EfuseClkFreq);
int XNvm_EfuseResetReadMode(void);
int XNvm_EfuseDisableProgramming(void);
#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
int XNvm_EfuseTempAndVoltChecks(XSysMon *SysMonInstPtr);
#endif /* XNVM_ENABLE_ENV_MONITOR_CHECKS */
#ifdef __cplusplus
}
#endif

#endif		// XNVM_UTILS_H

/** @} */
