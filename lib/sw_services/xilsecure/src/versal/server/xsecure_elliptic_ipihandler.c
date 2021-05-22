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
*      am    05/22/2021 Resolved MISRA C violation
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
#define XSECURE_ELLIPTIC_P521_ALIGN_BYTES	(2U)

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
	XSecure_EllipticKey Key;
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

	if (DstAddrHigh != 0x0U) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for DstAddr\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Key.Qx = (u8 *)(UINTPTR)DstAddr;
	if (CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Key.Qy = (u8 *)(UINTPTR)(DstAddr + Size);
	}
	else {
		Key.Qy = (u8 *)(UINTPTR)(DstAddr + Size +
					XSECURE_ELLIPTIC_P521_ALIGN_BYTES);
	}

	if (SrcAddrHigh != 0x0U) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for SrcAddr\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_EllipticGenerateKey((XSecure_EllipticCrvTyp)CurveType,
			(u8 *)(UINTPTR)SrcAddr,
			&Key);
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
	XSecure_EllipticSign Sign;

	Status = XPlmi_DmaXfr(SrcAddr, (UINTPTR)&EcdsaParams, sizeof(EcdsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (DstAddrHigh != 0x0U) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for DstAddr\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Sign.SignR = (u8 *)(UINTPTR)DstAddr;
	if (EcdsaParams.CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Sign.SignS = (u8 *)(UINTPTR)(DstAddr + EcdsaParams.Size);
	}
	else if (EcdsaParams.CurveType == (u32)XSECURE_ECC_NIST_P521){
		Sign.SignS = (u8 *)(UINTPTR)(DstAddr +
				XSECURE_ELLIPTIC_P521_ALIGN_BYTES +
				EcdsaParams.Size);
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (((EcdsaParams.HashAddr >> 32) != 0x0U) ||
		((EcdsaParams.PrivKeyAddr >> 32) != 0x0U) ||
		((EcdsaParams.EPrivKeyAddr >> 32) != 0x0U)) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for "
				"XSecure_EllipticGenSign\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_EllipticGenerateSignature(
			(XSecure_EllipticCrvTyp)EcdsaParams.CurveType,
			(u8 *)(UINTPTR)EcdsaParams.HashAddr,
			EcdsaParams.Size,
			(u8 *)(UINTPTR)EcdsaParams.PrivKeyAddr,
			(u8 *)(UINTPTR)EcdsaParams.EPrivKeyAddr,
			&Sign);

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
	XSecure_EllipticKey Key;
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

	if (SrcAddrHigh != 0x0U) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for SrcAddr\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Key.Qx = (u8 *)(UINTPTR)SrcAddr;

	if (CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Key.Qy = (u8 *)(UINTPTR)(SrcAddr + Size);
	}
	else {
		Key.Qy = (u8 *)(UINTPTR)(SrcAddr + Size +
					XSECURE_ELLIPTIC_P521_ALIGN_BYTES);
	}

	Status = XSecure_EllipticValidateKey((XSecure_EllipticCrvTyp)CurveType,
						&Key);

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
	XSecure_EllipticKey Key;
	XSecure_EllipticSign Sign;

	Status = XPlmi_DmaXfr(Addr, (UINTPTR)&EcdsaParams,
			sizeof(EcdsaParams),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (((EcdsaParams.PubKeyAddr >> 32) != 0x0U) ||
			((EcdsaParams.SignAddr >> 32) != 0x0U) ||
			((EcdsaParams.HashAddr >> 32) != 0x0U)) {
		XSecure_Printf(XSECURE_DEBUG_GENERAL,
				"64 bit address not supported for "
				"XSecure_EllipticVerifySign\r\n");
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Key.Qx = (u8 *)(UINTPTR)EcdsaParams.PubKeyAddr;
	if (EcdsaParams.CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Key.Qy = (u8 *)(UINTPTR)(EcdsaParams.PubKeyAddr +
				EcdsaParams.Size);
	}
	else if (EcdsaParams.CurveType == (u32)XSECURE_ECC_NIST_P521) {
		Key.Qy = (u8 *)(UINTPTR)(EcdsaParams.PubKeyAddr +
				EcdsaParams.Size +
				XSECURE_ELLIPTIC_P521_ALIGN_BYTES);
	}
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Sign.SignR = (u8 *)(UINTPTR)EcdsaParams.SignAddr;

	if (EcdsaParams.CurveType == (u32)XSECURE_ECC_NIST_P384) {
		Sign.SignS = (u8 *)(UINTPTR)(EcdsaParams.SignAddr +
				EcdsaParams.Size);
	}
	else {
		Sign.SignS =
			(u8 *)(UINTPTR)(EcdsaParams.SignAddr +
					XSECURE_ELLIPTIC_P521_ALIGN_BYTES +
					EcdsaParams.Size);
	}

	Status = XSecure_EllipticVerifySign(
			(XSecure_EllipticCrvTyp)EcdsaParams.CurveType,
			(u8 *)(UINTPTR)EcdsaParams.HashAddr,
			EcdsaParams.Size,
			&Key,
			&Sign);

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
