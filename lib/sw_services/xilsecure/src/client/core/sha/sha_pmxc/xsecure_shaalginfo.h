/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file client/core/sha/sha_pmxc/xsecure_shaalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/04/23 Initial Release
*	vss	09/21/23 Fixed doxygen warnings
* 5.3   mb      04/01/24 Updated minor version
* 5.4   kpt     06/30/24 Updated version number
* 5.6   vss     09/30/25 Renamed file to xsecure_shaalginfo.h
* 5.7   tbk     04/09/26 Changed file path and modified XSecure_ShaGetCryptoAlgInfo
*                         Moved function implementation to .c file
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_SHAALGINFO_H
#define XSECURE_SHAALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/** @addtogroup xsecure_sha3_client_apis XilSecure SHA Client APIs
 * @{
 */

void XSecure_ShaGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SHAALGINFO_H */
