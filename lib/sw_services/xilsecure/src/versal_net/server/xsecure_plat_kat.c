/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.c
*
* This file contains known answer tests for versal net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
* 5.2   kpt  07/12/2023 Added pairwise consistency test for RSA
*       yog  08/07/2023 Removed trng kat functions
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_plat_kat.h"
#include "xsecure_rsa.h"
#include "xsecure_hmac.h"
#include "xsecure_error.h"
#include "xil_util.h"
#include "xsecure_sha384.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function performs KAT on HMAC (SHA3-384).
 *
 * @param SecureSha3 Pointer to SHA3 instance
 *
 * @return	returns the error codes
 *		returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;
	XSecure_HmacRes Hmac = {0U};
	XSecure_Hmac HmacInstance;
	const u8 HmacExpected[XSECURE_HASH_SIZE_IN_BYTES] = {
		0x0E,0x1D,0x1E,0x2A,0x22,0x6F,0xB9,0x56,
		0x10,0x4F,0x10,0x00,0x8A,0x50,0xE3,0x5E,
		0xAB,0x2E,0x37,0xB5,0xE0,0x9F,0xA1,0x68,
		0x2F,0xE4,0x93,0x59,0x71,0x96,0xCC,0x1B,
		0x40,0xFD,0xCB,0xDD,0x93,0x4F,0x01,0x3A,
		0xB2,0x64,0xE9,0xC5,0x2B,0xB0,0x2E,0x52
	};
	u8 *HmacKey = XSecure_GetKatAesKey();
	u8 *HmacMsg = XSecure_GetKatMessage();

	Status = XSecure_HmacInit(&HmacInstance, SecureSha3,
				(UINTPTR)HmacKey, XSECURE_KAT_KEY_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_INIT_ERROR;
		goto END;
	}
	Status = XSecure_HmacUpdate(&HmacInstance, (UINTPTR)HmacMsg,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_UPDATE_ERROR;
		goto END;
	}
	Status = XSecure_HmacFinal(&HmacInstance, &Hmac);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_FINAL_ERROR;
		goto END;
	}
	Status = XSECURE_HMAC_KAT_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (HmacExpected[Index] != Hmac.Hash[Index]) {
			Status = XSECURE_HMAC_KAT_ERROR;
			goto END;
		}
	}

	if(Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}
END:
	(void)memset((void *)Hmac.Hash, (u32)0,
			XSECURE_HASH_SIZE_IN_BYTES);

	return Status;
}

/*****************************************************************************/
/**
 * This function performs KAT on SHA-384.
 *
 * @return	returns the error codes
 *		returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XSecure_Sha384Kat(void)
{
	volatile int Status = (int)XSECURE_SHA384_KAT_ERROR;
	volatile u32 Index;
	u8 *Data = XSecure_GetKatMessage();
	u8 CalculatedHash[XSECURE_HASH_SIZE_IN_BYTES];
	const u8 ExpectedHash[XSECURE_HASH_SIZE_IN_BYTES] = {
		0x5AU, 0x2CU, 0xFCU, 0x1CU, 0xC1U, 0x1EU, 0x61U, 0x1BU,
		0xD1U, 0xEAU, 0x4EU, 0x51U, 0xC8U, 0x72U, 0x73U, 0x40U,
		0x01U, 0xCDU, 0x53U, 0x95U, 0x5DU, 0xC6U, 0xF9U, 0xFFU,
		0x42U, 0xD1U, 0x66U, 0xA1U, 0x6BU, 0x76U, 0x2EU, 0x42U,
		0x42U, 0x24U, 0xC2U, 0xBEU, 0xC4U, 0xEAU, 0x40U, 0xD4U,
		0xF9U, 0x9CU, 0x90U, 0x10U, 0xF6U, 0x18U, 0xFFU, 0x95U
	};

	XSecure_Sha384Digest(Data, XSECURE_KAT_MSG_LEN_IN_BYTES, CalculatedHash);

	for (Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (CalculatedHash[Index] != ExpectedHash[Index]) {
			Status = (int)XSECURE_SHA384_KAT_ERROR;
			goto END;
		}
	}

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
#ifndef PLM_RSA_EXCLUDE

/*****************************************************************************/
/**
 * This function performs pairwise consistency test for generated RSA key pair using
 * OAEP encrypt and decrypt operation.
 *
 * @param PrivKey Pointer to the private key
 * @param PubKey  Pointer to the public key
 * @param ShaInstancePtr Pointer to the SHA instance used during OAEP encoding for MGF
 * @param Shatype is SHA algorithm type used for MGF
 *
 * @return
 *        XST_SUCCESS - On Success
 *        ErrorCode   - On Failure
 *
 *****************************************************************************/
int XSecure_RsaPwct(XSecure_RsaKey *PrivKey, XSecure_RsaKey *PubKey, void *ShaInstancePtr, XSecure_ShaType Shatype)
{
	volatile int Status = XST_FAILURE;
	XSecure_Rsa RsaInstance = {0U};
	u8 *Message = XSecure_GetKatMessage();
	XSecure_RsaOaepParam OaepParam = {0U};
	u8 EncOutput[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];
	u8 DecOutput[XSECURE_KAT_MSG_LEN_IN_BYTES];

	Status = XSecure_RsaInitialize(&RsaInstance, PubKey->Modulus, PubKey->ModExt, PubKey->Exponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	OaepParam.InputDataAddr = (u64)(UINTPTR)Message;
	OaepParam.InputDataSize = XSECURE_KAT_MSG_LEN_IN_BYTES;
	OaepParam.OutputDataAddr = (u64)(UINTPTR)EncOutput;
	OaepParam.ShaInstancePtr = (void*)ShaInstancePtr;
	OaepParam.ShaType = Shatype;
	Status = XSecure_RsaOaepEncrypt(&RsaInstance, &OaepParam);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaInitialize(&RsaInstance, PrivKey->Modulus, PrivKey->ModExt, PrivKey->Exponent);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	OaepParam.InputDataAddr = (u64)(UINTPTR)EncOutput;
	OaepParam.OutputDataAddr = (u64)(UINTPTR)DecOutput;
	OaepParam.ShaInstancePtr = (void*)ShaInstancePtr;
	OaepParam.ShaType = Shatype;
	Status = XSecure_RsaOaepDecrypt(&RsaInstance, &OaepParam);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = Xil_SMemCmp(DecOutput, OaepParam.OutputDataSize, Message, XSECURE_KAT_MSG_LEN_IN_BYTES,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_PWCT_MEM_CMP_FAILED_ERROR;
	}

END:
	return Status;
}
#endif
