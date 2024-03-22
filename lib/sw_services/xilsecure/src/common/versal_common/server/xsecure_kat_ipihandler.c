/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All rights reserved.
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
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.00  kpt  07/15/2022 Initial release
* 1.01  kpt  12/13/2022 Added Trng initialization in XSecure_EllipticSignGenKat
* 1.02  ng   05/10/2023 Removed XSecure_PerformKatOperation and implemented
*                       redundant call for XPlmi_ClearKatMask
* 5.2   ng   07/13/2023 Added SDT support
*       yog  08/07/2023 Removed trng init call in XSecure_EllipticSignGenKat API
*                       since trng is being initialised in server API's
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_kat.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xplmi.h"
#include "xsecure_error.h"
#include "xil_util.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/
#define XSECURE_PMCDMA_DEVICEID		PMCDMA_0_DEVICE
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
	u32 *Pload = NULL;

	if (NULL == Cmd) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	/** Call the respective API handler according to API ID */
	switch (Pload[0U] & XSECURE_API_ID_MASK) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
		/**   - @ref XSecure_AesDecKat */
		Status = XSecure_AesDecKat();
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		/**   - @ref XSecure_AesDecCmKat */
		Status = XSecure_AesDecCmKat();
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_KAT):
		/**   - @ref XSecure_AesEncKat */
		Status = XSecure_AesEncKat();
		break;
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUB_ENC_KAT):
		/**   - @ref XSecure_RsaPubEncKat */
		Status = XSecure_RsaPubEncKat();
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DEC_KAT):
		/**   - @ref XSecure_RsaPrivateDecKat */
		Status = XSecure_RsaPrivateDecKat();
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT):
		/**   - @ref XSecure_EllipticSignVerifyKat */
		Status = XSecure_EllipticSignVerifyKat((XSecure_EccCrvClass)Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_GEN_KAT):
		/**   - @ref XSecure_EllipticSignGenKat */
		Status = XSecure_EllipticSignGenKat((XSecure_EccCrvClass)Pload[1U]);
		break;
#endif
#endif
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		/**   - @ref XSecure_ShaKat */
		Status = XSecure_ShaKat();
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
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

END:
	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_AES_DEC_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_AES_DEC_KAT_MASK);
	}
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

END:
	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_AES_CMKAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_AES_CMKAT_MASK);
	}
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

END:
	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_AES_ENC_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_AES_ENC_KAT_MASK);
	}
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

	Status = XSecure_EllipticVerifySignKat((XSecure_EllipticCrvClass)CurveClass);
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK);
	}
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

	Status = XSecure_EllipticSignGenerateKat((XSecure_EllipticCrvClass)CurveClass);

	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK);
	}

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

	Status = XSecure_RsaPublicEncryptKat();
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_RSA_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_RSA_KAT_MASK);
	}
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

	Status = XSecure_RsaPrivateDecryptKat();
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK);
	}
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

END:
	/* Update KAT status in to RTC area */
	if (Status != XST_SUCCESS) {
		XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, XPLMI_SECURE_SHA3_KAT_MASK);
	}
	else {
		XPlmi_SetKatMask(XPLMI_SECURE_SHA3_KAT_MASK);
	}
	return Status;
}
