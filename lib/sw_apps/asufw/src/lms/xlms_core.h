/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xlms_core.h
*
* This file contains the interface for LMS authentication methods for ASUFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ss   01/07/26 Initial release
*
* </pre>
*
* @note
*
**************************************************************************************************/
/**
* @addtogroup xasufw_lms_core_apis ASUFW LMS Core APIs
* @{
*/
#ifndef XLMS_CORE_H_
#define XLMS_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xlms_ots.h"
#include "xlms.h"
#include "xlms_hss.h"
#include "xsha.h"
#include "xasufw_dma.h"
#include "xasu_lmsinfo.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XLms_HssInit(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
		const XAsu_LmsHssSignVerifyParams *HssParamsPtr);
s32 XLms_HashMessage(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr, u64 DataAddr, u32 DataLen, u32 Mode);
s32 XLms_HssFinish(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr, u64 SignatureAddr, u32 SignatureLen);
s32 XLms_SignatureVerification(XSha *ShaInstPtr, XAsufw_Dma *DmaPtr,
	const XAsu_LmsHssSignVerifyParams *LmsSignVerifyParams);
s32 XLms_GetLmsHashAlgo(u32 PubAlgo, const u8* const PubKey, u32 *SignAlgo);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XLMS_CORE_H_ */
/** @} */
