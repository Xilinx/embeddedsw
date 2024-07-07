/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_sha.h
* @addtogroup xsecure_sha_versal_aiepg2_apis XilSecure SHA Versal_Aiepg2 APIs
* @{
* @cond xsecure_internal
*
* This file Contains the function prototypes, defines and macros for
* the SHA2/3 hardware module.
*
* @note
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSECURE_SHA_H_
#define XSECURE_SHA_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sss.h"
#include "xsecure_sha_common.h"
#include "xsecure_sha_hw.h"
#include "xpmcdma.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal
 * @{
 */
#define XSECURE_SHA_512_HASH_LEN		(64U) /**< SHA_512 block length */
#define XSECURE_SHA3_256_HASH_LEN		(32U) /**< SHA3_256 block length */
#define XSECURE_SHA2_256_BLOCK_LEN		(64U) /**< SHA2_256 block length */
#define XSECURE_SHA3_BLOCK_LEN			(104U)/**< SHA3 block length */
#define XSECURE_SHA2_384_BLOCK_LEN		(128U)/**< SHA2_384 block length */
#define XSECURE_SHAKE_256_BLOCK_LEN		(136U)/**< SHAKE_256 block length */
#define XSECURE_SHA3_384_HASH_LEN		(48U) /**< SHA3_384 hash length */
#define XSECURE_SHA2_384_HASH_LEN		(48U) /**< SHA2_384 hash length */
#define XSECURE_SHAKE_256_HASH_LEN		(32U) /**< SHAKE_256 hash length */
#define XSECURE_SHA2_256_HASH_LEN		(32U) /**< SHA2_256 hash length */
#define XSECURE_SHA3_384_HASH_WORD_LEN		(XSECURE_SHA3_384_HASH_LEN / XCSUDMA_WORD_SIZE)
							/**< SHA3_384 hash word length */
#define XSECURE_SHA2_384_HASH_WORD_LEN		(XSECURE_SHA2_384_HASH_LEN / XCSUDMA_WORD_SIZE)
							/**< SHA2_384 hash word length */
#define XSECURE_SHAKE_256_HASH_WORD_LEN		(XSECURE_SHAKE_256_HASH_LEN / XCSUDMA_WORD_SIZE)
							/**< SHAKE_256 hash word length */
#define XSECURE_SHA2_256_HASH_WORD_LEN		(XSECURE_SHA2_256_HASH_LEN / XCSUDMA_WORD_SIZE)
							/**< SHA2_256 hash word length */
#define XSECURE_SHA_RESET_OFFSET            	(0x04U) /**< Reset Register */
#define XSECURE_SHA_MODE_OFFSET             	(0xA0U) /**< SHA Mode Register */
#define XSECURE_SHA_AUTO_PADDING_OFFSET     	(0xA4U) /**< SHA Auto Padding Register */
#define XSECURE_SHA_DONE_OFFSET             	(0x08U) /**< SHA Done Register */
#define XSECURE_SHA_RESET_ASSERT		(1U)	/**< SHA Reset Assert Register */
#define XSECURE_SHA_RESET_DEASSERT		(0U)	/**< SHA Reset Deassert Register */
#define	XSECURE_SHA_START_VALUE			(1U)	/**< SHA Start Value */
#define	XSECURE_SHA_DONE_VALUE			(1U)	/**< SHA Done Value */
#define	XSECURE_SHA_AUTO_MODE_ENABLE		(1U)	/**< SHA AUTO_MODE enable */
#define SHA256					(0U) /** SHA256 mode */
#define SHA384					(1U) /** SHA384 mode */
#define SHA512					(2U) /** SHA512 mode */
#define SHAKE256				(4U) /** SHAKE256 mode */
#define XSECURE_SHA_TIMEOUT_MAX			(0x1FFFFU) /**< SHA Timeout max */
#define XCSUDMA_WORD_SIZE			(4U)	/**< WORD size */
#define XSECURE_HASH_SIZE_IN_BYTES		(48U) 	/**< SHA3-384 hash in size of bytes */
#define XSECURE_SHA_DIGEST_OFFSET		(0x10U) /**< SHA Digest Register */

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/***************************** Function Prototypes ******************************************/
int XSecure_ShaInitialize(XSecure_Sha* const InstancePtr, XPmcDma* DmaPtr);
int XSecure_ShaStart(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode);
int XSecure_ShaUpdate(XSecure_Sha* const InstancePtr, u64 DataAddr, const u32 Size);
int XSecure_ShaFinish(XSecure_Sha* const InstancePtr, u64 HashAddr, u32 HashBufSize);
int XSecure_ShaDigest(XSecure_Sha* const InstancePtr, XSecure_ShaMode ShaMode, const u64 DataAddr,
u32 DataSize, u64 HashAddr, u32 HashBufSize);
int XSecure_ShaLastUpdate(XSecure_Sha *InstancePtr);
void XSecure_ShaSetDataContext(XSecure_Sha *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_SHA_H_ */
