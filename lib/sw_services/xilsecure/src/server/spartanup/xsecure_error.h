/******************************************************************************
* Copyright (c) 2024-2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file server/spartanup/xsecure_error.h
 *
 * This file contains xilsecure error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 5.5   kpt  08/18/2024 Initial release
 * 5.6   aa   07/29/2025 Added error codes
 * 5.6   tus  08/19/2025 Add error codes for SHA
 * 5.7   tvp  02/19/2026 Added XSECURE_SHA_MAX_HASH_SIZE_EXCEED_ERROR
 *       mb   03/13/2026 Add support for ECC curves for SPARTANUPLUSAES1 device
 *       mb   04/17/2026 Add error code XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XSECURE_ERROR_H_
#define XSECURE_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/** XilSecure Error Codes */
enum {
	XSECURE_SHA_INIT_ERROR = 0x02,		/**< 0x02 - Error when SHA
						   init fails. */
	XSECURE_SHA_LAST_UPDATE_ERROR,		/**< 0x03 - Error when SHA
						   last update fails. */
	XSECURE_SHA_PMC_DMA_UPDATE_ERROR,	/**< 0x04 - Error when DMA driver
						   fails to update the data to SHA */
	XSECURE_SHA_TIMEOUT_ERROR,		/**< 0x05 - Error when timeout */

	XSECURE_SHA_KAT_FAILED_ERROR,		/**< 0x06 - Error when SHA hash
						   not matched with expected hash */
	XSECURE_SHA_FINISH_ERROR,		/**<0x07 - Error when SHA finish fails */

	XSECURE_SHA_INVALID_PARAM,		/**< 0x08 - Invalid Argument */

	XSECURE_SSS_INVALID_PARAM,		/**< 0x09 - Invalid Argument */

	XSECURE_SHA_STATE_MISMATCH_ERROR,	/**< 0x0A - State mismatch */

	XSECURE_SHA384_INVALID_PARAM,		/**< 0x0B - Invalid Param for SHA384 digest calculation*/

	XSECURE_SHA384_KAT_ERROR,		/**< 0x0C - Error when SHA384 KAT fails */

	XSECURE_SHA_DMA_COMPONENT_IS_NOT_READY,  /**< 0x0D - Error when DMA component is not ready */

	XSECURE_SHA_DMA_TRANSFER_ERROR,		/**< 0x0E - Error when SHA DMA transfer fails */

	XSECURE_SHA_PADDING_BUFFER_INIT_ERROR,	/**< 0x0F - Error when SHA padding buffer
						    initialization fails */

	XSECURE_SHA_NIST_PADDING_ERROR,		/**< 0x10 - Error during NIST padding
						     operation */
	XSECURE_SHA_MAX_HASH_SIZE_EXCEED_ERROR,	/**< 0x11 - Error when requested hash size exceeds
						     maximum supported hash size */
	XSECURE_RESERVED_SHA1 = 0x30,		/**< 0x30 - Reserved for SHA1 */

	XSECURE_AES_GCM_TAG_MISMATCH = 0x40,	/**< 0x40 - user provided GCM tag does
						   not match calculated tag */
	XSECURE_AES_KEY_CLEAR_ERROR,
						/**< 0x41 - AES key clear error */
	XSECURE_AES_DPA_CM_NOT_SUPPORTED,	/**< 0x42 - AES DPA CM is not
						   supported on device */
	XSECURE_AES_INVALID_PARAM,		     /**< 0x43 - Invalid Argument */
	XSECURE_AES_STATE_MISMATCH_ERROR,	     /**< 0x44 - State mismatch */
	XSECURE_AES_DEVICE_KEY_NOT_ALLOWED,	     /**< 0x45 - Access to device keys
							is not allowed for IPI for
							any operations */
	XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED,	/**< 0x46 - Error when PUF Key is selected as key source and PUF key is zeroized */
	XSECURE_AES_UNALIGNED_SIZE_ERROR,      /**< 0x47 - Error when data is unaligned*/
	XSECURE_AES_INVALID_MODE,             /**< 0x48 - Error when invalid mode is used for AES operation */
	XSECURE_AES_KAT_DATA_MISMATCH_ERROR,	/**< 0x46 - Error when AES data
                                                   not matched with
						   expected data  */
	XSECURE_AES_KAT_FAILED_ERROR,		/**< 0x47 - AES KAT fails  */

	XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x48 - Error when AESDPACM
							key write fails. */
	XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR,   /**< 0x49 - Error when AESDPACM
                                                        key load fails. */
	XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR,       /**< 0x4A - Error when AESDPACM
							sss config fails */
	XSECURE_AESDPACM_KAT_FAILED_ERROR,  /**< 0x4B - Error when AESDPACM
							KAT failed */
	XSECURE_AESKAT_INVALID_PARAM,		     /**< 0x4C - Invalid Argument */

	XSECURE_AES_DMA_COMPONENT_IS_NOT_READY, /**< 0x4D - Error when DMA component is not ready */

	XSECURE_LMS_INVALID_PARAM = 0x95,			/**< 0x95 - LMS invalid input param */
	XSECURE_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR,	/**< 0x96 - LMS OTS Signature
					Verification - OTS Sign type mismatch with public key */
	XSECURE_LMS_TYPE_UNSUPPORTED_ERROR,
					/**< 0x97 - LMS Type not supported */
	XSECURE_LMS_TYPE_LOOKUP_GLITCH_ERROR,
					/**< 0x98 - LMS parameter look up - glitch detected */
	XSECURE_LMS_OTS_SIGN_SHA_DIGEST_FAILED_ERROR,	/**< 0x99 - LMS OTS Signature
				Verification - hash chain per iteration sha digest failed */
	XSECURE_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR,	/**< 0x9A - LMS OTS Signature
				Verification - parameter lookup failed - not supported type */
	XSECURE_AUTH_LMS_OTS_INVALID_SIGN_ADDR_ERROR,	/**< 0x9B - LMS OTS Signature
				Verification - when signature to be used is at invalid address */
	XSECURE_AUTH_LMS_DIGEST_CHECKSUM_FAILED_ERROR,	/**< 0x9C - LMS OTS Signature
				Verification - checksum calculation failed during
				digest calculation */
	XSECURE_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR,	/**< 0x9D - LMS OTS Signature
				Verification - Digest calculation operation failed */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_ADDR_ERROR,		/**< 0x9E - LMS Signature
				Verification - invalid address for expected public key
				at Merkle tree root */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_LEN_1_ERROR,		/**< 0x9F - LMS Signature
				Verification - invalid len for expected public key at
				Merkle tree root - less than or equal to 4 */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR,		/**< 0xA0 - LMS Signature
				Verification - invalid len for expected public key at Merkle
				tree root - total less than required by parameters */
	XSECURE_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR,	/**< 0xA1 - LMS Signature Verification
				 - LMS public key parameter pick up failed - un supported type */
	XSECURE_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR,	/**< 0xA2 - LMS Signature
				Verification - LMS type mismatch between LMS
				signature and public key */
	XSECURE_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR,/**< 0xA3 - LMS OTS type mismatch
						 - between LMS OTS signature and public key */
	XSECURE_LMS_SIGN_LEN_1_ERROR,	/**< 0xA4 - LMS Signature Verification
					 - LMS signature length less than or equal 4 bytes */
	XSECURE_LMS_SIGN_LEN_2_ERROR,	/**< 0xA5 - LMS Signature Verification - LMS signature
					 length less than required for LMS OTS signature */
	XSECURE_LMS_SIGN_LEN_3_ERROR,	/**< 0xA6 - LMS Signature Verification - LMS signature
				length error - total len less than required by parameters */
	XSECURE_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR,	/**< 0xA7 - LMS Signature Verification
				 - LMS OTS parameters look up failed - unsupported type */
	XSECURE_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR,	/**< 0xA8 - LMS Signature Verification
				 - LMS signature parameters lookup failed - un supported type */
	XSECURE_LMS_SIGN_INVALID_NODE_NUMBER_ERROR,	/**< 0xA9 - LMS Signature Verification
				- invalid node number 'q' in a Merkle tree */
	XSECURE_LMS_SIGN_OTS_OP_ERROR,			/**< 0xAA - LMS Signature Verification
				- LMS OTS signature verification failed */
	XSECURE_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR,	/**< 0xAB - LMS Signature
				Verification - calculated LMS public key did not match with
				expected - authentication failed */
	XSECURE_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR,	/**< 0xAC - LMS Signature
				Verification - LMS public key comparison glitch detected -
				authentication failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR,	/**< 0xAD - LMS Signature
				Verification - LMS signature to public key - leaf node
				sha digest failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_ODD_FAILED_ERROR,/**< 0xAE - LMS Signature
				Verification - LMS signature to public key - odd internal
				node sha digest failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_EVEN_FAILED_ERROR,/**< 0xAF - LMS Signature
				Verification - LMS signature to public key - even internal
				node sha digest failed */
	XSECURE_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR,/**< 0xB0 - LMS Signature
				Verification - SHA algorithm mismatch between one selected in
				Boot header and LMS & OTS - level 0 */
	XSECURE_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L1_ERROR,/**< 0xB1 - LMS Signature
				Verification - SHA algorithm mismatch between one selected
				in Boot header and LMS & OTS - level 1 */
	XSECURE_LMS_HSS_PUB_KEY_INVALID_LEN_1_ERROR,	/**< 0xB2 - LMS HSS Signature
				verification - HSS public key at an invalid address */
	XSECURE_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR,	/**< 0xB3 - LMS HSS Signature
				verification - HSS pub key len less than required */
	XSECURE_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR,	/**< 0xB4 - LMS HSS Signature
				verification - only two levels of Merkle trees are supported */
	XSECURE_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR,	/**< 0xB5 - LMS HSS
				Signature verification - HSS pub key's LMS type parameter
				look up failed for level 0 tree */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR,	/**< 0xB6 - LMS HSS
				Signature verification - HSS pub key's LMS type parameter look
				up failed for level 1 tree - in HSS init */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR,	/**< 0xB7 - LMS HSS
				Signature verification - HSS pub key's LMS type parameter look
				up failed for level 1 tree - in HSS Finish */
	XSECURE_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR,	/**< 0xB8 - LMS HSS
				Signature verification - HSS pub key's LMS OTS type parameter
				look up failed for level 0 tree */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR,	/**< 0xB9 - LMS HSS
				Signature verification - HSS pub key's LMS OTS type parameter look
				up failed for level 1 tree */
	XSECURE_LMS_HSS_SIGN_INVALID_LEN_1_ERROR,	/**< 0xBA - LMS HSS Signature verification
				 - HSS pub key's LMS OTS type parameter look up for level
				1 tree - temporal glitch detected */
	XSECURE_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR,	/**< 0xBB - LMS HSS Signature verification
				 - HSS signature len does not fit OTS signature for level 0 */
	XSECURE_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR,	/**< 0xBC - LMS HSS Signature verification
				 - HSS signature len does not fit OTS signature for level 1 */
	XSECURE_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR,	/**< 0xBD - LMS HSS Signature
				verification - HSS pub key & Signature levels mismatch */
	XSECURE_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR,		/**< 0xBE - LMS HSS Signature
				verification - Level 0 LMS auth op failed */
	XSECURE_LMS_HSS_L1_PUB_KEY_AUTH_FAILED_ERROR,		/**< 0xBF - LMS HSS Signature
				verification - Level 1 LMS auth op failed */
	XSECURE_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR,	/**< 0xC0 - LMS HSS Signature
				verification - OTS sign at level 1 is less than 4 bytes */
	XSECURE_LMS_PUB_OP_FAILED_ERROR,	/**< 0xC1 - LMS HSS Signature verification
				 - authenticated public key for level 1 tree copy failed */
	XSECURE_LMS_PUB_OP_FAILED_1_ERROR,	/**< 0xC2 - LMS HSS Signature verification
				 - authenticated public key for level 1 tree copy
				failed - iteration mismatch */
	XSECURE_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR,	/**< 0xC3 - LMS OTS
				Checksum - buffer for digest is of invalid length (0) */
	XSECURE_LMS_OTS_TYPE_UNSUPPORTED_ERROR,		/**< 0xC4 - LMS OTS
							type not supported */
	XSECURE_LMS_OTS_TYPE_LOOKUP_GLITCH_ERROR,	/**< 0xC5 - LMS OTS
							parameter look up - glitch detected */
	XSECURE_LMS_MEM_COPY_ERROR = 0xD9,	/**< 0xD9 - Error in copying from buffer in LMS operation */
	XSECURE_LMS_HSS_KEY_ZEROIZE_ERROR,	/**< 0xDA - Error in zeroizing LMS HSS key */

	XSECURE_ELLIPTIC_NON_SUPPORTED_CRV = 0xC6,		/**< 0xC6 - Elliptic Curve not supported */
	XSECURE_ELLIPTIC_KEY_ZERO,			/**< 0xC7 - Public key is zero */
	XSECURE_ELLIPTIC_KEY_WRONG_ORDER,		/**< 0xC8 - Wrong order of Public key */
	XSECURE_ELLIPTIC_KEY_NOT_ON_CRV,		/**< 0xC9 - Key not found on curve */
	XSECURE_ELLIPTIC_BAD_SIGN,			/**< 0xCA - Signature provided for
							verification is bad */
	XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN,   /**< 0xCB - Incorrect hash length
							for sign generation */
	XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN,   /**< 0xCC - Incorrect hash length
							for sign verification */
	XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM,	/**< 0xCD - Bad random number used
							for sign generation */
	XSECURE_ELLIPTIC_GEN_KEY_ERR,		     /**< 0xCE - Error in generating Public key */
	XSECURE_ELLIPTIC_INVALID_PARAM,		     /**< 0xCF - Invalid argument */
	XSECURE_ELLIPTIC_VER_SIGN_R_ZERO,               /**< 0xD0 - R set to zero */
	XSECURE_ELLIPTIC_VER_SIGN_S_ZERO,               /**< 0xD1 - S set to zero */
	XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR,        /**< 0xD2 - R is not within ECC order */
	XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR,        /**< 0xD3 - S is not within ECC order */

	XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED = 0xF4, /**< 0xF4 - Error when crypto accelerator is disabled */
	XSECURE_ERR_INVALID_CORE = 0xF5,		/**< 0xF5 - Invalid core selected */
	XSECURE_ERR_GLITCH_DETECTED = 0xFC,		/**< 0xFC - Error glitch detected */
	XSECURE_ERR_IN_TRNG_SELF_TESTS,		/**< 0xFD - Error in TRNG self tests */
	XSECURE_ERR_TRNG_INIT_N_CONFIG,		/**< 0xFE - Error in TRNG Instantiate and configuration */
};

#define XSECURE_SHA3_KAT_FAILED_ERROR		XSECURE_SHA_KAT_FAILED_ERROR
						/**< SHA3 KAT failed error */

#define XSECURE_SHA3_INVALID_PARAM		XSECURE_SHA_INVALID_PARAM
						/**< SHA3 invalid parameter error */

#define XSECURE_SHA3_PMC_DMA_UPDATE_ERROR	XSECURE_SHA_PMC_DMA_UPDATE_ERROR
						/**< SHA3 PMC DMA update error */

#define XSECURE_SHA3_FINISH_ERROR		XSECURE_SHA_FINISH_ERROR
						/**< SHA3 finish error */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ERROR_H_ */
