/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_kdfinfo.h
 *
 * This file contains the KDF definitions which are common across the client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/15/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/

#ifndef XASU_KDFINFO_H_
#define XASU_KDFINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* KDF module command IDs */
#define XASU_KDF_GENERATE_SHA2_CMD_ID	(0U) /**< Command ID for KDF generate for SHA2 command */
#define XASU_KDF_GENERATE_SHA3_CMD_ID	(1U) /**< Command ID for KDF generate for SHA3 command */
#define XASU_KDF_KAT_CMD_ID		(2U) /**< Command ID for KDF KAT command */

#define XASU_KDF_MAX_CONTEXT_LEN		(1024U) /**< Maximum context length */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains KDF params info. */
typedef struct {
	u64 KeyInAddr; /**< Address of the input key buffer */
	u64 ContextAddr; /**< Address of the buffer holding the fixed input data */
	u64 KeyOutAddr; /**< Address of the buffer to hold the keying material output from KDF */
	u32 KeyInLen; /**< Length of the input key */
	u32 ContextLen; /**< Length of the Context */
	u32 KeyOutLen; /**< Length of the keying material output to be generated from KDF */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_SHA256 / XASU_SHA_MODE_SHA384 / XASU_SHA_MODE_SHA512 /
		* XASU_SHA_MODE_SHAKE256) */
} XAsu_KdfParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDFINFO_H_*/
