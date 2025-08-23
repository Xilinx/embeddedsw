/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_rsainfo.h
 *
 * This file contains the RSA definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   08/20/24 Initial release
 *       ss   09/23/24 Fixed doxygen comments
 *       ss   02/04/25 Added client support for RSA padding scheme
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_RSAINFO_H_
#define XASU_RSAINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/

/* RSA module command IDs */
#define XASU_RSA_PUB_ENC_CMD_ID			(0U) /**< Command ID for RSA public encryption */
#define XASU_RSA_PVT_DEC_CMD_ID			(1U) /**< Command ID for RSA private decryption */
#define XASU_RSA_PVT_CRT_DEC_CMD_ID		(2U) /**< Command ID for RSA private CRT
							decryption */
#define XASU_RSA_OAEP_ENC_SHA2_CMD_ID		(3U) /**< Command ID for OAEP encryption with SHA2 */
#define XASU_RSA_OAEP_DEC_SHA2_CMD_ID		(4U) /**< Command ID for OAEP decryption with SHA2 */
#define XASU_RSA_OAEP_ENC_SHA3_CMD_ID		(5U) /**< Command ID for OAEP encryption with SHA3 */
#define XASU_RSA_OAEP_DEC_SHA3_CMD_ID		(6U) /**< Command ID for OAEP decryption with SHA3 */
#define XASU_RSA_PSS_SIGN_GEN_SHA2_CMD_ID	(7U) /**< Command ID for PSS sign generation
							with SHA2 */
#define XASU_RSA_PSS_SIGN_VER_SHA2_CMD_ID	(8U) /**< Command ID for PSS sign verification
							with SHA2 */
#define XASU_RSA_PSS_SIGN_GEN_SHA3_CMD_ID	(9U) /**< Command ID for PSS sign generation with
							SHA3 */
#define XASU_RSA_PSS_SIGN_VER_SHA3_CMD_ID	(10U) /**< Command ID for PSS sign verification
							with SHA3 */
#define XASU_RSA_OAEP_ENC_DEC_KAT_CMD_ID	(11U) /**< KAT command for RSA OAEP encryption and
							decryption */
#define XASU_RSA_PSS_SIGN_GEN_VER_KAT_CMD_ID	(12U) /**< KAT command for RSA PSS sign generation
							and verification */

/* RSA key size */
#define XRSA_2048_KEY_SIZE		(256U) /**< 2048 bit key size in bytes */
#define XRSA_3072_KEY_SIZE		(384U) /**< 3072 bit key size in bytes */
#define XRSA_4096_KEY_SIZE		(512U) /**< 4096 bit key size in bytes */

#define XRSA_MAX_KEY_SIZE_IN_WORDS	(XRSA_4096_KEY_SIZE / 4U) /**< RSA max key size in
										words */
#define XRSA_MAX_PRIME_SIZE_IN_WORDS	(64U) /**< RSA max prime size in words */

/* RSA input data type for sign generation or verification */
#define XASU_RSA_HASHED_INPUT_DATA		(1U)	/**< Input data hash is already
									calculated */

/* RSA operation flags */
#define XASU_RSA_ENCRYPTION			(1U)	/**< Encryption operation */
#define XASU_RSA_EXPONENTIATION_DECRYPTION	(2U)	/**< Exponentiation Decryption operation */
#define XASU_RSA_SIGN_GENERATION		(3U)	/**< Sign generation operation */
#define XASU_RSA_SIGN_VERIFICATION		(4U)	/**< Sign verification operation */

/** @} */
/************************************** Type Definitions *****************************************/
/** This structure contains RSA public key parameters info. */
typedef struct {
	u32 Keysize;					/**< Key size in bytes*/
	u32 Modulus[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< Rsa key Modulus */
	u32 PubExp;					/**< Public exponent */
} XAsu_RsaPubKeyComp;

/** This structure contains RSA private key parameters info. */
typedef struct {
	XAsu_RsaPubKeyComp PubKeyComp;			/**< Contains public key components */
	u32 PvtExp[XRSA_MAX_KEY_SIZE_IN_WORDS];		/**< Private Exponent */
	u32 PrimeCompOrTotient[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< Prime component or
									Totient */
	u32 PrimeCompOrTotientPrsnt;		/**< 0 : Ignore PrimeComporTotient array
						     1 : Array Consists totient
						     2 : Array Consists Prime component*/
} XAsu_RsaPvtKeyComp;

/** This structure contains RSA private key CRT parameters info. */
typedef struct {
	XAsu_RsaPubKeyComp PubKeyComp;			/**< Contains public key components */
	u32 Prime1[XRSA_MAX_PRIME_SIZE_IN_WORDS];	/**< Prime number 1 */
	u32 Prime2[XRSA_MAX_PRIME_SIZE_IN_WORDS];	/**< Prime number 2 */
	u32 DP[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Derived component from Prime1*/
	u32 DQ[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Derived component from Prime2*/
	u32 QInv[XRSA_MAX_PRIME_SIZE_IN_WORDS];		/**< Inverse of Q*/
} XAsu_RsaCrtKeyComp;

/** This structure contains RSA exponent value (R square mod N). */
typedef struct {
	u32 RRn[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< R square modN exponent value */
} XAsu_RsaRRModN;

/** This structure contains RSA exponent values (R mod N, R square mod N). */
typedef struct {
	XAsu_RsaRRModN RRModn;			/**< Contains R square modN component */
	u32 Rn[XRSA_MAX_KEY_SIZE_IN_WORDS];	/**< R modN exponent value */
} XAsu_RsaRModN;

/** This structure contains RSA params info. */
typedef struct {
	u64 InputDataAddr; 	/**< Address of RSA input data */
	u64 OutputDataAddr; 	/**< Address of RSA output data */
	u64 ExpoCompAddr;	/**< Address of RSA exponent data */
	u64 KeyCompAddr; 	/**< Address of RSA key component structure :
						  *	XAsu_RsaPubKeyComp - for public key operations and
						  *	XAsu_RsaPvtKeyComp - for private key operations  */
	u32 Len;			/**< Data Len */
	u32 KeySize;		/**< Key Size */
} XAsu_RsaParams;

/** This structure contains RSA PSS padding params info. */
typedef struct {
	XAsu_RsaParams  XAsu_RsaOpComp;	/**< RSA parameters */
	u64 SignatureDataAddr;	/**< Address of RSA signature which acts as :
							* - input - For signature verification and
							* - output - For signature generation */
	u32 SignatureLen;	/**< RSA signature data length */
	u32 SaltLen;		/**< RSA salt len for PSS padding */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 InputDataType;	/**< Input data type
					* - 0: Plain data where Hash has to be calculated
					* - 1: Hashed data */
	u8 Reserved;	/**< Reserved */
} XAsu_RsaPaddingParams;

/** This structure contains RSA OAEP padding params info. */
typedef struct {
	XAsu_RsaParams  XAsu_RsaOpComp;	/**< Contains client components */
	u64 OptionalLabelAddr;	/**< RSA optional label address for OAEP padding */
	u32 OptionalLabelSize;	/**< RSA optional label size for OAEP padding */
	u8 ShaType; /**< Hash family type (XASU_SHA2_TYPE / XASU_SHA3_TYPE) */
	u8 ShaMode; /**< SHA Mode, where XASU_SHA_MODE_SHAKE256 is valid only for SHA3 Type
		* (XASU_SHA_MODE_256 / XASU_SHA_MODE_384 / XASU_SHA_MODE_512 /
		* XASU_SHA_MODE_SHAKE256) */
	u8 Reserved; /**< Reserved */
	u8 Reserved1; /**< Reserved */
} XAsu_RsaOaepPaddingParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the specified RSA key length.
 *
 * @param	Len	Length of the RSA key
 *
 *************************************************************************************************/
static inline s32 XAsu_RsaValidateKeySize(u32 Len)
{
	volatile s32 Status = XST_FAILURE;

    if ((Len == XRSA_2048_KEY_SIZE) || (Len == XRSA_3072_KEY_SIZE) ||
		(Len == XRSA_4096_KEY_SIZE)) {
			Status = XST_SUCCESS;
	}

	return Status;
}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_RSAINFO_H_ */
