/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_kat.h
 *
 * This file contains function declarations, macro and structure defines related to SHA, RSA,
 * ECC, HMAC and ECIES KAT functionality in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   05/15/24 Initial release
 *       ma   07/08/24 Add task based approach at queue level
 *       ss   08/20/24 Added RSA KAT function.
 *       yog  08/21/24 Added kat support for ECC
 *       am   08/22/24 Added AES KAT
 *       am   08/24/24 Added AES DPA CM KAT support
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 * 1.1   ss   12/02/24 Added kat support for ECDH
 *       ma   12/12/24 Updated resource allocation logic
 *       yog  01/02/25 Added HMAC KAT
 *       ma   01/15/25 Added KDF KAT
 *       yog  02/21/25 Added ECIES KAT
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_KAT_H_
#define XASUFW_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsha.h"
#include "xaes.h"
#include "xasufw_resourcemanager.h"
#include "xrsa.h"
#include "xecc.h"
#include "xrsa_ecc.h"
#include "xhmac.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_KAT_STATUS_NOT_RUN	(0x0U) /** KAT not run status. */
#define XASUFW_KAT_STATUS_FAIL		(0x1U) /** KAT failed status. */
#define XASUFW_KAT_STATUS_PASS		(0x3U) /** KAT passed status. */

/**
 * @brief	Gets pointer to module information array.
 */
static inline XAsu_CryptoAlgInfo *GetModuleInfoPtr(void)
{
	return (XAsu_CryptoAlgInfo *)XASU_RTCA_MODULE_INFO_BASEADDR;
}

/**
 * @brief	Returns pointer to the KatStatus for the specified module.
 */
static inline u8 *XAsu_GetKatStatusPtr(u32 ModuleId)
{
	return &GetModuleInfoPtr()[ModuleId].KatStatus;
}

/** Set KAT status as passed. */
#define XASUFW_SET_KAT_PASSED(ModuleId) \
	(GetModuleInfoPtr()[ModuleId].KatStatus = XASUFW_KAT_STATUS_PASS)

/** Mark KAT status as failed. */
#define XASUFW_MARK_KAT_FAILED(ModuleId) \
	(GetModuleInfoPtr()[ModuleId].KatStatus = XASUFW_KAT_STATUS_FAIL)

/** Check whether KAT status is passed or not. */
#define XASUFW_IS_KAT_PASSED(ModuleId) \
	(GetModuleInfoPtr()[ModuleId].KatStatus == XASUFW_KAT_STATUS_PASS)

/************************************ Function Prototypes ****************************************/
s32 XAsufw_RunKatTaskHandler(void *KatTask);
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, XAsufw_Dma *AsuDmaPtr, XAsufw_Resource ShaResource,
	u32 ShaMode);
#ifdef XASU_RSA_PADDING_ENABLE
s32 XAsufw_RsaEncDecOaepOpKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_RsaPssSignGenAndVerifOpKat(XAsufw_Dma *AsuDmaPtr);
#endif
s32 XAsufw_EccCoreKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_RsaEccKat(XAsufw_Dma *AsuDmaPtr, u8 CurveType);
s32 XAsufw_AesOperationKat(XAsufw_Dma *AsuDmaPtr, u8 EngineMode);
s32 XAsufw_AesDpaCmKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_P256EcdhKat(XAsufw_Dma *AsuDmaPtr);
#ifdef XASU_HMAC_ENABLE
s32 XAsufw_HmacOperationKat(XAsufw_Dma *AsuDmaPtr);
#endif
#ifdef XASU_KDF_ENABLE
s32 XAsufw_KdfOperationKat(XAsufw_Dma *AsuDmaPtr);
#endif
#ifdef XASU_ECIES_ENABLE
s32 XAsufw_EciesOperationKat(XAsufw_Dma *AsuDmaPtr);
#endif
#ifdef XASU_KEYWRAP_ENABLE
s32 XAsufw_KeyWrapOperationKat(XAsufw_Dma *AsuDmaPtr);
#endif

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_KAT_H_ */
/** @} */
