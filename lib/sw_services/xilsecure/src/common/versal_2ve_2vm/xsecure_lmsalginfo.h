/******************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_lmsalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.6   vss     09/18/25 Added FIPS info read support for lms
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_LMSALGINFO_H
#define XSECURE_LMSALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_LMS_MAJOR_VERSION	6U /**< Major version of LMS */
#define XSECURE_LMS_MINOR_VERSION	0U /**< Minor version of LMS */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the LMS crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XSecure_LmsGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XSECURE_LMS_MAJOR_VERSION, XSECURE_LMS_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMSALGINFO_H */
