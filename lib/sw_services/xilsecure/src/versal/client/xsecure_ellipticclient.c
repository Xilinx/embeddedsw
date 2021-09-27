/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_ellipticclient.c
*
* This file contains the implementation of the client interface functions for
* ECDSA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  03/23/21 Initial release
* 4.5   kal  03/23/20 Updated file version to sync with library version
* 4.6   kpt  09/27/21 Fixed compilation warnings
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xsecure_defs.h"
#include "xsecure_ipi.h"
#include "xsecure_ellipticclient.h"

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to generate elliptic signature
 * 		for a given hash and curve type
 *
 * @param	CrvType		- Type of elliptic curve
 * @param	HashAddr	- Address of the hash for which sign has to be
 * 				generated
 * @param	Size		- Length of the hash in bytes
 * @param	PrivKeyAddr	- Address of the static private key
 * @param	EPrivKeyAddr	- Address of the Ephemeral private key
 * @param	SignAddr	- Address of the signature buffer
 *
 * @return 	- XST_SUCCESS - On success
 *		- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *		- XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number
 *					used for sign generation
 *		- XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash
 *						length for sign generation
 *		- XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_EllipticGenerateSign(u32 CurveType, u64 HashAddr, u32 Size,
			u64 PrivKeyAddr, u64 EPrivKeyAddr, u64 SignAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_EllipticSignGenParams EcdsaParams
				__attribute__ ((aligned (64)));
	u64 Buffer;

	EcdsaParams.CurveType = CurveType;
	EcdsaParams.HashAddr = HashAddr;
	EcdsaParams.Size = Size;
	EcdsaParams.PrivKeyAddr = PrivKeyAddr;
	EcdsaParams.EPrivKeyAddr = EPrivKeyAddr;
	Buffer = (u64)(UINTPTR)&EcdsaParams;

	Xil_DCacheFlushRange((INTPTR)Buffer, sizeof(EcdsaParams));

	Status = XSecure_ProcessIpiWithPayload4(XSECURE_API_ELLIPTIC_GENERATE_SIGN,
			(u32)Buffer, (u32)(Buffer >> 32),
			(u32)SignAddr,(u32)(SignAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to generate Public Key for a
 * 		given curve type
 *
 * @param	CrvType		- Type of elliptic curve
 * @param	PrivKeyAddr	- Address of the static private key
 * @param	PubKeyAddr	- Address of the buffer where public key to be
 * 				stored.
 *
 * @return	- XST_SUCCESS - On success
 *		- XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve
 *						is not supported
 *		- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *		- XSECURE_ELLIPTIC_GEN_KEY_ERR - Error in generating Public key
 *
 ******************************************************************************/
int XSecure_EllipticGenerateKey(u32 CurveType, u64 PrivKeyAddr, u64 PubKeyAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload5(XSECURE_API_ELLIPTIC_GENERATE_KEY,
			CurveType, (u32)PrivKeyAddr, (u32)(PrivKeyAddr >> 32),
			(u32)PubKeyAddr, (u32)(PubKeyAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate the public key for
 * 		a given curve type
 *
 * @param	CrvType		- Type of elliptic curve
 * @param	KeyAddr		- Address of the pubilc key to be validated
 *
 * @return	XST_SUCCESS 			- On success
 *		XSECURE_ELLIPTIC_INVALID_PARAM	- On invalid argument
 *		XSECURE_ELLIPTIC_KEY_ZERO	- When Public key is zero
 *		XSECURE_ELLIPTIC_KEY_WRONG_ORDER- Wrong order of Public key
 *		XSECURE_ELLIPTIC_KEY_NOT_ON_CRV	- When Key is not found on
 *						the curve
 *		XST_FAILURE			- On failure
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey(u32 CurveType, u64 KeyAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload3(
			XSECURE_API_ELLIPTIC_VALIDATE_KEY, CurveType,
			(u32)KeyAddr, (u32)(KeyAddr >> 32));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to verify signature for a
 * 		given hash, key and curve type
 *
 * @param	CrvType		- Type of elliptic curve
 * @param	HashAddr	- Address of the hash for which sign has to be
 * 				generated
 * @param	Size		- Length of the hash in bytes
 * @param	PubKeyAddr	- Address of the pubilc key
 * @param	SignAddr	- Address of the signature buffer

 * @return	- XST_SUCCESS - On success
 *		- XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *		- XSECURE_ELLIPTIC_BAD_SIGN - When signature provided
 *						for verification is bad
 *		- XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash
 *						length for sign verification
 *		- XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *		- XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *		- XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC
 *							order
 *		- XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC
 *							order
 *		- XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign(u32 CurveType, u64 HashAddr, u32 Size,
                        u64 PubKeyAddr, u64 SignAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_EllipticSignVerifyParams EcdsaParams
				__attribute__ ((aligned (64)));
	u64 Buffer;

	EcdsaParams.CurveType = CurveType;
	EcdsaParams.HashAddr = HashAddr;
	EcdsaParams.Size = Size;
	EcdsaParams.PubKeyAddr = PubKeyAddr;
	EcdsaParams.SignAddr = SignAddr;
	Buffer = (u64)(UINTPTR)&EcdsaParams;

	Xil_DCacheFlushRange((INTPTR)Buffer, sizeof(EcdsaParams));

	Status = XSecure_ProcessIpiWithPayload2(
			XSECURE_API_ELLIPTIC_VERIFY_SIGN,
			(u32)Buffer, (u32)(Buffer >> 32));

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to perform KAT on ECC core
 *
 * @param	CrvType		- Type of elliptic curve
 *
 * @return	- XST_SUCCESS - On success
 * 		- XSECURE_ELLIPTIC_KAT_KEY_NOTVALID_ERROR - When elliptic key
 * 							is not valid
 *		- XSECURE_ELLIPTIC_KAT_FAILED_ERROR	- When elliptic KAT
 *							fails
 *
 ******************************************************************************/
int XSecure_EllipticKat(u32 CurveType)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ProcessIpiWithPayload1(XSECURE_API_ELLIPTIC_KAT,
			CurveType);

	return Status;
}
