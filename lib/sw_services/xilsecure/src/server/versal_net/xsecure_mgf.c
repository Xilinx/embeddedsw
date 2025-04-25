/******************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
*       dd      10/11/23 MISRA-C violation Rule 12.1 fixed
*       dd      10/11/23 MISRA-C violation Rule 8.13 fixed
* 5.3   kpt     12/13/23 Added SHA384 MGF support
*       kpt     03/22/24 Fixed MISRA-C violation Rule 17.7
*       kal     07/24/24 Code refactoring for versal_2ve_2vm
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_mgf_server_apis XilSecure MGF Server APIs
* @{
*/
/***************************** Include Files *********************************/

#include "xsecure_mgf.h"
#include "xsecure_sha384.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_I2OSP_LIMIT		(256U) /**< Integer to octet stream primitive limit */

/************************** Function Prototypes ******************************/

static int XSecure_ShaMgfStart(XSecure_ShaMode Shatype, void *InstancePtr);
static int XSecure_ShaMgfUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size);
static int XSecure_ShaMgfFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr);
static int XSecure_ShaMgfDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr);

/************************** Variable Definitions *****************************/

static XSecure_HashAlgInfo XSecure_MgfHashList[] =
{{XSECURE_SHA3_384, XSECURE_HASH_SIZE_IN_BYTES, XSecure_ShaMgfStart, XSecure_ShaMgfUpdate, XSecure_ShaMgfFinish, XSecure_ShaMgfDigest},
 {XSECURE_SHA2_384, XSECURE_HASH_SIZE_IN_BYTES, XSecure_ShaMgfStart, XSecure_ShaMgfUpdate, XSecure_ShaMgfFinish, XSecure_ShaMgfDigest}};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function starts the sha engine
 *
 * @param	Shatype		holds the XSecure_ShaMode value of chosen sha algo.
 * @param	InstancePtr	is the pointer to SHA algorithm used.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfStart(XSecure_ShaMode Shatype, void *InstancePtr)
{
	volatile int Status = XST_FAILURE;

	if (Shatype == XSECURE_SHA3_384) {
		Status = XSecure_Sha3Start((XSecure_Sha3 *)InstancePtr);
	}
	else {
		XSecure_Sha384Start();
		Status = XST_SUCCESS;
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates the data to SHA algorithm
 *
 * @param	Shatype		holds the XSecure_ShaMode value of chosen sha algo.
 * @param	InstancePtr	is the pointer to SHA algorithm used.
 * @param	InDataAddr	is the input data address.
 * @param	Size		is the size of the input data.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;

	if (Shatype == XSECURE_SHA3_384) {
		Status = XSecure_Sha3Update64Bit((XSecure_Sha3 *)InstancePtr, InDataAddr, Size);
	}
	else {
		Status = XSecure_Sha384Update((u8*)(UINTPTR)InDataAddr, Size);
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates SHA engine with final data which includes
 * 		SHA padding and reads final hash on complete data
 *
 * @param	Shatype		holds the XSecure_ShaMode value of chosen sha algo.
 * @param	InstancePtr	is the pointer to SHA algorithm used.
 * @param	HashAddr	is the address where the hash is stored.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	if (Shatype == XSECURE_SHA3_384) {
		Status = XSecure_Sha3Finish((XSecure_Sha3 *)InstancePtr, (XSecure_Sha3Hash *)(UINTPTR)HashAddr);
	}
	else {
		Status = XSecure_Sha384Finish((XSecure_Sha2Hash*)(UINTPTR)HashAddr);
	}

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function updates and calculates sha digest on data.
 *
 * @param	Shatype		holds the XSecure_ShaMode value of chosen sha algo.
 * @param	InstancePtr	is the pointer to SHA algorithm used.
 * @param	InDataAddr	is the input data address.
 * @param	Size		is the size of the input data.
 * @param	HashAddr	is the address where the hash is stored.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	if (Shatype == XSECURE_SHA3_384) {
		Status = XSecure_Sha3Digest((XSecure_Sha3 *)InstancePtr, (UINTPTR)InputDataAddr, Size,
				(XSecure_Sha3Hash *)(UINTPTR)HashAddr);
	}
	else {
		Status = XSecure_Sha384Digest((u8*)(UINTPTR)InputDataAddr, Size, (u8*)(UINTPTR)HashAddr);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a
 * 		specified length.
 *
 * @param	Integer	is the variable in which input should be provided.
 * @param	Size	holds the required size.
 * @param	Convert	is a pointer in which output will be updated.
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
 *		hash algorithm.
 *
 * @param	Shatype	holds the XSecure_ShaMode value of chosen sha algo.
 *
 * @return
 *		 - Pointer to XSecure_HashAlgInfo instance
 *		 - NULL  Otherwise
 *
 ******************************************************************************/
XSecure_HashAlgInfo *XSecure_GetHashInstance(XSecure_ShaMode Shatype)
{
	XSecure_HashAlgInfo *MgfHashPtr = NULL;

	if ((Shatype == XSECURE_SHA3_384) || (Shatype == XSECURE_SHA2_384)) {
		MgfHashPtr =  &XSecure_MgfHashList[Shatype];
	}

	return MgfHashPtr;
}

/*****************************************************************************/
/**
 * @brief	This function takes an input of variable length and
 *		a desired output length as input, and provides fixed output
 *		mask using cryptographic hash function. i.e. Mask Generation Function(MGF).
 *
 * @param	ShaType		is of type XSecure_ShaMode.
 * @param	InstancePtr	is pointer to the sha instance.
 * @param	MgfInput	is pointer to XSecure_MgfInput instance.
 *
 * @return
 *		 - XST_SUCCESS  On success.
 *		 - XST_INVALID_PARAM  If any input parameter is invalid
 *		 - XST_FAILURE  On failure.
 *
 ******************************************************************************/
int XSecure_MaskGenFunc(XSecure_ShaMode ShaType, void *InstancePtr, XSecure_MgfInput *MgfInput)
{
	volatile int Status = XST_FAILURE;
	u32 Counter = 0U;
	u8 Hash[XSECURE_SHA3_HASH_LENGTH_IN_BYTES];
	u8 Bytes[XSECURE_WORD_SIZE] = {0U};
	u32 Size = 0U;
	u32 NoOfIterations = 0U;
	u8 *OutputPtr;
	const XSecure_HashAlgInfo *HashFunList = XSecure_GetHashInstance(ShaType);

	if (MgfInput == NULL) {
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

		Status = HashFunList->ShaStart(HashFunList->Shatype, InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaUpdate(HashFunList->Shatype, InstancePtr, (UINTPTR)MgfInput->Seed, MgfInput->SeedLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaUpdate(HashFunList->Shatype, InstancePtr, (UINTPTR)Bytes, XSECURE_WORD_SIZE);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = HashFunList->ShaFinish(HashFunList->Shatype, InstancePtr, (u64)(UINTPTR)Hash);
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
