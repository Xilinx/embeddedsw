/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file common/versal_2vp_p/xsecure_shaalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------------------------------------
* 5.7   tvp     03/20/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_SHAALGINFO_H
#define XSECURE_SHAALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/************************************** Constant Definitions **************************************/
#define XSECURE_SHA_MAJOR_VERSION	6U /**< Major version of SHA */
#define XSECURE_SHA_MINOR_VERSION	0U /**< Minor version of SHA */

/****************** Macros (Inline Functions) Definitions *********************/

/**************************************************************************************************/
/**
 *
 * This function returns the SHA crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
static XSECURE_ALWAYS_INLINE void XSecure_ShaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_SHA_MAJOR_VERSION, XSECURE_SHA_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHAALGINFO_H */
