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
*       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
*
* </pre>
*
* @note
*
***************************************************************************************************/
/**
* @addtogroup xrsa_ecc_server_apis RSA ECC Server APIs
* @{
*/
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
#define XRSA_ECC_SUPPORT_NIST_P521	XASUFW_RSA_ECC_SUPPORT_NIST_P521
	/**< Macro to enable/disable support of NIST P-521 curve */
#define XRSA_ECC_SUPPORT_NIST_P192	XASUFW_RSA_ECC_SUPPORT_NIST_P192
	/**< Macro to enable/disable support of NIST P-192 curve */
#define XRSA_ECC_SUPPORT_NIST_P224	XASUFW_RSA_ECC_SUPPORT_NIST_P224
	/**< Macro to enable/disable support of NIST P-224 curve */
#define XRSA_ECC_SUPPORT_BRAINPOOL_P256	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P256
	/**< Macro to enable/disable support of Brainpool P-256 curve */
#define XRSA_ECC_SUPPORT_BRAINPOOL_P320	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P320
	/**< Macro to enable/disable support of Brainpool P-320 curve */
#define XRSA_ECC_SUPPORT_BRAINPOOL_P384	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P384
	/**< Macro to enable/disable support of Brainpool P-384 curve */
#define XRSA_ECC_SUPPORT_BRAINPOOL_P512	XASUFW_RSA_ECC_SUPPORT_BRAINPOOL_P512
	/**< Macro to enable/disable support of Brainpool P-512 curve */
/** @} */

/** This typedef is used to know the prime and binary curve values. */
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
/** @} */
