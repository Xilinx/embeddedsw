/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ECDH_MAJOR_VERSION	5 /**< Major version of ECDH */
#define XSECURE_ECDH_MINOR_VERSION	3 /**< Minor version of ECDH */

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
