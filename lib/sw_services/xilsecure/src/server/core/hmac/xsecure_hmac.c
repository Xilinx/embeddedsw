/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/hmac/xsecure_hmac.c
*
* This file contains the implementation of the HMAC APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 5.0   vns 05/30/22 Initial release
*       kpt 07/24/22 Moved XSecure_HmacKat into xsecure_kat_plat.c
* 5.2   kpt 07/27/23 Fix security review comments
*       dd  10/11/23 MISRA-C violation Rule 10.3 fixed
*       dd  10/11/23 MISRA-C violation Rule 8.13 fixed
* 5.3   kpt 11/24/23 Replace Xil_SMemSet with Xil_SecureZeroize
*       har 01/16/24 Corrected length of IntHash for zeroization
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_hmac_apis XilSecure HMAC APIs
* @{
*/
/***************************** Include Files *********************************/

#include "xsecure_hmac.h"
#include "xsecure_sha_hw.h"
#include "xsecure_error.h"
#include "xsecure_utils.h"
#include "xil_sutil.h"

/************************** Function Prototypes ******************************/
static int XSecure_PreProcessKey(XSecure_Hmac *InstancePtr,
					u64 KeyAddr, u32 KeyLen, u32 KeyOut, u32 KeyOutLen);
static void XSecure_HmacXor(const u32 *Data, const u8 Value, u32 *Result);

/************************** Variable Definitions *****************************/

/************************** Macros Definitions *****************************/
#define XSECURE_HMAC_8BIT_SHIFT		(8U) /**<8 Bit shift for HMAC*/
#define XSECURE_HMAC_16BIT_SHIFT	(16U) /**<16 Bit shift for HMAC*/
#define XSECURE_HMAC_24BIT_SHIFT	(24U) /**<24 Bit shift for HMAC*/
#define XSECURE_HMAC_WORD_LEN		(4U) /**<HMAC word length*/

#define XSECURE_HMAC_IPAD_VALUE	(0x36U) /**<HMAC IPAD value*/
#define XSECURE_HMAC_OPAD_VALUE	(0x5CU) /**<HMAC OPAD value*/
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * @brief	This function initializes the HMAC instance
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	Sha3InstancePtr	is the pointer to the XSecure_Sha3 instance.
 * @param	KeyAddr		holds the address of HMAC key.
 * @param	KeyLen		variable holds the length of the key.
 *
 * @return
 *		 - XST_SUCCESS - If initialization was successful.
 *		 - XSECURE_HMAC_INVALID_PARAM - If any input parameter is invalid.
 *		 - XST_FAILURE - On failure.
 *
 ******************************************************************************/
int XSecure_HmacInit(XSecure_Hmac *InstancePtr,
		XSecure_Sha3 *Sha3InstancePtr, u64 KeyAddr, u32 KeyLen)
{
	volatile int Status = XST_FAILURE;
	u8 K0[XSECURE_SHA3_BLOCK_LEN];

	if ((InstancePtr == NULL) || (KeyLen == 0x0U)) {
		Status = (int)XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}
	if ((Sha3InstancePtr == NULL) ||
			(Sha3InstancePtr->ShaState == XSECURE_SHA_UNINITIALIZED)) {
		Status = (int)XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}

	InstancePtr->Sha3InstPtr = Sha3InstancePtr;

	Status = XSecure_PreProcessKey(InstancePtr, KeyAddr, KeyLen, (u32)(UINTPTR)K0, XSECURE_SHA3_BLOCK_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/** - Calculate K0 xor ipad */
	XSecure_HmacXor((const u32 *)K0, XSECURE_HMAC_IPAD_VALUE,
					(u32 *)InstancePtr->IPadRes);

	/** - Calculate K0 Xor Opad */
	XSecure_HmacXor((const u32 *)K0, XSECURE_HMAC_OPAD_VALUE,
					(u32 *)InstancePtr->OPadRes);

	Status = XSecure_ShaStart(Sha3InstancePtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)InstancePtr->IPadRes,
					XSECURE_SHA3_BLOCK_LEN);
END:
	if (Status != XST_SUCCESS) {
		Status |= Xil_SMemSet((void *)InstancePtr->IPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
								XSECURE_SHA3_BLOCK_LEN);
		Status |= Xil_SMemSet((void *)InstancePtr->OPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
								XSECURE_SHA3_BLOCK_LEN);
		/** - Set SHA under reset */
		XSecure_SetReset(Sha3InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);
	}
	Status |= Xil_SecureZeroize(K0, XSECURE_SHA3_BLOCK_LEN);

RET:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the data to be authenticated, which can be called
 * 		repeatedly with chunks of the message to be authenticated
 * 		(len bytes at data).
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	DataAddr	holds the address of data to be updated.
 * @param	Len		variable holds the length of data.
 *
 * @return
 *		 - XST_SUCCESS - If initialization was successful.
 *		 - XSECURE_HMAC_INVALID_PARAM - If any input parameter is invalid.
 *		 - XST_FAILURE - On failure.
 *
 ******************************************************************************/
int XSecure_HmacUpdate(XSecure_Hmac *InstancePtr, u64 DataAddr, u32 Len)
{
	volatile int Status = XST_FAILURE;

	if ((InstancePtr == NULL) || (InstancePtr->Sha3InstPtr == NULL) ||
			(Len == 0x0U)) {
		Status = (int)XSECURE_HMAC_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ShaUpdate(InstancePtr->Sha3InstPtr,
		(UINTPTR)DataAddr, Len);

	if (Status != XST_SUCCESS) {
		Status |= Xil_SMemSet((void *)InstancePtr->IPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
								XSECURE_SHA3_BLOCK_LEN);
		Status |= Xil_SMemSet((void *)InstancePtr->OPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
								XSECURE_SHA3_BLOCK_LEN);
		/** - Set SHA under reset */
		XSecure_SetReset(InstancePtr->Sha3InstPtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function calculates the final HMAC
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	Hmac		is the pointer of 48 bits which holds the resultant HMAC.
 *
 * @return
 *		 - XST_SUCCESS - If initialization was successful.
 *		 - XSECURE_HMAC_INVALID_PARAM - If any input parameter is invalid.
 *		 - XST_FAILURE - On failure.
 *
 * @note	Though HMAC can be truncated, as the truncation of data may
 *		lead to security implications, this function always fills the
 *		HMAC pointer with 48 bytes of data.
 *
 ******************************************************************************/
int XSecure_HmacFinal(XSecure_Hmac *InstancePtr, XSecure_HmacRes *Hmac)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	XSecure_Sha3 *Sha3InstancePtr;
	XSecure_Sha384Hash IntHash;

	if ((InstancePtr == NULL) || (InstancePtr->Sha3InstPtr == NULL) ||
			(Hmac == NULL)) {
		Status = (int)XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}

	Sha3InstancePtr = InstancePtr->Sha3InstPtr;

	/** - Calculate final hash on IPAD || MSG */
	Status = XSecure_ShaFinish(InstancePtr->Sha3InstPtr,
				(u64)(UINTPTR)&IntHash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaStart(Sha3InstancePtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)InstancePtr->OPadRes,
			XSECURE_SHA3_BLOCK_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_ShaUpdate(Sha3InstancePtr, (u64)(UINTPTR)IntHash.Hash,
				   XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	Status = XSecure_ShaFinish(InstancePtr->Sha3InstPtr,
			(u64)(UINTPTR)Hmac->Hash, sizeof(Hmac->Hash));
	RetStatus = Status;
END:
	if ((Status != XST_SUCCESS) && (RetStatus != XST_SUCCESS)) {
		RetStatus = Status;
	}

	XSecure_SetReset(InstancePtr->Sha3InstPtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);

	RetStatus |= Xil_SMemSet((void *)InstancePtr->IPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
								XSECURE_SHA3_BLOCK_LEN);
	RetStatus |= Xil_SMemSet((void *)InstancePtr->OPadRes, XSECURE_SHA3_BLOCK_LEN, 0U,
							XSECURE_SHA3_BLOCK_LEN);
	RetStatus |= Xil_SecureZeroize(IntHash.Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

RET:
	return RetStatus;
}

/*****************************************************************************/
/**
 *
 * @brief	This function pre process the key to SHA3 block length and the final key is
 * 		been stored into the K0 array.
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	KeyAddr		is the variable which holds the address of key for HMAC
 * @param	KeyLen		variable holds the length of the key
 * @param	KeyOut		is the variable which holds the output key buffer's address
 * @param	KeyOutLen	is the variable which specifies the output key length based on SHA mode
 *
 * @return
 *		 - XST_SUCCESS - If initialization was successful.
 *		 - XST_FAILURE - On failure.
 *
 ******************************************************************************/
static int XSecure_PreProcessKey(XSecure_Hmac *InstancePtr,
		u64 KeyAddr, u32 KeyLen, u32 KeyOut, u32 KeyOutLen)
{
	volatile int Status = XST_FAILURE;
	u8 *K0 = (u8 *)(UINTPTR)KeyOut;

	/** - KDF uses this path, for UDS 48 bytes key in DICE */
	if (KeyLen < KeyOutLen) {
		/*
		 * if Key provided is less than SHA 3 block length
		 * Append Zeros to the key provided
		 */
		XSecure_MemCpy64((u64)(UINTPTR)K0, KeyAddr, KeyLen);

		Status = Xil_SMemSet((void *)&K0[KeyLen], KeyOutLen - KeyLen, 0U,
							  KeyOutLen - KeyLen);
	}
	else if (KeyLen > KeyOutLen) {
		/*
		 * If provided key length is greater than SHA3 block length
		 * Calculate hash on key and append with zero to
		 * make K0 to the length of SHA3 Block length
		 */
		Status = XSecure_ShaDigest(InstancePtr->Sha3InstPtr, XSECURE_SHA3_384,
			KeyAddr, KeyLen, (u64)(UINTPTR)K0, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XST_FAILURE;
		Status = Xil_SMemSet((void *)&K0[XSECURE_SHA_384_HASH_SIZE_IN_BYTES],
							  KeyOutLen - XSECURE_SHA_384_HASH_SIZE_IN_BYTES, 0U,
							  KeyOutLen - XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	}
	else {
		/** - if Key provided is of SHA 3 block length */
		XSecure_MemCpy64((u64)(UINTPTR)K0, KeyAddr, KeyOutLen);
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function performs XOR operation on provided data of SHA3 block length
 * 		with constant provided on whole bytes of data and result is been updated.
 *
 * @param	Data	is the pointer which holds data to be XORed.
 * @param	Value	with which XOR operation to be performed.
 * @param	Result	is the pointer of SHA3 block length array which is been
 * 			updated with the result.
 *
 ******************************************************************************/
static void XSecure_HmacXor(const u32 *Data, const u8 Value, u32 *Result)
{
	u32 Index;
	u32 ValData = ((u32)Value << XSECURE_HMAC_24BIT_SHIFT) |
		((u32)Value << XSECURE_HMAC_16BIT_SHIFT) |
		((u32)Value << XSECURE_HMAC_8BIT_SHIFT) | (u32)Value;

	for (Index = 0x0U;
		Index < (XSECURE_SHA3_BLOCK_LEN / XSECURE_HMAC_WORD_LEN);
		Index++) {
		Result[Index] = Data[Index] ^ ValData;
	}
}
/** @} */
