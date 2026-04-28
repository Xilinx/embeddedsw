/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_keyunwrapalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.3   mss     10/19/23 Initial Release
* 5.4   kpt     06/30/24 Updated version number
* 5.5   vss     04/25/25 Updated minor version
* 5.7   tbk     04/09/26 Changed file path and modified XSecure_KeyUnwrapGetCryptoAlgInfo
*                         Moved function implementation to .c file
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_KEYUNWRAPALGINFO_H
#define XSECURE_KEYUNWRAPALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"
#include "xsecure_alghelper.h"
#include "xsecure_plat_defs.h"

/**************************** Constant Definitions ****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/** @addtogroup xsecure_keyunwrap_client_apis XilSecure Key Unwrap Client APIs
 * @{
 */

void XSecure_KeyUnwrapGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_KEYUNWRAPALGINFO_H */
