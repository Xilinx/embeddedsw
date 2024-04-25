/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_plat_ipihandler.c
* @addtogroup xsecure_apis XilSecure versal net platform handler APIs
* @{
* @cond xsecure_internal
* This file contains the xilsecure versalnet IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 5.1  kpt   01/13/2023 Initial release
* 5.2  vns   07/06/2023 Separated IPI commands of Update Crypto Status
*      dd    10/11/2023 MISRA-C violation Rule 8.8 fixed
*      dd    10/11/2023 MISRA-C violation Rule 8.13 fixed
* 5.3  kpt   12/13/2023 Added RSA quiet mode support
*      kpt   01/19/2023 Fix passing public exponent
*      ss    04/05/2024 Fixed doxygen warnings
*      vss   04/25/2024 Use XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK
*      			instead of XPLMI_SECURE_RSA_KAT_MASK
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_defs.h"
#include "xil_util.h"
#include "xplmi_plat.h"
#include "xsecure_plat_ipihandler.h"
#include "xsecure_keyunwrap.h"
#include "xsecure_plat_rsa.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/
#ifndef PLM_RSA_EXCLUDE
#define XSECURE_RSA_KEY_ADDRESS		(0xF2008000U) /**< Address to copy RSA input parameters
                                                               * when the provided SyndromeAddr is 64-bit */
#endif

/************************** Function Prototypes *****************************/
static int XSecure_UpdateCryptoMask(XSecure_CryptoStatusOp CryptoOp, u32 CryptoMask, u32 CryptoVal);
#ifndef PLM_RSA_EXCLUDE
static int XSecure_GetRsaPublicKeyIpi(u32 PubKeyAddrLow, u32 PubKeyAddrHigh);
static int XSecure_KeyUnwrapIpi(u32 KeyWrapAddrLow, u32 KeyWrapAddrHigh);
static int XSecure_RsaExpQOperationIpi(u32 RsaParamAddrLow, u32 RsaParamAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
#endif

/*****************************************************************************/
/**
 * @brief   This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return
 *	-	XST_SUCCESS - If the handler execution is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_PlatIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	const u32 *Pload = Cmd->Payload;
	u32 CryptoMask = 0U;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_UPDATE_HNIC_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_HNIC_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_CPM5N_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_CPM5N_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_PCIDE_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_PCIDE_AES_MASK;
		break;
	case XSECURE_API(XSECURE_API_UPDATE_PKI_CRYPTO_STATUS):
		CryptoMask = XPLMI_SECURE_PKI_CRYPTO_MASK;
		break;
#ifndef PLM_RSA_EXCLUDE
	case XSECURE_API(XSECURE_API_GET_KEY_WRAP_RSA_PUBLIC_KEY):
		Status = XSecure_GetRsaPublicKeyIpi(Pload[0U], Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_KEY_UNWRAP):
		Status = XSecure_KeyUnwrapIpi(Pload[0U], Pload[1U]);
		break;
	case XSECURE_API(XSECURE_API_RSA_PRIVATE_DECRYPT):
		Status = XSecure_RsaExpQOperationIpi(Pload[0U], Pload[1U], Pload[2U], Pload[3U]);
		break;
#endif
	default:
		CryptoMask = 0U;
		break;
	}

	if (CryptoMask != 0U) {
		Status = XSecure_UpdateCryptoMask((XSecure_CryptoStatusOp)Pload[0],
					CryptoMask, (Pload[1U] & CryptoMask));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sets or clears Crypto bit mask of given NodeId
 *
 * @param   CryptoOp	   - Operation to set or clear crypto bit mask
 * @param   CryptoMask	   - Crypto Mask of the module
 * @param   CryptoVal      - Crypto value to be updated
 *
 * @return
	-	XST_SUCCESS - If set or clear is successful
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
static int XSecure_UpdateCryptoMask(XSecure_CryptoStatusOp CryptoOp, u32 CryptoMask, u32 CryptoVal)
{
	int Status = XST_FAILURE;

	if ((CryptoOp != XSECURE_CRYPTO_STATUS_SET) && (CryptoOp != XSECURE_CRYPTO_STATUS_CLEAR)) {
		goto END;
	}

	if (CryptoMask != 0U) {
		if (CryptoOp != XSECURE_CRYPTO_STATUS_SET) {
			XPlmi_UpdateCryptoStatus(CryptoMask, ~CryptoVal);
		}
		else {
			XPlmi_UpdateCryptoStatus(CryptoMask, CryptoVal);
		}
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE

/*****************************************************************************/
/**
 * @brief   The function returns the public key of RSA key pair generated for
 *          Key Wrap/Unwrap operation.
 *
 * @param   PubKeyAddrLow    - Lower address of the RsaPubKeyAddr structure.
 * @param   PubKeyAddrHigh   - Higher address of the RsaPubKeyAddr structure.
 *
 * @return
	-	XST_SUCCESS - On Success
 *	-	ErrorCode - On failure
 *
 ******************************************************************************/
 static int XSecure_GetRsaPublicKeyIpi(u32 PubKeyAddrLow, u32 PubKeyAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 PubKeyAddr = ((u64)PubKeyAddrHigh << 32U) | (u64)PubKeyAddrLow;
	const XSecure_RsaPubKey *RsaPubKey =  XSecure_GetRsaPublicKey();
	XSecure_RsaPubKeyAddr RsaPubKeyAddr = {0U};
	u32 PubExp = 0U;

	if (RsaPubKey == NULL) {
		goto END;
	}

	/**< TODO- Need to check whether public key is generated */

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&RsaPubKeyAddr, PubKeyAddr, sizeof(XSecure_RsaPubKeyAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64(RsaPubKeyAddr.ModulusAddr, (u64)(UINTPTR)RsaPubKey->Modulus, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PubExp = Xil_In32((u32)(UINTPTR)RsaPubKey->Exponent);
	XPlmi_Out64(RsaPubKeyAddr.ExponentAddr, PubExp);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function unwraps the input wrapped key and copies to secure shell.
 *
 * @param   KeyWrapAddrLow    - Lower address of the XSecure_KeyWrapData structure.
 * @param   KeyWrapAddrHigh   - Higher address of the XSecure_KeyWrapData structure.
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	ErrorCode - On failure
 *
 ******************************************************************************/
static int XSecure_KeyUnwrapIpi(u32 KeyWrapAddrLow, u32 KeyWrapAddrHigh)
{
	volatile int Status = XST_FAILURE;
	XPmcDma *PmcDmaPtr = XPlmi_GetDmaInstance(0U);
	u64 KeyWrapAddr = ((u64)KeyWrapAddrHigh << 32U) | (u64)KeyWrapAddrLow;
	XSecure_KeyWrapData KeyWrapData = {0U};

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&KeyWrapData, KeyWrapAddr, sizeof(XSecure_KeyWrapData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_KeyUnwrap(&KeyWrapData, PmcDmaPtr);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_RsaInitialize and
 *              XSecure_RsaExp server API.
 *
 * @param   RsaParamAddrLow  - Lower 32 bit address of the XSecure_RsaInParam
 *                             structure
 * @param   RsaParamAddrHigh - Higher 32 bit address of the XSecure_RsaInParam
 *                             structure
 * @param   DstAddrLow 	     - Lower 32 bit address of the output data
 *                             where decrypted data to be stored
 * @param   DstAddrHigh	     - Higher 32 bit address of the output data
 *                             where decrypted data to be stored
 *
 * @return
 *	-	XST_SUCCESS - If the Rsa decryption is successful
 *	-	ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_RsaExpQOperationIpi(u32 RsaParamAddrLow, u32 RsaParamAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 RsaParamAddr = ((u64)RsaParamAddrHigh << 32U) | (u64)RsaParamAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_RsaInParam RsaParams;
	XSecure_RsaExpKeyParam RsaKeyParam;
	XSecure_RsaExpOperationParam *RsaOperationParamPtr = (XSecure_RsaExpOperationParam *)(UINTPTR)XSECURE_RSA_KEY_ADDRESS;
	u8 *OutDataPtr = (u8 *)(UINTPTR)(XSECURE_RSA_KEY_ADDRESS + sizeof(XSecure_RsaExpOperationParam));
	u8 *PubExponentPtr = NULL;
	u8 *P = NULL;
	u8 *Q = NULL;
	u8 *Tot = NULL;
	u32 PubModulus[XSECURE_RSA_4096_SIZE_WORDS];

	if (XPlmi_IsKatRan(XPLMI_SECURE_RSA_PRIVATE_DEC_KAT_MASK) != TRUE) {
		Status = (int)XSECURE_ERR_KAT_NOT_EXECUTED;
		goto END;
	}

	Status = XPlmi_MemCpy64((UINTPTR)&RsaParams, RsaParamAddr, sizeof(XSecure_RsaInParam));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(RsaOperationParamPtr, sizeof(XSecure_RsaExpOperationParam), 0U, sizeof(XSecure_RsaExpOperationParam));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64((UINTPTR)&RsaKeyParam, RsaParams.KeyAddr, sizeof(XSecure_RsaExpKeyParam));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->InData, RsaParams.DataAddr,
		RsaParams.Size);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->Mod, RsaKeyParam.ModAddr, RsaParams.Size);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->Exp, RsaKeyParam.ExpAddr, RsaParams.Size);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (RsaKeyParam.IsPrimeAvail == TRUE) {
		Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->P, RsaKeyParam.PAddr, RsaKeyParam.PSize);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->Q, RsaKeyParam.QAddr, RsaKeyParam.QSize);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		P = RsaOperationParamPtr->P;
		Q = RsaOperationParamPtr->Q;
	}

	if (RsaKeyParam.IsTotAvail == TRUE) {
		Status = XPlmi_MemCpy64((UINTPTR)RsaOperationParamPtr->Tot, RsaKeyParam.TotAddr, RsaParams.Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Tot = RsaOperationParamPtr->Tot;
	}

	if (RsaKeyParam.IsPubExpAvail == TRUE) {
		Status = Xil_SMemSet(&PubModulus, XSECURE_RSA_4096_KEY_SIZE, 0U, XSECURE_RSA_4096_KEY_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		PubModulus[0U] = RsaKeyParam.PubExp;
		PubExponentPtr = (u8*)(UINTPTR)PubModulus;
	}

	Status = XSecure_RsaExp((unsigned char *)(UINTPTR)RsaOperationParamPtr->InData,
		RsaOperationParamPtr->Exp, RsaOperationParamPtr->Mod, P, Q, PubExponentPtr,
		Tot, (int)(RsaParams.Size * 8U), OutDataPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64(DstAddr, (UINTPTR)OutDataPtr, RsaParams.Size);

END:
	return Status;
}
#endif
