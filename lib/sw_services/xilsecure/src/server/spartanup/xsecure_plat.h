/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat.h
* This file contains versal specific code for spartan ultrascale plus.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   kpt   08/16/22 Initial release
* 5.6   mb    02/28/25 Added XSecure_AesOp enum
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_helper_server_apis Xilsecure Helper Server APIs
* @{
*/
#ifndef XSECURE_PLAT_H
#define XSECURE_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_aes_core_hw.h"
#include "xpmcdma.h"
#include "xsecure_error.h"

/************************** Constant Definitions ****************************/

#define XSECURE_AES_BASEADDR				(0x040F0000U)
						/**< AES Base Address */

#define XSECURE_SSS_ADDRESS		(0x040A0090U) /**< SSS base address */

#define XSECURE_SSS_MAX_SRCS		(5U)	/**< SSS Maximum resources */

#define XSECURE_SSS_SHA_MASK		(0xF00U) /**< SSS SHA3 instance 0 mask value*/

#define XSECURE_SSS_SHA_DMA0_VAL	(0x500U) /**< SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_AES_NO_CFG_DST_DMA 	(0xFFFFFFFFU) /**< Not to configure Dst DMA at this address in AES*/
#define XSECURE_ENABLE_BYTE_SWAP	(0x1U)	/**< Enables data swap in AES */

#define XSECURE_DISABLE_BYTE_SWAP	(0x0U)	/**< Disables data swap in AES */

#define XSECURE_SSS_SBI_MASK	(0xF000U)
#define XSECURE_SSS_AES_MASK	(0xF0U)
#define XSECURE_SSS_DMA0_MASK	(0xFU)
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)
#define XSECURE_SSS_SBI_DMA0_VAL	(0x5000U)
#define XSECURE_SSS_AES_DMA0_VAL	(0x50U)

/* Key select values */
#define XSECURE_AES_KEY_SEL_BH_KEY			(0xBDB06600U)
#define XSECURE_AES_KEY_SEL_BH_RD_KEY			(0xBDB08200U)
#define XSECURE_AES_KEY_SEL_EFUSE_KEY			(0xEFDE6600U)
#define XSECURE_AES_KEY_SEL_EFUSE_RED_KEY		(0xEFDE8200U)
#define XSECURE_AES_KEY_SEL_KUP_KEY			(0xBDC98200U)
#define XSECURE_AES_KEY_SEL_FAMILY_KEY		(0xFEDE8200U)
#define XSECURE_AES_KEY_SEL_PUF_KEY			(0xDBDE8200U)
#define XSECURE_AES_KEY_SEL_USR_KEY_0			(0xBD858201U)
#define XSECURE_AES_KEY_SEL_USR_KEY_1			(0xBD858202U)
#define XSECURE_AES_KEY_SEL_USR_KEY_2			(0xBD858204U)
#define XSECURE_AES_KEY_SEL_USR_KEY_3			(0xBD858208U)
#define XSECURE_AES_KEY_SEL_USR_KEY_4			(0xBD858210U)
#define XSECURE_AES_KEY_SEL_USR_KEY_5			(0xBD858220U)
#define XSECURE_AES_KEY_SEL_USR_KEY_6			(0xBD858240U)
#define XSECURE_AES_KEY_SEL_USR_KEY_7			(0xBD858280U)

/**
 * @name  AES_KEY_CLEAR register
 * @{
 */
/**< AES_KEY_CLEAR register offset and definitions */
#define XSECURE_AES_KEY_CLEAR_OFFSET			(0x00000014U)

#define XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK		(0x00200000U)

#define XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK		(0x00080000U)

#define XSECURE_AES_KEY_CLEAR_BH_KEY_MASK		(0x00040000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK	(0x00008000U)

#define XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK		(0x00001000U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK		(0x00000800U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK		(0x00000400U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK		(0x00000200U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK		(0x00000100U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK		(0x00000080U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK		(0x00000040U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK		(0x00000020U)

#define XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK		(0x00000010U)

#define XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK		(0x00000002U)

#define XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK	(0x00000001U)

#define XSECURE_AES_KEY_CLR_REG_CLR_MASK		(0x00000000U)

#define XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK		(0x003FFFF3U)

#define XSECURE_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK	(0x00288003U)

#define XSECURE_AES_KEY_DEC_SEL_BH_RED			(0x1U)
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED		(0x2U)

#define XSECURE_AES_INVALID_CFG				(0xFFFFFFFFU)

#define XSECURE_MAX_KEY_SOURCES			XSECURE_AES_EXPANDED_KEYS
										/**< Max key source value */

#define XCSUDMA_WORD_SIZE			(4U)	/**< WORD size */
#define XSECURE_SHA3_256_HASH_LEN		(32U) /**< SHA3_256 block length */
#define XSECURE_SHA2_256_BLOCK_LEN		(64U) /**< SHA2_256 block length */
#define XSECURE_SHAKE_256_BLOCK_LEN		(136U)/**< SHAKE_256 block length */
#define XSECURE_SHAKE_256_HASH_LEN		(32U) /**< SHAKE_256 hash length */
#define XSECURE_SHAKE_256_HASH_WORD_LEN		(XSECURE_SHAKE_256_HASH_LEN / XCSUDMA_WORD_SIZE)
							/**< SHAKE_256 hash word length */

#define SHA256					(0U) /** SHA256 mode */
#define SHAKE256				(1U) /** SHAKE256 mode */

/***************************** Type Definitions******************************/

/** Used for selecting the Key source of AES Core. */
typedef enum {
	XSECURE_AES_BH_KEY = 0,			/**< BH Key */
	XSECURE_AES_BH_RED_KEY,			/**< BH Red Key */
	XSECURE_AES_EFUSE_KEY,			/**< eFUSE Key */
	XSECURE_AES_EFUSE_RED_KEY,		/**< eFUSE Red Key */
	XSECURE_AES_KUP_KEY,			/**< KUP key */
	XSECURE_AES_FAMILY_KEY,         /**< Family key */
	XSECURE_AES_PUF_KEY,			/**< PUF key */
	XSECURE_AES_USER_KEY_0,			/**< User Key 0 */
	XSECURE_AES_USER_KEY_1,			/**< User Key 1 */
	XSECURE_AES_USER_KEY_2,			/**< User Key 2 */
	XSECURE_AES_USER_KEY_3,			/**< User Key 3 */
	XSECURE_AES_USER_KEY_4,			/**< User Key 4 */
	XSECURE_AES_USER_KEY_5,			/**< User Key 5 */
	XSECURE_AES_USER_KEY_6,			/**< User Key 6 */
	XSECURE_AES_USER_KEY_7,			/**< User Key 7 */
	XSECURE_AES_EXPANDED_KEYS,		/**< Expanded keys */
	XSECURE_AES_PUF_RED_EXPANDED_KEYS,	/**< AES PUF,RED,KUP keys */
	XSECURE_AES_ALL_KEYS,			/**< AES All keys */
	XSECURE_AES_INVALID_KEY,		/**< AES Invalid Key */
} XSecure_AesKeySrc;

/** Sources to be selected to configure secure stream switch. */
typedef enum {
	XSECURE_SSS_DMA0 = 0, /**< DMA0 */
	XSECURE_SSS_AES, /**< AES */
	XSECURE_SSS_SHA, /**< SHA */
	XSECURE_SSS_SBI, /**< SBI */
	XSECURE_SSS_INVALID /**< Invalid */
}XSecure_SssSrc;

/** This structure contains parameters to configure DMA for AES */
typedef struct {
	u64 SrcDataAddr;	/**< Address of source buffer */
	u64 DestDataAddr;	/**< Address of destination buffer */
	u8 SrcChannelCfg;	/**< DMA Source channel configuration */
	u8 DestChannelCfg;	/**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;	/**< Flag for last update in source */
	u8 IsLastChunkDest;	/**< Flag for last update in destination */
} XSecure_AesDmaCfg;

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

/** Used to select the AES Encrypt/ Decrypt operation. */
typedef enum {
	XSECURE_ENCRYPT,        /**< Encrypt operation */
	XSECURE_DECRYPT,        /**< Decrypt operation */
} XSecure_AesOp;

/***************************** Function Prototypes ***************************/
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
int XSecure_ValidateShaDataSize(const u32 Size);
int XSecure_ShaDmaXfer(XPmcDma *DmaPtr, u64 DataAddr, u32 Size, u8 IsLastUpdate);
int XSecure_CryptoCheck(void);

/**
 * @}
 * @endcond
 */

/***************************** Variable Prototypes  ***************************/

#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */
