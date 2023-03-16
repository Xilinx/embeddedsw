/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_hmac.c
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_hmac.h"
#include "xsecure_sha_hw.h"
#include "xsecure_error.h"
#include "xsecure_utils.h"
#include "xil_util.h"

/************************** Function Prototypes ******************************/
static int XSecure_PreProcessKey(XSecure_Hmac *InstancePtr,
					u64 KeyAddr, u32 KeyLen, u64 KeyOut);
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
 * @brief
 * This function initializes the HMAC instance
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	Sha3InstancePtr	is the pointer to the XSecure_Sha3 instance.
 * @param	KeyAddr holds the address of HMAC key.
 * @param	KeyLen variable holds the length of the key.
 *
 * @return	XST_SUCCESS if initialization was successful.
 * 		Error Code on failure.
 *
 ******************************************************************************/
int XSecure_HmacInit(XSecure_Hmac *InstancePtr,
		XSecure_Sha3 *Sha3InstancePtr, u64 KeyAddr, u32 KeyLen)
{
	volatile int Status = XST_FAILURE;
	u8 K0[XSECURE_SHA3_BLOCK_LEN];

	if ((InstancePtr == NULL) || (KeyLen == 0x0U)) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}
	if ((Sha3InstancePtr == NULL) ||
			(Sha3InstancePtr->Sha3State == XSECURE_SHA3_UNINITIALIZED)) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}

	InstancePtr->Sha3InstPtr = Sha3InstancePtr;

	Status = XSecure_PreProcessKey(InstancePtr, KeyAddr, KeyLen, (UINTPTR)K0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Calculate K0 xor ipad  */
	XSecure_HmacXor((const u32 *)K0, XSECURE_HMAC_IPAD_VALUE,
					(u32 *)InstancePtr->IPadRes);

	/* Calculate K0 Xor Opad */
	XSecure_HmacXor((const u32 *)K0, XSECURE_HMAC_OPAD_VALUE,
					(u32 *)InstancePtr->OPadRes);

	Status = XSecure_Sha3Start(Sha3InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstancePtr, (UINTPTR)InstancePtr->IPadRes,
					XSECURE_SHA3_BLOCK_LEN);
END:
	if (Status != XST_SUCCESS) {
		(void)memset((void *)InstancePtr->IPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		(void)memset((void *)InstancePtr->OPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		/* Set SHA under reset */
		XSecure_SetReset(Sha3InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);
	}
	(void)memset((void *)K0, (u32)0U, XSECURE_SHA3_BLOCK_LEN);
RET:

	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function updates the data to be authenticated, which can be called
 * repeatedly with chunks of the message to be authenticated
 * (len bytes at data).
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	DataAddr holds the address of data to be updated.
 * @param	Len variable holds the length of data.
 *
 * @return	XST_SUCCESS if initialization was successful.
 *		Error Code on failure.
 *
 ******************************************************************************/
int XSecure_HmacUpdate(XSecure_Hmac *InstancePtr, u64 DataAddr, u32 Len)
{
	volatile int Status = XST_FAILURE;

	if ((InstancePtr == NULL) || (InstancePtr->Sha3InstPtr == NULL) ||
			(Len == 0x0U)) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_Sha3Update(InstancePtr->Sha3InstPtr,
		(UINTPTR)DataAddr, Len);

	if (Status != XST_SUCCESS) {
		(void)memset((void *)InstancePtr->IPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		(void)memset((void *)InstancePtr->OPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		/* Set SHA under reset */
		XSecure_SetReset(InstancePtr->Sha3InstPtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function calculates the final HMAC
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	Hmac is the pointer of 48 bits which holds the resultant HMAC.
 *
 * @return	XST_SUCCESS if initialization was successful.
 * 		Error Code on failure.
 *
 * @note	Though HMAC can be truncated, as the truncation of data may
 *		lead to security implications, this function always fills the
 *		HMAC pointer with 48 bytes of data.
 *
 ******************************************************************************/
int XSecure_HmacFinal(XSecure_Hmac *InstancePtr, XSecure_HmacRes *Hmac)
{
	volatile int Status = XST_FAILURE;
	XSecure_Sha3 *Sha3InstancePtr;
	u8 IntHash[XSECURE_HASH_SIZE_IN_BYTES];

	if ((InstancePtr == NULL) || (InstancePtr->Sha3InstPtr == NULL) ||
			(Hmac == NULL)) {
		Status = XSECURE_HMAC_INVALID_PARAM;
		goto RET;
	}

	Sha3InstancePtr = InstancePtr->Sha3InstPtr;

	/* Calculate final hash on IPAD || MSG */
	Status = XSecure_Sha3Finish(InstancePtr->Sha3InstPtr,
				(XSecure_Sha3Hash *)IntHash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Start(Sha3InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstancePtr, (UINTPTR)InstancePtr->OPadRes,
			XSECURE_SHA3_BLOCK_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Update(Sha3InstancePtr, (UINTPTR)IntHash,
			XSECURE_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XSecure_Sha3Finish(InstancePtr->Sha3InstPtr,
			(XSecure_Sha3Hash *)Hmac->Hash);
END:
	if (Status != XST_SUCCESS) {
		XSecure_SetReset(InstancePtr->Sha3InstPtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
	}
	(void)memset((void *)InstancePtr->IPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
	(void)memset((void *)InstancePtr->OPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
	(void)memset((void *)IntHash, 0U, XSECURE_HASH_SIZE_IN_BYTES);
RET:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function pre process the key to SHA3 block length and the final key is
 * been stored into the K0 array.
 *
 * @param	InstancePtr	is the pointer to the XSecure_Hmac instance
 * @param	KeyAddr is the variable which holds the address of key for HMAC
 * @param	KeyLen variable holds the length of the key
 * @param	KeyOut is the variable which holds the output key buffer's address
 *
 * @return	XST_SUCCESS if initialization was successful.
 *		Error Code on failure.
 *
 ******************************************************************************/
static int XSecure_PreProcessKey(XSecure_Hmac *InstancePtr,
		u64 KeyAddr, u32 KeyLen, u64 KeyOut)
{
	volatile int Status = XST_FAILURE;
	volatile u8 Index;
	u32 *K0Ptr = (u32 *)(UINTPTR)KeyOut;
	u8 *K0 = (u8 *)K0Ptr;
	u32 *InKey = (u32 *)(UINTPTR)KeyAddr;

	/* KDF uses this path, for UDS 48 bytes key in DICE */
	if (KeyLen < XSECURE_SHA3_BLOCK_LEN) {
		/*
		 * if Key provided is less than SHA 3 block length
		 * Append Zeros to the key provided
		 */
		for (Index = 0U; Index < (KeyLen / XSECURE_HMAC_WORD_LEN);
					 Index++) {
			K0Ptr[Index] = InKey[Index];
		}
		(void)memset((void *)&K0[KeyLen], (u32)0,
			XSECURE_SHA3_BLOCK_LEN - KeyLen);

	}
	else if (KeyLen > XSECURE_SHA3_BLOCK_LEN) {
		/*
		 * If provided key length is greater than SHA3 block length
		 * Calculate hash on key and append with zero to
		 * make K0 to the length of SHA3 Block length
		 */
		 Status = XSecure_Sha3Digest(InstancePtr->Sha3InstPtr,
			(UINTPTR)KeyAddr, KeyLen, (XSecure_Sha3Hash *)K0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		(void)memset((void *)&K0[XSECURE_HASH_SIZE_IN_BYTES], (u32)0,
			XSECURE_SHA3_BLOCK_LEN - XSECURE_HASH_SIZE_IN_BYTES);

	}
	else {
		/* if Key provided is of SHA 3 block length */
		(void)memcpy((void *)K0, (const void *)(UINTPTR)KeyAddr,
			XSECURE_SHA3_BLOCK_LEN);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief
 * This function performs XOR operation on provided data of SHA3 block length
 * with constant provided on whole bytes of data and result is been updated.
 *
 * @param	Data is the pointer which holds data to be XORed.
 * @param	Value with which XOR operation to be performed.
 * @param	Result is the pointer of SHA3 block length array which is been
 * 			updated with the result.
 *
 * @return	None.
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

