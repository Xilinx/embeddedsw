/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_kat_ipihandler.c
* @addtogroup xsecure_apis XilSecure Versal KAT handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure KAT IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kpt   07/15/2022 Initial release
* 1.01  kpt   12/13/2022 Added Trng initialization in XSecure_EllipticSignGenKat
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xplmi.h"
#include "xsecure_error.h"
#include "xil_util.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/
#define XSECURE_PMCDMA_DEVICEID		PMCDMA_0_DEVICE_ID
			/**< PMCDMA device id */

/************************** Function Prototypes *****************************/
#ifndef PLM_SECURE_EXCLUDE
static int XSecure_AesDecKat(void);
static int XSecure_AesDecCmKat(void);
static int XSecure_AesEncKat(void);
#ifndef PLM_RSA_EXCLUDE
static int XSecure_RsaPubEncKat(void);
static int XSecure_RsaPrivateDecKat(void);
#endif
#ifndef PLM_ECDSA_EXCLUDE
static int XSecure_EllipticSignGenKat(XSecure_EccCrvClass CurveClass);
static int XSecure_EllipticSignVerifyKat(XSecure_EccCrvClass CurveClass);
#endif
#endif
static int XSecure_ShaKat(void);

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_KatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Pload[0U] & XSECURE_API_ID_MASK) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
		Status = XSecure_AesDecKat();
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		Status = XSecure_AesDecCmKat();
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_KAT):
		Status = XSecure_AesEncKat();
		break;
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUB_ENC_KAT):
		Status = XSecure_RsaPubEncKat();
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DEC_KAT):
		Status = XSecure_RsaPrivateDecKat();
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT):
		Status = XSecure_EllipticSignVerifyKat((XSecure_EccCrvClass)Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_GEN_KAT):
		Status = XSecure_EllipticSignGenKat((XSecure_EccCrvClass)Pload[1U]);
		break;
#endif
#endif
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		Status = XSecure_ShaKat();
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

#ifndef PLM_SECURE_EXCLUDE

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptKat server API
 *
 * @return
 *	-	XST_SUCCESS - If the KAT is successful
 * 	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if ((XSecureAesInstPtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(XSecureAesInstPtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecryptKat(XSecureAesInstPtr);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}
END:
	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_AES_DEC_KAT_MASK);
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesDecryptCmKat
 * 		server API
 *
 * @return
 *	-	XST_SUCCESS - If the KAT is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesDecCmKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if ((XSecureAesInstPtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(XSecureAesInstPtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesDecryptCmKat(XSecureAesInstPtr);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}
END:
	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_AES_CMKAT_MASK);
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_AesEncryptKat
 * 		server API
 *
 * @return
 *	-	XST_SUCCESS - If the KAT is successful
 *	-	XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
static int XSecure_AesEncKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if ((XSecureAesInstPtr->AesState == XSECURE_AES_ENCRYPT_INITIALIZED) ||
		(XSecureAesInstPtr->AesState == XSECURE_AES_DECRYPT_INITIALIZED)) {
		Status = (int)XSECURE_AES_KAT_BUSY;
		goto END;
	}

	/* Initialize the Aes driver so that it's ready to use */
	Status = XSecure_AesInitialize(XSecureAesInstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_AesEncryptKat(XSecureAesInstPtr);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}
END:
	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_AES_ENC_KAT_MASK);
	return Status;
}

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticVerifySignKat
 * 		server API
 *
 * @param	CurveClass	- Is a class of elliptic curve
 *
 * @return
 *	-	XST_SUCCESS - If the elliptic KAT is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticSignVerifyKat(XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_EllipticVerifySignKat((XSecure_EllipticCrvClass)CurveClass);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticSignGenerateKat
 * 		server API
 *
 * @param	CurveClass	- Is a class of elliptic curve
 *
 * @return
 *	-	XST_SUCCESS - If the elliptic KAT is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticSignGenKat(XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

   /*
	* Initialize TRNG if it is not initialized as VersalNet ECC library internally
	* uses TRNG API to generate random mask.
	*/
	Status = XSecure_TrngInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_EllipticSignGenerateKat((XSecure_EllipticCrvClass)CurveClass);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK);

END:
	return Status;
}
#endif

#ifndef PLM_RSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaPublicEncryptKat server
 * 		API
 *
 * @return
 *	-	XST_SUCCESS - If the Rsa Kat is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaPubEncKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_RsaPublicEncryptKat();
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_RSA_KAT_MASK);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaPrivateDecryptKat server
 * 		API
 *
 * @return
 *	-	XST_SUCCESS - If the Rsa Kat is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaPrivateDecKat(void)
{
	volatile int Status = XST_FAILURE;
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_RsaPrivateDecryptKat();
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK);

	return Status;
}
#endif

#endif

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_ShaKat server API
 *
 * @return
 *	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	volatile XSecure_KatOp KatOp = XSECURE_API_KAT_CLEAR;

	if (NULL == PmcDmaInstPtr) {
		goto END;
	}

	if (XSecureSha3InstPtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED) {
		Status = (int)XSECURE_SHA3_KAT_BUSY;
		goto END;
	}

	Status = XSecure_Sha3Initialize(XSecureSha3InstPtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_Sha3Kat(XSecureSha3InstPtr);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}
END:
	/* Update KAT status in to RTC area */
	XSecure_PerformKatOperation(KatOp, XPLMI_SECURE_SHA3_KAT_MASK);
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sets or clears KAT mask based on crypto kat
 *
 * @param	KatOp	- Operation to either set or clear the KAT mask
 * @param   KatMask - KAT mask to set or clear the RTC area
 *
 ******************************************************************************/
void XSecure_PerformKatOperation(XSecure_KatOp KatOp, u32 KatMask)
{
	volatile u8 CryptoKatEn = XPlmi_IsCryptoKatEn();
	volatile u8 CryptoKatEnTmp = XPlmi_IsCryptoKatEn();

	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		if (KatOp == XSECURE_API_KAT_CLEAR) {
			XPlmi_ClearKatMask(KatMask);
		}
		else if (KatOp == XSECURE_API_KAT_SET) {
			XPlmi_SetKatMask(KatMask);
		}
		else {
			/* For MISRA-C compliance */
		}
	}
}
