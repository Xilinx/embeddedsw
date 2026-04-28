/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file client/core/ecdsa/xsecure_ellipticalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   mmd     07/10/23 Initial Release
*       vss     09/21/23 Fixed doxygen warnings
* 5.3   kpt     03/30/24 Updated minor version
* 5.4   mb      05/23/24 Updated minor version for 24.2
* 5.6   vss     09/30/25 Updated version number
* 5.7   tbk     02/05/26 Changed file path and modified XSecure_EllipticGetCryptoAlgInfo
*                         Moved function implementation to .c file
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_ELLIPTICALGINFO_H
#define XSECURE_ELLIPTICALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/** @addtogroup xsecure_ecdsa_client_apis XilSecure ECDSA Client APIs
 * @{
 */

void XSecure_EllipticGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ELLIPTICALGINFO_H */
