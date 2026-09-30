/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file common/core/xsecure_defs.h
*
* This file contains the xsecure API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/20 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
* 4.6   har  07/14/21 Fixed doxygen warnings
* 4.7   kpt  11/29/21 Added macro XSecure_DCacheFlushRange
* 5.0   bm   07/06/22 Refactor versal and versal_net code
*       kpt  07/24/22 Added XSecure_EccCrvClass
* 5.1   skg  12/16/22 Added XSecure_AesAllParams
*       yog  05/03/23 Fixed MISRA C violation of Rule 12.2
* 5.2   yog  06/07/23 Added support for P-256 Curve
*       vss  07/14/23 Added enum for resource availability and also ipi mask macro
*       ng   07/15/23 Added support for system device tree flow
*       har  07/26/23 Renamed members of XSecure_EccCrvClass and added macros for
*                     backward compatibility
*       vss  09/11/23 Fixed Coverity warning EXPRESSION_WITH_MAGIC_NUMBERS
* 5.3   ng   01/28/24 Added SDT support
*       ng   03/26/24 Fixed header include in SDT flow
* 5.4   yog  04/29/24 Fixed doxygen warnings.
*       mb   05/23/24 Added support for P-192 Curve
*       mb   05/23/24 Added support for P-224 Curve
*       Pre  03/02/25 Removed macros and structures related to data context setting
*       yog  03/18/25 Added XSecure_EcdhParams
* 5.6   obs  09/17/25 Added XSECURE_SIZE_DOUBLE macro
* 5.7   tvp  02/19/26 Added XSecure_MldsaSignVerifyParams
*       tvp  02/19/26 Added XSecure_SlhdsaInputParams
*       tvp  03/09/26 Added XSecure_MldsaSignGenParams
*       tvp  03/17/26 Re-arrange Mldsa and Slhdsa Sign verify params
* 5.8   tus  08/19/26 Added XSecure_LibCapBit capability bit-position registry for
*                     the library-info query; library info uses the shared
*                     Xil_LibInfo container (xil_cryptoalginfo.h)
*       rap  09/09/26 Moved XSECURE_ECC_P521_SIZE_IN_BYTES here as the single
*                     client/server definition (same approach as RSA key sizes)
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_common_apis XilSecure Common Apis
* @{
*/
#ifndef XSECURE_DEFS_H
#define XSECURE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_cache.h"
#include "xil_sutil.h"
#include "xsecure_plat_defs.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Constant Definitions ****************************/
/**
 * @name  Debug related macros
 * @{
 */
/**< Enable client printfs by setting XSECURE_DEBUG to 1 */
#define XSECURE_DEBUG	(0U)

#if (XSECURE_DEBUG)
#define XSECURE_DEBUG_GENERAL (1U)
#else
#define XSECURE_DEBUG_GENERAL (0U) /**< General debug print control macro */
#endif
/** @} */
/***************** Macros (Inline Functions) Definitions *********************/
#define XSecure_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}
				/**< For prints in XilSecure library */
#ifndef XSECURE_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XSecure_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
	#else
		#define XSecure_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
	#endif
#else
	#define XSecure_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

#define XSECURE_API(ApiId)	((u32)ApiId)
				/**< Macro to typecast XILSECURE API ID */

#define XSECURE_API_ID_MASK	(0xFFU)
				/**< Mask for API ID in Secure IPI command */

#define XILSECURE_MODULE_ID			(0x05U)
				/**< XilSecure Module Id */
#define XSECURE_KAT_API_ERR_ID		(((u32)XILSECURE_MODULE_ID << 8U) | ((u32)XSECURE_API_KAT))
				/**< XilSecure KAT API error id */
#define XSECURE_KAT_MAJOR_ERROR 	(((u32)XPLMI_ERR_CDO_CMD + (XSECURE_KAT_API_ERR_ID & \
										XPLMI_ERR_CDO_CMD_MASK)))
				/**< Xilsecure KAT major error for client APIs */
#define XSECURE_ADDR_HIGH_SHIFT			(32U)
				/**< Shift to get higher address */
#define XSECURE_SIZE_DOUBLE	(2U)
				/**< Multiplier to double size */
#define XSECURE_IPI_MASK_DEF_VAL           (0U)
				/**< Default IPI mask value */
#define XSECURE_IPI_PARAMS_SIZE_ALIGN_MASK	(0x7U)
				/**< Mask used to validate that IPI parameter
				 *   structure size is 8-byte aligned */
#define XSECURE_ECDSA_PRIME		(XSECURE_ECC_PRIME)
	/**< This macro is for backward compatibility. For ECC Prime curves, use XSECURE_ECC_PRIME */
#define XSecure_EllipticCrvClass	XSecure_EccCrvClass
	/**< Alias for XSecure_EccCrvClass enum for backward compatibility */
#define XSECURE_ECC_P521_SIZE_IN_BYTES	(66U)
									/**< Size of NIST P-521 curve in bytes */
#define XSECURE_RSA_2048_KEY_SIZE	(2048U/8U) /**< RSA 2048 key size */
#define XSECURE_RSA_3072_KEY_SIZE	(3072U/8U) /**< RSA 3072 key size */
#define XSECURE_RSA_4096_KEY_SIZE	(4096U/8U) /**< RSA 4096 key size */
#define XSECURE_HASH_TYPE_SHA3		(48U) /**< SHA-3 hash size in bytes */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/** Structure for input parameters for RSA Encryption and Decryption */
typedef struct {
	u64 KeyAddr;	/**< Key Address */
	u64 DataAddr;	/**< Data Address */
	u32 Size;	/**< Size of key */
} XSecure_RsaInParam;

/** Structure for input parameters for RSA Sign verification */
typedef struct {
	u64 SignAddr;	/**< RSA Signature Address */
	u64 HashAddr;	/**< Hash Address */
	u32 Size;	/**< Length of hash */
} XSecure_RsaSignParams;

#define XSECURE_RSA_PSS_SIGNED_INPUT_DATA_LEN	(512U)
#define XSECURE_RSA_PSS_DATA_BLOCK_LEN		(480U)
#define XSECURE_RSA_PSS_ENCODED_MSG_HASH_SCRATCH_LEN	(64U)

/** RSA-PSS verification errors */
typedef enum {
	XSECURE_RSA_PSS_ERR_MEMSET_ENC = 1U,
	XSECURE_RSA_PSS_ERR_MEMSET_VARS,
	XSECURE_RSA_PSS_ERR_RSA_ENC,
	XSECURE_RSA_PSS_ERR_MGF,
	XSECURE_RSA_PSS_ERR_SHA,
	XSECURE_RSA_PSS_ERR_BC,
	XSECURE_RSA_PSS_ERR_MSDB,
	XSECURE_RSA_PSS_ERR_DB,
	XSECURE_RSA_PSS_ERR_HASH_CMP,
	XSECURE_RSA_PSS_ERR_CLEAR,
	XSECURE_RSA_PSS_ERR_INVALID,
} XSecure_RsaPssStatus;

/** Structure for input parameters for Ellitic Sign generation */
typedef struct {
	u64 HashAddr;	/**< Hash address */
	u64 PrivKeyAddr;	/**< Static Private key Address */
	u64 EPrivKeyAddr;	/**< Ephemeral private key Address */
	u32 CurveType;	/**< ECC curve type */
	u32 Size;	/**< Length of hash */
} XSecure_EllipticSignGenParams;

/** Structure for input parameters for Ellitic Sign Verification */
typedef struct {
	u64 HashAddr;	/**< Hash address */
	u64 PubKeyAddr;	/**< Public key Address */
	u64 SignAddr;	/**< Signature Address */
	u32 CurveType;	/**< ECC curve type */
	u32 Size;		/**< Length of hash */
} XSecure_EllipticSignVerifyParams;

/**
 * Structure for input parameters for Initialisation of
 * AES Encryption and Decryption
 */
typedef struct {
	u64 IvAddr;	/**< IV address */
	u32 OperationId;/**< Operation type - Encrypt or decrypt */
	u32 KeySrc;	/**< AES Key source */
	u32 KeySize;	/**< Size of AES key*/
} XSecure_AesInitOps;

/** Structure for input parameters for AES Encryption and Decryption */
typedef struct {
	u64 InDataAddr;	/**< Address of input data*/
	u32 Size;	/**< Length of input data*/
	u32 IsLast;	/**< Flag to indicate last update of data*/
} XSecure_AesInParams;

/** Structure for input parameters for ECDH */
typedef struct {
	u32 CurveType;		/**< ECC curve type */
	u32 PubKeyAddrHigh;	/**< Higher 32-bit address of public key */
	u32 PubKeyAddrLow;	/**< Lower 32-bit address of public key */
	u32 PrivKeyAddrHigh;	/**< Higher 32-bit address of private key */
	u32 PrivKeyAddrLow;	/**< Lower 32-bit address of private key */
	u32 SharedSecretAddrHigh;/**< Higher 32-bit address of Shared secret to be copied */
	u32 SharedSecretAddrLow;/**< Lower 32-bit address of Shared secret to be copied */
} XSecure_EcdhParams;

/** Structure for ECC private key generation parameters */
typedef struct {
	u64 PrivKeyAddr;		/**< Private key address */
	u32 PrivKeyLen;			/**< Length of private key in bytes */
	u8 Reserved[4U];		/**< Explicit padding for alignment */
} XSecure_EccPrivateKeyParams;

/**
 * Structure for input parameters for AES
 * Encryption and Decryption of single block of data.
 */
typedef struct {
	u64 IvAddr;	/**< IV address */
	u64 InDataAddr;	 /**< Input data address*/
	u64 OutDataAddr; /**< Output data address*/
	u64 GcmTagAddr; /**< Gcm Tag address*/
	u64 AadAddr;	/**< Aad address*/
	u32 OperationId;/**< Operation type - Encrypt or decrypt */
	u32 KeySrc;	/**< AES Key source */
	u32 KeySize;	/**< Size of AES key*/
	u32 Size;	/**< Length of input data*/
	u32 IsLast;	/**< Flag to indicate last update of data*/
	u32 AadSize;	/**< Aad size*/
	u32 IsGmacEnable; /**< Flag for GMAC operation */
	u32 IsUpdateAadEn; /**<Enable if operation includes update AAD */
} XSecure_AesDataBlockParams;

/** XilSecure AES operation types */
typedef enum {
	XSECURE_ENCRYPT,	/**< Encrypt operation */
	XSECURE_DECRYPT,	/**< Decrypt operation */
} XSecure_AesOp;

/** XilSecure ECC curve types */
typedef enum {
      XSECURE_ECC_NIST_P192 = 1,              /**< NIST P-192 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P224 = 2,	      /**< NIST P-256 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P256 = 3,	      /**< NIST P-256 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P384 = 4,              /**< NIST P-384 curve value in Ecdsa.h */
      XSECURE_ECC_NIST_P521 = 5               /**< NIST P-521 curve value in Ecdsa.h */
} XSecure_EllipticCrvTyp;

/** XilSecure ECC curve class */
typedef enum {
	XSECURE_ECC_PRIME = 0,	/**< Prime curve */
	XSECURE_ECC_BINARY = 1,	/**< Binary curve */
} XSecure_EccCrvClass;

/** XilSecure API KAT operations */
typedef enum {
	XSECURE_API_KAT_SET = 0U,				/**< 0U */
	XSECURE_API_KAT_CLEAR					/**< 1U */
} XSecure_KatOp;

/** Structure for input parameters for MLDSA Sign Verification */
typedef struct {
	u64 DataAddr;		/**< Address of message data buffer */
	u64 PubKeyAddr;		/**< Address of public key buffer */
	u64 SignAddr;		/**< Address of signature buffer */
	u64 ContextAddr;	/**< Address of optional context string buffer */
	u32 DataLen;		/**< Length of message data in bytes */
	u32 PubKeyLen;		/**< Length of public key in bytes */
	u32 SignLen;		/**< Length of signature in bytes */
	u32 ContextLen;		/**< Length of context string in bytes */
} XSecure_MldsaSignVerifyParams;

/** Structure for input parameters for MLDSA Sign Generation */
typedef struct {
	u64 DataAddr;		/**< Address of message data buffer */
	u64 SecretKeyAddr;	/**< Address of secret key buffer */
	u64 SignatureAddr;	/**< Address of output signature buffer */
	u32 DataLen;		/**< Length of message data in bytes */
	u32 SecretKeyLen;	/**< Length of secret key in bytes */
	u32 SignatureLen;	/**< Length of signature buffer in bytes */
} XSecure_MldsaSignGenParams;

/** Structure for input parameters for SLHDSA Sign Verification */
typedef struct {
	u64 DataAddr;		/**< Address of message data buffer */
	u64 SignatureAddr;	/**< Address of signature buffer */
	u64 ContextAddr;	/**< Address of optional context string buffer */
	u64 PublicKeyAddr;	/**< Address of public key buffer */
	u32 DataLen;		/**< Length of message data in bytes */
	u32 SignatureLen;	/**< Length of signature in bytes */
	u32 PublicKeyLen;	/**< Length of public key in bytes */
	u32 ContextLen;		/**< Length of context string in bytes */
} XSecure_SlhdsaInputParams;

/*
 * ---------------------------------------------------------------------------
 * XilSecure library information contract (Features-command library-info query)
 * ---------------------------------------------------------------------------
 * The definitions below form the shared, platform-agnostic contract used by
 * both the client and the server to exchange the library version and the
 * capability bitmap. They are common to every platform; whether a capability
 * bit is actually SET at runtime is decided per platform by the server-side
 * derivation (see server/core/generic/xsecure_server_libcaps.h).
 */

/**
 * Number of 32-bit capability words carried in the library information response.
 * Matches Xil_LibInfo.Capability[] (xil_cryptoalginfo.h). Append-only: never
 * shrink or renumber existing bits.
 */
#define XSECURE_LIB_CAP_WORDS			(2U)	/**< Capability bitmap word count */
#define XSECURE_LIB_CAP_BITS_PER_WORD		(32U)	/**< Number of capability bits per 32-bit word */

/**
 * XilSecure library capability bit positions (global registry).
 *
 * Each enumerator is a PERMANENT bit position within the capability bitmap that
 * the server reports. The platform note in each comment states on which
 * platforms the capability is defined/meaningful; whether a given bit is
 * actually SET at runtime still depends on that platform's build configuration.
 * Bits are packed contiguously and are never renumbered or reused - a new
 * capability simply takes the next free position.
 */
typedef enum {
	XSECURE_CAP_ECC_NIST_P192_BIT,		/**< NIST P-192 curve support      (Versal) */
	XSECURE_CAP_ECC_NIST_P224_BIT,		/**< NIST P-224 curve support      (Versal) */
	XSECURE_CAP_ECC_NIST_P256_BIT,		/**< NIST P-256 curve support      (Versal) */
	XSECURE_CAP_ECC_NIST_P384_BIT,		/**< NIST P-384 curve support      (Versal) */
	XSECURE_CAP_ECC_NIST_P521_BIT,		/**< NIST P-521 curve support      (Versal) */
	XSECURE_CAP_ECDSA_BIT,			/**< ECDSA engine present          (Versal) */
	XSECURE_CAP_ECC_BIG_ENDIAN_BIT,		/**< ECC APIs use big-endian data  (Versal) */
	XSECURE_CAP_RSA_BIT,			/**< RSA engine present            (Versal) */
	XSECURE_CAP_AES_BIT,			/**< AES engine present            (Versal) */
	XSECURE_CAP_LIB_CRYPTOALGINFO_BIT,	/**< Crypto algorithm-info report  (Versal) */
	XSECURE_CAP_ECDH_BIT,			/**< ECDH key agreement present    (VersalNet/Versal_2ve_2vm gated by PLM_ECDSA_EXCLUDE) */
	XSECURE_CAP_TRNG_BIT,			/**< TRNG engine present           (VersalNet/Versal_2ve_2vm gated by PLM_SECURE_EXCLUDE) */
	XSECURE_CAP_LMS_BIT			/**< LMS post-quantum sign-verify  (Versal_2ve_2vm) */
} XSecure_LibCapBit;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_DEFS_H */
/** @} */
