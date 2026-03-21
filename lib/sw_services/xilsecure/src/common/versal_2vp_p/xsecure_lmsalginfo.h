/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file versal_2vp_p/xsecure_lmsalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tvp     03/20/26 Initial release
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_LMSALGINFO_H
#define XSECURE_LMSALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/************************************** Constant Definitions **************************************/
#define XSECURE_LMS_MAJOR_VERSION	6U /**< Major version of LMS */
#define XSECURE_LMS_MINOR_VERSION	0U /**< Minor version of LMS */

/**************************** Macros (Inline Functions) Definitions *******************************/

/**************************************************************************************************/
/**
 *
 * This function returns the LMS crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 **************************************************************************************************/
static XSECURE_ALWAYS_INLINE void XSecure_LmsGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_LMS_MAJOR_VERSION, XSECURE_LMS_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMSALGINFO_H */
