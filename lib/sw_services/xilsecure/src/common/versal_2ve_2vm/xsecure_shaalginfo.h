/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_shaalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/04/23 Initial Release
*	vss	09/21/23 Fixed doxygen warnings
* 5.3   mb      04/01/24 Updated minor version
* 5.4   kpt     06/30/24 Updated version number
* 5.6   vss     09/30/25 Renamed file to xsecure_shaalginfo.h
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_SHAALGINFO_H
#define XSECURE_SHAALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_SHA_MAJOR_VERSION	6U /**< Major version of SHA */
#define XSECURE_SHA_MINOR_VERSION	0U /**< Minor version of SHA */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the SHA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_ShaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_SHA_MAJOR_VERSION, XSECURE_SHA_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHAALGINFO_H */
