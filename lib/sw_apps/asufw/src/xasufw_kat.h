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
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_KAT_H
#define XASUFW_KAT_H

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

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, XAsufw_Dma *AsuDmaPtr, XAsufw_Resource ShaResource);
s32 XAsufw_RsaEncDecKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_EccCoreKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_RsaEccKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_AesGcmKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_AesDecryptDpaCmKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_P192EcdhKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_HmacOperationKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_KdfOperationKat(XAsufw_Dma *AsuDmaPtr);
s32 XAsufw_EciesOperationKat(XAsufw_Dma *AsuDmaPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_KAT_H */
/** @} */
