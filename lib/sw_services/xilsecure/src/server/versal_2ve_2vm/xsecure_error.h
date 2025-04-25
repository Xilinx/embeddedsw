/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_error.h
 *
 * This file contains Xilsecure error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 5.4   kal  07/24/2024 Initial release
 *       sk   08/22/24 Added error code for key transfer to ASU
 *       mb   09/20/24 Added XSECURE_RSA_OP_MEM_CPY_FAILED_ERROR
 *       pre  03/02/2025 Added error codes for IPI events handling
 *
 * </pre>
 *
 ******************************************************************************/
/**
* @addtogroup xsecure_error_codes Xilsecure Error Codes
* @{
*/
#ifndef XSECURE_ERROR_H_
#define XSECURE_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum {
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

	XSECURE_AES_GCM_TAG_MISMATCH = 0x40,	/**< 0x40 - user provided GCM tag does
						   not match calculated tag */
	XSECURE_AES_KEY_CLEAR_ERROR,		/**< 0x41 - AES key clear error */
	XSECURE_AES_DPA_CM_NOT_SUPPORTED,	/**< 0x42 - AES DPA CM is not
						   supported on device */
	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x43 - Error when AES key write
						   fails. */
	XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR, /**< 0x44 - Error when AES
						      decrypt init fails. */
	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR, /**< 0x45 - Error when GCM tag
						   not matched with
						   user provided tag */
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
	XSECURE_AESDPACM_KAT_FAILED_ERROR,           /**< 0x4B - AESDPACM KAT fails*/
	XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR,    /**< 0x4C - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR,    /**< 0x4D - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR,    /**< 0x4E - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR,    /**< 0x4F - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR,    /**< 0x50 - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AES_INVALID_PARAM,		     /**< 0x51 - Invalid Argument */
	XSECURE_AESKAT_INVALID_PARAM,		     /**< 0x52 - Invalid Argument */
	XSECURE_AES_STATE_MISMATCH_ERROR,	     /**< 0x53 - State mismatch */
	XSECURE_AES_DEVICE_KEY_NOT_ALLOWED,	     /**< 0x54 - Access to device keys
							is not allowed for IPI for
							any operations */
	XSECURE_AES_KAT_DECRYPT_UPDATE_FAILED_ERROR, /**< 0x55 - Error when AES KAT decrypt
							update fails */
	XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR,	/**< 0x56 - Error when aad update fails */
	XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR,	/**< 0x57 - Error when encrypt
															init fails */
	XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR,	/**< 0x58 - Error when encrypt
															update fails */
	XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR,	/**< 0x59 - Error when encrypt
															final fails */
	XSECURE_KAT_GCM_TAG_MISMATCH_ERROR,	/**< 0x5A - Error when GCM mismatch occurs */
	XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED,	/**< 0x5B - Error when PUF Key is selected as key source and PUF key is zeroized */
	XSECURE_AES_UNALIGNED_SIZE_ERROR,      /**< 0x5C - Error when data is unaligned*/
	XSECURE_AES_INVALID_MODE,             /**< 0x5D - Error when invalid mode is used for AES operation */
    XSECURE_INVALID_RESOURCE,		/**< 0X5F - Error when resource is other than SHA/AES */

	XSECURE_RSA_OAEP_INVALID_PARAM = 0x70,        /**< 0x70 - RSA OAEP Invalid param */
	XSECURE_RSA_OAEP_INVALID_MSG_LEN,             /**< 0x71 - RSA OAEP Invalid Msg length */
	XSECURE_RSA_OAEP_DB_MISMATCH_ERROR,           /**< 0x72 - RSA OAEP DB mismatch error */
	XSECURE_RSA_OAEP_DATA_CPY_ERROR,              /**< 0x73 - RSA OAEP data copy failed */
	XSECURE_RSA_OAEP_DATA_CMP_ERROR,              /**< 0x74 - RSA OAEP data compare failed */
	XSECURE_RSA_OAEP_BYTE_MISMATCH_ERROR,         /**< 0x75 - RSA OAEP data byte mismatch error */
	XSECURE_RSA_PWCT_MEM_CMP_FAILED_ERROR,        /**< 0x76 - RSA PWCT failed */

	XSECURE_RSA_KAT_INIT_ERROR = 0x80,	/**< 0x80 - RSA KAT initialization failure */

	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR, /**< 0x81 - RSA KAT fails  */

	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR, /**< 0x82 - Error when RSA data
							not matched with expected data  */
	XSECURE_RSA_INVALID_PARAM_RESERVED,	     /**< 0x83 - Invalid Argument */
	XSECURE_RSAKAT_INVALID_PARAM,		     /**< 0x84 - Invalid Argument */
	XSECURE_RSA_STATE_MISMATCH_RESERVED,	     /**< 0x85 - State mismatch */
	XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR,	/**< 0x86 - RSA decrypt failed error */
	XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR,	/**< 0x87 - RSA when decrypted data doesn't
							match with plain text */
	XSECURE_RSA_KAT_PSS_SIGN_VER_ERROR,		/**< 0x88 - RSA pss sign verification failed */
	XSECURE_RSA_EXPONENT_INVALID_PARAM,		/**< 0x89 - RSA Exponent invalid parameter */

	/* The error codes from 0x90 to 0xBF are reserved for Versal net platform */
	XSECURE_HMAC_KAT_INIT_ERROR = 0x90,		/**< 0x90 - HMAC init failure */
	XSECURE_HMAC_KAT_UPDATE_ERROR,			/**< 0x91 - HMAC update failure */
	XSECURE_HMAC_KAT_FINAL_ERROR,			/**< 0x92 - HMAC final failure */
	XSECURE_HMAC_KAT_ERROR,				/**< 0x93 - HMAC KAT error */
	XSECURE_HMAC_INVALID_PARAM,			/**< 0x94 - HMAC invalid parameter */



	XSECURE_LMS_INVALID_PARAM,			/** 0x8F - LMS invalid input param */
	XSECURE_LMS_OTS_PUB_KEY_SIGN_TYPE_MISMATCH_ERROR,	/** 0x90, LMS OTS Signature Verification - OTS Sign type mismatch with public key */
	XSECURE_LMS_TYPE_UNSUPPORTED_ERROR,					/** 0x91, LMS Type not supported */
	XSECURE_LMS_TYPE_LOOKUP_GLITCH_ERROR,				/** 0x92, LMS parameter look up - glitch detected */
	XSECURE_LMS_OTS_SIGN_SHA_DIGEST_FAILED_ERROR,		/** 0x93, LMS OTS Signature Verification - hash chain per iteration sha digest failed */
	XSECURE_LMS_OTS_SIGN_UNSUPPORTED_TYPE_ERROR,		/** 0x94, LMS OTS Signature Verification - parameter lookup failed - not supported type */
	XSECURE_AUTH_LMS_OTS_INVALID_SIGN_ADDR_ERROR,		/** 0x95, LMS OTS Signature Verification - when signature to be used is at invalid address */
	XSECURE_AUTH_LMS_DIGEST_CHECKSUM_FAILED_ERROR,		/** 0x96, LMS OTS Signature Verification - checksum calculation failed during digest calculation */
	XSECURE_LMS_OTS_DIGEST_CHECKSUM_OP_FAILED_ERROR,	/** 0x97, LMS OTS Signature Verification - Digest calculation operation failed */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_ADDR_ERROR,		/** 0x98, LMS Signature Verification - invalid address for expected public key at Merkle tree root */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_LEN_1_ERROR,		/** 0x99, LMS Signature Verification - invalid len for expected public key at Merkle tree root - less than or equal to 4 */
	XSECURE_LMS_SIGN_EXPECTED_PUB_KEY_LEN_2_ERROR,		/** 0x9A, LMS Signature Verification - invalid len for expected public key at Merkle tree root - total less than required by parameters */
	XSECURE_LMS_PUB_KEY_UNSUPPORTED_TYPE_1_ERROR,		/** 0x9B, LMS Signature Verification - LMS public key parameter pick up failed - un supported type */
	XSECURE_LMS_PUB_KEY_LMS_SIGN_TYPE_MISMATCH_ERROR,	/** 0x9C, LMS Signature Verification - LMS type mismatch between LMS signature and public key */
	XSECURE_LMS_OTS_PUB_KEY_LMS_OTS_SIGN_TYPE_MISMATCH_ERROR,/** 0x9D, LMS OTS type mismatch between LMS OTS signature and public key */
	XSECURE_LMS_SIGN_LEN_1_ERROR,						/** 0x9E, LMS Signature Verification - LMS signature length less than or equal 4 bytes */
	XSECURE_LMS_SIGN_LEN_2_ERROR,						/** 0x9F, LMS Signature Verification - LMS signature length less than required for LMS OTS signature */
	XSECURE_LMS_SIGN_LEN_3_ERROR,						/** 0xA0, LMS Signature Verification - LMS signature length error - total len less than required by paramerters */
	XSECURE_LMS_SIGN_UNSUPPORTED_OTS_TYPE_1_ERROR,		/** 0xA1, LMS Signature Verification - LMS OTS parameters look up failed - unsupported type */
	XSECURE_LMS_SIGN_UNSUPPORTED_TYPE_1_ERROR,			/** 0xA2, LMS Signature Verification - LMS signature parameters lookup failed - un supported type */
	XSECURE_LMS_SIGN_INVALID_NODE_NUMBER_ERROR,			/** 0xA3, LMS Signature Verification - invalid node number 'q' in a Merkle tree */
	XSECURE_LMS_SIGN_OTS_OP_ERROR,						/** 0xA4, LMS Signature Verification - LMS OTS signature verification failed */
	XSECURE_LMS_PUB_KEY_AUTHENTICATION_FAILED_ERROR,	/** 0xA5, LMS Signature Verification - calculated LMS public key did not match with expected - authentication failed */
	XSECURE_LMS_PUB_KEY_AUTHENTICATION_GLITCH_ERROR,	/** 0xA6, LMS Signature Verification - LMS public key comparison glitch detected - authentication failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_LEAF_FAILED_ERROR,	/** 0xA7, LMS Signature Verification - LMS signature to public key - leaf node sha digest failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_ODD_FAILED_ERROR,/** 0xA8, LMS Signature Verification - LMS signature to public key - odd internal node sha digest failed */
	XSECURE_LMS_SIGN_VERIF_SHA_DIGEST_INTR_EVEN_FAILED_ERROR,/** 0xA9, LMS Signature Verification - LMS sign to public key - even internal node sha digest failed */
	XSECURE_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L0_ERROR,/** 0xAA, LMS Signature Verification - SHA algorithm mismatch between one selected in Boot header and LMS & OTS - level 0 */
	XSECURE_LMS_SIGN_VERIFY_BH_AND_TYPE_SHA_ALGO_MISMATCH_L1_ERROR,/** 0xAB, LMS Signature Verification - SHA algorithm mismatch between one selected in Boot header and LMS & OTS - level 1 */
	XSECURE_LMS_HSS_PUB_KEY_INVALID_LEN_1_ERROR,					/** 0xAC, LMS HSS Signature verification - HSS public key at an invalid address */
	XSECURE_LMS_HSS_PUB_KEY_INVALID_LEN_2_ERROR,					/** 0xAD, LMS HSS Signature verification - HSS pub key len less than required */
	XSECURE_LMS_HSS_SIGN_LEVEL_UNSUPPORTED_ERROR,					/** 0xAE, LMS HSS Signature verification - only two levels of Merkle trees are supported */
	XSECURE_LMS_HSS_L0_PUB_KEY_LMS_TYPE_UNSUPPORTED_ERROR,			/** 0xAF, LMS HSS Signature verification - HSS pub key's LMS type parameter look up failed for level 0 tree */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_1_UNSUPPORTED_ERROR,		/** 0xB0, LMS HSS Signature verification - HSS pub key's LMS type parameter look up failed for level 1 tree - in HSS init */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_TYPE_2_UNSUPPORTED_ERROR,		/** 0xB1, LMS HSS Signature verification - HSS pub key's LMS type parameter look up failed for level 1 tree - in HSS Finish */
	XSECURE_LMS_HSS_L0_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR,		/** 0xB2, LMS HSS Signature verification - HSS pub key's LMS OTS type parameter look up failed for level 0 tree */
	XSECURE_LMS_HSS_L1_PUB_KEY_LMS_OTS_TYPE_UNSUPPORTED_ERROR,		/** 0xB3, LMS HSS Signature verification - HSS pub key's LMS OTS type parameter look up failed for level 1 tree */
	XSECURE_LMS_HSS_SIGN_INVALID_LEN_1_ERROR,			/** 0xB4, LMS HSS Signature verification - HSS pub key's LMS OTS type parameter look up for level 1 tree - temporal glitch detected */
	XSECURE_LMS_HSS_L0_SIGN_INVALID_LEN_2_ERROR,		/** 0xB5, LMS HSS Signature verification - HSS signature len does not fit OTS signature for level 0 */
	XSECURE_LMS_HSS_L1_SIGN_INVALID_LEN_2_ERROR,		/** 0xB6, LMS HSS Signature verification - HSS signature len does not fit OTS signature for level 1 */
	XSECURE_LMS_HSS_SIGN_PUB_KEY_LEVEL_MISMATCH_ERROR,	/** 0xB7, LMS HSS Signature verification - HSS pub key & Signature levels mismatch */
	XSECURE_LMS_HSS_L0_PUB_KEY_AUTH_FAILED_ERROR,		/** 0xB8, LMS HSS Signature verification - Level 0 LMS auth op failed */
	XSECURE_LMS_HSS_L1_PUB_KEY_AUTH_FAILED_ERROR,		/** 0xB9, LMS HSS Signature verification - Level 1 LMS auth op failed */
	XSECURE_LMS_HSS_OTS_SIGN_INVALID_LEN_1_ERROR,		/** 0xBA, LMS HSS Signature verification - OTS sign at level 1 is less than 4 bytes */
	XSECURE_LMS_PUB_OP_FAILED_ERROR,			/** 0xBB, LMS HSS Signature verification - authenticated public key for level 1 tree copy failed */
	XSECURE_LMS_PUB_OP_FAILED_1_ERROR,			/** 0xBC, LMS HSS Signature verification - authenticated public key for level 1 tree copy failed - iteration mismatch */
	XSECURE_LMS_OTS_CHECKSUM_BUFF_INVALID_LEN_ERROR,	/** 0xBD, LMS OTS Checksum - buffer for digest is of invalid length (0) */
	XSECURE_LMS_OTS_TYPE_UNSUPPORTED_ERROR,			/** 0xBE, LMS OTS type not supported */
	XSECURE_LMS_OTS_TYPE_LOOKUP_GLITCH_ERROR,		/** 0xBF, LMS OTS parameter look up - glitch detected */

	XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR = 0xC0,   /**< 0xC0 -ECC key is not valid */

	XSECURE_ELLIPTIC_KAT_FAILED_ERROR,		/**< 0xC1 - Elliptic KAT fails */
	XSECURE_ELLIPTIC_NON_SUPPORTED_CRV,		/**< 0xC2 - Elliptic Curve not supported */
	XSECURE_ELLIPTIC_KEY_ZERO,			/**< 0xC3 - Public key is zero */
	XSECURE_ELLIPTIC_KEY_WRONG_ORDER,		/**< 0xC4 - Wrong order of Public key */
	XSECURE_ELLIPTIC_KEY_NOT_ON_CRV,		/**< 0xC5 - Key not found on curve */
	XSECURE_ELLIPTIC_BAD_SIGN,			/**< 0xC6 - Signature provided for
							verification is bad */
	XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN,   /**< 0xC7 - Incorrect hash length
							for sign generation */
	XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN,   /**< 0xC8 - Incorrect hash length
							for sign verification */
	XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM,	        /**< 0xC9 - Bad random number used
							for sign generation */
	XSECURE_ELLIPTIC_GEN_KEY_ERR,		     /**< 0xCA - Error in generating Public key */
	XSECURE_ELLIPTIC_INVALID_PARAM,		     /**< 0xCB - Invalid argument */
	XSECURE_ELLIPTIC_VER_SIGN_R_ZERO,               /**< 0xCC - R set to zero */
	XSECURE_ELLIPTIC_VER_SIGN_S_ZERO,               /**< 0xCD - S set to zero */
	XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR,        /**< 0xCE - R is not within ECC order */
	XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR,        /**< 0xCF - S is not within ECC order */
	XSECURE_ELLIPTIC_KAT_INVLD_CRV_ERROR,   /**< 0xD0 - Curve not supported for KAT */
	XSECURE_ELLIPTIC_KAT_SIGN_VERIFY_ERROR,	/**< 0xD1 - Signature verify error for KAT */
	XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_ERROR, /**< 0xD2 - Generate Signature error for KAT */
	XSECURE_ELLIPTIC_KAT_GENERATE_SIGNR_ERROR, /**< 0xD3 - Generate Signature R error for KAT */
	XSECURE_ELLIPTIC_KAT_GENERATE_SIGN_64BIT_ERROR, /**< 0xD4 - Generate Signature error for KAT */
	XSECURE_ELLIPTIC_KAT_64BIT_SIGN_VERIFY_ERROR,	/**< 0xD5 - Signature verify error for KAT */
	XSECURE_ECC_PRVT_KEY_GEN_ERR = 0xD6, 		/**< 0xD6 - ECC private key generation error */
	XSECURE_HSS_SHA2_256_KAT_ERROR,			/**< 0xD7 - LMS SHA2-256 KAT error */
	XSECURE_HSS_SHAKE_256_KAT_ERROR,		/**< 0xD8 - LMS SHAKE KAT error */

	/* Error codes related to key unwrap */
	XSECURE_ERR_KEY_STORE_SIZE = 0xE0,         /**< 0xE0 - Key store size error */
	XSECURE_ERR_NO_FREE_KEY_SLOT,              /**< 0xE1 - No free key slot available to store the key */
	XSECURE_ERR_KEY_WRAP_SIZE_MISMATCH,        /**< 0xE2 - Key size mismatch between wrap and unwrap */
	XSECURE_ERR_AES_KEY_SIZE_NOT_SUPPORTED,    /**< 0xE3 - AES key size is not supported */
	XSECURE_ERR_CODE_RESERVED = 0xF0,	/**< 0xF0 -
	                    Till 2022.1 - No tamper response when tamper interrupt is detected
	                    From 2022.2 - Reserved */
	XSECURE_IPI_ACCESS_NOT_ALLOWED,         /**< 0xF1 - Access to Xilsecure IPIs
						is disabled if IPI request is non secure */
	XSECURE_SHA3_KAT_BUSY,			/**< 0xF2 - SHA3 busy with earlier operation,
						Kat can't be executed */
	XSECURE_AES_KAT_BUSY,			/**< 0xF3 - AES busy with earlier operation,
						Kat can't be executed */
	XSECURE_ERR_CRYPTO_ACCELERATOR_DISABLED, /**< 0xF4 - Crypto Accelerators are disabled */
	XSECURE_ERR_KAT_NOT_EXECUTED,		    /**< 0xF5 - Error when KAT is not executed when
							crypto kat efuse bit is enabled */
	XSECURE_RSA_GEN_SIGN_FAILED_ERROR,    /**< 0xF6 - Error when RSA sign generation is failed */
	XSECURE_ERR_ASU_KTE_DONE_NOT_SET,	/**< 0xF7 - Error in Key Transfer to ASU */
	XSECURE_RSA_OP_MEM_CPY_FAILED_ERROR,	/**< 0xF8 - Error when MemCpy is failed after RSA operation */
	XSECURE_RSA_OP_MEM_CPY_AND_CHANGE_ENDIANNESS_FAILED_ERROR, /**< 0xF9 - Error during MemCpy and reversing the endianness */
	XSECURE_RSA_OP_MEM_SET_ERROR,		/**< 0xF9 - Error when MemSet is failed during RSA private operation */
	XSECURE_RSA_OP_REVERSE_ENDIANESS_ERROR,	/**< 0xFA - Error when reversing the endianness during RSA private operation */
	XSECURE_ERR_GLITCH_DETECTED,		/**< 0xFB - Error glitch detected */
	XSECURE_ERR_IN_TRNG_SELF_TESTS,		/**< 0xFC - Error in TRNG operation self tests */
	XSECURE_ERR_TRNG_INIT_N_CONFIG		/**< 0xFD - Error in TRNG Instantiate and configuration */

} XSecure_ErrorCodes;

#define XSECURE_SHA3_KAT_FAILED_ERROR		XSECURE_SHA_KAT_FAILED_ERROR

#define XSECURE_SHA3_INVALID_PARAM		XSECURE_SHA_INVALID_PARAM

#define XSECURE_SHA3_PMC_DMA_UPDATE_ERROR	XSECURE_SHA_PMC_DMA_UPDATE_ERROR

#define XSECURE_SHA3_FINISH_ERROR		XSECURE_SHA_FINISH_ERROR
/**
 * @}
*/
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ERROR_H_ */
