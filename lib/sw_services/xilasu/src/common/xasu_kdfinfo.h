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

/************************************ Constant Definitions ***************************************/
/* KDF module command IDs */
#define XASU_KDF_COMPUTE_SHA2_CMD_ID	(0U) /**< Command ID for KDF compute for SHA2 command */
#define XASU_KDF_COMPUTE_SHA3_CMD_ID	(1U) /**< Command ID for KDF compute for SHA3 command */
#define XASU_KDF_KAT_CMD_ID				(2U) /**< Command ID for KDF KAT command */
#define XASU_KDF_GET_INFO_CMD_ID		(3U) /**< Command ID for KDF Get Info command */

#define XASU_KDF_MAX_CONTEXT_LEN		(1024U) /**< Maximum context length */

/************************************** Type Definitions *****************************************/
/**
 * @brief This structure contains KDF params info
 */
typedef struct {
	u64 KeyInAddr; /**< Address of the input key buffer */
	u64 ContextAddr; /**< Address of the buffer holding the fixed input data */
	u64 KeyOutAddr; /**< Address of the buffer to hold the keying material output from KDF */
	u32 KeyInLen; /**< Length of the input key */
	u32 ContextLen; /**< Length of the Context */
	u32 KeyOutLen; /**< Length of the keying material output to be generated from KDF */
	u8 ShaType; /**< Hash family type (SHA2/SHA3) */
	u8 ShaMode; /**< Sha mode (SHA256/SHA384/SHA512/SHAKE256) */
} XAsu_KdfParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_KDFINFO_H_*/
/** @} */
