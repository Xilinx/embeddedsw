/*******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_utils.h
*
* This file contains NVM library utility functions APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
* 2.0	kal  03/08/2020 Added Utility APIs
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XNVM_UTILS_H
#define XNVM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xil_util.h"

/*************************** Constant Definitions *****************************/
#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS
#define XNVM_AES_KEY_SIZE_IN_WORDS	(XNVM_256_BITS_AES_KEY_LEN_IN_BYTES / 4)

/***************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/
u32 XNvm_ValidateAesKey(const char *Key);
u32 XNvm_ConvertHexToByteArray(const u8 * Bits, u8 * Bytes, u32 Len);
u32 XNvm_AesCrcCalc(u32 *Key);
u32 XNvm_ValidateHash(const char *Hash, u32 Len);
u32 XNvm_ConvertByteArrayToHex(const u8 * Bytes, u8 * Bits , u32 Len);
u32 XNvm_ValidateUserFuseStr(const char *UserFuseStr);
u32 XNvm_ValidateIvString(const char *IvStr);
#ifdef __cplusplus
}
#endif

#endif		// XNVM_UTILS_H
