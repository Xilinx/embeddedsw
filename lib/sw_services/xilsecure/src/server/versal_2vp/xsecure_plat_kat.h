/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_plat_kat.h
* This file contains versal_2vp specific code for KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.6   tvp  07/07/25 Initial release
*       tvp  07/07/25 Add API for HMAC KAT
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis Xilsecure KAT Server APIs
* @{
*/
#ifndef XSECURE_PLAT_KAT_H_
#define XSECURE_PLAT_KAT_H_

#ifdef __cplusplus
extern "c" {
#endif

/*************************************** Include Files ********************************************/
#include "xparameters.h"
#include "xsecure_sha.h"

#ifndef PLM_RSA_EXCLUDE
#include "xil_types.h"

/************************************ Constant Definitions ****************************************/
int XSecure_RsaPrivateDecryptKat(void);

#endif

int XSecure_HmacKat(XSecure_Sha3 *SecureSha3);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_H_ */
/** @} */
