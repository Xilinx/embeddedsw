/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_aesalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.5   mmd     08/18/24 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_AESALGINFO_H
#define XSECURE_AESALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_AES_MAJOR_VERSION	5U /**< Major version of AES */
#define XSECURE_AES_MINOR_VERSION	3U /**< Minor version of AES */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the AES crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_AesGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_AES_MAJOR_VERSION, XSECURE_AES_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_AESALGINFO_H */
