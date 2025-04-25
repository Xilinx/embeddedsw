/******************************************************************************
* Copyright (C) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.h
*
*
* This file contains KAT interface APIs for Versal_2Ve_2Vm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.4   kal  07/24/2024 Initial release
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis Xilsecure KAT Server APIs
* @{
*/
#ifndef XSECURE_PLAT_KAT_H_
#define XSECURE_PLAT_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_kat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
/**
 * @cond xsecure_internal
 * @{
 */
int XSecure_HmacKat(XSecure_Sha *SecureSha);
/**
 * @}
 * @endcond
 */
int XSecure_Sha384Kat(void);
int XSecure_RsaPrivateDecryptKat(void);
int XSecure_Sha2256Kat(XSecure_Sha *SecureSha);
int XSecure_ShakeKat(XSecure_Sha *SecureSha);
int XSecure_HssSha2256Kat(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr);
int XSecure_HssShake256Kat(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr);
int XSecure_LmsSha2256Kat(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr);
int XSecure_LmsShake256Kat(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_H_ */
