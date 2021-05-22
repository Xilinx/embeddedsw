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
/* Enable client printfs by setting XSECURE_DEBUG to 1 */
#define XSECURE_DEBUG	(0U)

#if (XSECURE_DEBUG)
#define XSECURE_DEBUG_GENERAL (1U)
#else
#define XSECURE_DEBUG_GENERAL (0U)
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XSecure_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}

/* Macro to typecast XILSECURE API ID */
#define XSECURE_API(ApiId)	((u32)ApiId)

#define XSECURE_API_ID_MASK	0xFFU

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u64 KeyAddr;
	u64 DataAddr;
	u32 Size;
} XSecure_RsaInParam;

typedef struct {
	u64 SignAddr;
	u64 HashAddr;
	u32 Size;
} XSecure_RsaSignParams;

typedef struct {
	u64 HashAddr;
	u64 PrivKeyAddr;
	u64 EPrivKeyAddr;
	u32 CurveType;
	u32 Size;
} XSecure_EllipticSignGenParams;

typedef struct {
	u64 HashAddr;
	u64 PubKeyAddr;
	u64 SignAddr;
	u32 CurveType;
	u32 Size;
} XSecure_EllipticSignVerifyParams;

typedef struct {
	u64 IvAddr;
	u32 OperationId;
	u32 KeySrc;
	u32 KeySize;
} XSecure_AesInitOps;

typedef struct {
	u64 InDataAddr;
	u32 Size;
	u32 IsLast;
} XSecure_AesInParams;

typedef enum {
	XSECURE_ENCRYPT,
	XSECURE_DECRYPT,
} XSecure_AesOp;

/* XilSecure API ids */
typedef enum {
	XSECURE_API_FEATURES = 0U,
	XSECURE_API_RSA_SIGN_VERIFY,
	XSECURE_API_RSA_PUBLIC_ENCRYPT,
	XSECURE_API_RSA_PRIVATE_DECRYPT,
	XSECURE_API_RSA_KAT,
	XSECURE_API_SHA3_UPDATE = 32U,
	XSECURE_API_SHA3_KAT,
	XSECURE_API_ELLIPTIC_GENERATE_KEY = 64U,
	XSECURE_API_ELLIPTIC_GENERATE_SIGN,
	XSECURE_API_ELLIPTIC_VALIDATE_KEY,
	XSECURE_API_ELLIPTIC_VERIFY_SIGN,
	XSECURE_API_ELLIPTIC_KAT,
	XSECURE_API_AES_INIT = 96U,
	XSECURE_API_AES_OP_INIT,
	XSECURE_API_AES_UPDATE_AAD,
	XSECURE_API_AES_ENCRYPT_UPDATE,
	XSECURE_API_AES_ENCRYPT_FINAL,
	XSECURE_API_AES_DECRYPT_UPDATE,
	XSECURE_API_AES_DECRYPT_FINAL,
	XSECURE_API_AES_KEY_ZERO,
	XSECURE_API_AES_WRITE_KEY,
	XSECURE_API_AES_LOCK_USER_KEY,
	XSECURE_API_AES_KEK_DECRYPT,
	XSECURE_API_AES_SET_DPA_CM,
	XSECURE_API_AES_DECRYPT_KAT,
	XSECURE_API_AES_DECRYPT_CM_KAT,
	XSECURE_API_MAX,
} XSecure_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_DEFS_H */
