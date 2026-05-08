/***************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file common/versal_2vp/xsecure_plat_defs.h
*
* This file contains the xsecure API IDs for versal_2vp
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.6   tvp  07/07/25 Initial release
*       tvp  07/07/25 Added XSECURE_WORD_LEN
* 5.7   tvp  11/18/25 Added support for generating shared secret
*       tvp  11/18/25 Added support for trng operations
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_common_apis Xilsecure Common Apis
* @{
*/
#ifndef XSECURE_PLAT_DEFS_H
#define XSECURE_PLAT_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/

/************************************ Constant Definitions ****************************************/
#define XSECURE_WORD_LEN		(4U) /**< Word length */

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Variable Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/** XilSecure API ids */
typedef enum {
	XSECURE_API_FEATURES = 0U,		/**< 0U */
	XSECURE_API_RSA_SIGN_VERIFY,		/**< 1U */
	XSECURE_API_RSA_PUBLIC_ENCRYPT,		/**< 2U */
	XSECURE_API_RSA_PRIVATE_DECRYPT,	/**< 3U */
	XSECURE_API_SHA3_UPDATE,		/**< 4U */
	XSECURE_API_ELLIPTIC_GENERATE_KEY,	/**< 5U */
	XSECURE_API_ELLIPTIC_GENERATE_SIGN,	/**< 6U */
	XSECURE_API_ELLIPTIC_VALIDATE_KEY,	/**< 7U */
	XSECURE_API_ELLIPTIC_VERIFY_SIGN,	/**< 8U */
	XSECURE_API_AES_INIT,			/**< 9U */
	XSECURE_API_AES_OP_INIT,		/**< 10U */
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
	XSECURE_API_AES_PERFORM_OPERATION = 23U, /**< 23U */
	/**< 24U to 32U reserved for versal_net*/
	XSECURE_API_GEN_SHARED_SECRET = 33U,	/**< 33U */
	XSECURE_API_ELLIPTIC_PRIVATE_KEY_GEN = 43U,	/**< 43U */
	XSECURE_API_MAX,			/**< 44U */
} XSecure_ApiId;

/** XilSecure API KAT ids */
typedef enum {
	XSECURE_API_AES_DECRYPT_KAT = 0U,		/**< 0U */
	XSECURE_API_AES_DECRYPT_CM_KAT,			/**< 1U */
	XSECURE_API_RSA_PUB_ENC_KAT,			/**< 2U */
	XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT,		/**< 3U */
	XSECURE_API_SHA3_KAT,				/**< 4U */
	XSECURE_API_AES_ENCRYPT_KAT,			/**< 5U */
	XSECURE_API_RSA_PRIVATE_DEC_KAT,		/**< 6U */
	XSECURE_API_ELLIPTIC_SIGN_GEN_KAT,		/**< 7U */
	XSECURE_API_TRNG_KAT,				/**< 8U */
} XSecure_KatId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_DEFS_H */
/** @} */
