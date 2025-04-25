/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_defs.h
*
* This file contains the xsecure API IDs for versal_2ve_2vm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*       sk   08/22/24 Added support for key transfer to ASU
*       am   02/24/25 Moved XSECURE_API_ASU_KEY_TRANSFER to xilplmi
*       pre  03/02/25 Added API IDs for SHA2 operation
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

/************************** Variable Definitions *****************************/
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

typedef struct {
	u64 DataAddr; /**< SHA2/3 data address */
	u64 HashAddr; /**< SHA2/3 hash address */
	u32 DataSize; /**< SHA2/3 data size */
	u32 HashBufSize; /**< SHA2/3 hash buffer size */
	u8 ShaMode; /**< SHA2/3 mode */
	u8 IsLast; /**< Is last update */
	u8 OperationFlags; /**< SHA2/3 operation flags */
} XSecure_ShaOpParams;

/**< XilSecure API ids */
typedef enum {
	XSECURE_API_FEATURES = 0U,		/**< 0U */
	XSECURE_API_RSA_SIGN_VERIFY,		/**< 1U */
	XSECURE_API_RSA_PUBLIC_ENCRYPT,		/**< 2U */
	XSECURE_API_RSA_PRIVATE_DECRYPT,	/**< 3U */
	XSECURE_API_SHA3_OPERATION,		/**< 4U */
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
	XSECURE_API_AES_PERFORM_OPERATION, 	/**< 23U */
	XSECURE_API_GEN_SHARED_SECRET = 33U,	/**< 33U */
	XSECURE_API_RESERVED = 36U,		/**< 36U */
	XSECURE_API_AES_PERFORM_OPERATION_AND_ZEROIZE_KEY,/**< 37U */
	XSECURE_API_SHA2_OPERATION, 		/**< 38U */
	XSECURE_API_MAX,			/**< 39U */
} XSecure_ApiId;

/**< XilSecure KAT ids */
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
	XSECURE_API_UPDATE_KAT_STATUS,			/**< 9U */
	XSECURE_API_SHA2_KAT,				/**< 10U */
} XSecure_KatId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_DEFS_H */
