/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_hash.h
*
* This file contains the SLH-DSA hash function API for ASUFW firmware.
*
* Declares the tweakable hash and PRF dispatch interface, the hash operations table each
* backend fills in, the XSha hardware engine context, and the SHA2 and SHAKE backend
* lookups. These interfaces are internal to the SLH-DSA module.
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

#ifndef XSLHDSA_HASH_H_
#define XSLHDSA_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsha.h"
#include "xasufw_dma.h"
#include "xslhdsa.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/
/** SHA-256 full digest size in bytes (FIPS 180-4). */
#define XSLHDSA_SHA256_DIGEST_BYTES	(32U)
/** SHA-512 full digest size in bytes (FIPS 180-4). */
#define XSLHDSA_SHA512_DIGEST_BYTES	(64U)
#define XSLHDSA_HMAC_INNER_PAD_BYTE	(0x36U)	/**< HMAC inner pad byte (RFC 2104). */
#define XSLHDSA_HMAC_OUTER_PAD_BYTE	(0x5CU)	/**< HMAC outer pad byte (RFC 2104). */
#define XSLHDSA_HMSG_MGF_N_PREFIX	(2U)	/**< H_msg MGF1 seed prefix fields: R || PK.seed. */

/************************************** Type Definitions *****************************************/
/** WOTS+ chain iteration configuration passed to XSlhDsa_Chain(). */
typedef struct {
	u32 StartIdx; /**< Starting chain index (inclusive). */
	u32 Steps; /**< Number of F iterations to apply. */
} XSlhDsa_ChainCfg;

/**
* Hardware hash streaming context for SLH-DSA hash backends.
*
* Binds the ASU XSha engine and ASUFW DMA channel from the SLH-DSA instance
* for incremental hash operations used by both SHA2 and SHAKE backends.
*/
typedef struct
{
	XSha *Sha;		/**< ASU XSha engine instance (SHA2 or SHA3/SHAKE mode). */
	XAsufw_Dma *Dma;	/**< ASUFW DMA channel for streaming input to XSha. */
} XSlhDsa_HashCtx;

/** Per-family hash primitive dispatch table; bound on the instance at param-set init. */
struct _XSlhDsa_HashOps
{
	s32 (*HashMsg)(XSlhDsa_Instance *InstPtr); /**< H_msg digest primitive. */
	s32 (*HashF)(XSlhDsa_Instance *InstPtr, const u8 *Input,
		     u8 *Output); /**< F hash primitive. */
	s32 (*HashH)(XSlhDsa_Instance *InstPtr, const u8 *Left,
		     const u8 *Right, u8 *Output); /**< H hash primitive. */
	s32 (*HashTl)(XSlhDsa_Instance *InstPtr, const u8 *Input,
		      u32 InputLen, u8 *Output); /**< T_l hash primitive. */
	s32 (*HashPrf)(XSlhDsa_Instance *InstPtr, u8 *Output); /**< PRF on sk_seed. */
	s32 (*HashPrfMsg)(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			  const u8 *Msg, u32 MsgLen, const u8 *Ctx, u32 CtxLen,
			  u8 *Output); /**< Message PRF for signing. */
	s32 (*Chain)(XSlhDsa_Instance *InstPtr, const u8 *Input,
		     const XSlhDsa_ChainCfg *Cfg, u8 *Output); /**< WOTS+ chain of F. */
};

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XSlhDsa_HashMsg(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_HashF(XSlhDsa_Instance *InstPtr, const u8 *Input, u8 *Output);
s32 XSlhDsa_HashH(XSlhDsa_Instance *InstPtr, const u8 *Left, const u8 *Right,
		       u8 *Output);
s32 XSlhDsa_HashTl(XSlhDsa_Instance *InstPtr, const u8 *Input, u32 InputLen,
			u8 *Output);
s32 XSlhDsa_HashPrf(XSlhDsa_Instance *InstPtr, u8 *Output);
s32 XSlhDsa_HashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			    const u8 *Msg, u32 MsgLen, const u8 *Ctx, u32 CtxLen, u8 *Output);
s32 XSlhDsa_Chain(XSlhDsa_Instance *InstPtr, const u8 *Input,
		       const XSlhDsa_ChainCfg *Cfg, u8 *Output);
const XSlhDsa_HashOps *XSlhDsa_HashOpsLookup(XSlhDsa_HashFamily Family);

s32 XSlhDsa_HashCtxInit(XSlhDsa_Instance *InstPtr, XSlhDsa_HashCtx *HashCtxPtr);
s32 XSlhDsa_HashShakeBegin(XSlhDsa_HashCtx *HashCtxPtr);
s32 XSlhDsa_HashShakeUpdate(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr, u32 Len, u32 EndLast);
s32 XSlhDsa_HashShakeFinish(XSlhDsa_HashCtx *HashCtxPtr, u8 *Out, u32 OutLen);
s32 XSlhDsa_HashShakePkSeedAddrLocal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			u8 *Out, u32 OutLen);
s32 XSlhDsa_HashShakePkSeedAddrDma(XSlhDsa_Instance *InstPtr, u64 MsgAddr, u32 MsgLen,
			u8 *Out, u32 OutLen);
s32 XSlhDsa_HashSha2Begin(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode);
s32 XSlhDsa_HashSha2UpdatePkSeedPrefix(XSlhDsa_HashCtx *HashCtxPtr,
					    XSlhDsa_Instance *InstPtr,
					    u32 ShaMode);
s32 XSlhDsa_HashSha2Update(XSlhDsa_HashCtx *HashCtxPtr, u64 Addr, u32 Len, u32 EndLast);
s32 XSlhDsa_HashSha2FinishTrunc(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode, u32 N, u8 *Out);
s32 XSlhDsa_HashSha2FinishFull(XSlhDsa_HashCtx *HashCtxPtr, u32 ShaMode, u8 *Out);
s32 XSlhDsa_HashSha2PkSeedAddrLocal(XSlhDsa_Instance *InstPtr, const u8 *Msg, u32 MsgLen,
			u32 ShaMode, u8 *Out, u32 N);

s32 XSlhDsa_Sha2HashMsg(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_Sha2HashF(XSlhDsa_Instance *InstPtr, const u8 *Input,
			u8 *Output);
s32 XSlhDsa_Sha2HashH(XSlhDsa_Instance *InstPtr, const u8 *Left,
			const u8 *Right, u8 *Output);
s32 XSlhDsa_Sha2HashTl(XSlhDsa_Instance *InstPtr, const u8 *Input,
			u32 InputLen, u8 *Output);
s32 XSlhDsa_Sha2HashPrf(XSlhDsa_Instance *InstPtr, u8 *Output);
s32 XSlhDsa_Sha2HashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			const u8 *Msg, u32 MsgLen,
			const u8 *Ctx, u32 CtxLen, u8 *Output);
s32 XSlhDsa_Sha2Chain(XSlhDsa_Instance *InstPtr, const u8 *Input,
			const XSlhDsa_ChainCfg *Cfg, u8 *Output);

s32 XSlhDsa_ShakeHashMsg(XSlhDsa_Instance *InstPtr);
s32 XSlhDsa_ShakeHashF(XSlhDsa_Instance *InstPtr, const u8 *Input,
			u8 *Output);
s32 XSlhDsa_ShakeHashH(XSlhDsa_Instance *InstPtr, const u8 *Left,
			const u8 *Right, u8 *Output);
s32 XSlhDsa_ShakeHashTl(XSlhDsa_Instance *InstPtr, const u8 *Input,
			u32 InputLen, u8 *Output);
s32 XSlhDsa_ShakeHashPrf(XSlhDsa_Instance *InstPtr, u8 *Output);
s32 XSlhDsa_ShakeHashPrfMsg(XSlhDsa_Instance *InstPtr, const u8 *OptRand,
			const u8 *Msg, u32 MsgLen,
			const u8 *Ctx, u32 CtxLen, u8 *Output);
s32 XSlhDsa_ShakeChain(XSlhDsa_Instance *InstPtr, const u8 *Input,
			const XSlhDsa_ChainCfg *Cfg, u8 *Output);

/************************************ Variable Definitions ***************************************/
extern const XSlhDsa_HashOps XSlhDsa_HashOpsShake; /**< SHAKE hash ops table. */
extern const XSlhDsa_HashOps XSlhDsa_HashOpsSha2; /**< SHA2 hash ops table. */

#endif /* XASU_SLHDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XSLHDSA_HASH_H_ */
/** @} */
