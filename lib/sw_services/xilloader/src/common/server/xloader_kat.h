/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xloader_kat.h
*
* This file contains all KAT related data.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- ----------------------------------------------------------------------------
* 2.4	rpu  04/15/2026 First release
* </pre>
*
* @note
*
***************************************************************************************************/

#ifndef XLOADER_KAT_H
#define XLOADER_KAT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ******************************************************/
#include "xplmi.h"
#include "xloader.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xloader_auth_enc.h"
#endif

/***************** Macros (Inline Functions) Definitions ******************************************/
#define XLOADER_PPDI_KAT_MASK		    (0x03U) /**< PPDI KAT mask */
/************************** Constant Definitions **************************************************/

/**************************** Type Definitions ****************************************************/

/***************************** Function Prototypes ************************************************/
int XLoader_Sha3Kat(XilPdi *PdiPtr);
void XLoader_ClearKatOnPPDI(XilPdi *PdiPtr, u32 PlmKatMask);
#ifndef PLM_SECURE_EXCLUDE
int XLoader_AesKatTest(XLoader_SecureParams *SecurePtr);
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
int XLoader_Sha2256Kat(XLoader_SecureParams *SecurePtr);
int XLoader_Shake256Kat(XLoader_SecureParams *SecurePtr);
int XLoader_LmsSha2256Kat(XLoader_SecureParams *SecurePtr);
int XLoader_LmsShake256Kat(XLoader_SecureParams *SecurePtr);
int XLoader_HssShake256Kat(XLoader_SecureParams *SecurePtr);
int XLoader_HssSha256Kat(XLoader_SecureParams *SecurePtr);
#endif
#endif /* PLM_SECURE_EXCLUDE */

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_KAT_H */