/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xmlkem.h
 *
 * This file contains the interface for the ML-KEM (FIPS 203) core wrapper for ASUFW. It is a thin
 * adapter over the liboqs "mlkem-native" backend, whose derandomized entry points it calls directly
 * rather than through the generic OQS_KEM API, and covers key generation, encapsulation and
 * decapsulation for all NIST parameter sets (ML-KEM-512/768/1024).
 *
 * Encapsulation and decapsulation accept the client request parameter structure directly and a DMA
 * instance, so the module handler passes the request through untouched; key generation is driven by
 * the key manager through the internal XMlkem_KeyGenParams below. Because the ASU is a 32-bit core
 * and the client passes 64-bit DDR addresses, the wrapper marshals the fixed-size client buffers
 * through DMA into a local ASU-addressable scratch region before running the crypto. Encapsulation
 * and decapsulation DMA their results back out; key generation leaves the pair in the scratch for
 * the caller to read, as described below.
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
 * @addtogroup xmlkem_server_apis ML-KEM Server APIs
 * @{
 */
#ifndef XMLKEM_H_
#define XMLKEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasu_mlkeminfo.h"
#include "xasufw_dma.h"
#include "xasufw_memory.h"

#ifdef XASU_MLKEM_ENABLE

/************************************ Constant Definitions ***************************************/
/*
 * ML-KEM's scratch region, carved out of the shared crypto pool.
 *
 * The ASU is a 32-bit core while clients pass 64-bit DDR addresses that it cannot dereference, so
 * the fixed-size client buffers (keys, ciphertext, shared secret, seed) are DMA'd into the scratch
 * and the crypto runs on them there. Encapsulation and decapsulation DMA their outputs back out;
 * key generation instead leaves the pair in the scratch for the caller to read, as described below.
 *
 * ML-DSA, RSA and the other users of the pool carve the same bytes their own way, which is safe
 * because all of them hold XASUFW_OVERLAY and so never run together.
 */
#define XMLKEM_SCRATCH_ADDR		(XASUFW_CRYPTO_ALGO_DATA_BLOCK)

/** Encapsulation holds the public key, the ciphertext and the shared secret at once. */
#define XMLKEM_ENCAPS_SCRATCH_LEN	(XASU_MLKEM_MAX_PUB_KEY_LEN + XASU_MLKEM_MAX_CT_LEN + \
					XASU_MLKEM_SHARED_SECRET_LEN)

/** Decapsulation holds the secret key, the ciphertext and the shared secret at once. */
#define XMLKEM_DECAPS_SCRATCH_LEN	(XASU_MLKEM_MAX_PRIV_KEY_LEN + XASU_MLKEM_MAX_CT_LEN + \
					XASU_MLKEM_SHARED_SECRET_LEN)

/**
 * Key generation carves the region differently, and holds more: its pairwise consistency test
 * encapsulates to the fresh public key and decapsulates with the fresh secret key, so the pair,
 * the ciphertext and both shared secrets are live together.
 *
 * XMlkem_KeyGen leaves the pair here rather than exporting it, so the caller reads the two keys at
 * these offsets and is responsible for clearing XMLKEM_KEYGEN_KEY_PAIR_LEN bytes from the base of
 * the region once it is done with them. The ciphertext and both shared secrets never leave the
 * module. The slots are sized for ML-KEM-1024, so a smaller parameter set fills only the front of
 * each.
 */
#define XMLKEM_KEYGEN_PK_OFFSET		(0U)
#define XMLKEM_KEYGEN_SK_OFFSET		(XASU_MLKEM_MAX_PUB_KEY_LEN)
#define XMLKEM_KEYGEN_KEY_PAIR_LEN	(XASU_MLKEM_MAX_PUB_KEY_LEN + XASU_MLKEM_MAX_PRIV_KEY_LEN)
#define XMLKEM_KEYGEN_CT_OFFSET		(XMLKEM_KEYGEN_KEY_PAIR_LEN)
#define XMLKEM_KEYGEN_SS_OFFSET		(XMLKEM_KEYGEN_CT_OFFSET + XASU_MLKEM_MAX_CT_LEN)
#define XMLKEM_KEYGEN_SCRATCH_LEN	(XMLKEM_KEYGEN_SS_OFFSET + \
					(2U * XASU_MLKEM_SHARED_SECRET_LEN))

#define XMLKEM_SCRATCH_LEN		((XMLKEM_DECAPS_SCRATCH_LEN > XMLKEM_KEYGEN_SCRATCH_LEN) ? \
					XMLKEM_DECAPS_SCRATCH_LEN : XMLKEM_KEYGEN_SCRATCH_LEN)

/**
 * Start of the pool above the scratch, on an 8-byte boundary so that the DMA transfers and the
 * backend operate on a naturally-aligned base. A caller that needs to receive ML-KEM output while
 * the scratch is busy takes its buffer from here; the known-answer test is the only one that does.
 */
#define XMLKEM_SCRATCH_END		(XMLKEM_SCRATCH_ADDR + \
					((((XMLKEM_SCRATCH_LEN) + 7U) / 8U) * 8U))

/************************************** Type Definitions *****************************************/
/**
 * This structure contains ML-KEM key pair generation parameters. It stays inside the firmware: the
 * key manager fills it in while servicing XASU_KM_GEN_MLKEM_KEY_PAIR_CMD_ID, whose own client
 * parameters are XAsu_KeyManagerParams. If SeedAddr is non-zero, the pair is derived from that
 * explicit 64-byte d || z seed; otherwise the seed is drawn from the TRNG. XMlkem_KeyGen does not
 * copy the pair anywhere, so the two key addresses state where the caller expects to find it,
 * which is the scratch region carved above.
 */
typedef struct {
	u64 PubKeyAddr;		/**< Address of the output public key (encapsulation key) */
	u64 PrivKeyAddr;	/**< Address of the output secret key (decapsulation key) */
	u64 SeedAddr;		/**< Address of the 64-byte d || z seed (0 => use TRNG) */
	u32 PubKeyLen;		/**< Length of the public key buffer in bytes */
	u32 PrivKeyLen;		/**< Length of the secret key buffer in bytes */
	u32 SeedLen;		/**< Length of the seed in bytes (0 or 64) */
	u8  ParamSet;		/**< Parameter set (XAsu_MlkemParamSet) */
	u8  Reserved[3];	/**< Reserved for alignment */
} XMlkem_KeyGenParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
s32 XMlkem_KeyGen(XAsufw_Dma *DmaPtr, const XMlkem_KeyGenParams *ParamsPtr);
s32 XMlkem_Encaps(XAsufw_Dma *DmaPtr, const XAsu_MlkemEncapsDecapsParams *ParamsPtr,
	const u8 *Seed, u32 SeedLen);
s32 XMlkem_Decaps(XAsufw_Dma *DmaPtr, const XAsu_MlkemEncapsDecapsParams *ParamsPtr);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_MLKEM_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XMLKEM_H_ */
/** @} */
