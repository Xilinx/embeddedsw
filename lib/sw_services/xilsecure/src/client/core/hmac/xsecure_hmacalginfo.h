/******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file client/core/hmac/xsecure_hmacalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.6   vss     09/18/25 Added FIPS info read support for HMAC
* 5.7   tbk     04/09/26 Changed file path and modified XSecure_HmacGetCryptoAlgInfo
*                         Moved function implementation to .c file
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_HMACALGINFO_H
#define XSECURE_HMACALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/**************************** Constant Definitions ****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/** @addtogroup xsecure_hmac_client_apis XilSecure HMAC Client APIs
 * @{
 */

void XSecure_HmacGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_HMACALGINFO_H */
