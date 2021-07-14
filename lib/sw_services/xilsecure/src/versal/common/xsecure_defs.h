/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_defs.h
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
*
* </pre>
* @note
*
******************************************************************************/

#ifndef XSECURE_DEFS_H
#define XSECURE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"

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
#define XSECURE_DEBUG_GENERAL (0U)
#endif
/** @} */
/***************** Macros (Inline Functions) Definitions *********************/
#define XSecure_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}
				/**< For prints in XilSecure library */

#define XSECURE_API(ApiId)	((u32)ApiId)
				/**< Macro to typecast XILSECURE API ID */

#define XSECURE_API_ID_MASK	0xFFU
				/**< Mask for API ID in Secure IPI command */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u64 KeyAddr;	/**< Key Address */
	u64 DataAddr;	/**< Data Address */
	u32 Size;	/**< Size of key */
} XSecure_RsaInParam;

typedef struct {
	u64 SignAddr;	/**< RSA Signature Address */
	u64 HashAddr;	/**< Hash Address */
	u32 Size;	/**< Length of hash */
} XSecure_RsaSignParams;

typedef struct {
	u64 HashAddr;	/**< Hash address */
	u64 PrivKeyAddr;	/**< Static Private key Address */
	u64 EPrivKeyAddr;	/**< Ephemeral private key Address */
	u32 CurveType;	/**< ECC curve type */
	u32 Size;	/**< Length of hash */
} XSecure_EllipticSignGenParams;

typedef struct {
	u64 HashAddr;	/**< Hash address */
	u64 PubKeyAddr;	/**< Public key Address */
	u64 SignAddr;	/**< Signature Address */
	u32 CurveType;	/**< ECC curve type */
	u32 Size;		/**< Length of hash */
} XSecure_EllipticSignVerifyParams;

typedef struct {
	u64 IvAddr;	/**< IV address */
	u32 OperationId;/**< Operation type - Encrypt or decrypt */
	u32 KeySrc;	/**< AES Key source */
	u32 KeySize;	/**< Size of AES key*/
} XSecure_AesInitOps;

typedef struct {
	u64 InDataAddr;	/**< Address of input data*/
	u32 Size;	/**< Length of input data*/
	u32 IsLast;	/**< Flag to indicate last update of data*/
} XSecure_AesInParams;

typedef enum {
	XSECURE_ENCRYPT,	/**< Encrypt operation */
	XSECURE_DECRYPT,	/**< Decrypt operation */
} XSecure_AesOp;

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
	XSECURE_API_MAX,			/**< 110U */
} XSecure_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_DEFS_H */
