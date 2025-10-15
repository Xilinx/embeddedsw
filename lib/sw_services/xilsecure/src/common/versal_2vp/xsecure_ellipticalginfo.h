/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_ellipticalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.6   tvp  07/07/25 Initial Release
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_ELLIPTICALGINFO_H
#define XSECURE_ELLIPTICALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/************************************ Constant Definitions ****************************************/
#define XSECURE_ELLIPTIC_MAJOR_VERSION	6U /**< Major version of ELLIPTIC */
#define XSECURE_ELLIPTIC_MINOR_VERSION	0U /**< Minor version of ELLIPTIC */

/*************************** Macros (Inline Functions) Definitions ********************************/

/**************************************************************************************************/
/**
 *
 * This function returns the Elliptic crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information.
 *
 * @return
 * 		- None.
 *
 **************************************************************************************************/
static XSECURE_ALWAYS_INLINE void XSecure_EllipticGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_ELLIPTIC_MAJOR_VERSION,
					     XSECURE_ELLIPTIC_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTICALGINFO_H */
