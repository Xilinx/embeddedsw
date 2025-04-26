/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xtrngpsv_alginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.4   mmd     07/04/23 Initial Release
*       dd      08/07/23 Updated doxygen comments
*       mb      09/14/23 Fix MISRA- C violations for Rule 10.1
* 1.5   mb      04/01/24 Update minor version number
* 1.7   vss     04/25/25 Updated minor version
*
* </pre>
*
******************************************************************************/

#ifndef XTRNGPSV_ALGINFO_H
#define XTRNGPSV_ALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"

/**************************** Constant Definitions ****************************/
#define XTRNGPSV_MAJOR_VERSION	1U /**< Major version of Trngpsv driver */
#define XTRNGPSV_MINOR_VERSION	7U /**< Minor version of Trngpsv driver */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the TRNG crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XTrngpsv_GetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XTRNGPSV_MAJOR_VERSION, XTRNGPSV_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XTRNGPSV_AESALGINFO_H */
