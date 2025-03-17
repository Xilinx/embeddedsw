/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/
/*************************************************************************************************/
/**
*
* @file xkeywrap.h
*
* This file contains declarations for xkeywrap.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/24/25 Initial release
*
* </pre>
*
*
**************************************************************************************************/
/**
* @addtogroup xkeywrap_server_apis Keywrap Server APIs
* @{
*/

#ifndef XKEYWRAP_H_
#define XKEYWRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xrsa_padding.h"
#include "xaes.h"
#include "xasufw_dma.h"
#include "xasu_keywrapinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************ Variable Definitions ***************************************/

/************************************ Macro Definitions ******************************************/

/************************************ Type Definitions *******************************************/

/************************************ Function Prototypes ****************************************/
s32 XKeyWrap(const XAsu_KeyWrapParams *KeyWrapParamsPtr, XAsufw_Dma *AsuDmaPtr,
                XSha *ShaInstancePtr, XAes *AesInstancePtr);
s32 XKeyUnwrap(const XAsu_KeyWrapParams *KeyUnwrapParamsPtr, XAsufw_Dma *AsuDmaPtr,
                XSha *ShaInstancePtr, XAes *AesInstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XKEYWRAP_H_ */
/** @} */
