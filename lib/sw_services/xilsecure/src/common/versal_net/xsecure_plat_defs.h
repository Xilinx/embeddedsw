/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_defs.h
*
* This file contains the xsecure API IDs for versalnet
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
*       kpt  07/24/22 Added XSECURE_API_KAT and additional KAT ids
*       dc   08/26/22 Removed gaps in between the API IDs
* 5.1   kpt  01/04/22 Add macros related to KAT for external modules
* 5.2   vns  07/06/23 Added separate IPI commands for Crypto Status and KAT status updates
*       kpt  07/09/23 Added Key wrap and unwrap structures and macros
*       mmd  07/20/23 Added reading FIPS info for HMAC and SHA2
*       kpt  08/30/23 Updated XSECURE_KAT_HDR_LEN
*	vss  09/21/23 Fixed doxygen warnings
* 5.3   kpt  12/13/23 Added XSECURE_API_RSA_SCA_RESISTANCE_PRIVATE_DECRYPT
*       har  02/05/24 Added XSECURE_API_AES_OP_N_ZEROIZE_KEY
* 5.3   mb   04/01/24 Updated minor version
*       mb   04/15/24 Updated SHA2 minor version
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*       kpt  05/14/24 Fix backward compatabilty issue
*       kpt  05/26/24 Add support for RSA CRT and RRN operation
*       kpt  06/13/24 Add support for XSECURE_API_RSA_RELEASE_KEY
*       kpt  06/30/24 Updated version number
* 5.5   vss  04/25/25 Updated minor version
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_common_apis Xilsecure Common Apis
* @{
*/
#ifndef XSECURE_PLAT_DEFS_H
#define XSECURE_PLAT_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**< KAT macros */
#define XSECURE_KAT_HDR_LEN		(1U) /**< Header Length*/
#define XSECURE_MAX_KAT_MASK_LEN	(3U) /**< Maximum mask length*/
#define XSECURE_MIN_KAT_MASK_LEN	(1U) /**< Minimum mask length*/

#define XSECURE_WORD_LEN		(4U) /**< Word length */
#define XSECURE_ADDR_HIGH_SHIFT		(32U) /**< Shift for getting higher address*/

#define XSECURE_KEY_ID_LEN_IN_BYTES (128U) /**< Key id length in bytes */

#define XSECURE_SHARED_KEY_STORE_SIZE_OFFSET   (8U)           /**< Key size offset */
#define XSECURE_SHARED_KEY_SLOT_STATUS_START_OFFSET (12U)     /**< Bitmap offset */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/**< XilSecure API ids */
typedef enum {
	XSECURE_API_FEATURES = 0U,		/**< 0U */
	XSECURE_API_RSA_SIGN_VERIFY,		/**< 1U */
	XSECURE_API_RSA_PUBLIC_ENCRYPT,		/**< 2U */
	XSECURE_API_RSA_PRIVATE_DECRYPT,	/**< 3U */
	XSECURE_API_SHA3_UPDATE,			/**< 4U */
	XSECURE_API_ELLIPTIC_GENERATE_KEY,	/**< 5U */
	XSECURE_API_ELLIPTIC_GENERATE_SIGN,	/**< 6U */
	XSECURE_API_ELLIPTIC_VALIDATE_KEY,	/**< 7U */
	XSECURE_API_ELLIPTIC_VERIFY_SIGN,	/**< 8U */
	XSECURE_API_AES_INIT,				/**< 9U */
	XSECURE_API_AES_OP_INIT,			/**< 10U */
	XSECURE_API_AES_UPDATE_AAD,		/**< 11U */
	XSECURE_API_AES_ENCRYPT_UPDATE,		/**< 12U */
	XSECURE_API_AES_ENCRYPT_FINAL,		/**< 13U */
	XSECURE_API_AES_DECRYPT_UPDATE,		/**< 14U */
	XSECURE_API_AES_DECRYPT_FINAL,		/**< 15U */
	XSECURE_API_AES_KEY_ZERO,		/**< 16U */
	XSECURE_API_AES_WRITE_KEY,		/**< 17U */
	XSECURE_API_AES_LOCK_USER_KEY,		/**< 18U */
	XSECURE_API_AES_KEK_DECRYPT,		/**< 19U */
	XSECURE_API_AES_SET_DPA_CM,		/**< 20U */
	XSECURE_API_KAT,			/**< 21U */
	XSECURE_API_TRNG_GENERATE,		/**< 22U */
	XSECURE_API_AES_PERFORM_OPERATION, /**< 23U */
	XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS,  /**< 24U */
	XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS, /**< 25U */
	XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS, /**< 26U */
	XSECURE_API_UPDATE_PKI_CRYPTO_STATUS, 	/**< 27U */
	XSECURE_API_UPDATE_DDR_KAT_STATUS,	/**< 28U */
	XSECURE_API_UPDATE_HNIC_KAT_STATUS,	/**< 29U */
	XSECURE_API_UPDATE_CPM5N_KAT_STATUS,	/**< 30U */
	XSECURE_API_UPDATE_PCIDE_KAT_STATUS,	/**< 31U */
	XSECURE_API_UPDATE_PKI_KAT_STATUS,	/**< 32U */
	XSECURE_API_GEN_SHARED_SECRET,		/**< 33U */
	/* API Id 34 is reserved */
	XSECURE_API_KEY_UNWRAP = 35,                 	/**< 35U */
	XSECURE_API_RESERVED, /**< 36U */
	XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY,/**< 37U */
	XSECURE_API_RSA_RELEASE_KEY,/**< 38U */
	XSECURE_API_MAX,				/**< 39U */
} XSecure_ApiId;

/**< XilSecure KAT ids */
typedef enum {
	XSECURE_API_AES_DECRYPT_KAT = 0U,		/**< 0U */
	XSECURE_API_AES_DECRYPT_CM_KAT,			/**< 1U */
	XSECURE_API_RSA_PUB_ENC_KAT,			/**< 2U */
	XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT,	/**< 3U */
	XSECURE_API_SHA3_KAT,					/**< 4U */
	XSECURE_API_AES_ENCRYPT_KAT,			/**< 5U */
	XSECURE_API_RSA_PRIVATE_DEC_KAT,		/**< 6U */
	XSECURE_API_ELLIPTIC_SIGN_GEN_KAT,		/**< 7U */
	XSECURE_API_TRNG_KAT,					/**< 8U */
	XSECURE_API_UPDATE_KAT_STATUS,			/**< 9U */
} XSecure_KatId;

typedef enum {
	XSECURE_DDR_0_AES_GCM_KAT = 0U,  /**< 0U */
	XSECURE_DDR_0_AES_XTS_KAT, 		 /**< 1U */
	XSECURE_DDR_0_KDF_KAT, 			 /**< 2U */
	XSECURE_DDR_0_TRNG_KAT,			 /**< 3U */
	XSECURE_DDR_1_AES_GCM_KAT, 		 /**< 4U */
	XSECURE_DDR_1_AES_XTS_KAT, 		 /**< 5U */
	XSECURE_DDR_1_KDF_KAT, 			 /**< 6U */
	XSECURE_DDR_1_TRNG_KAT,			 /**< 7U */
	XSECURE_DDR_2_AES_GCM_KAT, 		 /**< 8U */
	XSECURE_DDR_2_AES_XTS_KAT, 		 /**< 9U */
	XSECURE_DDR_2_KDF_KAT, 			 /**< 10U */
	XSECURE_DDR_2_TRNG_KAT, 		 /**< 11U */
	XSECURE_DDR_3_AES_GCM_KAT, 		 /**< 12U */
	XSECURE_DDR_3_AES_XTS_KAT, 		 /**< 13U */
	XSECURE_DDR_3_KDF_KAT, 			 /**< 14U */
	XSECURE_DDR_3_TRNG_KAT, 		 /**< 15U */
	XSECURE_DDR_4_AES_GCM_KAT, 		 /**< 16U */
	XSECURE_DDR_4_AES_XTS_KAT, 		 /**< 17U */
	XSECURE_DDR_4_KDF_KAT, 			 /**< 18U */
	XSECURE_DDR_4_TRNG_KAT, 		 /**< 19U */
	XSECURE_DDR_5_AES_GCM_KAT,		 /**< 20U */
	XSECURE_DDR_5_AES_XTS_KAT,		 /**< 21U */
	XSECURE_DDR_5_KDF_KAT, 			 /**< 22U */
	XSECURE_DDR_5_TRNG_KAT,			 /**< 23U */
	XSECURE_DDR_6_AES_GCM_KAT,		 /**< 24U */
	XSECURE_DDR_6_AES_XTS_KAT,		 /**< 25U */
	XSECURE_DDR_6_KDF_KAT, 			 /**< 26U */
	XSECURE_DDR_6_TRNG_KAT, 		 /**< 27U */
	XSECURE_DDR_7_AES_GCM_KAT, 		 /**< 28U */
	XSECURE_DDR_7_AES_XTS_KAT, 		 /**< 29U */
	XSECURE_DDR_7_KDF_KAT, 			 /**< 30U */
	XSECURE_DDR_7_TRNG_KAT,			 /**< 31U */
} XSecure_DDRKatId;

typedef enum {
	XSECURE_HNIC_AES_GCM_ENC_KAT = 0U,		/**< 0U */
	XSECURE_HNIC_AES_GCM_DECRYPT_KAT,		/**< 1U */
	XSECURE_HNIC_AES_CM_ENC_KAT,			/**< 2U */
	XSECURE_HNIC_AES_CM_DEC_KAT,			/**< 3U */
	XSECURE_HNIC_AES_GCM_EN_CHECK,			/**< 4U */
	XSECURE_HNIC_AES_GCM_BYPASS_CHECK,		/**< 5U */
} XSecure_HnicKatId;

typedef enum {
	XSECURE_CPM5N_AES_XTS_ENC_KAT = 8U,		/**< 8U */
	XSECURE_CPM5N_AES_XTS_DEC_KAT,			/**< 9U */
	XSECURE_CPM5N_AES_CM_ENC_KAT,			/**< 10U */
	XSECURE_CPM5N_AES_CM_DEC_KAT,			/**< 11U */
	XSECURE_CPM5N_AES_XTS_EN_CHECK,			/**< 12U */
	XSECURE_CPM5N_AES_XTS_BYPASS_CHECK,		/**< 13U */
} XSecure_Cpm5nKatId;

typedef enum {
	XSECURE_PCIDE_AES_ENC_KAT = 16U,	/**< 16U */
	XSECURE_PCIDE_AES_DEC_KAT			/**< 17U */
} XSecure_PciIdeKatId;

typedef enum {
	XSECURE_PKI_ECC_P192_SIGN_GEN_KAT = 0U,		/**< 0U */
	XSECURE_PKI_ECC_P192_SIGN_VER_KAT,			/**< 1U */
	XSECURE_PKI_ECC_P192_PWCT,					/**< 2U */
	XSECURE_PKI_ECC_P192_CM_SIGN_GEN_KAT,		/**< 3U */
	XSECURE_PKI_ECC_P192_CM_SIGN_VER_KAT,		/**< 4U */
	XSECURE_PKI_ECC_P192_CM_PWCT,				/**< 5U */
	XSECURE_PKI_ECC_P384_SIGN_GEN_KAT,			/**< 6U */
	XSECURE_PKI_ECC_P384_SIGN_VER_KAT,			/**< 7U */
	XSECURE_PKI_ECC_P384_PWCT,					/**< 8U */
	XSECURE_PKI_ECC_P384_CM_SIGN_GEN_KAT,		/**< 9U */
	XSECURE_PKI_ECC_P384_CM_SIGN_VER_KAT,		/**< 10U */
	XSECURE_PKI_ECC_P384_CM_PWCT,				/**< 11U */
	XSECURE_PKI_ECC_P521_SIGN_GEN_KAT,			/**< 12U */
	XSECURE_PKI_ECC_P521_SIGN_VER_KAT,			/**< 13U */
	XSECURE_PKI_ECC_P521_PWCT,					/**< 14U */
	XSECURE_PKI_ECC_P521_CM_SIGN_GEN_KAT,		/**< 15U */
	XSECURE_PKI_ECC_P521_CM_SIGN_VER_KAT,		/**< 16U */
	XSECURE_PKI_ECC_P521_CM_PWCT,				/**< 17U */
	XSECURE_PKI_ECC_Secp256k1_SIGN_GEN_KAT,		/**< 18U */
	XSECURE_PKI_ECC_Secp256k1_SIGN_VER_KAT,		/**< 19U */
	XSECURE_PKI_ECC_Secp256k1_PWCT,				/**< 20U */
	XSECURE_PKI_ECC_Secp256k1_CM_SIGN_GEN_KAT,	/**< 21U */
	XSECURE_PKI_ECC_Secp256k1_CM_SIGN_VER_KAT,	/**< 22U */
	XSECURE_PKI_ECC_Secp256k1_CM_PWCT,			/**< 23U */
	XSECURE_PKI_ECC_fp256_SIGN_GEN_KAT,			/**< 24U */
	XSECURE_PKI_ECC_fp256_SIGN_VER_KAT,			/**< 25U */
	XSECURE_PKI_ECC_fp256k_PWCT,				/**< 26U */
	XSECURE_PKI_ECC_fp256_CM_SIGN_GEN_KAT,		/**< 27U */
	XSECURE_PKI_ECC_fp256_CM_SIGN_VER_KAT,		/**< 28U */
	XSECURE_PKI_ECC_fp256k_CM_PWCT,				/**< 29U */
	XSECURE_PKI_EdDSA_Ed25519_SIGN_GEN_KAT,		/**< 30U */
	XSECURE_PKI_EdDSA_Ed25519_SIGN_VER_KAT,		/**< 31U */
	XSECURE_PKI_EdDSA_Ed25519_PWCT = 32U,		/**< 32U */
	XSECURE_PKI_EdDSA_Ed25519_CM_SIGN_GEN_KAT,	/**< 33U */
	XSECURE_PKI_EdDSA_Ed25519_CM_SIGN_VER_KAT,	/**< 34U */
	XSECURE_PKI_EdDSA_Ed25519_CM_PWCT,			/**< 35U */
	XSECURE_PKI_EdDSA_Ed448_SIGN_GEN_KAT,		/**< 36U */
	XSECURE_PKI_EdDSA_Ed448_SIGN_VER_KAT,		/**< 37U */
	XSECURE_PKI_EdDSA_Ed448_PWCT,				/**< 38U */
	XSECURE_PKI_EdDSA_Ed448_CM_SIGN_GEN_KAT,	/**< 39U */
	XSECURE_PKI_EdDSA_Ed448_CM_SIGN_VER_KAT,	/**< 40U */
	XSECURE_PKI_EdDSA_Ed448_CM_PWCT,			/**< 41U */
	XSECURE_PKI_ECDH_Curve25519_ENC_KAT,		/**< 42U */
	XSECURE_PKI_ECDH_Curve25519_DEC_KAT,		/**< 43U */
	XSECURE_PKI_ECDH_Curve25519_PWCT,			/**< 44U */
	XSECURE_PKI_ECDH_Curve25519_CM_ENC_KAT,		/**< 45U */
	XSECURE_PKI_ECDH_Curve25519_CM_DEC_KAT,		/**< 46U */
	XSECURE_PKI_ECDH_Curve25519_CM_PWCT,		/**< 47U */
	XSECURE_PKI_ECDH_Curve448_ENC_KAT,			/**< 48U */
	XSECURE_PKI_ECDH_Curve448_DEC_KAT,			/**< 49U */
	XSECURE_PKI_ECDH_Curve448_PWCT,				/**< 50U */
	XSECURE_PKI_ECDH_Curve448_CM_ENC_KAT,		/**< 51U */
	XSECURE_PKI_ECDH_Curve448_CM_DEC_KAT,		/**< 52U */
	XSECURE_PKI_ECDH_Curve448_CM_PWCT,			/**< 53U */
	XSECURE_PKI_RSA_2048_SIGN_GEN_KAT,			/**< 54U */
	XSECURE_PKI_RSA_2048_SIGN_VER_KAT,			/**< 55U */
	XSECURE_PKI_RSA_2048_ENC_KAT,				/**< 56U */
	XSECURE_PKI_RSA_2048_DEC_KAT,				/**< 57U */
	XSECURE_PKI_RSA_2048_PWCT,					/**< 58U */
	XSECURE_PKI_RSA_2048_CM_SIGN_GEN_KAT,		/**< 59U */
	XSECURE_PKI_RSA_2048_CM_SIGN_VER_KAT,		/**< 60U */
	XSECURE_PKI_RSA_2048_CM_ENC_KAT,			/**< 61U */
	XSECURE_PKI_RSA_2048_CM_DEC_KAT,			/**< 62U */
	XSECURE_PKI_RSA_2048_CM_PWCT,				/**< 63U */
	XSECURE_PKI_RSA_3072_SIGN_GEN_KAT = 64U,	/**< 64U */
	XSECURE_PKI_RSA_3072_SIGN_VER_KAT,			/**< 65U */
	XSECURE_PKI_RSA_3072_ENC_KAT,				/**< 66U */
	XSECURE_PKI_RSA_3072_DEC_KAT,				/**< 67U */
	XSECURE_PKI_RSA_3072_PWCT,					/**< 68U */
	XSECURE_PKI_RSA_3072_CM_SIGN_GEN_KAT,		/**< 69U */
	XSECURE_PKI_RSA_3072_CM_SIGN_VER_KAT,		/**< 70U */
	XSECURE_PKI_RSA_3072_CM_ENC_KAT,			/**< 71U */
	XSECURE_PKI_RSA_3072_CM_DEC_KAT,			/**< 72U */
	XSECURE_PKI_RSA_3072_CM_PWCT,				/**< 73U */
	XSECURE_PKI_RSA_4096_SIGN_GEN_KAT,			/**< 74U */
	XSECURE_PKI_RSA_4096_SIGN_VER_KAT,			/**< 75U */
	XSECURE_PKI_RSA_4096_ENC_KAT,				/**< 76U */
	XSECURE_PKI_RSA_4096_DEC_KAT,				/**< 77U */
	XSECURE_PKI_RSA_4096_PWCT,					/**< 78U */
	XSECURE_PKI_RSA_4096_CM_SIGN_GEN_KAT,		/**< 79U */
	XSECURE_PKI_RSA_4096_CM_SIGN_VER_KAT,		/**< 80U */
	XSECURE_PKI_RSA_4096_CM_ENC_KAT,			/**< 81U */
	XSECURE_PKI_RSA_4096_CM_DEC_KAT,			/**< 82U */
	XSECURE_PKI_RSA_4096_CM_PWCT,				/**< 83U */
	XSECURE_PKI_TRNG_HEALTH_TEST,				/**< 84U */
	XSECURE_PKI_TRNG_DRBG_KAT,					/**< 85U */
	XSECURE_PKI_SHA2_256_KAT,					/**< 86U */
	XSECURE_PKI_SHA2_384_KAT,					/**< 87U */
	XSECURE_PKI_SHA2_512_KAT,					/**< 88U */
} XSecure_PkiKatId;

typedef enum {
	XSECURE_CRYPTO_STATUS_SET = 0U, /**< 0U */
	XSECURE_CRYPTO_STATUS_CLEAR, /**< 1U */
}XSecure_CryptoStatusOp;

typedef enum {
	XSECURE_ENC_OP = 0U,	/**< Encryption operation */
	XSECURE_DEC_OP,		/**< Decryption operation */
} XSecure_KeyOp;

/** AES Key metadata for Key wrap unwrap */
typedef struct {
	XSecure_KeyOp KeyOp; /**< Key Operation */
	u32 AesKeySize;      /**< AES Key size */
	u8 KeyId[XSECURE_KEY_ID_LEN_IN_BYTES];      /**< Unique ID to identify key */
} XSecure_KeyMetaData;

/** Input and output parameters for Key wrap */
typedef struct {
	XSecure_KeyMetaData KeyMetaData; /**< Key Meta data */
	u64 KeyWrapAddr;	/**< Key Wrap address */
	u32 TotalWrappedKeySize; /**< Total Wrapped key size */
} XSecure_KeyWrapData;

/** Addresses of Input parameters of RSA */
typedef struct {
	u64 ModulusAddr;	/**< Modulus address */
	u64 ExponentAddr;	/**< Exponent address */
} XSecure_RsaPubKeyAddr;

typedef enum {
	XSECURE_RSA_EXPQ_MODE = 0, /**< RSA EXPQ mode */
	XSECURE_RSA_CRT_MODE,      /**< RSA CRT mode */
	XSECURE_RSA_EXPOPT_MODE    /**< RSA expopt mode */
} XSecure_RsaOperationMode;

typedef struct {
	u64 ExpAddr;		/**< Exponent address */
	u64 ModAddr;		/**< Modulus address */
	u64 PAddr;			/**< First factor address */
	u64 QAddr;			/**< Second factor address */
	u64 DPAddr;			/**< Private exponent 1 */
	u64 DQAddr;			/**< Private exponent 2 */
	u64 QInvAddr;		/**< Q inverse address */
	u64 TotAddr;		/**< Totient address */
	u64 RNAddr;			/**< R address */
	u64 RRNAddr;		/**< RR address */
	u32 PSize;			/**< Size of first factor(P) in bytes */
	u32 QSize;			/**< Size of first factor(Q) in bytes */
	u32 PubExp;			/**< Public exponent */
	u32 IsPrimeAvail;	/**< Prime number available */
	u32 IsPrivExpAvail; /**< Private exponent available i.e. DP and DQ */
	u32 IsTotAvail;		/**< Totient Available */
	u32 IsPubExpAvail;	/**< Public exponent available */
	XSecure_RsaOperationMode OpMode; /**< RSA operation mode */
} XSecure_RsaKeyParam;

/**************************** Constant Definitions ****************************/
#define XSECURE_HMAC_MAJOR_VERSION	5 /**< Major version of HMAC */
#define XSECURE_HMAC_MINOR_VERSION	7 /**< Minor version of HMAC */

#define XSECURE_SHA2_MAJOR_VERSION	5 /**< Major version of SHA2 */
#define XSECURE_SHA2_MINOR_VERSION	7 /**< Minor version of SHA2 */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * @brief	This function returns the HMAC crypto algorithm information.
 *
 * @param	AlgInfo	Pointer to memory for holding the crypto algorithm information
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_HmacGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_HMAC_MAJOR_VERSION, XSECURE_HMAC_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

/******************************************************************************/
/**
 *
 * @brief	This function returns the SHA2 crypto algorithm information.
 *
 * @param	AlgInfo	Pointer to memory for holding the crypto algorithm information
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_Sha2GetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_SHA2_MAJOR_VERSION, XSECURE_SHA2_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_DEFS_H */
/** @} */
