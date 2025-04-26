/******************************************************************************
* Copyright (c) 2023 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_ecdhalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mss     12/10/23 Initial Release
* 5.4   kpt     06/30/24 Updated version number
* 5.5   vss     04/25/25 Updated minor version
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_ECDHALGINFO_H
#define XSECURE_ECDHALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ECDH_MAJOR_VERSION	5 /**< Major version of ECDH */
#define XSECURE_ECDH_MINOR_VERSION	7 /**< Minor version of ECDH */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the ECDH crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_EcdhGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_ECDH_MAJOR_VERSION, XSECURE_ECDH_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ECDHALGINFO_H */
