/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_ellipticalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/10/23 Initial Release
*       vss     09/21/23 Fixed doxygen warnings
*       vss     09/11/23 Fixed MISRA-C Rule 12.2 violation
* 5.3   kpt     03/30/24 Updated minor version
* </pre>
*
******************************************************************************/

#ifndef XSECURE_ELLIPTICALGINFO_H
#define XSECURE_ELLIPTICALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_util.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ELLIPTIC_MAJOR_VERSION	5U /**< Major version of ELLIPTIC */
#define XSECURE_ELLIPTIC_MINOR_VERSION	3U /**< Minor version of ELLIPTIC */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the Elliptic crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_EllipticGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_ELLIPTIC_MAJOR_VERSION, XSECURE_ELLIPTIC_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTICALGINFO_H */
