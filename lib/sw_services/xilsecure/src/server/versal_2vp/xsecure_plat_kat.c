/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_plat_kat.c
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
/*************************************** Include Files ********************************************/

#include "xparameters.h"
#include "xsecure_error.h"
#include "xsecure_hmac.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xsecure_sha.h"
#include "xil_sutil.h"
#include "xsecure_kat.h"
#include "xsecure_plat_kat.h"

/************************************ Function Prototypes *****************************************/

/************************************ Function Definitions ****************************************/

#ifndef PLM_RSA_EXCLUDE
/**************************************************************************************************/
/**
 * @brief	This function performs private decrypt KAT on RSA core.
 *
 * @return
 *		- XST_SUCCESS  On success.
 *		- XSECURE_RSA_KAT_INIT_ERROR  When RSA initialization fails.
 *		- XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR  When RSA KAT fails.
 *		- XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR  Error when RSA data not matched with
 *		  expected data.
 *
 **************************************************************************************************/
int XSecure_RsaPrivateDecryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
	u32 *RsaModulusPtr = XSecure_GetKatRsaModulus();
	u32 *RsaModExtPtr = XSecure_GetKatRsaModExt();
	u32 *RsaExpCtDataPtr = XSecure_GetKatRsaCtData();
	u32 *RsaDataPtr = XSecure_GetKatRsaData();
	u32 *RsaPrivateExpPtr = XSecure_GetKatRsaPrivateExp();
	u32 RsaOutput[XSECURE_RSA_2048_SIZE_WORDS];

	/** Initialize RSA */
	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)RsaModulusPtr, (u8 *)RsaModExtPtr,
			(u8 *)RsaPrivateExpPtr);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_INIT_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	/** Perform RSA private decrypt operation */
	Status = XSecure_RsaPrivateDecrypt(&XSecureRsaInstance, (u8 *)RsaExpCtDataPtr,
		XSECURE_RSA_2048_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	/** Validate the decrypted data with the expected data provided */
	for (Index = 0U; Index < XSECURE_RSA_2048_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaDataPtr[Index]) {
			Status = (int)XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR;
			goto END_CLR;
		}
	}
	if (Index == XSECURE_RSA_2048_SIZE_WORDS) {
		Status = XST_SUCCESS;
	}

END_CLR:
	SStatus = Xil_SecureZeroize((u8*)RsaOutput, XSECURE_RSA_2048_KEY_SIZE);
	SStatus |= Xil_SecureZeroize((u8*)&XSecureRsaInstance, sizeof(XSecure_Rsa));
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

END:
	return Status;
}
#endif

/**************************************************************************************************/
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
 **************************************************************************************************/
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_HmacRes Hmac = {0U};
	XSecure_Hmac HmacInstance;
	const u8 HmacExpected[XSECURE_HASH_SIZE_IN_BYTES] = {
		0x0E, 0x1D, 0x1E, 0x2A, 0x22, 0x6F, 0xB9, 0x56, 0x10, 0x4F, 0x10, 0x00, 0x8A, 0x50,
		0xE3, 0x5E, 0xAB, 0x2E, 0x37, 0xB5, 0xE0, 0x9F, 0xA1, 0x68, 0x2F, 0xE4, 0x93, 0x59,
		0x71, 0x96, 0xCC, 0x1B, 0x40, 0xFD, 0xCB, 0xDD, 0x93, 0x4F, 0x01, 0x3A, 0xB2, 0x64,
		0xE9, 0xC5, 0x2B, 0xB0, 0x2E, 0x52 };
	u8 *HmacKey = XSecure_GetKatAesKey();
	u8 *HmacMsg = XSecure_GetKatMessage();

	Status = XSecure_HmacInit(&HmacInstance, SecureSha3, (UINTPTR)HmacKey,
				XSECURE_KAT_KEY_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_HMAC_KAT_INIT_ERROR;
		goto END;
	}

	Status = XSecure_HmacUpdate(&HmacInstance, (UINTPTR)HmacMsg, XSECURE_KAT_MSG_LEN_IN_BYTES);
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

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}
END:
	SStatus = Xil_SecureZeroize(Hmac.Hash, XSECURE_HASH_SIZE_IN_BYTES);
	if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = SStatus;
	}

	return Status;
}
/** @} */
