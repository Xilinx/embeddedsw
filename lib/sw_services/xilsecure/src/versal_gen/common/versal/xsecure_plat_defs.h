/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_defs.h
*
* This file contains the xsecure API IDs for versal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.0   bm   07/06/22 Initial release
*       kpt  07/24/22 Added XSECURE_API_KAT and additional KAT ids
*       dc   08/26/22 Removed gaps between the API IDs
* 5.1   skg  12/16/22 Added IPI commands for Encrypt/Decrypt Init,update,Final
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_PLAT_DEFS_H
#define XSECURE_PLAT_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/

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
	XSECURE_API_KAT,				/**< 21U */
	/**< 22U reserved for versal_net*/
	XSECURE_API_AES_PERFORM_OPERATION = 23U, /**< 23U */
	XSECURE_API_MAX,			/**< 24U */
} XSecure_ApiId;

typedef enum {
	XSECURE_API_AES_DECRYPT_KAT = 0U,		/**< 0U */
	XSECURE_API_AES_DECRYPT_CM_KAT,			/**< 1U */
	XSECURE_API_RSA_PUB_ENC_KAT,			/**< 2U */
	XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT,	/**< 3U */
	XSECURE_API_SHA3_KAT,					/**< 4U */
	XSECURE_API_AES_ENCRYPT_KAT,			/**< 5U */
	XSECURE_API_RSA_PRIVATE_DEC_KAT,		/**< 6U */
	XSECURE_API_ELLIPTIC_SIGN_GEN_KAT,		/**< 7U */
} XSecure_KatId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_DEFS_H */
