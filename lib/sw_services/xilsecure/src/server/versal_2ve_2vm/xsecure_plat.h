/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
*
* This file contains Versal_2Ve_2Vm specific code for Xilsecure server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       sk   08/22/24 Added defines for key transfer to ASU
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Platform specific helper APIs in Xilsecure server
* @{
*/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_error.h"
#include "xsecure_aes_core_hw.h"
#include "xpmcdma.h"
#include "xtrngpsx.h"
#include "xsecure_plat_defs.h"
#include "xsecure_trng.h"
#include "xsecure_core.h"

/************************** Constant Definitions ****************************/

#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */

#define XSECURE_SSS_MAX_SRCS	(8U)	/**< Maximum resources */

#define XSECURE_SSS_SHA3_MASK		(0xF0000U) /**<SSS SHA3 instance mask value*/
#define XSECURE_SSS_SHA2_MASK		(0xF000000U) /**<SSS SHA3 instance mask value*/

#define XSECURE_SSS_SHA3_DMA0_VAL	(0xC0000U) /**<SSS SHA3 instance  DMA0 value*/
#define XSECURE_SSS_SHA3_DMA1_VAL	(0x70000U) /**<SSS SHA3 instance  DMA1 value*/

#define XSECURE_SSS_SHA2_DMA0_VAL	(0xA000000U) /**<SSS SHA2 instance DMA0 value*/
#define XSECURE_SSS_SHA2_DMA1_VAL	(0xF000000U) /**<SSS SHA2 instance DMA1 value*/

#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)  /**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)  /**< Disables data swap in AES */

#if !defined(XSECURE_TRNG_USER_CFG_SEED_LIFE)
#define XSECURE_TRNG_USER_CFG_SEED_LIFE XTRNGPSX_USER_CFG_SEED_LIFE
								/**< User configuration seed life*/
#endif

#if !defined(XSECURE_TRNG_USER_CFG_DF_LENGTH)
#define XSECURE_TRNG_USER_CFG_DF_LENGTH XTRNGPSX_USER_CFG_DF_LENGTH
								/**< User configuration DF length */
#endif

#if !defined(XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF
								/**< Adapt test cutoff */
#endif

#if !defined(XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF)
#define XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF XTRNGPSX_USER_CFG_REP_TEST_CUTOFF
								/**< Rep test cutoff */
#endif

#define XSECURE_AES_KTE_GO_ADDRESS		(0xF11E0234U)	/**< AES KTE GO address */
#define XSECURE_AES_KTE_RESTART_ADDRESS		(0xF11E0238U)	/**< AES Key Transfer Engine Restart register */
#define XSECURE_AES_KTE_DONE_ADDRESS		(0xF11E023CU)	/**< AES Key Transfer Engine Done */
#define XSECURE_AES_KTE_CNT_ADDRESS		(0xF11E0240U)	/**< AES Key Transfer Engine Count address */
#define XSECURE_AES_KTE_GO_ENABLE		(0x1U)		/**< AES KTE GO enable */
#define XSECURE_AES_KTE_GO_DISABLE		(0x0U)		/**< AES KTE GO disable */
#define XSECURE_AES_KTE_DONE_MASK		(0x1U) 		/**< AES Key Transfer Engine Done mask */
#define XSECURE_AES_KTE_CNT_MASK		(0x6U)		/**< AES Key Transfer Engine Count value */
#define XSECURE_AES_KTE_DONE_POLL_TIMEOUT	(4000U)		/**< AES Key Transfer complete poll timeout */

#define XSECURE_SSS_SBI_MASK	(0xF00000U)
#define XSECURE_SSS_AES_MASK	(0xF000U)
#define XSECURE_SSS_DMA1_MASK	(0xF0U)
#define XSECURE_SSS_DMA0_MASK	(0xFU)
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)
#define XSECURE_SSS_SBI_DMA0_VAL	(0x500000U)
#define XSECURE_SSS_SBI_DMA1_VAL	(0xB00000U)
#define XSECURE_SSS_AES_DMA0_VAL	(0xE000U)
#define XSECURE_SSS_AES_DMA1_VAL	(0x5000U)

#define XCSUDMA_WORD_SIZE			(4U)	/**< WORD size */
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

#define SHA256					(0U) /** SHA256 mode */
#define SHA384					(1U) /** SHA384 mode */
#define SHA512					(2U) /** SHA512 mode */
#define SHAKE256				(4U) /** SHAKE256 mode */

/***************************** Type Definitions******************************/

/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1,  /**< Ignore */
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_DMA1, /**< DMA1 */
	XSECURE_SSS_PTPI, /**< PTPI */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA3, /**< SHA3 */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_SHA2, /**< SHA2 */
	XSECURE_SSS_INVALID /**< Invalid */
} XSecure_SssSrc;

typedef struct {
	u64 SrcDataAddr;        /**< Address of source buffer */
	u64 DestDataAddr;       /**< Address of destination buffer */
	u8 SrcChannelCfg;       /**< DMA Source channel configuration */
	u8 DestChannelCfg;      /**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;      /**< Flag for last update in source */
	u8 IsLastChunkDest;     /**< Flag for last update in destination */
}XSecure_AesDmaCfg;

typedef struct {
	u32 RegOffset;	/**< Register offset for key source */
	u32 KeySrcSelVal;	/**< Selection value for key source */
	u8  UsrWrAllowed;	/**< User write allowed or not for key source */
	u8  DecAllowed;		/**< Decryption allowed or not for key source */
	u8  EncAllowed;		/**< Encryption allowed or not for key source */
	u8  KeyDecSrcAllowed;	/**< Key decryption source allowed */
	u32 KeyDecSrcSelVal;	/**< Selection value for key decryption source*/
	u32 KeyClearVal;	/**< Key source clear value*/
} XSecure_AesKeyLookup;

/***************************** static inline function ***************************/

/*******************************************************************************************/
/**
* @brief	This function validates SHA input data size.
*
* @param	Size - Input data size in bytes.
*
* @return
*		XST_SUCCESS - Upon Success.
*
 *******************************************************************************************/
static inline int XSecure_ValidateShaDataSize(const u32 Size) {
	(void)Size;
	return XST_SUCCESS;
}

/***************************** Function Prototypes ***************************/

void XSecure_SetRsaCryptoStatus(void);
void XSecure_UpdateCryptoStatus(UINTPTR BaseAddress, u32 Op);
void XSecure_UpdateTrngCryptoStatus(u32 Op);
void XSecure_ConfigureDmaByteSwap(u32 Op);
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
       XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
int XSecure_ECCRandInit(void);
XTrngpsx_Instance *XSecure_GetTrngInstance(void);
int XSecure_InitiateASUKeyTransfer(void);
int XSecure_ShaDmaXfer(XPmcDma *InstancePtr, u64 DataAddr, u32 Size, u8 IsLastUpdate);
int XSecure_MemCpyAndChangeEndianness(u64 DestAddress, u64 SrcAddress, u32 Length);
/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */

/* @} */
