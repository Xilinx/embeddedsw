/******************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file client/core/lms_hss/xsecure_lmsalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.6   vss     09/18/25 Added FIPS info read support for lms
* 5.7   tvp     04/29/26 Changed file path and modified XSecure_LmsGetCryptoAlgInfo
*                         Moved function implementation to .c file
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

/****************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/** @addtogroup xsecure_lms_client_apis XilSecure LMS Client APIs
 * @{
 */

void XSecure_LmsGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMSALGINFO_H */
