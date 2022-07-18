/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_commondefs.h
* @addtogroup xnvm_api_ids XilNvm API IDs
* @{
*
* @cond xnvm_internal
* This file contains the xilnvm Versal_Net API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  01/05/22 Initial release
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_COMMON_DEFS_H
#define XNVM_COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/************************** Constant Definitions ****************************/
/**@cond xnvm_internal
 * @{
 */
/* Key and Iv length definitions for Versal eFuse */
#define XNVM_EFUSE_AES_KEY_LEN_IN_WORDS			(8U)
#define XNVM_EFUSE_IV_LEN_IN_WORDS                      (3U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS		(8U)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
} XNvm_PpkHash;

typedef struct {
	u32 Iv[XNVM_EFUSE_IV_LEN_IN_WORDS];
} XNvm_Iv;

typedef struct {
	u32 Key[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
} XNvm_AesKey;

typedef enum {
	XNVM_EFUSE_META_HEADER_IV_RANGE = 0,
	XNVM_EFUSE_BLACK_IV,
	XNVM_EFUSE_PLM_IV_RANGE,
	XNVM_EFUSE_DATA_PARTITION_IV_RANGE
} XNvm_IvType;

typedef enum {
	XNVM_EFUSE_PPK0 = 0,
	XNVM_EFUSE_PPK1,
	XNVM_EFUSE_PPK2
} XNvm_PpkType;

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_COMMON_DEFS_H */
