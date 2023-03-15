/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 4.7   kpt  11/29/21 Replaced Xil_DCacheFlushRange with
*                     XSecure_DCacheFlushRange
*       kpt  01/13/21 Allocated CDO structure's in shared memory set by the
*                     user
*       am   03/08/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 5.0   kpt  07/24/22 Moved XSecure_EllipticKat in to xsecure_katclient.c
* 5.1   am   03/09/23 Replaced xsecure payload lengths with xmailbox payload lengths
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_ellipticclient.h"

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to generate elliptic signature
 * 		for a given hash and curve type
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CrvType		- Type of elliptic curve
 * @param	HashAddr	- Address of the hash for which sign has to be
 * 				generated
 * @param	Size		- Length of the hash in bytes
 * @param	PrivKeyAddr	- Address of the static private key
 * @param	EPrivKeyAddr	- Address of the Ephemeral private key
 * @param	SignAddr	- Address of the signature buffer
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_BAD_RAND_NUM - When Bad random number
 *					used for sign generation
 *	-	XSECURE_ELLIPTIC_GEN_SIGN_INCORRECT_HASH_LEN - Incorrect hash
 *					length for sign generation
 *	-	XST_FAILURE - On failure
 *
 ******************************************************************************/
int XSecure_EllipticGenerateSign(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 HashAddr,
			u32 Size, u64 PrivKeyAddr, u64 EPrivKeyAddr, u64 SignAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_EllipticSignGenParams *EcdsaParams = NULL;
	u64 Buffer;
	u32 MemSize;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EcdsaParams);

	if ((EcdsaParams == NULL) || (MemSize < sizeof(XSecure_EllipticSignGenParams))) {
		goto END;
	}

	EcdsaParams->CurveType = CurveType;
	EcdsaParams->HashAddr = HashAddr;
	EcdsaParams->Size = Size;
	EcdsaParams->PrivKeyAddr = PrivKeyAddr;
	EcdsaParams->EPrivKeyAddr = EPrivKeyAddr;
	Buffer = (u64)(UINTPTR)EcdsaParams;

	XSecure_DCacheFlushRange(EcdsaParams, sizeof(XSecure_EllipticSignGenParams));

		/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_ELLIPTIC_GENERATE_SIGN);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32);
	Payload[3U] = (u32)(SignAddr);
	Payload[4U] = (u32)(SignAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to generate Public Key for a
 * 		given curve type
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CrvType		- Type of elliptic curve
 * @param	PrivKeyAddr	- Address of the static private key
 * @param	PubKeyAddr	- Address of the buffer where public key to be
 * 				stored.
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_NON_SUPPORTED_CRV - When elliptic Curve
 *						is not supported
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_GEN_KEY_ERR - Error in generating Public key
 *
 ******************************************************************************/
int XSecure_EllipticGenerateKey(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 PrivKeyAddr,
						u64 PubKeyAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_6U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_ELLIPTIC_GENERATE_KEY);
	Payload[1U] = CurveType;
	Payload[2U] = (u32)PrivKeyAddr;
	Payload[3U] = (u32)(PrivKeyAddr >> 32);
	Payload[4U] = (u32)PubKeyAddr;
	Payload[5U] = (u32)(PubKeyAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to validate the public key for
 * 		a given curve type
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CrvType		- Type of elliptic curve
 * @param	KeyAddr		- Address of the pubilc key to be validated
 *
 * @return
 *	-	XST_SUCCESS 			- On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM	- On invalid argument
 *	-	XSECURE_ELLIPTIC_KEY_ZERO	- When Public key is zero
 *	-	XSECURE_ELLIPTIC_KEY_WRONG_ORDER- Wrong order of Public key
 *	-	XSECURE_ELLIPTIC_KEY_NOT_ON_CRV	- When Key is not found on
 *						the curve
 *	-	XST_FAILURE			- On failure
 *
 *****************************************************************************/
int XSecure_EllipticValidateKey(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 KeyAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_ELLIPTIC_VALIDATE_KEY);
	Payload[1U] = CurveType;
	Payload[2U] = (u32)KeyAddr;
	Payload[3U] = (u32)(KeyAddr >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to verify signature for a
 * 		given hash, key and curve type
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	CrvType		- Type of elliptic curve
 * @param	HashAddr	- Address of the hash for which sign has to be
 * 				generated
 * @param	Size		- Length of the hash in bytes
 * @param	PubKeyAddr	- Address of the pubilc key
 * @param	SignAddr	- Address of the signature buffer

 * @return
 *	-	XST_SUCCESS - On success
 *	-	XSECURE_ELLIPTIC_INVALID_PARAM - On invalid argument
 *	-	XSECURE_ELLIPTIC_BAD_SIGN - When signature provided
 *					for verification is bad
 *	-	XSECURE_ELLIPTIC_VER_SIGN_INCORRECT_HASH_LEN - Incorrect hash
 *						length for sign verification
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ZERO - R set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ZERO - S set to zero
 *	-	XSECURE_ELLIPTIC_VER_SIGN_R_ORDER_ERROR - R is not within ECC
 *							order
 *	-	XSECURE_ELLIPTIC_VER_SIGN_S_ORDER_ERROR - S is not within ECC
 *							order
 *	-	XST_FAILURE - On failure
 *
 *****************************************************************************/
int XSecure_EllipticVerifySign(XSecure_ClientInstance *InstancePtr, u32 CurveType, u64 HashAddr,
						u32 Size, u64 PubKeyAddr, u64 SignAddr)
{
	volatile int Status = XST_FAILURE;
	XSecure_EllipticSignVerifyParams *EcdsaParams = NULL;
	u64 Buffer;
	u32 MemSize;
	volatile u32 Payload[XMAILBOX_PAYLOAD_LEN_3U] = {0U};

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}


	MemSize = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EcdsaParams);

	if ((EcdsaParams == NULL) || (MemSize < sizeof(XSecure_EllipticSignVerifyParams))) {
		goto END;
	}

	EcdsaParams->CurveType = CurveType;
	EcdsaParams->HashAddr = HashAddr;
	EcdsaParams->Size = Size;
	EcdsaParams->PubKeyAddr = PubKeyAddr;
	EcdsaParams->SignAddr = SignAddr;
	Buffer = (u64)(UINTPTR)EcdsaParams;

	XSecure_DCacheFlushRange(EcdsaParams, sizeof(XSecure_EllipticSignVerifyParams));

	/* Fill IPI Payload */
	Payload[0U] = HEADER(0U, XSECURE_API_ELLIPTIC_VERIFY_SIGN);
	Payload[1U] = (u32)Buffer;
	Payload[2U] = (u32)(Buffer >> 32);

	Status = XSecure_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
