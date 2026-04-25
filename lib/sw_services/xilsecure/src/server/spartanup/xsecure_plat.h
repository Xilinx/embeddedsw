/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/spartanup/xsecure_plat.h
* This file contains versal specific code for spartan ultrascale plus.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.5   kpt   08/16/22 Initial release
* 5.6   mb    02/28/25 Added XSecure_AesOp enum
* 5.6   aa    07/21/25 Removed unused macros
*       mb    09/11/25 Added SHA3_384 mode related macros.
* 5.7   mb    03/13/26 Add support for ECC curves for SPARTANUPLUSAES1 device
*       tbk   04/07/26 Added combined KUP and expanded key clear mask and enum
* 5.7   mb    04/17/26 Update crypto check for AES/SHA/RSA
*
* </pre>
*
******************************************************************************/
/**
 * @addtogroup xsecure_helper_server_apis Platform specific helper APIs in XilSecure server
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
#ifdef SPARTANUPLUSAES1
#include "xsecure_trng.h"
#include "xsecure_config.h"
#endif

/************************** Constant Definitions ****************************/

#define XSECURE_AES_BASEADDR		(0x040F0000U)	/**< AES base address */

#define XSECURE_SSS_ADDRESS		(0x040A0090U) /**< SSS base address */

#define XSECURE_SSS_MAX_SRCS		(5U)	/**< SSS Maximum resources */

#define XSECURE_SSS_SHA_MASK		(0xF00U) /**< SSS SHA3 instance 0 mask value*/

#define XSECURE_SSS_SHA_DMA0_VAL	(0x500U) /**< SSS SHA3 instance 0 DMA0 value*/
#define XSECURE_AES_NO_CFG_DST_DMA 	(0xFFFFFFFFU) /**< Not to configure Dst DMA at this address in AES*/

#define XSECURE_SSS_SBI_MASK	(0xF000U)	/**< SSS SBI mask */
#define XSECURE_SSS_AES_MASK	(0xF0U)	/**< SSS AES mask */
#define XSECURE_SSS_DMA0_MASK	(0xFU)	/**< SSS DMA0 mask */
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)	/**< SSS Source select mask */
#define XSECURE_SSS_SBI_DMA0_VAL	(0x5000U)	/**< SSS SBI DMA0 value */
#define XSECURE_SSS_AES_DMA0_VAL	(0x50U)	/**< SSS AES DMA0 value */

/* Key select values */
#define XSECURE_AES_KEY_SEL_BH_KEY		(0xBDB06600U)	/**< BH Key */
#define XSECURE_AES_KEY_SEL_BH_RD_KEY		(0xBDB08200U)	/**< BH Read Key */
#define XSECURE_AES_KEY_SEL_EFUSE_KEY		(0xEFDE6600U)	/**< eFUSE Key */
#define XSECURE_AES_KEY_SEL_EFUSE_RED_KEY	(0xEFDE8200U)	/**< eFUSE Red Key */
#define XSECURE_AES_KEY_SEL_KUP_KEY		(0xBDC98200U)	/**< KUP Key */
#define XSECURE_AES_KEY_SEL_FAMILY_KEY		(0xFEDE8200U)	/**< Family Key */
#define XSECURE_AES_KEY_SEL_PUF_KEY		(0xDBDE8200U)	/**< PUF Key */
#define XSECURE_AES_KEY_SEL_USR_KEY_0		(0xBD858201U)	/**< User Key 0 */
#define XSECURE_AES_KEY_SEL_USR_KEY_1		(0xBD858202U)	/**< User Key 1 */
#define XSECURE_AES_KEY_SEL_USR_KEY_2		(0xBD858204U)	/**< User Key 2 */
#define XSECURE_AES_KEY_SEL_USR_KEY_3		(0xBD858208U)	/**< User Key 3 */
#define XSECURE_AES_KEY_SEL_USR_KEY_4		(0xBD858210U)	/**< User Key 4 */
#define XSECURE_AES_KEY_SEL_USR_KEY_5		(0xBD858220U)	/**< User Key 5 */
#define XSECURE_AES_KEY_SEL_USR_KEY_6		(0xBD858240U)	/**< User Key 6 */
#define XSECURE_AES_KEY_SEL_USR_KEY_7		(0xBD858280U)	/**< User Key 7 */

/**
 * @name  AES_KEY_CLEAR register
 * @{
 */
/**< AES_KEY_CLEAR register offset and definitions */
#define XSECURE_AES_KEY_CLEAR_OFFSET			(0x00000014U)
							/**< AES Key Clear register offset */

#define XSECURE_AES_KEY_CLEAR_PUF_KEY_MASK		(0x00200000U)
							/**< PUF key clear mask */

#define XSECURE_AES_KEY_CLEAR_BH_RED_KEY_MASK		(0x00080000U)
							/**< BH Red key clear mask */

#define XSECURE_AES_KEY_CLEAR_BH_KEY_MASK		(0x00040000U)
							/**< BH key clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_RED_KEY_MASK	(0x00008000U)
							/**< eFUSE Red key clear mask */

#define XSECURE_AES_KEY_CLEAR_EFUSE_KEY_MASK		(0x00001000U)
							/**< eFUSE key clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_7_MASK		(0x00000800U)
							/**< User key 7 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_6_MASK		(0x00000400U)
							/**< User key 6 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_5_MASK		(0x00000200U)
							/**< User key 5 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_4_MASK		(0x00000100U)
							/**< User key 4 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_3_MASK		(0x00000080U)
							/**< User key 3 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_2_MASK		(0x00000040U)
							/**< User key 2 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_1_MASK		(0x00000020U)
							/**< User key 1 clear mask */

#define XSECURE_AES_KEY_CLEAR_USER_KEY_0_MASK		(0x00000010U)
							/**< User key 0 clear mask */

#define XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK		(0x00000002U)
							/**< KUP key clear mask */

#define XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK	(0x00000001U)
							/**< AES key zeroize mask */

#define XSECURE_AES_KEY_CLR_REG_CLR_MASK		(0x00000000U)
							/**< Key clear register clear mask */

#define XSECURE_AES_KEY_CLEAR_ALL_KEYS_MASK		(0x003FFFF3U)
							/**< Clear all keys mask */

#define XSECURE_AES_KEY_CLEAR_PUF_RED_EXPANDED_KEYS_MASK	(0x00288003U)
							/**< PUF Red expanded keys clear mask */

/** Combined mask to clear KUP key and expanded (zeroize) key simultaneously */
#define XSECURE_AES_KEY_CLEAR_KUP_AND_EXP_KEYS_MASK	(XSECURE_AES_KEY_CLEAR_KUP_KEY_MASK | \
							 XSECURE_AES_KEY_CLEAR_AES_KEY_ZEROIZE_MASK)
/** @} */

#define XSECURE_EFUSE_CONTROLS_ADDRESS		(0x04161004U) /**< Efuse Controls base address */
#define XSECURE_EFUSE_EXPORT_CONTROL_MASK	(0x00000020U) /**< Export control efuse bit mask */

#define XSECURE_PMC_GLOBAL_CRYPTO_DIS_ADDRESS	(0x040A0240U) /**< PMC Global Crypto Disable Address */
#define XSECURE_AES_CRYPTO_DIS_MASK		(0x00000001U) /**< AES Crypto Disable mask */
#define XSECURE_SHA_CRYPTO_DIS_MASK		(0x00000002U) /**< SHA Crypto Disable mask */

#define XSECURE_AES_KEY_DEC_SEL_BH_RED		(0x1U)
				/**< BH Red Key Decryption Source Select */
#define XSECURE_AES_KEY_DEC_SEL_EFUSE_RED	(0x2U)
				/**< eFUSE Red Key Decryption Source Select */

#define XSECURE_AES_INVALID_CFG		(0xFFFFFFFFU)	/**< Invalid Configuration */

#define XSECURE_MAX_KEY_SOURCES		XSECURE_AES_EXPANDED_KEYS
						/**< Max key source value */

#define XSECURE_SHA3_256_HASH_LEN		(32U) /**< SHA3_256 block length */
#define XSECURE_SHA2_256_HASH_LEN		(32U) /**< SHA2_256 hash length */
#define XSECURE_SHAKE_256_HASH_LEN		(32U) /**< SHAKE_256 hash length */

#define SHA256					(0U) /**< SHA256 mode */
#define SHAKE256				(1U) /**< SHAKE256 mode */

/** Macros applicable for only SPARTANUPLUSAES1 device */
#ifdef SPARTANUPLUSAES1
#define SHA384					(2U) /** SHA384 mode */
#define XSECURE_SHA3_384_HASH_LEN		(48U) /**< SHA3_384 hash length */
#define XSECURE_RSA_CRYPTO_DIS_MASK		(0x00000004U) /**< RSA Crypto Disable mask */
#endif

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
	XSECURE_AES_KUP_AND_EXPANDED_KEYS,	/**< KUP and Expanded keys */
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

/** Sources to be selected the core to check sticky bits for AES, SHA, RSA */
typedef enum {
	XSECURE_CORE_AES = 0,	/**< AES Core */
	XSECURE_CORE_SHA,	/**< SHA Core */
#ifdef SPARTANUPLUSAES1
	XSECURE_CORE_RSA_ECC	/**< RSA/ECC Core */
#endif
} XSecure_CoreSrc;

/** This structure contains parameters to configure DMA for AES */
typedef struct {
	u64 SrcDataAddr;	/**< Address of source buffer */
	u64 DestDataAddr;	/**< Address of destination buffer */
	u8 SrcChannelCfg;	/**< DMA Source channel configuration */
	u8 DestChannelCfg;	/**< DMA destination channel configuration  */
	u8 IsLastChunkSrc;	/**< Flag for last update in source */
	u8 IsLastChunkDest;	/**< Flag for last update in destination */
} XSecure_AesDmaCfg;

/** Used to select the AES Encrypt/ Decrypt operation. */
typedef enum {
	XSECURE_ENCRYPT,        /**< Encrypt operation */
	XSECURE_DECRYPT,        /**< Decrypt operation */
} XSecure_AesOp;

/** XilSecure ECC curve types */
typedef enum {
      XSECURE_ECC_NIST_P192 = 1,			/**< NIST P-192 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P224 = 2,			/**< NIST P-224 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P256 = 3,			/**< NIST P-256 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P384 = 4,			/**< NIST P-384 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P521 = 5				/**< NIST P-521 curve value in Ecdsa.h */
} XSecure_EllipticCrvTyp;

/***************************** Function Prototypes ***************************/
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_AesPlatPmcDmaCfgAndXfer(XPmcDma *PmcDmaPtr, const XSecure_AesDmaCfg *AesDmaCfg, u32 Size, UINTPTR BaseAddress);
void XSecure_AesPmcDmaCfgEndianness(XPmcDma *InstancePtr,
	XPmcDma_Channel Channel, u8 EndianType);
int XSecure_AesValidateSize(u32 Size, u8 IsLastChunk);
int XSecure_ShaDmaXfer(void *InstancePtr, u64 DataAddr, u32 Size, u8 IsLastUpdate);
int XSecure_CryptoCheck(XSecure_CoreSrc CoreSrc);
#ifdef SPARTANUPLUSAES1
void XSecure_UpdateTrngCryptoStatus(u32 Op);
int XSecure_GetRandomNum(u8 *Output, u32 Size);
int XSecure_ECCRandInit(void);
#endif /** SPARTANUPLUSAES1 */

/**
 * @}
 * @endcond
 */

/***************************** Variable Prototypes  ***************************/

/** @} */
#ifdef __cplusplus
}
#endif

#endif /** XSECURE_PLAT_H */
