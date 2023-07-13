/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_mgf.c
* This file contains code related to mask generate function.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/23/23 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_mgf.h"
#include "xsecure_sha.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_I2OSP_LIMIT		(256U) /**< Integer to octet stream primitive limit */

/************************** Function Prototypes ******************************/

static int XSecure_ShaStart(void *InstancePtr);
static int XSecure_ShaUpdate(void *InstancePtr, u64 InDataAddr, u32 Size);
static int XSecure_ShaFinish(void *InstancePtr, u64 HashAddr);
static int XSecure_ShaDigest(void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr);

/************************** Variable Definitions *****************************/

static XSecure_HashAlgInfo XSecure_MgfHashList[] =
{{XSECURE_SHA3_384, XSECURE_SHA3_HASH_LENGTH_IN_BYTES, XSecure_ShaStart, XSecure_ShaUpdate, XSecure_ShaFinish, XSecure_ShaDigest}};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function starts the sha engine
 *
 * @param	InstancePtr is the pointer to SHA algorithm used.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Errorcode on failure.
 *
 ******************************************************************************/
static int XSecure_ShaStart(void *InstancePtr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_Sha3Start((XSecure_Sha3 *)InstancePtr);

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates the data to SHA algorithm
 *
 * @param	InstancePtr is the pointer to SHA algorithm used.
 * @param	InDataAddr is the input data address.
 * @param	Size is the size of the input data.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Errorcode on failure.
 *
 ******************************************************************************/
static int XSecure_ShaUpdate(void *InstancePtr, u64 InDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_Sha3Update64Bit((XSecure_Sha3 *)InstancePtr, InDataAddr, Size);

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates SHA engine with final data which includes
 * 		SHA padding and reads final hash on complete data
 *
 * @param	InstancePtr is the pointer to SHA algorithm used.
 * @param	HashAddr is the address where the hash is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Errorcode on failure.
 *
 ******************************************************************************/
static int XSecure_ShaFinish(void *InstancePtr, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_Sha3Finish((XSecure_Sha3 *)InstancePtr, (XSecure_Sha3Hash *)(UINTPTR)HashAddr);

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates and calculates sha digest on data.
 *
 * @param	InstancePtr is the pointer to SHA algorithm used.
 * @param	InDataAddr is the input data address.
 * @param	Size is the size of the input data.
 * @param	HashAddr is the address where the hash is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Errorcode on failure.
 *
 ******************************************************************************/
static int XSecure_ShaDigest(void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_Sha3Digest((XSecure_Sha3 *)InstancePtr, (UINTPTR)InputDataAddr, Size,
			(XSecure_Sha3Hash *)(UINTPTR)HashAddr);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a
 * 		specified length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 * @return
 * 			- None
 *
 ******************************************************************************/
static inline void XSecure_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XSECURE_I2OSP_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns hash function list of user specified
 *              hash algorithm.
 *
 * @param	Shatype holds the XSecure_ShaType value of choosen sha algo.
 *
 * @return
 *              - Pointer to XSecure_HashAlgInfo instance
 *              - NULL otherwise
 *
 ******************************************************************************/
XSecure_HashAlgInfo *XSecure_GetHashInstance(XSecure_ShaType Shatype)
{
	XSecure_HashAlgInfo *MgfHashPtr = NULL;

	if (Shatype == XSECURE_SHA3_384) {
		MgfHashPtr =  &XSecure_MgfHashList[Shatype];
	}

	return MgfHashPtr;
}

/*****************************************************************************/
/**
 * @brief	This function takes an input of variable length and
 *          a desired output length as input, and provides fixed output
 *          mask using cryptographic hash function. i.e. Mask Generation Function(MGF).
 *
 * @param	ShaType is of type XSecure_ShaType.
 * @param	InstancePtr is pointer to the sha instance.
 * @param	MgfInput is pointer to XSecure_MgfInput instance.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Errorcode on failure.
 *
 ******************************************************************************/
int XSecure_MaskGenFunc(XSecure_ShaType ShaType, void *InstancePtr, XSecure_MgfInput *MgfInput)
{
	volatile int Status = XST_FAILURE;
	u32 Counter = 0U;
	u8 Hash[XSECURE_SHA3_HASH_LENGTH_IN_BYTES];
	u8 Bytes[XSECURE_WORD_SIZE] = {0U};
	u32 Size = 0U;
	u32 NoOfIterations = 0U;
	u8 *OutputPtr;
	XSecure_HashAlgInfo *HashFunList = XSecure_GetHashInstance(ShaType);

	if (InstancePtr == NULL || MgfInput == NULL) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if ((MgfInput->Seed == NULL) || (MgfInput->Output == NULL) || (MgfInput->OutputLen == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (HashFunList == NULL) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	Size = MgfInput->OutputLen;
	if (Size > HashFunList->HashLen) {
		Size = HashFunList->HashLen;
	}

	OutputPtr = MgfInput->Output;
	NoOfIterations = (MgfInput->OutputLen + HashFunList->HashLen - 1U) / HashFunList->HashLen;
	while (Counter < NoOfIterations) {
		XSecure_I2Osp(Counter, XSECURE_WORD_SIZE, Bytes);

		Status = HashFunList->ShaStart(InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaUpdate(InstancePtr, (UINTPTR)MgfInput->Seed, MgfInput->SeedLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaUpdate(InstancePtr, (UINTPTR)Bytes, XSECURE_WORD_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaFinish(InstancePtr, (u64)(UINTPTR)Hash);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = XST_FAILURE;
		Status = Xil_SMemCpy(OutputPtr, Size, Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		OutputPtr = &OutputPtr[Size];
		Counter = Counter + 1U;
		if (Counter == (NoOfIterations - 1U)) {
			Size = MgfInput->OutputLen - ((NoOfIterations - 1U) * HashFunList->HashLen);
		}
	}

END:
	return Status;
}
