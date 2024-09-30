/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xecc.h
* This file contains declarations for xecc.c file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -----------------------------------------------------------------------------
* 1.0   yog  06/19/2024 First Release
*       yog  08/19/2024 Received Dma instance from handler
*       yog  09/26/2024 Added doxygen groupings and fixed doxygen comments.
*
* </pre>
*
**************************************************************************************************/
/**
* @addtogroup xecc_server_apis ECC Server APIs
* @{
*/

#ifndef XECC_H_
#define XECC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasufw_sss.h"
#include "xasufw_dma.h"
#include "xecc_hw.h"

/************************************ Constant Definitions ***************************************/
#define XECC_CURVE_TYPE_NIST_P256	(0x0U) /**< Curve type value for P-256 curve */
#define XECC_CURVE_TYPE_NIST_P384	(0x1U) /**<  Curve type value for P-384 curve*/
#define XECC_P384_SIZE_IN_BYTES         (48U) /**< P384 Curve size in bytes */
#define XECC_P256_SIZE_IN_BYTES         (32U) /**< P256 Curve size in bytes */

/************************************** Type Definitions *****************************************/
typedef struct _XEcc_Config
	XEcc_Config; /**< This typedef is to create alias name for _XEcc_Config*/
typedef struct _XEcc XEcc; /**< This typedef is to create alias name for _XEcc */

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
XEcc *XEcc_GetInstance(u32 DeviceId);
s32 XEcc_Initialize(XEcc *InstancePtr);
s32 XEcc_GeneratePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, u64 PubKeyAddr);
s32 XEcc_ValidatePublicKey(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PubKeyAddr);
s32 XEcc_GenerateSignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			   u64 PrivKeyAddr, const u8 *EphemeralKeyPtr, u64 HashAddr, u32 HashBufLen,
			   u64 SignAddr);
s32 XEcc_VerifySignature(XEcc *InstancePtr, XAsufw_Dma *DmaPtr, u32 CurveType, u32 CurveLen,
			 u64 PubKeyAddr, u64 HashAddr, u32 HashBufLen, u64 SignAddr);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XECC_H */
/** @} */
