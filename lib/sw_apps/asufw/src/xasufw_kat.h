/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_kat.h
 *
 * This file contains function declarations, macro and structure defines related to SHA, RSA, and
 * ECC KAT functionality in ASUFW.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_SHAKAT_H
#define XASUFW_SHAKAT_H

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

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ShaKat(XSha *XAsufw_ShaInstance, u32 QueueId, XAsufw_Resource ShaResource);
s32 XAsufw_RsaPubEncKat(u32 QueueId);
s32 XAsufw_EccCoreKat(XEcc *XAsufw_EccInstance, u32 QueueId);
s32 XAsufw_RsaEccKat(u32 QueueId);
s32 XAsufw_AesGcmKat(XAes *AesInstance, u32 QueueId);
s32 XAsufw_AesDecryptDpaCmKat(XAes *AesInstance, u32 QueueId);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_KAT_H */
/** @} */
