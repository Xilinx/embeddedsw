/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrom_lms.h
*
* This file contains the interface for LMS authentication methods
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.4   kal  07/24/24 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/
#ifndef XLOADER_LMS_H_
#define XLOADER_LMS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/
#include "xil_types.h"
#include "xsecure_lms_ots.h"
#include "xsecure_lms.h"
#include "xsecure_lms_hss.h"
#include "xsecure_sha.h"

/************************** Constant Definitions ******************************/

/***************************** Type Definitions********************************/

/***************************** Function Prototypes ****************************/
int XSecure_HssInit(XSecure_Sha *ShaInstPtr,
	XPmcDma *DmaPtr, u8* SignBuff,
	u32 SignatureLen, u8* PublicKey,
	u32 PublicKeyLen);
int XSecure_LmsHashMessage(XSecure_Sha *ShaInstPtr,
	u8* Data,
	u32 DataLen, XSecure_ShaMode Mode);
int XSecure_HssFinish(XSecure_Sha *ShaInstPtr,
	XPmcDma *DmaPtr, u8* SignBuff,
	u32 SignatureLen);
int XSecure_LmsSignatureVerification(XSecure_Sha *ShaInstPtr,
	XPmcDma *DmaPtr, u8* Data, u32 DataLen,
	u32 PreHashedMsg, u8* LmsSign,
	u32 LmsSignLen, u8* ExpectedPubKey,
	u32 PubKeyLen);
int XSecure_GetLmsHashAlgo(u32 PubAlgo, const u8* const PubKey, XSecure_ShaMode *SignAlgo);

#ifdef __cplusplus
}
#endif

#endif /* XLOADER_LMS_H_ */
/** @} */
