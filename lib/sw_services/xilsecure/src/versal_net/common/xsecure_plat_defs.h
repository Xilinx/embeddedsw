/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
* 1.0   bm   07/06/22 Initial release
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
	XSECURE_API_RSA_KAT,			/**< 4U */
	XSECURE_API_SHA3_UPDATE = 32U,		/**< 32U */
	XSECURE_API_SHA3_KAT,			/**< 33U */
	XSECURE_API_ELLIPTIC_GENERATE_KEY = 64U,/**< 64U */
	XSECURE_API_ELLIPTIC_GENERATE_SIGN,	/**< 65U */
	XSECURE_API_ELLIPTIC_VALIDATE_KEY,	/**< 66U */
	XSECURE_API_ELLIPTIC_VERIFY_SIGN,	/**< 67U */
	XSECURE_API_ELLIPTIC_KAT,		/**< 68U */
	XSECURE_API_AES_INIT = 96U,		/**< 96U */
	XSECURE_API_AES_OP_INIT,		/**< 97U */
	XSECURE_API_AES_UPDATE_AAD,		/**< 98U */
	XSECURE_API_AES_ENCRYPT_UPDATE,		/**< 99U */
	XSECURE_API_AES_ENCRYPT_FINAL,		/**< 100U */
	XSECURE_API_AES_DECRYPT_UPDATE,		/**< 101U */
	XSECURE_API_AES_DECRYPT_FINAL,		/**< 102U */
	XSECURE_API_AES_KEY_ZERO,		/**< 103U */
	XSECURE_API_AES_WRITE_KEY,		/**< 104U */
	XSECURE_API_AES_LOCK_USER_KEY,		/**< 105U */
	XSECURE_API_AES_KEK_DECRYPT,		/**< 106U */
	XSECURE_API_AES_SET_DPA_CM,		/**< 107U */
	XSECURE_API_AES_DECRYPT_KAT,		/**< 108U */
	XSECURE_API_AES_DECRYPT_CM_KAT,		/**< 109U */
	XSECURE_API_TRNG_GENERATE,		/**< 110U */
	XSECURE_API_TRNG_KAT,			/**< 111U */
	XSECURE_API_MAX,			/**< 112U */
} XSecure_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_DEFS_H */
