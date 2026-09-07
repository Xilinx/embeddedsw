/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/crypto_kat/xsecure_kat_ipihandler.c
*
* This file contains the Xilsecure KAT IPI handlers implementation.
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
* 5.4   yog  04/29/2024 Fixed doxygen grouping and doxygen warnings.
* 	kal  07/24/2024 Code refactoring for versal_2ve_2vm
*	vss  10/23/2024 Removed AES duplicate code
*       pre  03/02/2025 Implemented task based event notification functionality for AES &
*                       SHA IPI events
* 5.8   tbk  09/02/2026 Modify handling of KAT mask update to XSecure_KatIpiHandler via
*                       XSecure_UpdateKatMask helper and call KAT APIs directly
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis XilSecure KAT Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_kat.h"
#include "xsecure_plat_kat.h"
#include "xsecure_kat_ipihandler.h"
#include "xplmi.h"
#include "xsecure_error.h"
#include "xil_sutil.h"
#include "xsecure_init.h"
#include "xsecure_resourcehandling.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static void XSecure_UpdateKatMask(volatile int Status, u32 KatMask);

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 * 		and updates the RTCA KAT mask after the KAT runs. Event-handling
 * 		failures do not change the previous KAT bit.
 *
 * @param 	Cmd	is a pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS - If the handler execution is successful
 *		 - XST_INVALID_PARAM - If Cmd is NULL or invalid API ID is received.
 *		 - XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XSecure_KatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u32 *Pload = NULL;
	u32 ApiId;
	volatile u32 KatMask = 0U;
	XSecure_Sha3 *XSecureSha3InstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#ifndef PLM_SECURE_EXCLUDE
	XSecure_Aes *AesInstPtr = XSecure_GetAesInstance();
#endif

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;
	ApiId = Pload[0U] & XSECURE_API_ID_MASK;

#ifndef PLM_SECURE_EXCLUDE
	if ((ApiId == (u32)XSECURE_API_AES_DECRYPT_KAT) || (ApiId == (u32)XSECURE_API_AES_ENCRYPT_KAT) ||
	    (ApiId == (u32)XSECURE_API_AES_DECRYPT_CM_KAT)) {
		Status = XSecure_IpiEventHandling(Cmd, XPLMI_AES_CORE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif

	Status = XST_FAILURE;
	/** - Call the respective API handler according to API ID */
	switch (ApiId) {
#ifndef PLM_SECURE_EXCLUDE
	case XSECURE_API(XSECURE_API_AES_DECRYPT_KAT):
		/**   - XSecure_AesDecryptKat */
		KatMask = XPLMI_SECURE_AES_DEC_KAT_MASK;
		Status = XSecure_AesDecryptKat(AesInstPtr);
		break;
	case XSECURE_API(XSECURE_API_AES_DECRYPT_CM_KAT):
		/**   - XSecure_AesDecryptCmKat */
		KatMask = XPLMI_SECURE_AES_CMKAT_MASK;
		Status = XSecure_AesDecryptCmKat(AesInstPtr);
		break;
	case XSECURE_API(XSECURE_API_AES_ENCRYPT_KAT):
		/**   - XSecure_AesEncryptKat */
		KatMask = XPLMI_SECURE_AES_ENC_KAT_MASK;
		Status = XSecure_AesEncryptKat(AesInstPtr);
		break;
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_RSA_PUB_ENC_KAT):
		/**   - XSecure_RsaPublicEncryptKat */
		KatMask = XPLMI_SECURE_RSA_KAT_MASK;
		Status = XSecure_RsaPublicEncryptKat();
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DEC_KAT):
		/**   - XSecure_RsaPrivateDecryptKat */
		KatMask = XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK;
		Status = XSecure_RsaPrivateDecryptKat();
		break;
#endif
#ifndef PLM_ECDSA_EXCLUDE
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_VERIFY_KAT):
		/**   - XSecure_EllipticVerifySignKat */
		KatMask = XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK;
		Status = XSecure_EllipticVerifySignKat((XSecure_EllipticCrvClass)Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_SIGN_GEN_KAT):
		/**   - XSecure_EllipticSignGenerateKat */
		KatMask = XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK;
		Status = XSecure_EllipticSignGenerateKat((XSecure_EllipticCrvClass)Pload[1U]);
		break;
#endif
#endif
	case XSECURE_API(XSECURE_API_SHA3_KAT):
		/**   - XSecure_Sha3Kat */
		/** - SHA IPI event handling */
		Status = XSecure_IpiEventHandling(Cmd, XPLMI_SHA3_CORE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		KatMask = XPLMI_SECURE_SHA3_KAT_MASK;
		Status = XSecure_Sha3Kat(XSecureSha3InstPtr);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSECURE_REDUNDANT_IMPL(XSecure_UpdateKatMask, Status, KatMask);

END:
#ifndef PLM_SECURE_EXCLUDE
	if (AesInstPtr->AesState == XSECURE_AES_INITIALIZED) {
		SStatus = XSecure_MakeResFree(XPLMI_AES_CORE);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
#endif
	if (XSecureSha3InstPtr->ShaState == XSECURE_SHA_INITIALIZED) {
		SStatus = XSecure_MakeResFree(XPLMI_SHA3_CORE);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the KAT mask based on the KAT status.
 *
 * @param	Status	is the status returned by the KAT API
 * @param	KatMask	is the KAT mask to set or clear
 *
 ******************************************************************************/
 static void XSecure_UpdateKatMask(volatile int Status, u32 KatMask)
 {
	 volatile int StatusTmp = Status;
	 XSECURE_REDUNDANT_IMPL(XPlmi_ClearKatMask, KatMask);
	 if ((Status == XST_SUCCESS) && (StatusTmp == XST_SUCCESS)) {
		 XPlmi_SetKatMask(KatMask);
	 }
 }

/** @} */
