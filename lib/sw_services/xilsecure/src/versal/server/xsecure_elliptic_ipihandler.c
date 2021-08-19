/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_elliptic_ipihandler.c
*
* This file contains the xilsecure elliptic IPI handlers implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   03/23/2021 Initial release
* 4.6  gm    07/16/2021 Added support for 64-bit address
*      rb    08/11/2021 Fix compilation warnings
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_dma.h"
#include "xsecure_defs.h"
#include "xsecure_elliptic.h"
#include "xsecure_elliptic_ipihandler.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes *****************************/
static int XSecure_EllipticGenKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh, u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_EllipticGenSign(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh);
static int XSecure_EllipticValidatePubKey(u32 CurveType,
	u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_EllipticVerifySignature(u32 SrcAddrLow, u32 SrcAddrHigh);
static int XSecure_EllipticExecuteKat(u32 CurveType);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function calls respective IPI handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XSecure_EllipticIpiHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	u32 *Pload = Cmd->Payload;

	switch (Cmd->CmdId & XSECURE_API_ID_MASK) {
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_KEY):
		Status = XSecure_EllipticGenKey(Pload[0], Pload[1], Pload[2],
				Pload[3], Pload[4]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_GENERATE_SIGN):
		Status = XSecure_EllipticGenSign(Pload[0], Pload[1], Pload[2],
				Pload[3]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_VALIDATE_KEY):
		Status = XSecure_EllipticValidatePubKey(Pload[0], Pload[1],
				Pload[2]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_VERIFY_SIGN):
		Status = XSecure_EllipticVerifySignature(Pload[0], Pload[1]);
		break;
	case XSECURE_API(XSECURE_API_ELLIPTIC_KAT):
		Status = XSecure_EllipticExecuteKat(Pload[0]);
		break;
	default:
		XSecure_Printf(XSECURE_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticGenerateKey
 * 		server API
 *
 * @param	CurveType	- Is a type of elliptic curve
 * 		SrcAddrLow	- Lower 32 bit address of the
 * 				static private key
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				static private key
 * 		DstAddrLow	- Lower 32 bit address of the public key
 * 				to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the public key
 * 				to be stored
 *
 * @return	- XST_SUCCESS - If the elliptic key generation is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticGenKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh, u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	u32 OffSet = 0U;

	if (CurveType == (u32)XSECURE_ECC_NIST_P384) {
		OffSet = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (CurveType == (u32)XSECURE_ECC_NIST_P521) {
		OffSet = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecure_EllipticKeyAddr KeyAddr = {DstAddr, (DstAddr + (u64)OffSet)};
	Status = XSecure_EllipticGenerateKey_64Bit(
			(XSecure_EllipticCrvTyp)CurveType,
			SrcAddr, (XSecure_EllipticKeyAddr *) &KeyAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticGenerateSignature
 * 		server API
 *
 * @param
 * 		SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_EllipticSignGenParams structure
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_EllipticSignGenParams structure
 * 		DstAddrLow	- Lower 32 bit address of the signature
 * 				to be stored
 * 		DstAddrHigh	- Higher 32 bit address of the signature
 * 				to be stored
 *
 * @return	- XST_SUCCESS - If the elliptic sign generation is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticGenSign(u32 SrcAddrLow, u32 SrcAddrHigh,
	u32 DstAddrLow, u32 DstAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u64 DstAddr = ((u64)DstAddrHigh << 32U) | (u64)DstAddrLow;
	XSecure_EllipticSignGenParams EcdsaParams;

	Status = XPlmi_DmaXfr(SrcAddr, (UINTPTR)&EcdsaParams, sizeof(EcdsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_EllipticHashData HashInfo = {EcdsaParams.HashAddr,
			EcdsaParams.Size};
	XSecure_EllipticSignAddr SignAddr = {DstAddr,
			(DstAddr + (u64)EcdsaParams.Size)};

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
 * @brief       This function handler calls XSecure_EllipticValidateKey
 * 		server API
 *
 * @param	CurveType	- Is a type of elliptic curve
 * 		SrcAddrLow	- Lower 32 bit address of the public key
 * 		SrcAddrHigh	- Higher 32 bit address of the public key
 *
 * @return	- XST_SUCCESS - If the elliptic key validation is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticValidatePubKey(u32 CurveType, u32 SrcAddrLow,
	u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 SrcAddr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	u32 Size;

	if (CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Size = XSECURE_ECC_P384_SIZE_IN_BYTES;
	}
	else if (CurveType == (u32)XSECURE_ECC_NIST_P521) {
		Size = XSECURE_ECC_P521_SIZE_IN_BYTES;
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	XSecure_EllipticKeyAddr KeyAddr = {SrcAddr, (SrcAddr + (u64)Size)};
	Status = XSecure_EllipticValidateKey_64Bit(
			(XSecure_EllipticCrvTyp)CurveType,
			(XSecure_EllipticKeyAddr *) &KeyAddr);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticVerifySign
 * 		server API
 *
 * @param
 * 		SrcAddrLow	- Lower 32 bit address of the
 * 				XSecure_EllipticSignVerifyParams structure
 * 		SrcAddrHigh	- Higher 32 bit address of the
 * 				XSecure_EllipticSignVerifyParams structure
 *
 * @return	- XST_SUCCESS - If the elliptic sign verify is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticVerifySignature(u32 SrcAddrLow, u32 SrcAddrHigh)
{
	volatile int Status = XST_FAILURE;
	u64 Addr = ((u64)SrcAddrHigh << 32U) | (u64)SrcAddrLow;
	XSecure_EllipticSignVerifyParams EcdsaParams;

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&EcdsaParams,
			sizeof(EcdsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSecure_EllipticKeyAddr KeyAddr = {EcdsaParams.PubKeyAddr,
			(EcdsaParams.PubKeyAddr + (u64)EcdsaParams.Size)};
	XSecure_EllipticHashData HashInfo = {EcdsaParams.HashAddr,
			EcdsaParams.Size};
	XSecure_EllipticSignAddr SignAddr = {EcdsaParams.SignAddr,
			(EcdsaParams.SignAddr + (u64)EcdsaParams.Size)};
	Status = XSecure_EllipticVerifySign_64Bit(
			(XSecure_EllipticCrvTyp)EcdsaParams.CurveType,
			(XSecure_EllipticHashData *) &HashInfo,
			(XSecure_EllipticKeyAddr *) &KeyAddr,
			(XSecure_EllipticSignAddr *) &SignAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function handler calls XSecure_EllipticKat
 * 		server API
 *
 * @param	CurveType	- Is a type of elliptic curve
 *
 * @return	- XST_SUCCESS - If the elliptic KAT is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XSecure_EllipticExecuteKat(u32 CurveType)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_EllipticKat(CurveType);

	return Status;
}
