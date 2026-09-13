/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xkeymanager_crypto.h
 *
 * This file contains the crypto primitives that xkeymanager_crypto.c implements: weak-key
 * rejection, KAT readiness, IV advance, SHA-384 digest and compare, and AES-256-CBC over a
 * buffer, along with the key, IV and digest sizes they work in.
 *
 * Nothing declared here knows about a chunked body or a blob header, so it is the bottom layer
 * that the body and the format layers above it are built from.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vm   09/15/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xkeymanager_server_apis Key Manager Server APIs
* @{
*/
#ifndef XKEYMANAGER_CRYPTO_H_
#define XKEYMANAGER_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif
#include "xasufw_dma.h"
#include "xasufw_util.h"
#include "xaes.h"
#include "xsha.h"

#ifdef XASU_KEYMANAGER_ENABLE
/************************************ Constant Definitions ***************************************/
/** AES-256 key size. */
#define XKEYMANAGER_AES_KEY_SIZE			(32U)

/** SHA-384 chunk digest size. */
#define XKEYMANAGER_HASH_SIZE			(48U)

/** AES-CBC IV size. */
#define XKEYMANAGER_AES_IV_SIZE			(16U)

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************** Type Definitions *****************************************/
/** Buffer, key and IV of one AES-256-CBC operation. */
typedef struct {
	const u8 *Key;		/**< 32-byte AES key. */
	const u8 *Iv;		/**< 16-byte IV. */
	const u8 *Input;	/**< Input buffer. */
	u8 *Output;		/**< Output buffer. */
	u32 Len;		/**< Data length in bytes, a multiple of the AES block size. */
} XKeyManager_CbcParams;

/************************************ Function Prototypes ****************************************/
/** Key hygiene. */
s32 XKeyManager_RejectWeakKey(const u8 *Key, u32 KeyLen);
s32 XKeyManager_GenerateStrongRandom(u8 *Buf, u32 Len);

/** IV derivation shared by the chunk blocks and the chunk-0 secrets pieces. */
s32 XKeyManager_AdvanceIv(const u8 *BaseIv, u32 Increment, u8 *OutIv);

/** Digest and comparison. */
s32 XKeyManager_Sha384Digest(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr, const u8 *Data, u32 DataLen,
		       u8 *Hash);
s32 XKeyManager_RedundantSha384Compare(XSha *ShaInstancePtr, XAsufw_Dma *DmaPtr, const u8 *Data,
				 u32 DataLen, const u8 *ExpectedHash);

/** AES-256-CBC over an arbitrary buffer. */
s32 XKeyManager_EncryptBufferCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			   const XKeyManager_CbcParams *ParamsPtr);
s32 XKeyManager_DecryptBufferCbc(XAsufw_Dma *DmaPtr, XAes *AesInstancePtr,
			   const XKeyManager_CbcParams *ParamsPtr);

#endif /* XASU_KEYMANAGER_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XKEYMANAGER_CRYPTO_H_ */
/** @} */
