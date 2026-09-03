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
* along with the instance management, keep-alive, pre-hash, key generation, signature
* generation and signature verification interfaces.
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

#ifndef XSLHDSA_H_
#define XSLHDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xslhdsa_addr.h"
#include "xasu_slhdsainfo.h"
#include "xasufw_dma.h"
#include "xsha.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
/** Keep-alive counter update period (us). */
#define XSLHDSA_KEEPALIVE_INTERVAL_US	(200000U)
/** Upper bound (us) on one key-generation or signing operation. */
#define XSLHDSA_KEEPALIVE_MAX_OP_US	(120000000U)

#define XSLHDSA_MAX_N	(32U)	/**< Max n (bytes) across the supported parameter sets. */
#define XSLHDSA_PK_N_FIELDS	(2U)	/**< PK fields: PK.seed || PK.root. */
#define XSLHDSA_PK_SEED_FIELD	(0U)	/**< PK.seed field index. */
#define XSLHDSA_PK_ROOT_FIELD	(1U)	/**< PK.root field index. */
#define XSLHDSA_SK_N_FIELDS	(4U)	/**< SK n-byte field count. */
#define XSLHDSA_SK_SEED_FIELD	(0U)	/**< SK.seed field index. */
#define XSLHDSA_SK_PRF_FIELD	(1U)	/**< SK.prf field index. */
#define XSLHDSA_SK_PKSEED_FIELD	(2U)	/**< PK.seed field index within SK. */
#define XSLHDSA_SK_PKROOT_FIELD	(3U)	/**< PK.root field index within SK. */
#define XSLHDSA_WOTS_LG_W		(4U)	/**< FIPS 205 lg(w) for supported sets. */
#define XSLHDSA_WOTS_LEN2_FAST_N_MIN	(9U)	/**< Min n for len_2 = 3 fast path. */
#define XSLHDSA_WOTS_LEN2_FAST_N_MAX	(136U)	/**< Max n for len_2 = 3 fast path. */
#define XSLHDSA_WOTS_LEN2_FAST	(3U)	/**< FIPS 205 len_2 fast-path value. */
#define XSLHDSA_MAX_XMSS_HEIGHT	(9U)	/**< Max XMSS subtree height h' (FIPS 205 Table 2). */
/**
 * XMSS treehash stack depth. Full-tree walks pass SubHeight == h', so the array must
 * hold h' entries (indices 0..h'-1) plus one spare.
 */
#define XSLHDSA_XMSS_STACK_DEPTH (XSLHDSA_MAX_XMSS_HEIGHT + 1U)
#define XSLHDSA_MAX_FORS_HEIGHT	(14U)	/**< Max FORS tree height a (FIPS 205 Table 2). */
/**
 * FORS treehash stack depth. Auth-path walks pass SubHeight in 0..a-1, so depth == max a
 * already provides one spare past the largest SubHeight.
 */
#define XSLHDSA_FORS_STACK_DEPTH	(XSLHDSA_MAX_FORS_HEIGHT)
/** WOTS+/XMSS/FORS scratch bytes (SHAKE-256s worst case). */
#define XSLHDSA_WORK_MEM_BYTES	(2144U)
/** H_msg digest buffer size in bytes (worst case m=47). */
#define XSLHDSA_MSG_DIGEST_BYTES	(47U)
#define XSLHDSA_NODE_BUF_BYTES	(32U)	/**< Single n-byte node buffer for hypertree walks. */
/** WOTS+ base-2^b digit buffer (max chain count). */
#define XSLHDSA_WOTS_WEIGHT_WORDS	(67U)

/************************************** Type Definitions *****************************************/
/** Keep-alive state for one long-running SLH-DSA operation. */
typedef struct {
	u64 OpStart;	/**< PIT snapshot taken when the operation started. */
	u64 LastUpdate;	/**< PIT snapshot of the most recent keep-alive counter update. */
	u8 Active;	/**< Non-zero while a key-generation or signing operation is in progress. */
	u8 Reserved[7];	/**< Reserved to pad the structure to 24 bytes */
} XSlhDsa_KeepAliveCtx;

struct XSlhDsa_Instance;

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
} XSlhDsa_Param;
/**
 * ASUFW SLH-DSA runtime context.
 *
 * Unlike reference slh_var_t, this structure binds external DMA buffers, firmware
 * scratch memory, and optional SHA/DMA engine handles for hardware-accelerated paths.
 */
typedef struct XSlhDsa_Instance {
	XSlhDsa_KeepAliveCtx KeepAlive; /**< Keep-alive state for long ops. */
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

	u8 PkSeed[XSLHDSA_MAX_N]; /**< Public-key seed (n bytes). */
	u8 PkRoot[XSLHDSA_MAX_N]; /**< Public-key root (n bytes). */
	u8 SkSeed[XSLHDSA_MAX_N]; /**< Secret-key seed (n bytes). */
	u8 SkPrf[XSLHDSA_MAX_N]; /**< Secret-key PRF key (n bytes). */
	u8 PreHashMode; /**< XASU_SLHDSA_MSG_PURE or XASU_SLHDSA_MSG_PREHASH. */
	u8 HashId; /**< Pre-hash function identifier (SHA256 for 128s, SHA512 for 256s). */
	u8 Reserved[6]; /**< Reserved to pad the structure to 272 bytes. */
} XSlhDsa_Instance;

/**
 * This structure contains SLH-DSA key pair generation parameters. It stays inside the firmware:
 * the key manager fills it in while servicing XASU_KM_GEN_SLHDSA_KEY_PAIR_CMD_ID, whose own client
 * parameters are XAsu_SlhdsaKeyGenParams. The two key addresses state where the pair is to be
 * transferred, which is the client buffers today and can become a firmware scratch region once the
 * key vault holds SLH-DSA keys, without changing this interface. The two lengths are resolved from
 * the parameter set by the caller, so the core does not resolve them a second time.
 */
typedef struct {
	u64 PubKeyAddr;		/**< Address of the output public key */
	u64 PrivKeyAddr;	/**< Address of the output secret key */
	u32 PubKeyLen;		/**< Length of the public key buffer in bytes */
	u32 PrivKeyLen;		/**< Length of the secret key buffer in bytes */
	u8  ParamSet;		/**< Parameter set (XASU_SLHDSA_PARAM_*) */
	u8  Reserved[3];	/**< Reserved for alignment */
} XSlhDsa_KeyGenParams;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XSLHDSA_BIT_MASK(i) ((1U << (i)) - 1U)	/**< Mask of i low bits: (2^i - 1). */

/************************************ Function Prototypes ****************************************/
void XSlhDsa_KeepAliveBegin(struct XSlhDsa_Instance *InstPtr);
void XSlhDsa_KeepAliveEnd(struct XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_KeepAlivePoll(struct XSlhDsa_Instance *InstPtr);

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

const u8 *XSlhDsa_PreHashGetOid(u8 HashId);
s32 XSlhDsa_PreHashBuildPrefix(XSlhDsa_Instance *InstPtr, const u8 *Ctx, u32 CtxLen);
s32 XSlhDsa_PreparePreHashMsg(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
				   const u8 *Ctx, u32 CtxLen);

s32 XSlhDsa_KeyGenInternal(XSlhDsa_Instance *InstPtr, const u8 *SkSeed,
			       const u8 *SkPrf, const u8 *PkSeed, u8 *Sk, u8 *Pk);

s32 XSlhDsa_SignInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
		      const u8 *Ctx, u32 CtxLen, const u8 *Sk, const u8 *OptRand,
		      u8 *Sig, u32 SigLen);

s32 XSlhDsa_VerifyCore(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_VerifyInternal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			const u8 *Sig, u32 SigLen, const u8 *Ctx, u32 CtxLen, const u8 *Pk);

/*
 * Request-level entry points called by the ASUFW SLH-DSA command handler. Each one validates the
 * client parameters, binds the instance to the requested parameter set, draws the randomness the
 * operation needs and resolves the request buffers before running the *Internal algorithm above.
 */
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
