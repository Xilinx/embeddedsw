/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file server/spartanup/xsecure_sha3alginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     08/18/24 Initial Release
* 5.7   tbk     04/22/26 Updated version number
* 5.8   ank     10/07/26 Updated minor version to 6.2
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_SHA3ALGINFO_H
#define XSECURE_SHA3ALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_SHA3_MAJOR_VERSION	6U /**< Major version of SHA3 */
#define XSECURE_SHA3_MINOR_VERSION	2U /**< Minor version of SHA3 */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the SHA3 crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static XSECURE_ALWAYS_INLINE void XSecure_Sha3GetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_SHA3_MAJOR_VERSION, XSECURE_SHA3_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHA3ALGINFO_H */
