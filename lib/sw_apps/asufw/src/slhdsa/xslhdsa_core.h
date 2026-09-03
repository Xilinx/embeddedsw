/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_core.h
*
* This file contains the SLH-DSA FORS, WOTS+, XMSS and hypertree API for ASUFW firmware.
*
* These interfaces are internal to the SLH-DSA module and are consumed by xslhdsa.c and
* by the tree primitives themselves.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/

#ifndef XSLHDSA_CORE_H_
#define XSLHDSA_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xslhdsa.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
* Hypertree index pair parsed from the H_msg digest suffix.
*
* IdxTree holds the reduced tree index (idx_tree); IdxLeaf holds the reduced leaf index
* (idx_leaf) for the bottom hypertree layer per FIPS 205 Section 9.
*/
typedef struct
{
	u64 IdxTree;  /**< Reduced hypertree tree index (idx_tree). */
	u64 IdxLeaf;  /**< Reduced hypertree leaf index (idx_leaf) for layer 0. */
} XSlhDsa_HtIndices;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XSlhDsa_Base2b(const u8 *Input, u32 DigitWidth, u32 OutLen, u32 *DigitsOut);
s32 XSlhDsa_ForsEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Md, u8 *Dst);
s32 XSlhDsa_ForsPkFromSig(XSlhDsa_Instance *InstPtr, const u8 *SignFors,
			       const u8 *Md, u8 *PkFors);

s32 XSlhDsa_WotsChainFromZero(XSlhDsa_Instance *InstPtr, u32 Steps, u8 *Out);
s32 XSlhDsa_WotsEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Msg, u8 *Dst);
s32 XSlhDsa_WotsPkFromSign(XSlhDsa_Instance *InstPtr, const u8 *WotsSign,
				const u8 *Msg, u8 *WotsPk);

s32 XSlhDsa_XmssSubtreeRoot(XSlhDsa_Instance *InstPtr, u32 FirstLeaf,
				 u32 SubHeight, u8 *Root);
s32 XSlhDsa_XmssEmitSig(XSlhDsa_Instance *InstPtr, const u8 *Msg,
			     u32 IdxLeaf, u8 *LayerSign);
s32 XSlhDsa_XmssPkFromSign(XSlhDsa_Instance *InstPtr, u32 IdxLeaf,
				const u8 *SignXmss, const u8 *Msg, u8 *Node);

void XSlhDsa_HtParseDigest(const XSlhDsa_Param *ParamPtr, const u8 *Digest,
				XSlhDsa_HtIndices *HtIndices);
s32 XSlhDsa_HtEmitSig(XSlhDsa_Instance *InstPtr,
			   const XSlhDsa_HtIndices *HtIndices, u8 *LayerMsg,
			   u8 *SigHt);
s32 XSlhDsa_HtVerify(XSlhDsa_Instance *InstPtr, const u8 *Msg,
			  const u8 *SignHt, const XSlhDsa_HtIndices *HtIndices);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_SLHDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XSLHDSA_CORE_H_ */
/** @} */
