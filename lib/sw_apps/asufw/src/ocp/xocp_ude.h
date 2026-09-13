/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xocp.h
 *
 * This file contains declarations for xocp.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   yog  08/21/25 Initial release
 *       vm   09/15/26 Added UDE data sign and verify APIs and the UDE public key cache
 *                     invalidation used by provisioning
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xocp_ude_server_apis OCP UDE server APIs
* @{
*/
#ifndef XOCP_UDE_H_
#define XOCP_UDE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xasu_ocpinfo.h"
#include "xasufw_dma.h"
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************** Type Definitions *****************************************/

/************************************ Function Prototypes ****************************************/
s32 XOcp_GenerateUdeResponse(XAsufw_Dma *DmaPtr, const XAsu_OcpUdeParams *OcpUdeParamsPtr);
s32 XOcp_EncryptUdeKeys(XAsufw_Dma *DmaPtr, u32 UdeKeyId, u8 *EncPvtKey, u64 CertBufAddr,
			u32 CertBufMaxSize, u32 *CertSize);
s32 XOcp_GetUdeSlotInfo(u32 UdeKeyId, u32 *UdeUserKeyAddrPtr, u8 *IvIncValPtr);
s32 XOcp_UdeSignData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, u8 *SignatureOut);
s32 XOcp_UdeVerifyData(XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen, const u8 *Signature);
void XOcp_InvalidateUdePubKeyCache(void);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_OCP_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XOCP_UDE_H_ */
/** @} */
