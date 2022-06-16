/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
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
* 4.9   vns 05/30/22 Initial release
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
#define XSECURE_HMAC_8BIT_SHIFT		(8U)
#define XSECURE_HMAC_16BIT_SHIFT	(16U)
#define XSECURE_HMAC_24BIT_SHIFT	(24U)
#define XSECURE_HMAC_WORD_LEN		(4U)

#define XSECURE_HMAC_IPAD_VALUE	(0x36U)
#define XSECURE_HMAC_OPAD_VALUE	(0x5CU)
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
	int Status = XST_FAILURE;
	u8 K0[XSECURE_SHA3_BLOCK_LEN];

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Sha3InstancePtr != NULL);
	Xil_AssertNonvoid(KeyLen != 0x0U);

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

	int Status = XST_FAILURE;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3InstPtr != NULL);
	Xil_AssertNonvoid(Len != 0x0U);

	Status = XSecure_Sha3Update(InstancePtr->Sha3InstPtr,
		(UINTPTR)DataAddr, Len);

	if (Status != XST_SUCCESS) {
		(void)memset((void *)InstancePtr->IPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		(void)memset((void *)InstancePtr->OPadRes, 0U, XSECURE_SHA3_BLOCK_LEN);
		/* Set SHA under reset */
		XSecure_SetReset(InstancePtr->Sha3InstPtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
	}

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

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3InstPtr != NULL);
	Xil_AssertNonvoid(Hmac != NULL);

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
 * @param	Key is the variable which holds the key for HMAC
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
	int Status = XST_FAILURE;
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

/*****************************************************************************/
/**
 * This function performs KAT on HMAC (SHA3-384).
 *
 * @param 	none
 *
 * @return	returns the error codes
 *		returns XST_SUCCESS on success
 *
 *****************************************************************************/
int XSecure_HmacKat(XSecure_Sha3 *SecureSha3)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;
	XSecure_HmacRes Hmac = {0U};
	XSecure_Hmac HmacInstance;
	const u8 HmacExpected[XSECURE_HASH_SIZE_IN_BYTES] = {
		0x79U, 0xF0U, 0xFAU, 0x0BU, 0x3BU, 0x7CU, 0xA6U, 0xF6U,
		0xF9U, 0x91U, 0xA7U, 0x87U, 0x4FU, 0x05U, 0x6FU, 0x32U,
		0x26U, 0x40U, 0xD8U, 0x34U, 0xF6U, 0x48U, 0xDCU, 0x33U,
		0x36U, 0xD9U, 0x72U, 0xFBU, 0x0BU, 0xD9U, 0xF4U, 0x36U,
		0x91U, 0xE3U, 0x5BU, 0x12U, 0xC6U, 0x3BU, 0xB9U, 0x82U,
		0xEDU, 0x38U, 0xF5U, 0x18U, 0xB9U, 0xA5U, 0x90U, 0x97U
	};
	const u8 HmacKey[48U] = {
		0x91U, 0xF8U, 0xECU, 0x84U, 0x8DU, 0x6FU, 0x81U, 0x14U,
		0x31U, 0xCBU, 0xDEU, 0xEEU, 0x15U, 0x0BU, 0x93U, 0xAFU,
		0x6FU, 0x67U, 0x8BU, 0xE9U, 0x9CU, 0x90U, 0x3FU, 0x81U,
		0xFCU, 0x38U, 0x29U, 0x55U, 0x03U, 0xD5U, 0x7CU, 0x22U,
		0x8DU, 0xA2U, 0x12U, 0xA6U, 0x72U, 0xE7U, 0xA6U, 0x01U,
		0x5BU, 0x7BU, 0x43U, 0x61U, 0xD4U, 0x87U, 0xFCU, 0xDEU
	};
	const u8 HmacMsg[48U] = {
		0x1AU, 0x40U, 0xE8U, 0x96U, 0xD0U, 0xC0U, 0xC1U, 0x3EU,
		0x78U, 0x24U, 0xC3U, 0xEFU, 0x86U, 0xE0U, 0x23U, 0x55U,
		0xFEU, 0xB6U, 0x29U, 0xEAU, 0x88U, 0x7CU, 0xE4U, 0xD2U,
		0xC7U, 0x1FU, 0x1DU, 0x02U, 0xE7U, 0xE8U, 0x89U, 0xA8U,
		0x75U, 0xFEU, 0x42U, 0xC7U, 0x74U, 0x2DU, 0x78U, 0x22U,
		0xADU, 0xE5U, 0x64U, 0x5CU, 0x46U, 0x86U, 0x7EU, 0x5DU
	};

	Status = XSecure_HmacInit(&HmacInstance, SecureSha3,
				(UINTPTR)HmacKey, sizeof(HmacKey));
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_INIT_ERROR;
		goto END;
	}
	Status = XSecure_HmacUpdate(&HmacInstance, (UINTPTR)HmacMsg,
				sizeof(HmacMsg));
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_UPDATE_ERROR;
		goto END;
	}
	Status = XSecure_HmacFinal(&HmacInstance, &Hmac);
	if (Status != XST_SUCCESS) {
		Status = XSECURE_HMAC_KAT_FINAL_ERROR;
		goto END;
	}
	Status = XSECURE_HMAC_KAT_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (HmacExpected[Index] != Hmac.Hash[Index]) {
			Status = XSECURE_HMAC_KAT_ERROR;
			goto END;
		}
	}

	if(Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = XST_SUCCESS;
	}
END:
	(void)memset((void *)Hmac.Hash, (u32)0,
			XSECURE_HASH_SIZE_IN_BYTES);

	return Status;
}
