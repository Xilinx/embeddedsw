/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xrsa_eccinfo.h
*
* This file contains the macros and types related to elliptic curve information
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   yog  07/11/24 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/
#ifndef XRSA_ECCINFO_H_
#define XRSA_ECCINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "Ecdsa.h"

/************************************ Constant Definitions ***************************************/
/**
 * @name Supported ECC curves
 * @{
 */
/**< Macros to enable/disable support of NIST P-192, P-224 and NIST P-521 curve */
#define XRSA_ECC_SUPPORT_NIST_P521	XASUFW_RSA_ECC_SUPPORT_NIST_P521
#define XRSA_ECC_SUPPORT_NIST_P192	XASUFW_RSA_ECC_SUPPORT_NIST_P192
#define XRSA_ECC_SUPPORT_NIST_P224	XASUFW_RSA_ECC_SUPPORT_NIST_P224
#define XRSA_ECC_SUPPORT_BRAINPOOL_P256	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P256
#define XRSA_ECC_SUPPORT_BRAINPOOL_P320	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P320
#define XRSA_ECC_SUPPORT_BRAINPOOL_P384	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P384
#define XRSA_ECC_SUPPORT_BRAINPOOL_P512	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P512

/** @} */

typedef enum {
	XRSA_ECC_PRIME = 0,		/**< Prime curve value in Ecdsa.h */
	XRSA_ECC_BINARY = 1,		/**< Binary curve value in Ecdsa.h */
} XRsa_EccCrvClass;

/************************************** Type Definitions *****************************************/
extern EcdsaCrvInfo XRsa_EccCrvsDb[];

/************************************* Function Prototypes ***************************************/
u32 XRsa_EccCrvsGetCount(void);

#ifdef __cplusplus
}
#endif

#endif /* XRSA_ECCINFO_H */
