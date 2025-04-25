/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_lms_core.h
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
#ifndef XSECURE_LMS_CORE_H_
#define XSECURE_LMS_CORE_H_

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
typedef struct {
	u8* SignBuff;	/** Buffer to store signature to be verified */
	u32 SignatureLen;	/** Length of signature to be verified */
	u8* PublicKey;	/** Buffer to store public key to verify signature */
	u32 PublicKeyLen;	/** Length of public key to verify signature */
} XSecure_HssInitParams;

typedef struct {
	u8* Data;	/** Buffer to store data which is signed */
	u32 DataLen;	/** Length of data which is signed */
	u32 PreHashedMsg;	/** Flag to indicate if data is prehashed */
	u8* LmsSign;	/** Buffer to store signature to be verified */
	u32 LmsSignLen;	/** Length of signature to be verified */
	u8* ExpectedPubKey;	/** Buffer to store expected public key */
	u32 PubKeyLen;	/** Length of public key to verify signature */
} XSecure_LmsSignVerifyParams;

/***************************** Type Definitions********************************/

/***************************** Function Prototypes ****************************/
int XSecure_HssInit(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr,
	XSecure_HssInitParams *HssInitParams);
int XSecure_LmsHashMessage(XSecure_Sha *ShaInstPtr, u8* Data, u32 DataLen,
	XSecure_ShaMode Mode);
int XSecure_HssFinish(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr, u8* SignBuff,
	u32 SignatureLen);
int XSecure_LmsSignatureVerification(XSecure_Sha *ShaInstPtr, XPmcDma *DmaPtr,
	XSecure_LmsSignVerifyParams *LmsSignVerifyParams);
int XSecure_GetLmsHashAlgo(u32 PubAlgo, const u8* const PubKey, XSecure_ShaMode *SignAlgo);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_LMS_CORE_H_ */
/** @} */
