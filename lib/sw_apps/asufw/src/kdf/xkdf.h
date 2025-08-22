/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xkdf.h
*
* This file contains declarations for xkdf.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ma   01/15/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xkdf_server_apis KDF Server APIs
* @{
*/

#ifndef XKDF_H_
#define XKDF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xsha.h"
#include "xasufw_dma.h"
#include "xasu_kdfinfo.h"
#include "xaes.h"

#ifdef XASU_KDF_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/

/************************************ Function Prototypes ****************************************/
s32 XKdf_Generate(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr, const XAsu_KdfParams *KdfParams);
s32 XKdf_CmacGenerate(XAsufw_Dma *DmaPtr, const XAsu_KdfParams *KdfParams, u32 AesKeySrc);
#endif /* XASU_KDF_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XKDF_H_ */
/** @} */
