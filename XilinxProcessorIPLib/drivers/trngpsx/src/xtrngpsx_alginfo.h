/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xtrngpsx_alginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.1   mmd     07/04/23 Initial Release
* 1.2   mb      04/01/24 Updated minor version
* 1.4   vss     04/25/25 Updated minor version
* 1.5   ank     09/26/25 Fixed MISRA-C Violations
* </pre>
*
******************************************************************************/

#ifndef XTRNGPSX_ALGINFO_H
#define XTRNGPSX_ALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/
#define XTRNGPSX_MAJOR_VERSION	1U
#define XTRNGPSX_MINOR_VERSION	5U

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the TRNG crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 * @return	None
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XTrngpsx_GetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XTRNGPSX_MAJOR_VERSION, XTRNGPSX_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XTRNGPSX_ALGINFO_H */
