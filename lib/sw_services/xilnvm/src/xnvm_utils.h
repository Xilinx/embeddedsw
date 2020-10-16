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
* 2.0	kal  02/28/2020 Added utility APIs XNvm_ValidateHash, XNvm_AesCrcCalc
*                       XNvm_ConvertBytesToBits and XNvm_ConvertBitsToBytes
*       kal  04/11/2020 Renamed conversion APIs to XNvm_ConvertHexToByteArray
*       		and XNvm_ConvertByteArrayToHex
*       kal  05/04/2020 Moved few utility functions to application and removed
*       		usage of conversion APIs as the same functionality is
*       		achieved by bit-wise operators.
* 2.1	am 	 08/19/2020 Resolved MISRA C violations.
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
#include "xil_util.h"

/*************************** Constant Definitions *****************************/
#define XNVM_256_BITS_AES_KEY_LEN_IN_BYTES (256U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_256_BITS_AES_KEY_LEN_IN_CHARS (XNVM_256_BITS_AES_KEY_LEN_IN_BYTES * 2U)
#define XNVM_128_BITS_AES_KEY_LEN_IN_BYTES (128U / XIL_SIZE_OF_BYTE_IN_BITS)
#define XNVM_128_BITS_AES_KEY_LEN_IN_CHARS (XNVM_128_BITS_AES_KEY_LEN_IN_BYTES * 2U)

#define XNVM_MAX_AES_KEY_LEN_IN_CHARS	XNVM_256_BITS_AES_KEY_LEN_IN_CHARS
#define XNVM_AES_KEY_SIZE_IN_WORDS	(XNVM_256_BITS_AES_KEY_LEN_IN_BYTES / 4U)
#define XNVM_IV_STRING_LEN		(24U)

/***************************** Type Definitions *******************************/

/*************************** Function Prototypes ******************************/
int XNvm_ValidateAesKey(const char *Key);
u32 XNvm_AesCrcCalc(const u32 *Key);

#ifdef __cplusplus
}
#endif

#endif		// XNVM_UTILS_H
