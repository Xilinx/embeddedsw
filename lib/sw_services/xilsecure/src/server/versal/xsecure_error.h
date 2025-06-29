/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_error.h
 *
 * This file contains xilsecure error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  03/19/2020 Initial release
 * 4.2   rpo  03/19/2020 Updated file version to sync with library version
 * 4.3   ana  06/05/2020 Added XSECURE_SHA3_FINISH_ERROR error code
 *       har  08/24/2020 Added ECDSA error codes
 *       rpo  09/10/2020 Added Invalid argument error codes
 *       rpo  09/21/2020 New error code added for crypto state mismatch
 *       am   09/24/2020 Resolved MISRA C violations
 *       har  10/12/2020 Addressed security review comments
 *       ana  10/15/2020 Updated doxygen comments
 * 4.5   har  01/18/2021 Added error code for invalid ECC curve
 *       kpt  02/04/2021 Added error code for tamper response
 *       har  05/18/2021 Added error code XSECURE_IPI_ACCESS_NOT_ALLOWED
 *                       Added error code XSECURE_AES_DEVICE_KEY_NOT_ALLOWED
 * 4.8   ma   07/08/2022 Added support for secure lockdown
 *       dc   07/13/2022 Reserved error codes reservation for Versal net
 * 5.0   kpt  07/24/2022 Added error codes for KAT
 * 5.2   am   06/22/2023 Added KAT error code
 *       kpt  07/13/2023 Added XSECURE_AES_INVALID_MODE error code
 * 5.4   yog  04/29/2024 Fixed doxygen warnings.
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

/** Error codes of XilSecure for versal. */
typedef enum {
	XSECURE_SHA3_INIT_ERROR = 0x02,		/**< 0x02 - Error when SHA3
						   init fails. */
	XSECURE_SHA3_LAST_UPDATE_ERROR,		/**< 0x03 - Error when SHA3
						   last update fails. */
	XSECURE_SHA3_PMC_DMA_UPDATE_ERROR,	/**< 0x04 - Error when DMA driver
						   fails to update the data to SHA3 */
	XSECURE_SHA3_TIMEOUT_ERROR,		/**< 0x05 - Error when timeout */

	XSECURE_SHA3_KAT_FAILED_ERROR,		/**< 0x06 - Error when SHA3 hash
						   not matched with expected hash */
	XSECURE_SHA3_FINISH_ERROR,		/**<0x07 - Error when SHA3 finish fails */

	XSECURE_SHA3_INVALID_PARAM,		/**< 0x08 - Invalid Argument */

	XSECURE_SSS_INVALID_PARAM,		/**< 0x09 - Invalid Argument */

	XSECURE_SHA3_STATE_MISMATCH_ERROR,	/**< 0x0A - State mismatch */

	XSECURE_AES_GCM_TAG_MISMATCH = 0x40,	/**< 0x40 - user provided GCM tag does
						   not match calculated tag */
	XSECURE_AES_KEY_CLEAR_ERROR,
						/**< 0x41 - AES key clear error */
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
	XSECURE_AES_KAT_UPDATE_AAD_FAILED_ERROR,/**< 0x56 - Error when aad update
														fails */
	XSECURE_AES_KAT_ENCRYPT_INIT_FAILED_ERROR, /**< 0x57 - Error when encrypt
														init fails */
	XSECURE_AES_KAT_ENCRYPT_UPDATE_FAILED_ERROR,/**< 0x58 - Error when encrypt
														update fails */
	XSECURE_AES_KAT_ENCRYPT_FINAL_FAILED_ERROR,/**< 0x59 - Error when encrypt
														final fails */
	XSECURE_KAT_GCM_TAG_MISMATCH_ERROR,/**< 0x5A - Error when GCM mismatch occurs */
	XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED,	/**< 0x5B - Error when PUF Key is selected as key source and PUF key is zeroized */
	XSECURE_AES_UNALIGNED_SIZE_ERROR,	/**< 0x5C - Error when data is unaligned*/
	XSECURE_AES_INVALID_MODE,             /**< 0x5D - Error when invalid mode is used for AES operation */
	XSECURE_INVALID_RESOURCE, /**< 0X5F - Error when resource is other than SHA/AES */

	XSECURE_RSA_KAT_INIT_ERROR = 0x80,	/**< 0x80 - RSA KAT initialization failure */

	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR, /**< 0x81 - RSA KAT fails  */

	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR, /**< 0x82 - Error when RSA data
							not matched with
							expected data  */
	XSECURE_RSA_INVALID_PARAM_RESERVED,	     /**< 0x83 - Invalid Argument */
	XSECURE_RSAKAT_INVALID_PARAM,		     /**< 0x84 - Invalid Argument */
	XSECURE_RSA_STATE_MISMATCH_RESERVED,	     /**< 0x85 - State mismatch */
	XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR,	/**< 0x86 - RSA decrypt failed error */
	XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR,	/**< 0x87 - RSA when decrypted data doesn't
							match with plain text */

	/**
	 * The error codes from 0x90 to 0xBF are reserved for Versal net platform
	 * This range is reserved please don't define any error codes for Versal
	 */

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
	XSECURE_ERR_KAT_NOT_EXECUTED		    /**< 0xF5 - Error when KAT is not executed when
												crypto kat efuse bit is enabled */

} XSecure_ErrorCodes;

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ERROR_H_ */
/** @} */
