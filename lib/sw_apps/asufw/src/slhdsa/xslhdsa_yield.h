/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_yield.h
*
* This file contains cooperative yield support for long-running SLH-DSA key generation
* and signing in ASUFW firmware.
*
* Declares the yield context stored on the SLH-DSA instance, slice budget constants,
* and the session begin, end, poll and resume interfaces used by the command handlers
* and the tree primitives.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   09/09/26 Initial release
*       kk   09/29/26 Added yield parking by leaf budget with task resume and a KeyGen
*                     stage for the pairwise consistency test, and held the client keygen
*                     request on the yield context
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/

#ifndef XSLHDSA_YIELD_H_
#define XSLHDSA_YIELD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xslhdsa_addr.h"
#include "xasufw_status.h"
#include "xasu_sharedmem.h"
#include "xasu_slhdsainfo.h"
#include "xasu_keymanagerinfo.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
#define XSLHDSA_MAX_YIELD_SLICES	(8000U)	/**< Livelock cap on cooperative yield slices. */
#define XSLHDSA_YIELD_UNIT_XMSS_LEAF	(0U) /**< Budget for the XMSS treehash leaf loop. */
#define XSLHDSA_YIELD_UNIT_FORS_LEAF	(1U) /**< Budget for the FORS treehash leaf loop. */
#define XSLHDSA_KEYGEN_STAGE_NONE	(0U) /**< KeyGen not yet started or finished. */
#define XSLHDSA_KEYGEN_STAGE_PREFIX_DONE	(1U) /**< KeyGen SK/PK prefix assembled. */
#define XSLHDSA_KEYGEN_STAGE_PWCT_SIGN	(2U) /**< KeyGen pair built; PWCT sign in progress. */
#define XSLHDSA_SIGN_STAGE_NONE		(0U) /**< Sign not yet started or finished. */
#define XSLHDSA_SIGN_STAGE_PRF_DONE	(1U) /**< Sign PRF_msg stage complete. */
#define XSLHDSA_SIGN_STAGE_HMSG_DONE	(2U) /**< Sign H_msg digest stage complete. */

/************************************** Type Definitions *****************************************/
/** This structure contains cooperative-yield session and slice state. */
typedef struct {
	u32 ReqId;		/**< Active IPI request identifier. */
	u32 Slices;		/**< Cooperative yield slices consumed (livelock guard). */
	u32 XmssLeaves;		/**< XMSS leaves built in the current slice. */
	u32 ForsLeaves;		/**< FORS leaves built in the current slice. */
	u8 Active;		/**< Non-zero while a yield session is active. */
	u8 HashDmaResume;	/**< Non-zero: SHA absorb of M is done; next
				  *  PRF_msg/H_msg only finishes. */
	u8 HashWipeFail;	/**< Non-zero: a PRF_msg/H_msg buffer wipe failed during a
				  *  SHA message DMA wait; reported at session end. */
} XSlhDsa_YieldSession;

/** This structure contains firmware-side SLH-DSA key pair generation parameters. */
typedef struct {
	u64 PubKeyAddr;		/**< Address of the output public key */
	u64 PrivKeyAddr;	/**< Address of the output secret key */
	u32 PubKeyLen;		/**< Length of the public key buffer in bytes */
	u32 PrivKeyLen;		/**< Length of the secret key buffer in bytes */
	u8  ParamSet;		/**< Parameter set (XASU_SLHDSA_PARAM_*) */
} XSlhDsa_KeyGenParams;

/** This structure contains KeyGen buffers that must outlive yield slices. */
typedef struct {
	XAsu_SlhdsaKeyGenParams Params;	/**< First-slice client keygen request, reused on resume. */
	u8 Root[XSLHDSA_MAX_N];		/**< Top XMSS root. */
	u8 Stage;			/**< Key-generation progress stage. */
} XSlhDsa_YieldKeygen;

/** This structure contains hedged-sign state that must outlive yield slices. */
typedef struct {
	XAsu_SlhdsaSignParams Params;	/**< First-slice IPI sign parameters, reused on resume. */
	u8 AddRand[XSLHDSA_MAX_N];	/**< Hedged add_rand. */
	u8 Stage;			/**< Signature-generation progress stage. */
} XSlhDsa_YieldSign;

/** This structure contains verify parameters that must outlive a SHA message DMA wait. */
typedef struct {
	XAsu_SlhdsaSignVerifyParams Params;
				/**< First-slice IPI verify parameters, reused on resume. */
} XSlhDsa_YieldVerify;

/** This structure contains XMSS treehash, emit and auth-path resume state. */
typedef struct {
	u8 *EmitLayerSign;	/**< Saved layer signature buffer pointer. */
	u8 *AuthPathSigBase;	/**< Saved signature base pointer for auth-path resume. */
	u8 *AuthPathOut;	/**< Saved next auth-path node output pointer. */
	u8 *RootOut;		/**< Saved Root output pointer for XmssSubtreeRoot resume. */
	u32 FirstLeaf;		/**< Saved FirstLeaf for XmssSubtreeRoot resume. */
	u32 SubHeight;		/**< Saved SubHeight for XmssSubtreeRoot resume. */
	u32 LeafSeq;		/**< Saved leaf sequence counter in XMSS treehash. */
	u32 LeafIdx;		/**< Saved current leaf index in XMSS treehash. */
	s32 StackDepth;		/**< Saved XMSS treehash stack depth. */
	u32 EmitIdxLeaf;	/**< Saved leaf index for XmssEmitSig resume. */
	u32 AuthPathIdxLeaf;	/**< Saved leaf index for auth-path walk. */
	u32 AuthPathLevelIdx;	/**< Saved level index in auth-path loop. */
	u8 NodeStack[XSLHDSA_XMSS_STACK_DEPTH][XSLHDSA_MAX_N];
				/**< Saved XMSS treehash node stack. */
	u8 Resume;		/**< Non-zero when XmssSubtreeRoot yield is in progress. */
	u8 EmitResume;		/**< Non-zero when XmssEmitSig yield is in progress. */
	u8 EmitWotsDone;	/**< Non-zero when WOTS+ emit finished before auth path. */
	u8 AuthPathResume;	/**< Non-zero when XmssWriteAuthPathNodes yield is in
				  *  progress. */
} XSlhDsa_YieldXmss;

/** This structure contains FORS treehash and emit resume state. */
typedef struct {
	u8 *EmitSigBase;	/**< Saved signature base pointer for ForsEmitSig resume. */
	u8 *EmitDst;		/**< Saved next signature write pointer in emit loop. */
	u32 TreeBaseLeaf;	/**< Saved BaseLeaf for ForsSubtreeRoot resume. */
	u32 TreeSubHeight;	/**< Saved SubHeight for ForsSubtreeRoot resume. */
	u32 TreeSeq;		/**< Saved leaf sequence counter in FORS treehash. */
	u32 TreeLeafIdx;	/**< Saved current leaf index in FORS treehash. */
	s32 TreeStackDepth;	/**< Saved FORS treehash stack depth. */
	u32 EmitTreeIdx;	/**< Saved FORS tree index in emit loop. */
	u32 EmitLevelIdx;	/**< Saved auth-path level index in emit loop. */
	u8 TreeNodeStack[XSLHDSA_FORS_STACK_DEPTH][XSLHDSA_MAX_N];
				/**< Saved FORS treehash node stack. */
	u8 TreeResume;		/**< Non-zero when ForsSubtreeRoot yield is in progress. */
	u8 EmitResume;		/**< Non-zero when ForsEmitSig yield is in progress. */
} XSlhDsa_YieldFors;

/** This structure contains hypertree upper-layer emit resume state. */
typedef struct {
	u64 IdxTree;		/**< Saved idx_tree for hypertree walk. */
	u8 *SigBase;		/**< Saved signature HT section base pointer. */
	u8 *LayerSigOut;	/**< Saved next layer signature output pointer. */
	u32 LevelIdx;		/**< Saved hypertree layer index. */
	u32 IdxLeaf;		/**< Saved idx_leaf for hypertree walk. */
	u8 LayerMsg[XSLHDSA_MAX_N];
				/**< Stable hypertree layer message (FORS PK then roots). */
	u8 Resume;		/**< Non-zero when HtEmitUpperLayers yield is in progress. */
	u8 EmitPending;		/**< Non-zero when XmssEmitSig must finish on resume. */
} XSlhDsa_YieldHt;

/**
 * This structure contains saved cooperative-yield context for SLH-DSA long operations.
 */
typedef struct {
	XSlhDsa_YieldSession Session;	/**< Slice budget state and request identity. */
	XSlhDsa_YieldKeygen Keygen;	/**< Key-generation buffers and stage. */
	XSlhDsa_YieldSign Sign;		/**< Hedged-sign add_rand and stage. */
	XSlhDsa_YieldVerify Verify;	/**< Verify request snapshot across a SHA DMA wait. */
	XSlhDsa_YieldXmss Xmss;		/**< XMSS treehash, emit and auth-path resume. */
	XSlhDsa_YieldFors Fors;		/**< FORS treehash and emit resume. */
	XSlhDsa_YieldHt Ht;		/**< Hypertree upper-layer emit resume. */
} XSlhDsa_YieldCtx;

/** This typedef is to create alias name for _XSlhDsa_Instance. */
typedef struct _XSlhDsa_Instance XSlhDsa_Instance;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
void XSlhDsa_YieldSessionBegin(XSlhDsa_Instance *InstPtr, u32 ReqId);
s32 XSlhDsa_YieldSessionEnd(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_YieldPoll(XSlhDsa_Instance *InstPtr, u32 Unit);
s32 XSlhDsa_YieldPollLeaf(XSlhDsa_Instance *InstPtr, u32 Unit);
s32 XSlhDsa_YieldScheduleResume(XSlhDsa_Instance *InstPtr,
				const XAsu_ReqBuf *ReqBuf, u32 ReqId);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_SLHDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XSLHDSA_YIELD_H_ */
/** @} */
