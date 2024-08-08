/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_kat.c
* This file contains versal specific code for KAT.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   kpt   03/12/24 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsecure_error.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa.h"
#endif
#include "xil_util.h"
#include "xsecure_kat.h"
#include "xsecure_plat_kat.h"

/************************** Function Prototypes *****************************/

/*************************** Function Definitions *****************************/\

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function performs private decrypt KAT on RSA core
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR - When RSA KAT fails
 *	-	XSECURE_RSA_KAT_DECRYPT_DATA_MISMATCH_ERROR - Error when RSA data not
 *							matched with expected data
 *
 *****************************************************************************/
int XSecure_RsaPrivateDecryptKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	volatile u32 Index;
	XSecure_Rsa XSecureRsaInstance;
    u32 *RsaModulus = XSecure_GetKatRsaModulus();
	u32 *RsaModExt = XSecure_GetKatRsaModExt();
    u32 *RsaExpCtData = XSecure_GetKatRsaCtData();
    u32 *RsaData = XSecure_GetKatRsaData();
	u32 *RsaPrivateExp = XSecure_GetKatRsaPrivateExp();
	u32 RsaOutput[XSECURE_RSA_2048_SIZE_WORDS];

	Status = XSecure_RsaInitialize(&XSecureRsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)RsaPrivateExp);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_INIT_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_RsaPrivateDecrypt(&XSecureRsaInstance, (u8 *)RsaExpCtData,
		XSECURE_RSA_2048_KEY_SIZE, (u8 *)RsaOutput);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_KAT_DECRYPT_FAILED_ERROR;
		goto END_CLR;
	}

	/* Initialized to error */
	Status = (int)XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR;
	for (Index = 0U; Index < XSECURE_RSA_2048_SIZE_WORDS; Index++) {
		if (RsaOutput[Index] != RsaData[Index]) {
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
