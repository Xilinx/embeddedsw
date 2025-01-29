/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic_ipihandler.c
*
* This file contains the Xilsecure elliptic IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/2021 Initial release
* 4.6  gm    07/16/2021 Added support for 64-bit address
*      rb    08/11/2021 Fix compilation warnings
* 4.7  kpt   03/18/2022 Replaced XPlmi_Dmaxfr with XPlmi_MemCpy64
* 5.0  kpt   07/24/2022 Moved XSecure_EllipticExecuteKat in to xsecure_kat_plat_ipihandler.c
* 5.1  yog   05/03/2023 Fixed MISRA C violation of Rule 10.3
* 5.2  yog   06/07/2023 Added support for P-256 Curve
*      yog   08/07/2023 Removed trng init call in XSecure_EllipticIpiHandler API
*                       since trng is being initialised in server API's
*      am    08/17/2023 Replaced curve size check with XSecure_EllipticGetCrvSize() call
*      ng    02/12/2024 optimised u8 vars to u32 for size reduction
* 5.3  kpt   03/22/2024 Fix MISRA C violation of Rule 10.3
*      kpt   03/22/2024 Fixed Branch past initialization
*      ss    04/05/2024 Fixed doxygen warnings
* 5.4  yog   04/29/2024 Fixed doxygen warnings.
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_ecdsa_server_apis XilSecure ECDSA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_elliptic.h"
#include "xsecure_elliptic_ipihandler.h"
#include "xstatus.h"
#include "xplmi.h"
#include "xplmi_tamper.h"
#include "xsecure_error.h"
#include "xsecure_kat.h"
#include "xsecure_init.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_EllipticGenKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh, u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_EllipticGenSign(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_EllipticValidatePubKey(u32 CurveType,
	u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_EllipticVerifySignature(u32 SrcAddrLow, u32 SrcAddrHigh);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd	is pointer to the command structure
 *
 * @return
 *		 - XST_SUCCESS  If the handler execution is successful
 *		 - XST_INVALID_PARAM  If Cmd is NULL or API ID is invalid
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XSecure_EllipticIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = NULL;

	if ((Cmd == NULL) || (Cmd->Payload == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Pload = Cmd->Payload;

	/** Call the respective API handler according to API ID */
	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
		/**   - XSecure_EllipticGenKey */
		Status = XSecure_EllipticGenKey(Pload[0], Pload[1], Pload[2],
				Pload[3], Pload[4]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
		/**    - XSecure_EllipticGenSign */
		Status = XSecure_EllipticGenSign(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
		/**    - XSecure_EllipticValidatePubKey */
		Status = XSecure_EllipticValidatePubKey(Pload[0], Pload[1],
				Pload[2]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
		/**    - XSecure_EllipticVerifySignature */
		Status = XSecure_EllipticVerifySignature(Pload[0], Pload[1]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params from the IPI
 * 		command and calls XSecure_EllipticGenerateKey server API.
 *
 * @param	CurveType	Is a type of elliptic curve
 * @param	SrcAddrLow	Lower 32 bit address of the static private key
 * @param	SrcAddrHigh	Higher 32 bit address of the static private key
 * @param	DstAddrLow	Lower 32 bit address of the public key to be stored
 * @param	DstAddrHigh	Higher 32 bit address of the public key to be stored
 *
 * @return
 *		 - XST_SUCCESS  If the elliptic key generation is successful
 *		 - XSECURE_ELLIPTIC_NON_SUPPORTED_CRV  If curve size is 0
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticGenKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh, u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	u32 Size = 0U;
	XSecure_EllipticKeyAddr KeyAddr;

	Size = XSecure_EllipticGetCrvSize((XSecure_EllipticCrvTyp)CurveType);
	if (Size == 0U) {
		Status = (int)XSECURE_ELLIPTIC_NON_SUPPORTED_CRV;
		goto END;
	}

	KeyAddr.Qx = DstAddr;
	KeyAddr.Qy = (DstAddr + (u64)Size);

	Status = XSecure_EllipticGenerateKey_64Bit(
			(XSecure_EllipticCrvTyp)CurveType,
			SrcAddr, (XSecure_EllipticKeyAddr *) &KeyAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (XPlmi_IsCryptoKatEn() == (u32)TRUE) {
		XPlmi_ClearKatMask(XPLMI_SECURE_ECC_PWCT_KAT_MASK);
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XSECURE_KAT_MAJOR_ERROR, Status, StatusTmp,
			XSecure_EllipticPwct, (XSecure_EllipticCrvTyp)CurveType,
			SrcAddr, (XSecure_EllipticKeyAddr *)&KeyAddr);
		if ((Status == XST_SUCCESS) && (StatusTmp == XST_SUCCESS)) {
			XPlmi_SetKatMask(XPLMI_SECURE_ECC_PWCT_KAT_MASK);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params of the
 * 		XSECURE_API_ELLIPTIC_GENERATE_SIGN IPI command and calls
 * 		XSecure_EllipticGenerateSignature server API.
 *
 * @param	SrcAddrLow	Lower 32 bit address of the
 * 				XSecure_EllipticSignGenParams structure
 * @param	SrcAddrHigh	Higher 32 bit address of the
 * 				XSecure_EllipticSignGenParams structure
 * @param	DstAddrLow	Lower 32 bit address of the signature to be stored
 * @param	DstAddrHigh	Higher 32 bit address of the signature to be stored
 *
 * @return
 *		 - XST_SUCCESS  If the elliptic sign generation is successful
 *		 - XST_INVALID_PARAM  If curve size is 0
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticGenSign(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_EllipticSignGenParams EcdsaParams;
	XSecure_EllipticHashData HashInfo;
	XSecure_EllipticSignAddr SignAddr;
	u32 Size = 0U;

	Status = XPlmi_MemCpy64((UINTPTR)&EcdsaParams, SrcAddr, sizeof(EcdsaParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Size = XSecure_EllipticGetCrvSize((XSecure_EllipticCrvTyp)EcdsaParams.CurveType);
	if (Size == 0U) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HashInfo.Addr = EcdsaParams.HashAddr;
	HashInfo.Len = EcdsaParams.Size;

	SignAddr.SignR = DstAddr;
	SignAddr.SignS = (DstAddr + (u64)Size);

	Status = XST_FAILURE;
	Status = XSecure_EllipticGenerateSignature_64Bit(
			(XSecure_EllipticCrvTyp)EcdsaParams.CurveType,
			(XSecure_EllipticHashData *) &HashInfo,
			EcdsaParams.PrivKeyAddr,
			EcdsaParams.EPrivKeyAddr,
			(XSecure_EllipticSignAddr *) &SignAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params of
 * 		XSECURE_API_ELLIPTIC_VALIDATE_KEY IPI command and calls
 * 		XSecure_EllipticValidateKey server API.
 *
 * @param	CurveType	Is a type of elliptic curve
 * @param	SrcAddrLow	Lower 32 bit address of the public key
 * @param	SrcAddrHigh	Higher 32 bit address of the public key
 *
 * @return
 *		 - XST_SUCCESS  If the elliptic key validation is successful
 *		 - XST_INVALID_PARAM  If curve size is 0
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticValidatePubKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_EllipticKeyAddr KeyAddr;
	u32 Size = 0U;

	Size = XSecure_EllipticGetCrvSize((XSecure_EllipticCrvTyp)CurveType);
	if (Size == 0U) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	KeyAddr.Qx = SrcAddr;
	KeyAddr.Qy = (SrcAddr + (u64)Size);

	Status = XSecure_EllipticValidateKey_64Bit(
			(XSecure_EllipticCrvTyp)CurveType,
			(XSecure_EllipticKeyAddr *) &KeyAddr);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handler extracts the payload params with respect
 * 		to XSECURE_API_ELLIPTIC_VERIFY_SIGN IPI command and calls
 * 		XSecure_EllipticGenerateKey server API.
 *
 *
 * @param	SrcAddrLow	Lower 32 bit address of the
 * 				XSecure_EllipticSignVerifyParams structure
 * @param	SrcAddrHigh	Higher 32 bit address of the
 * 				XSecure_EllipticSignVerifyParams structure
 *
 * @return
 *		 - XST_SUCCESS  If the elliptic sign verify is successful
 *		 - XST_INVALID_PARAM  If curve size is 0
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticVerifySignature(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_EllipticSignVerifyParams EcdsaParams;
	XSecure_EllipticKeyAddr KeyAddr;
	XSecure_EllipticHashData HashInfo;
	XSecure_EllipticSignAddr SignAddr;
	u32 Size = 0U;

	Status = XPlmi_MemCpy64((UINTPTR)&EcdsaParams, Addr, sizeof(EcdsaParams));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Size = XSecure_EllipticGetCrvSize((XSecure_EllipticCrvTyp)EcdsaParams.CurveType);
	if (Size == 0U) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	KeyAddr.Qx = EcdsaParams.PubKeyAddr;
	KeyAddr.Qy = (EcdsaParams.PubKeyAddr + (u64)Size);

	HashInfo.Addr = EcdsaParams.HashAddr;
	HashInfo.Len = EcdsaParams.Size;

	SignAddr.SignR = EcdsaParams.SignAddr;
	SignAddr.SignS = (EcdsaParams.SignAddr + (u64)Size);

	Status = XST_FAILURE;
	/** Verify the signature for the provided hash, key and curve type */
	Status = XSecure_EllipticVerifySign_64Bit(
			(XSecure_EllipticCrvTyp)EcdsaParams.CurveType,
			(XSecure_EllipticHashData *) &HashInfo,
			(XSecure_EllipticKeyAddr *) &KeyAddr,
			(XSecure_EllipticSignAddr *) &SignAddr);

END:
	return Status;
}

#endif
/** @} */
