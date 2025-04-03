/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_aes_common.h
 *
 * This file contains the AES function prototypes which are common across the client and server.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   10/03/24 Initial release
 * 1.1   am   03/14/25 Renamed XAsu_AesValidateIv() to XAsu_AesValidateIvParams() and
 *                     XAsu_AesValidateTag() to XAsu_AesValidateTagParams()
 *       am   04/03/25 Optimized XASU_AES_IS_AAD_SUPPORTED_MODE macro for AAD update
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_aes_common_apis AES Common APIs
 * @{
*/

#ifndef XASU_AES_COMMON_H_
#define XASU_AES_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xasu_def.h"
#include "xasu_aesinfo.h"

/************************** Constant Definitions *************************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/** This macro checks if AAD processing is allowed for the given AES engine mode. */
#define XASU_AES_IS_AAD_SUPPORTED_MODE(EngineMode) \
	(((EngineMode) == XASU_AES_GCM_MODE) && \
	((EngineMode) == XASU_AES_CMAC_MODE) && \
	((EngineMode) == XASU_AES_CCM_MODE))

/** This macro checks if given data length is invalid. */
#define XASU_AES_IS_INVALID_DATALEN(DataLen) \
	(((DataLen) == 0U) || ((DataLen) > XASU_ASU_DMA_MAX_TRANSFER_LENGTH))

/************************************ Variable Definitions ***************************************/

/************************************ Function Prototypes ****************************************/
s32 XAsu_AesValidateIvParams(u8 EngineMode, u64 IvAddr, u32 IvLen);
s32 XAsu_AesValidateTagParams(u8 EngineMode, u64 TagAddr, u32 TagLen);

#ifdef __cplusplus
}
#endif

#endif  /* XASU_AES_COMMON_H_ */
/** @} */
