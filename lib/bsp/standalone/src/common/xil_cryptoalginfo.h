/******************************************************************************/
/**
* Copyright (c) 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
