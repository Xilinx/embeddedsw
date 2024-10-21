/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.c
*
* This file contains known answer tests for Versal Net
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 5.0   kpt  07/15/2022 Initial release
* 5.2   kpt  07/12/2023 Added pairwise consistency test for RSA
*       yog  08/07/2023 Removed trng kat functions
*       dd   10/11/23 MISRA-C violation Rule 10.3 fixed
*       dd   10/11/23 MISRA-C violation Rule 17.7 fixed
*       dd   10/11/23 MISRA-C violation Rule 8.13 fixed
* 5.3   kpt  12/07/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       kpt  12/13/23 Added RSA CRT support for PWCT
* 5.3   ng   01/28/24 Added SDT support
*       kpt  03/15/24 Added RSA private decrypt KAT
*       ng   03/26/24 Fixed header include in SDT flow
* 5.4   yog  04/29/24 Fixed doxygen grouping.
*       kpt  06/13/24 Add support for RSA key generation.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis Xilsecure KAT Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_plat_kat.h"
#include "xsecure_rsa.h"
#include "xsecure_hmac.h"
#include "xsecure_error.h"
#include "xil_sutil.h"
#include "xsecure_sha384.h"

#ifdef SDT
#include "xsecure_config.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function performs KAT on HMAC (SHA3-384).
 *
 * @param	SecureSha3	Pointer to SHA3 instance
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_HMAC_KAT_INIT_ERROR  If HMAC init fails
 *		 - XSECURE_HMAC_KAT_UPDATE_ERROR  If HMAC update fails
 *		 - XSECURE_HMAC_KAT_FINAL_ERROR  If HMAC final fails
 *		 - XSECURE_HMAC_KAT_ERROR  If HMAC KAT fails
 *
 *****************************************************************************/
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
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
		Status = (int)XSECURE_HMAC_KAT_INIT_ERROR;
		goto END;
	}
	Status = XSecure_HmacUpdate(&HmacInstance, (UINTPTR)HmacMsg,
				XSECURE_KAT_MSG_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_HMAC_KAT_UPDATE_ERROR;
		goto END;
	}
	Status = XSecure_HmacFinal(&HmacInstance, &Hmac);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_HMAC_KAT_FINAL_ERROR;
		goto END;
	}
	Status = (int)XSECURE_HMAC_KAT_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (HmacExpected[Index] != Hmac.Hash[Index]) {
			Status = (int)XSECURE_HMAC_KAT_ERROR;
			goto END;
		}
	}

	if(Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}
END:
	SStatus = Xil_SecureZeroize(Hmac.Hash, XSECURE_HASH_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs KAT on SHA-384.
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_SHA384_KAT_ERROR  If SHA384 KAT fails
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_Sha384Kat(void)
{
	volatile int Status = (int)XSECURE_SHA384_KAT_ERROR;
	volatile int SStatus = (int)XSECURE_SHA384_KAT_ERROR;
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

	Status = XSecure_Sha384Digest(Data, XSECURE_KAT_MSG_LEN_IN_BYTES, CalculatedHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

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
	SStatus = Xil_SecureZeroize(CalculatedHash, XSECURE_HASH_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	return Status;
}
#ifndef PLM_RSA_EXCLUDE

/*****************************************************************************/
/**
 * @brief	This function performs pairwise consistency test for generated RSA key pair using
 * OAEP encrypt and decrypt operation.
 *
 * @param	PrivKey		Pointer to the XSecure_RsaKey
 * @param	PubKey		Pointer to the XSecure_RsaPubKey
 * @param	ShaInstancePtr	Pointer to the SHA instance used during OAEP encoding for MGF
 * @param	Shatype		is SHA algorithm type used for MGF
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XSECURE_RSA_PWCT_MEM_CMP_FAILED_ERROR  If RSA pwct comparison fails
 *		 - XST_FAILURE  On Failure
 *
 *****************************************************************************/
int XSecure_RsaPwct(XSecure_RsaPrivKey *PrivKey, XSecure_RsaPubKey *PubKey, void *ShaInstancePtr, XSecure_ShaMode Shatype)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	XSecure_Rsa RsaInstance = {0U};
	const u8 *Message = XSecure_GetKatMessage();
	XSecure_RsaOaepParam OaepParam = {0U};
	u8 EncOutput[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES];
	u8 DecOutput[XSECURE_KAT_MSG_LEN_IN_BYTES];
	u32 PubExp = 0U;

	if ((PrivKey == NULL) || (PubKey == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SChangeEndiannessAndCpy((u8*)&PubExp, XSECURE_RSA_PUB_EXP_SIZE, (u8*)PubKey->PubExp, XSECURE_RSA_PUB_EXP_SIZE,
				XSECURE_RSA_PUB_EXP_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaInitialize(&RsaInstance, PubKey->Mod, NULL, (u8*)&PubExp);
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

	Status = Xil_SReverseData(EncOutput, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	OaepParam.InputDataAddr = (u64)(UINTPTR)EncOutput;
	OaepParam.OutputDataAddr = (u64)(UINTPTR)DecOutput;
	OaepParam.ShaInstancePtr = (void*)ShaInstancePtr;
	OaepParam.ShaType = Shatype;
	Status = XSecure_RsaOaepDecrypt(PrivKey, &OaepParam);
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
	SStatus = Xil_SecureZeroize(EncOutput, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	SStatus |= Xil_SecureZeroize(DecOutput, XSECURE_KAT_MSG_LEN_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function performs private decrypt KAT on RSA core
 *
 * @return
 *		 - XST_SUCCESS  On success
 *		 - XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR  Error when RSA data not
 *							matched with expected data
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_RsaPrivateDecryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	u32 *RsaModulus = XSecure_GetKatRsaModulus();
	u32 *RsaCtData = XSecure_GetKatRsaCtData();
	u32 *RsaPrivExp = XSecure_GetKatRsaPrivateExp();
	u8 DataVal;
	u8 *RsaData = (u8*)XSecure_GetKatRsaData();
	u8 RsaOutput[XSECURE_RSA_2048_KEY_SIZE];
	u8 Mod[XSECURE_RSA_2048_KEY_SIZE];
	u8 Exp[XSECURE_RSA_2048_KEY_SIZE];
	u8 Data[XSECURE_RSA_2048_KEY_SIZE];
	static const u32 Totient[XSECURE_RSA_2048_SIZE_WORDS] = {
		0x95BD7BB0U, 0x05F0C930U, 0x78681CE6U, 0x39AA5CD0U,
		0x5811E800U, 0xBD1BB27DU, 0xEB2A366DU, 0x60121A86U,
		0x5CE10B2DU, 0x8F5B5DFEU, 0xDC729B60U, 0x709D283AU,
		0xCC4B8288U, 0xE44D962CU, 0x66253E9FU, 0x6867B3BDU,
		0xE1D323D6U, 0xD404F7EEU, 0xD0AE43F5U, 0x2A0BFC95U,
		0xCF2E5F7FU, 0x4127F2D1U, 0xC78667ACU, 0x80C8EC95U,
		0x3654EDB3U, 0xEB4FDB52U, 0x161235A0U, 0x45671A4EU,
		0xCE991592U, 0x7157CA40U, 0x2894DB37U, 0x4B31AA13U,
		0x995E1472U, 0x7C62AF06U, 0x3EE151AEU, 0x1FA34E7AU,
		0x5F300035U, 0xB0E8830BU, 0x63B1F63BU, 0xFE531F5CU,
		0xBA50A110U, 0xA93369D3U, 0x96B97E1BU, 0x61CBD28AU,
		0x5D0BD65CU, 0x3F1F71D9U, 0x46E737B0U, 0x91D56A0DU,
		0xCF2ECB6CU, 0x65AA3084U, 0xD8F9216AU, 0x98DF7569U,
		0x70B44AFCU, 0x77C08511U, 0x8328A54CU, 0xFD0C30B7U,
		0xE09D5555U, 0xAC131C20U, 0x67EF31CDU, 0xDBF9B357U,
		0x6FA31D8CU, 0x1728A249U, 0xF1060147U, 0xAEFEC693U
	};

	Status = Xil_SChangeEndiannessAndCpy(Mod, XSECURE_RSA_2048_KEY_SIZE, RsaModulus, XSECURE_RSA_2048_KEY_SIZE, XSECURE_RSA_2048_KEY_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SChangeEndiannessAndCpy(Exp, XSECURE_RSA_2048_KEY_SIZE, RsaPrivExp, XSECURE_RSA_2048_KEY_SIZE, XSECURE_RSA_2048_KEY_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SChangeEndiannessAndCpy(Data, XSECURE_RSA_2048_KEY_SIZE, RsaCtData, XSECURE_RSA_2048_KEY_SIZE, XSECURE_RSA_2048_KEY_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaExp((unsigned char *)(UINTPTR)Data, Exp, Mod, NULL, NULL, NULL,
		(u8*)Totient, (int)(XSECURE_RSA_2048_KEY_SIZE * 8U), RsaOutput);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	for (Index = 0U; Index < (XSECURE_RSA_2048_KEY_SIZE/2U); Index++) {
		DataVal = RsaOutput[XSECURE_RSA_2048_KEY_SIZE - Index - 1U];
		RsaOutput[XSECURE_RSA_2048_KEY_SIZE - Index - 1U] =  RsaOutput[Index];
		RsaOutput[Index] = DataVal;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_2048_KEY_SIZE; Index++) {
		if (RsaOutput[Index] != RsaData[Index]) {
			Status = (int)XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_2048_KEY_SIZE) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = Xil_SecureZeroize((u8*)RsaOutput, XSECURE_RSA_2048_KEY_SIZE);
	SStatus |= Xil_SecureZeroize((u8*)Exp, XSECURE_RSA_2048_KEY_SIZE);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}

#endif
/** @} */
