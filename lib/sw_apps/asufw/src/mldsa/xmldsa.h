/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xmldsa.h
 *
 * This file contains the interface for the ML-DSA (FIPS 204) core wrapper for ASUFW. It is a
 * thin adapter over the liboqs "mldsa-native" backend. It covers the full FIPS 204 surface: pure
 * ML-DSA and HashML-DSA (pre-hash), hedged and deterministic signing, context strings, and
 * seed-based key generation.
 *
 * The wrapper accepts the client request parameter structure directly and a DMA instance, so the
 * module handler passes the request through untouched. Because the ASU is a 32-bit core and the
 * client passes 64-bit DDR addresses, the wrapper marshals the fixed-size client buffers (keys,
 * signature, context, seed) through DMA into a local ASU-addressable scratch region.
 * Arbitrary-length messages are never buffered:
 * they are streamed directly from the client address through the hardware SHA-2/SHA-3 engines
 * (pre-hash), or folded into the FIPS 204 message representative (mu) via the hardware SHAKE256
 * engine for the pure external-mu signing/verification path.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xmldsa_server_apis ML-DSA Server APIs
 * @{
 */
#ifndef XMLDSA_H_
#define XMLDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mldsainfo.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"
#include "xsha.h"

#ifdef XASU_MLDSA_ENABLE

/************************************ Constant Definitions ***************************************/
/*
 * ML-DSA's scratch region, carved out of the shared crypto pool.
 *
 * The ASU is a 32-bit core while clients pass 64-bit DDR addresses that it cannot dereference, so
 * the fixed-size client buffers (keys, signature, context, seed) are DMA'd into the scratch and the
 * crypto runs on them there. Signing DMAs the signature back out; key generation instead leaves the
 * pair in the scratch for the caller to read, as described below. Messages are not buffered: they
 * stream from the client address through the hardware SHA engines.
 *
 * ML-KEM, RSA and the other users of the pool carve the same bytes their own way, which is safe
 * because all of them hold XASUFW_OVERLAY and so never run together.
 */
#define XMLDSA_SCRATCH_ADDR		(XASUFW_CRYPTO_ALGO_DATA_BLOCK)

/** Signing holds the secret key, the signature and the context at once. */
#define XMLDSA_SIGN_SCRATCH_LEN		(XASU_MLDSA_MAX_PRIV_KEY_LEN + XASU_MLDSA_MAX_SIGN_LEN + \
					XASU_MLDSA_MAX_CTX_LEN)

/**
 * Key generation carves the region differently, and holds more: its pairwise consistency test
 * signs with the fresh secret key and verifies with the fresh public key, so the pair, the test
 * signature and the xi seed are all live together. mu and the domain-separation prefix are stack
 * locals, as they are in signing.
 *
 * XMldsa_KeyGen leaves the pair here rather than exporting it, so the caller reads the two keys at
 * these offsets and is responsible for clearing XMLDSA_KEYGEN_KEY_PAIR_LEN bytes from the base of
 * the region once it is done with them. The seed and the test signature never leave the module.
 */
#define XMLDSA_KEYGEN_SK_OFFSET		(0U)
#define XMLDSA_KEYGEN_PK_OFFSET		(XASU_MLDSA_MAX_PRIV_KEY_LEN)
#define XMLDSA_KEYGEN_KEY_PAIR_LEN	(XASU_MLDSA_MAX_PRIV_KEY_LEN + XASU_MLDSA_MAX_PUB_KEY_LEN)
#define XMLDSA_KEYGEN_SIG_OFFSET	(XMLDSA_KEYGEN_KEY_PAIR_LEN)
#define XMLDSA_KEYGEN_SEED_OFFSET	(XMLDSA_KEYGEN_SIG_OFFSET + XASU_MLDSA_MAX_SIGN_LEN)
#define XMLDSA_KEYGEN_SCRATCH_LEN	(XMLDSA_KEYGEN_SEED_OFFSET + XASU_MLDSA_KEYGEN_SEED_LEN)

#define XMLDSA_SCRATCH_LEN		((XMLDSA_SIGN_SCRATCH_LEN > XMLDSA_KEYGEN_SCRATCH_LEN) ? \
					XMLDSA_SIGN_SCRATCH_LEN : XMLDSA_KEYGEN_SCRATCH_LEN)

/**
 * Start of the pool above the scratch, on an 8-byte boundary so that the DMA transfers and the
 * backend operate on a naturally-aligned base. A caller that needs to receive ML-DSA output while
 * the scratch is busy takes its buffer from here; the known-answer test is the only one that does.
 */
#define XMLDSA_SCRATCH_END		(XMLDSA_SCRATCH_ADDR + \
					((((XMLDSA_SCRATCH_LEN) + 7U) / 8U) * 8U))

/************************************** Type Definitions *****************************************/
/**
 * This structure contains ML-DSA key pair generation parameters. It stays inside the firmware: the
 * key manager fills it in while servicing XASU_KM_GEN_MLDSA_KEY_PAIR_CMD_ID, whose own client
 * parameters are XAsu_KeyManagerParams. If SeedAddr is non-zero, seed-based KeyGen (explicit xi
 * seed) is used; otherwise the seed is drawn from the TRNG. XMldsa_KeyGen does not copy the pair
 * anywhere, so the two key addresses state where the caller expects to find it, which is the
 * scratch region carved above.
 */
typedef struct {
	u64 PubKeyAddr;		/**< Address of the output public key */
	u64 PrivKeyAddr;	/**< Address of the output secret key */
	u64 SeedAddr;		/**< Address of the 32-byte xi seed (0 => use TRNG) */
	u32 PubKeyLen;		/**< Length of the public key buffer in bytes */
	u32 PrivKeyLen;		/**< Length of the secret key buffer in bytes */
	u32 SeedLen;		/**< Length of the seed in bytes (0 or 32) */
	u8  ParamSet;		/**< Parameter set (XAsu_MldsaParamSet) */
	u8  Reserved[3];	/**< Reserved for alignment */
} XMldsa_KeyGenParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XMldsa_KeyGen(XAsufw_Dma *DmaPtr, XSha *Sha3Ptr, const XMldsa_KeyGenParams *ParamsPtr);
s32 XMldsa_Sign(XAsufw_Dma *DmaPtr, XSha *Sha2Ptr, const XAsu_MldsaSignVerifyParams *ParamsPtr,
	const u8 *RndPtr, u32 RndLen);
s32 XMldsa_Verify(XAsufw_Dma *DmaPtr, XSha *Sha2Ptr, const XAsu_MldsaSignVerifyParams *ParamsPtr);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_MLDSA_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XMLDSA_H_ */
/** @} */
