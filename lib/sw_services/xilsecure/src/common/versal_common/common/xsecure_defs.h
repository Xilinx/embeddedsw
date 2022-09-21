/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
* 4.7   kpt  11/29/21 Added macro XSecure_DCacheFlushRange
* 5.0   bm   07/06/22 Refactor versal and versal_net code
*       kpt  07/24/22 Added XSecure_EccCrvClass
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
#include "xil_cache.h"
#include "xsecure_plat_defs.h"

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
#ifndef XSECURE_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XSecure_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
	#else
		#define XSecure_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
	#endif
#else
	#define XSecure_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

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

typedef enum {
	XSECURE_ECDSA_PRIME = 0,	/**< Prime curve */
	XSECURE_ECDSA_BINARY = 1,	/**< Binary curve */
}XSecure_EccCrvClass;


#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_DEFS_H */
