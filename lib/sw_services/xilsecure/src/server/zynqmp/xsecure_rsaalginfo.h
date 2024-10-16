/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_rsaalgoinfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/04/23 Initial Release
*	vss	09/21/23 Fixed doxygen warnings
* 5.3   am      12/18/23 Updated rsaalgoinfo version to 5.3
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_RSAALGINFO_H
#define XSECURE_RSAALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_RSA_MAJOR_VERSION	5U /**< Major version of RSA */
#define XSECURE_RSA_MINOR_VERSION	3U /**< Minor version of RSA */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the RSA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_RsaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_RSA_MAJOR_VERSION, XSECURE_RSA_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_RSAALGINFO_H */
