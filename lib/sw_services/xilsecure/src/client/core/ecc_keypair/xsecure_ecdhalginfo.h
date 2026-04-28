/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file client/core/ecc_keypair/xsecure_ecdhalginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------------------------------
* 5.7   tbk     04/24/26 Initial Release
*
* </pre>
*
***************************************************************************************************/

#ifndef XSECURE_ECDHALGINFO_H
#define XSECURE_ECDHALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xil_cryptoalginfo.h"
#include "xil_sutil.h"

/************************************** Constant Definitions **************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/*************************************** Function Prototypes **************************************/

/** @addtogroup xsecure_ecdsa_client_apis XilSecure ECDSA Client APIs
 * @{
 */

void XSecure_EcdhGetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ECDHALGINFO_H */
