/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa.h
*
* This file contains the SLH-DSA instance state and top-level operation API for ASUFW
* firmware.
*
* Declares the instance and parameter-set types shared by every SLH-DSA source file,
* along with the instance management, cooperative yield, pre-hash, key generation, signature
* generation and signature verification interfaces.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   07/10/26 Initial release
*       kk   09/18/26 Size a yield slice by leaves built rather than by elapsed time
*       kk   09/23/26 Run a pairwise consistency test on every generated key pair
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/

#ifndef XSLHDSA_H_
#define XSLHDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xslhdsa_addr.h"
#include "xslhdsa_yield.h"
#include "xasu_slhdsainfo.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xsha.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
#define XSLHDSA_PK_N_FIELDS	(2U)	/**< PK fields: PK.seed || PK.root. */
#define XSLHDSA_PK_SEED_FIELD	(0U)	/**< PK.seed field index. */
#define XSLHDSA_PK_ROOT_FIELD	(1U)	/**< PK.root field index. */
#define XSLHDSA_SK_N_FIELDS	(4U)	/**< SK n-byte field count. */
#define XSLHDSA_SK_SEED_FIELD	(0U)	/**< SK.seed field index. */
#define XSLHDSA_SK_PRF_FIELD	(1U)	/**< SK.prf field index. */
#define XSLHDSA_SK_PKSEED_FIELD	(2U)	/**< PK.seed field index within SK. */
#define XSLHDSA_SK_PKROOT_FIELD	(3U)	/**< PK.root field index within SK. */

/*
 * SLH-DSA's scratch region, carved out of the shared crypto pool.
 *
 * The ASU is a 32-bit core while clients pass 64-bit DDR addresses that it cannot dereference, so
 * the key is copied into the scratch and the hashes run on it there. Messages are not buffered:
 * they stream from the client address through the hardware SHA engines. The only thing staged
 * here is the key, SK.seed || SK.prf || PK.seed || PK.root (or that trailing half for a
 * public-key load), sized for SK at the widest supported n so one region serves both loads.
 *
 * The staged key stays here for the whole sign or key generation, including across a yield. Every
 * command that uses this pool holds XASUFW_OVERLAY for its whole duration, and so do these two, so
 * no other user of the pool runs between slices. The key is copied in once, on the first slice,
 * from the caller address.
 */
#define XSLHDSA_SCRATCH_ADDR		(XASUFW_CRYPTO_ALGO_DATA_BLOCK)

/** Staged key region: the SK field count at the widest supported n. */
#define XSLHDSA_SCRATCH_LEN		(XSLHDSA_SK_N_FIELDS * XSLHDSA_MAX_N)

/** Signature scratch: largest supported signature (SHAKE/SHA2-256s). */
#define XSLHDSA_SIG_SCRATCH_BYTES	(XASU_SLHDSA_MAX_SIG_SIZE)

/**
 * Start of the pool above the scratch, on an 8-byte boundary so that the DMA transfers and the
 * backend operate on a naturally-aligned base. It bounds the staged key region against the pool
 * reserved in lscript.ld. The remainder of that pool stays free for a caller that needs a buffer
 * of its own while the scratch is busy; the PWCT signature uses overlay data instead.
 */
#define XSLHDSA_SCRATCH_END		(XSLHDSA_SCRATCH_ADDR + \
					((((XSLHDSA_SCRATCH_LEN) + 7U) / 8U) * 8U))

#define XSLHDSA_WOTS_LG_W		(4U)	/**< FIPS 205 lg(w) for supported sets. */
#define XSLHDSA_WOTS_LEN2_FAST_N_MIN	(9U)	/**< Min n for len_2 = 3 fast path. */
#define XSLHDSA_WOTS_LEN2_FAST_N_MAX	(136U)	/**< Max n for len_2 = 3 fast path. */
#define XSLHDSA_WOTS_LEN2_FAST	(3U)	/**< FIPS 205 len_2 fast-path value. */
/** WOTS+/XMSS/FORS scratch bytes (SHAKE-256s worst case). */
#define XSLHDSA_WORK_MEM_BYTES	(2144U)
/** H_msg digest buffer size in bytes (worst case m=47). */
#define XSLHDSA_MSG_DIGEST_BYTES	(47U)
#define XSLHDSA_NODE_BUF_BYTES	(32U)	/**< Single n-byte node buffer for hypertree walks. */
/** WOTS+ base-2^b digit buffer (max chain count). */
#define XSLHDSA_WOTS_WEIGHT_WORDS	(67U)

/************************************** Type Definitions *****************************************/
/** Supported SLH-DSA parameter set identifiers (FIPS 205 Table 2). */
typedef enum {
	XSLHDSA_PARAM_SET_SHAKE_256S = 0, /**< SLH-DSA-SHAKE-256s parameter set. */
	XSLHDSA_PARAM_SET_SHA2_128S, /**< SLH-DSA-SHA2-128s parameter set. */
	XSLHDSA_PARAM_SET_SHA2_256S, /**< SLH-DSA-SHA2-256s parameter set. */
	XSLHDSA_PARAM_SET_SHAKE_128S, /**< SLH-DSA-SHAKE-128s parameter set. */
	XSLHDSA_PARAM_SET_MAX /**< Number of supported parameter sets (exclusive bound). */
} XSlhDsa_ParamSet;
/** Lookup-table index of the unused dummy row (not a supported parameter set). */
#define XSLHDSA_PARAM_IDX_RESERVED	(XSLHDSA_PARAM_SET_MAX)
/** Underlying hash family for a parameter set. */
typedef enum {
	XSLHDSA_HASH_SHAKE = 0, /**< SHAKE-based SLH-DSA instantiation. */
	XSLHDSA_HASH_SHA2, /**< SHA2-based SLH-DSA instantiation. */
	XSLHDSA_HASH_UNSUPPORTED = 0xFF /**< Marker used by the reserved lookup row. */
} XSlhDsa_HashFamily;
/** Forward declaration of the per-family hash ops dispatch table. */
typedef struct _XSlhDsa_HashOps XSlhDsa_HashOps;
/**
* Expanded SLH-DSA parameter view used by sign/verify/keygen paths.
* Populated from the static parameter-set lookup table when bound to an instance.
*/
typedef struct {
	XSlhDsa_HashFamily hash_family; /**< Hash family (SHAKE or SHA2). */
	u32 n; /**< FIPS n: hash output width in bytes. */
	u32 h; /**< FIPS h: total hypertree height. */
	u32 d; /**< FIPS d: number of hypertree layers. */
	u32 xmss_height; /**< FIPS h': XMSS subtree height. */
	u32 fors_height; /**< FIPS a: FORS tree height. */
	u32 fors_count; /**< FIPS k: number of FORS trees. */
	u32 wots_win_bits; /**< FIPS lg_w: Winternitz parameter in bits. */
	u32 m; /**< FIPS m: message digest length in bytes. */
	u32 yield_xmss_leaves; /**< XMSS leaves built per cooperative yield slice. */
	u32 yield_fors_leaves; /**< FORS leaves built per cooperative yield slice. */
} XSlhDsa_Param;
/**
 * ASUFW SLH-DSA runtime context.
 *
 * Unlike reference slh_var_t, this structure binds external DMA buffers, firmware
 * scratch memory, and optional SHA/DMA engine handles for hardware-accelerated paths.
 */
struct _XSlhDsa_Instance {
	XSlhDsa_YieldCtx Yield; /**< Cooperative yield/resume state for long ops. */
	u64 MsgAddr; /**< DMA address of the message buffer. */
	u64 SigAddr; /**< DMA address of the signature buffer. */
	u64 PkAddr; /**< DMA address of the external public-key buffer. */

	XSlhDsa_Adrs *Addr; /**< Active domain-separation address for hash calls. */
	const XSlhDsa_HashOps *HashOps; /**< Hash backend bound at param-set init. */
	const u8 *MsgLocalPtr; /**< Client-supplied PH(M) digest buffer. */
	u8 *WorkMem; /**< Firmware scratch for WOTS+/XMSS/FORS intermediates. */
	u8 *MsgDigest; /**< Buffer holding the H_msg digest output. */
	u8 *NodeBuf; /**< Single n-byte node buffer for hypertree operations. */
	u32 *WotsWeights; /**< WOTS+ base-2^b digit buffer. */
	void *ShaInst; /**< Optional SHA hardware instance for accelerated hashing. */
	void *DmaInst; /**< Optional DMA engine instance for buffer transfers. */
	XSlhDsa_Param Param; /**< Materialized parameter view for the active set. */
	u32 MsgLen; /**< Message length in bytes. */
	u32 SigLen; /**< Signature buffer length in bytes. */
	u32 MsgLocalLen; /**< Length of MsgLocalPtr (32 or 64 for pre-hash). */
	u32 CtxPrefixLen; /**< Context prefix length in WorkMem (pure or pre-hash). */

	const u8 *PkSeed; /**< Public-key seed (n bytes), NULL when unstaged. */
	const u8 *PkRoot; /**< Public-key root (n bytes), NULL when unstaged. */
	const u8 *SkSeed; /**< Secret-key seed (n bytes), NULL when unstaged. */
	const u8 *SkPrf; /**< Secret-key PRF key (n bytes), NULL when unstaged. */
	u8 *KeyScratch; /**< Staged key region, XSLHDSA_SCRATCH_LEN bytes of shared pool. */
	u8 PreHashMode; /**< XASU_SLHDSA_MSG_PURE or XASU_SLHDSA_MSG_PREHASH. */
	u8 HashId; /**< Pre-hash function identifier (SHA256 for 128s, SHA512 for 256s). */
};

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XSLHDSA_BIT_MASK(i) ((1U << (i)) - 1U)	/**< Mask of i low bits: (2^i - 1). */

/*
 * Accessors for the SHA message DMA resume flag. They live here rather than in xslhdsa_yield.h
 * because that header only forward declares XSlhDsa_Instance, so it cannot reach Yield.Session.
 * Keeping the set, test and clear together is what stops the resume rule being restated per
 * module: the hash layer sets and clears the flag, the sign and verify entry points test it to
 * skip re-staging first-slice inputs, and the handler tests it to arm DMA-done rather than the
 * resume task.
 */
/*************************************************************************************************/
/**
* @brief	This function records that the SHA absorb of the message is complete, so the next
*		PRF_msg or H_msg entry only has to finish the hash.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
*
* @note
*	The caller validates InstPtr before flagging the pending DMA.
*
*************************************************************************************************/
static inline void XSlhDsa_YieldSetHashDmaResume(XSlhDsa_Instance *InstPtr)
{
	InstPtr->Yield.Session.HashDmaResume = 1U;
}

/*************************************************************************************************/
/**
* @brief	This function reports whether a SHA message DMA wait is outstanding, which means
*		the message absorb is done and only the hash finish is pending.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
*
* @return
*		- 1U, if PRF_msg or H_msg is waiting on, or resuming after, a non-blocking SHA
*		  message DMA.
*		- 0U, if InstPtr is NULL, or no SHA message DMA wait is outstanding, which
*		  includes a first slice and a cooperative yield slice.
*
*************************************************************************************************/
static inline u32 XSlhDsa_YieldIsHashDmaResume(const XSlhDsa_Instance *InstPtr)
{
	return ((InstPtr != NULL) && (InstPtr->Yield.Session.HashDmaResume != 0U)) ? 1U : 0U;
}

/*************************************************************************************************/
/**
* @brief	This function clears the SHA message DMA resume flag.
*
* @param	InstPtr	Pointer to the SLH-DSA instance.
*
* @note
*	The caller validates InstPtr before clearing the resume flag.
*
*************************************************************************************************/
static inline void XSlhDsa_YieldClearHashDmaResume(XSlhDsa_Instance *InstPtr)
{
	InstPtr->Yield.Session.HashDmaResume = 0U;
}

/************************************ Function Prototypes ****************************************/
u32 XSlhDsa_WotsMsgChainCount(const XSlhDsa_Param *ParamPtr);
u32 XSlhDsa_WotsCsChainCount(const XSlhDsa_Param *ParamPtr);
u32 XSlhDsa_WotsChainCount(const XSlhDsa_Param *ParamPtr);
u32 XSlhDsa_SignatureBytes(const XSlhDsa_Param *ParamPtr);

XSlhDsa_Instance *XSlhDsa_GetInstance(void);
s32 XSlhDsa_BindParam(XSlhDsa_Instance *InstPtr, XSlhDsa_ParamSet ParamSet);
s32 XSlhDsa_LookupParamSet(XSlhDsa_ParamSet ParamSet,
				const XSlhDsa_Param **ParamPtrPtr);
s32 XSlhDsa_LoadPk(XSlhDsa_Instance *InstPtr, const u8 *Pk);
s32 XSlhDsa_LoadSk(XSlhDsa_Instance *InstPtr, const u8 *Sk);
void XSlhDsa_SetMsg(XSlhDsa_Instance *InstPtr, const u8 *Msg,
			u32 MsgLen);
void XSlhDsa_SetSig(XSlhDsa_Instance *InstPtr, const u8 *Sig,
			u32 SigLen);
void XSlhDsa_SetPkAddr(XSlhDsa_Instance *InstPtr, const u8 *Pk);
void XSlhDsa_SetMsgMode(XSlhDsa_Instance *InstPtr, u8 PreHashMode, u8 HashId);
s32 XSlhDsa_ClearSensitiveState(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_ZeroizeSigScratch(void);

const u8 *XSlhDsa_PreHashGetOid(u8 HashId);
s32 XSlhDsa_PreHashBuildPrefix(XSlhDsa_Instance *InstPtr, const u8 *Ctx, u32 CtxLen);
s32 XSlhDsa_PreparePreHashMsg(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
				   const u8 *Ctx, u32 CtxLen);

s32 XSlhDsa_KeyGenInternal(XSlhDsa_Instance *InstPtr, const u8 *SkSeed,
			       const u8 *SkPrf, const u8 *PkSeed);
s32 XSlhDsa_SignInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
		      const u8 *Ctx, u32 CtxLen, const u8 *Sk, const u8 *OptRand,
		      u8 *Sig, u32 SigLen);
s32 XSlhDsa_VerifyCore(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_VerifyInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			const u8 *Sig, u32 SigLen, const u8 *Ctx, u32 CtxLen, const u8 *Pk);

s32 XSlhDsa_KeyGen(XAsufw_Dma *DmaPtr, XSha *ShaPtr,
			const XSlhDsa_KeyGenParams *ParamsPtr);
s32 XSlhDsa_Sign(XAsufw_Dma *DmaPtr, XSha *ShaPtr, const XAsu_SlhdsaSignParams *ParamsPtr);
s32 XSlhDsa_Verify(XAsufw_Dma *DmaPtr, XSha *ShaPtr,
			const XAsu_SlhdsaSignVerifyParams *ParamsPtr);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_SLHDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XSLHDSA_H_ */
/** @} */
