/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_kat_ipihandler.c
* @addtogroup xsecure_apis XilSecure versal net KAT handler APIs
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
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xsecure_defs.h"
#include "xsecure_error.h"
#include "xil_util.h"
#include "xsecure_init.h"
#include "xplmi.h"

#ifndef PLM_SECURE_EXCLUDE
/************************** Constant Definitions *****************************/
#define XSECURE_PMCDMA_DEVICEID		PMCDMA_0_DEVICE_ID
			/**< PMCDMA device id */

/************************** Function Prototypes *****************************/
static int XSecure_AesExecuteDecKat(void);
static int XSecure_AesExecuteDecCmKat(void);
static int XSecure_EllipticSignVerifyKat(XSecure_EccCrvClass CurveClass);
static int XSecure_RsaPubEncKat(void);
static int XSecure_AesExecuteEncKat(void);
static int XSecure_EllipticSignGenKat(XSecure_EccCrvClass CurveClass);
static int XSecure_RsaPrivateDecKat(void);
static int XSecure_TrngKat(void);
#endif
static int XSecure_ShaKat(void);
static int XSecure_UpdateKatStatus(XSecure_KatId KatOp, XSecure_KatId KatId);
static int XSecure_KatOp(XSecure_KatId KatOp, u32 KatMask);

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
		Status = XSecure_AesExecuteDecKat();
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		Status = XSecure_AesExecuteDecCmKat();
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT):
		Status = XSecure_EllipticSignVerifyKat(Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_RSA_PUB_ENC_KAT):
		Status = XSecure_RsaPubEncKat();
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_KAT):
		Status = XSecure_AesExecuteEncKat();
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DEC_KAT):
		Status = XSecure_RsaPrivateDecKat();
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_GEN_KAT):
		Status = XSecure_EllipticSignGenKat(Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_TRNG_KAT):
		Status = XSecure_TrngKat();
		break;
#endif
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		Status = XSecure_ShaKat();
		break;
	case XSECURE_API(XSECURE_API_KAT_CLEAR):
	case XSECURE_API(XSECURE_API_KAT_SET):
		Status = XSecure_UpdateKatStatus(Pload[1U], Pload[2U]);
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
static int XSecure_AesExecuteDecKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

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

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_AES_DEC_KAT_MASK);
END:
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
static int XSecure_AesExecuteDecCmKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

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

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_AES_CMKAT_MASK);

END:
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
static int XSecure_AesExecuteEncKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Aes *XSecureAesInstPtr = XSecure_GetAesInstance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(XSECURE_PMCDMA_DEVICEID);
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

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

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_ENC_KAT_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticVerifySignKat
 * 		server API
 *
 * @param	CurveType	- Is a type of elliptic curve
 *
 * @return
 *	-	XST_SUCCESS - If the elliptic KAT is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticSignVerifyKat(XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_EllipticVerifySignKat((XSecure_EllipticCrvClass)CurveClass);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_KAT_MASK);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticSignGenerateKat
 * 		server API
 *
 * @param	CurveType	- Is a type of elliptic curve
 *
 * @return
 *	-	XST_SUCCESS - If the elliptic KAT is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticSignGenKat(XSecure_EccCrvClass CurveClass)
{
	volatile int Status = XST_FAILURE;
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_EllipticSignGenerateKat((XSecure_EllipticCrvClass)CurveClass);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_ECC_SIGN_GEN_SHA384_KAT_MASK);

	return Status;
}

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
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_RsaPublicEncryptKat();
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_RSA_KAT_MASK);

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
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_RsaPrivateDecryptKat();
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_RSA_PRIVATE_DEC_KAT_MASK);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_TrngPreOperationalSelfTests Server API
 *
 * @return	- XST_SUCCESS - If the KAT is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_TrngKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_TrngInstance *TrngInstance = XSecure_GetTrngInstance();
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

	Status = XSecure_TrngPreOperationalSelfTests(TrngInstance);
	if (Status != XST_SUCCESS) {
		KatOp = XSECURE_API_KAT_CLEAR;
	}
	else {
		KatOp = XSECURE_API_KAT_SET;
	}

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_TRNG_KAT_MASK);

	return Status;
}

#endif

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_ShaKat server API
 *
 * @return
	-	XST_SUCCESS - If the sha update/fnish is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_ShaKat(void)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(0U);
	XSecure_KatId KatOp = XSECURE_API_KAT_CLEAR;

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

	/* Update KAT status in to RTC area */
	XSecure_KatOp(KatOp, XPLMI_SECURE_SHA3_KAT_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sets or clears KAT mask of given KatId
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateKatStatus(XSecure_KatId KatOp, XSecure_KatId KatId) {
	int Status = XST_FAILURE;
	u32 KatMask = 0U;

	switch((u32)KatId) {
		case XSECURE_API_CPM5N_AES_XTS:
			KatMask = XPLMI_SECURE_CPM5N_AES_XTS_KAT_MASK;
			break;
		case XSECURE_API_CPM5N_AES_PCI_IDE:
			KatMask = XPLMI_SECURE_CPM5N_PCI_IDE_KAT_MASK;
			break;
		case XSECURE_API_NICSEC_KAT:
			KatMask = XPLMI_SECURE_NICSEC_KAT_MASK;
			break;
		default:
			XSecure_Printf(XSECURE_DEBUG_GENERAL,"Invalid KATId for operation");
			break;
	}
	if (KatMask != 0U) {
		Status = XSecure_KatOp(KatOp, KatMask);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sets or clears KAT mask based on crypto kat
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_KatOp(XSecure_KatId KatOp, u32 KatMask)
{
	volatile u8 CryptoKatEn = XPlmi_IsCryptoKatEn();
	volatile u8 CryptoKatEnTmp = CryptoKatEn;
	int Status = XST_FAILURE;

	if ((CryptoKatEn == TRUE) || (CryptoKatEnTmp == TRUE)) {
		if (KatOp == XSECURE_API_KAT_CLEAR) {
			XPlmi_ClearKatMask(KatMask);
		}
		else if (KatOp == XSECURE_API_KAT_SET) {
			XPlmi_SetKatMask(KatMask);
		}
		else {
			goto END;
		}
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
