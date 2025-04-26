/******************************************************************************
* Copyright (c) 2023 -2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.3   kpt     03/30/24 Updated minor version
* 5.4   mb      05/23/24 Updated minor version for 24.2
* 5.5   vss     04/25/25 Updated minor version
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
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ELLIPTIC_MAJOR_VERSION	5 /**< Major version of ELLIPTIC */
#define XSECURE_ELLIPTIC_MINOR_VERSION	7 /**< Minor version of ELLIPTIC */

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
