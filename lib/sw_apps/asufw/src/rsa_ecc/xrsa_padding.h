/**************************************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xrsa_padding.h
* This file contains declarations for xrsa_padding.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   ss   02/04/25 Initial Release
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xrsa_padding_apis RSA Padding APIs
* @{
*/

#ifndef XRSA_PADDING_H_
#define XRSA_PADDING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xrsa.h"
#include "xsha.h"
#include "xasu_rsainfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XRsa_OaepEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr);
s32 XRsa_OaepDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		    const XAsu_RsaOaepPaddingParams *PaddingParamsPtr);
s32 XRsa_PssEncode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		   const XAsu_RsaPaddingParams *PaddingParamsPtr);
s32 XRsa_PssDecode(XAsufw_Dma *DmaPtr, XSha *ShaInstancePtr,
		   const XAsu_RsaPaddingParams *PaddingParamsPtr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XRSA_PADDING_H_ */
/** @} */
