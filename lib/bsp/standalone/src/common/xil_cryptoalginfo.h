/******************************************************************************/
/**
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_cryptoalginfo.h
* @{
* @details
*
* Crypto algotithm information structure declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 9.0   mmd      07/04/23 First release.
* 9.4   mku      09/29/25 Added XSECURE_ALWAYS_INLINE macro useful for function inlining.
* </pre>
*
*****************************************************************************/
#ifndef XIL_CRYPTOALGINFO_H
#define XIL_CRYPTOALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ALWAYS_INLINE __attribute__((always_inline)) inline /**< Always inline function */

/**************************** Type Definitions *******************************/
typedef enum _Xil_CryptoAlgNistStatus {
	NOT_APPLICABLE = 0x00,
	NIST_COMPLIANT = 0x11,
	NIST_NON_COMPLIANT = 0xFE,
} Xil_CryptoAlgNistStatus;

typedef struct _Xil_CryptoAlgInfo {
	u32 Version;
	Xil_CryptoAlgNistStatus NistStatus;
} Xil_CryptoAlgInfo;

#ifdef __cplusplus
}
#endif

#endif /* XIL_CRYPTOALGINFO_H */
