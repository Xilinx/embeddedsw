/******************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/

static int XSecure_UpdateCryptoMask(XSecure_CryptoStatusOp CryptoOp, u32 CryptoMask, u32 CryptoVal);
#ifndef PLM_RSA_EXCLUDE
static int XSecure_GetRsaPublicKeyIpi(u32 PubKeyAddrHigh, u32 PubKeyAddrLow);
static int XSecure_KeyUnwrapIpi(u32 KeyWrapAddrHigh, u32 KeyWrapAddrLow);
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
	u32 *Pload = Cmd->Payload;
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
 * @param   PubKeyAddrHigh   - Higher address of the RsaPubKeyAddr structure.
 * @param   PubKeyAddrLow    - Lower address of the RsaPubKeyAddr structure.
 *
 * @return
	-	XST_SUCCESS - On Success
 *	-	ErrorCode - On failure
 *
 ******************************************************************************/
 int XSecure_GetRsaPublicKeyIpi(u32 PubKeyAddrHigh, u32 PubKeyAddrLow)
{
	volatile int Status = XST_FAILURE;
	u64 PubKeyAddr = ((u64)PubKeyAddrHigh << 32U) | (u64)PubKeyAddrLow;
	XSecure_RsaKey *RsaPubKey =  XSecure_GetRsaPublicKey();
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
 * @param   KeyWrapAddrHigh   - Higher address of the XSecure_KeyWrapData structure.
 * @param   KeyWrapAddrLow    - Lower address of the XSecure_KeyWrapData structure.
 *
 * @return
 *	-	XST_SUCCESS - On Success
 *	-	ErrorCode - On failure
 *
 ******************************************************************************/
static int XSecure_KeyUnwrapIpi(u32 KeyWrapAddrHigh, u32 KeyWrapAddrLow)
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

#endif
