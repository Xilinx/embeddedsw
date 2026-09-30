/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/generic/xsecure_mgf.c
*
* Implements the mask generation function (MGF) for RSA-PSS and OAEP.
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
#include "xsecure_sha.h"
#include "xsecure_error.h"
#include "xil_sutil.h"
#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
#include "xsecure_sha384.h"
#endif

/************************** Constant Definitions *****************************/

#define XSECURE_I2OSP_LIMIT		(256U) /**< Integer to octet-string primitive limit. */

/************************** Function Prototypes ******************************/

/** @cond INTERNAL */
static inline int XSecure_MgfHashCalculate(XSecure_ShaMode ShaType, void *InstancePtr,
	u64 SeedAddr, u32 SeedLen, const u8 *Counter, u8 *Hash);
static inline void XSecure_I2Osp(u32 Integer, u32 Size, u8 *Convert);
#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
static int XSecure_ShaMgfStart(XSecure_ShaMode Shatype, void *InstancePtr);
static int XSecure_ShaMgfUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size);
static int XSecure_ShaMgfFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr);
static int XSecure_ShaMgfDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr);
static int XSecure_ShaMgfSoftStart(XSecure_ShaMode Shatype, void *InstancePtr);
static int XSecure_ShaMgfSoftUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size);
static int XSecure_ShaMgfSoftFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr);
static int XSecure_ShaMgfSoftDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr);
#endif
/** @endcond */

/************************** Variable Definitions *****************************/

#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
/**
 * SHA3-384 uses the SHA engine. SHA2-384 is the software implementation used
 * by Versal Net key wrap.
 */
static XSecure_HashAlgInfo XSecure_MgfHashList[] =
{{XSECURE_SHA3_384, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSecure_ShaMgfStart,
 XSecure_ShaMgfUpdate, XSecure_ShaMgfFinish, XSecure_ShaMgfDigest},
 {XSECURE_SHA2_384, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, XSecure_ShaMgfSoftStart,
 XSecure_ShaMgfSoftUpdate, XSecure_ShaMgfSoftFinish, XSecure_ShaMgfSoftDigest},
};
#else
/** SHA3-384 MGF descriptor. Hashing is direct, so the pointers are unused. */
static XSecure_HashAlgInfo XSecure_MgfHashList[] =
{{XSECURE_SHA3_384, XSECURE_SHA_384_HASH_SIZE_IN_BYTES, NULL, NULL, NULL, NULL}};
#endif

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Returns the hash algorithm table entry for the requested SHA mode.
 *
 * @param	Shatype	SHA algorithm identifier.
 *
 * @return
 *		 - Pointer to an XSecure_HashAlgInfo instance on success.
 *		 - NULL if the algorithm is not supported.
 *
 ******************************************************************************/
 XSecure_HashAlgInfo *XSecure_GetHashInstance(XSecure_ShaMode Shatype)
 {
	 XSecure_HashAlgInfo *MgfHashPtr = NULL;
	 u32 Index = (u32)Shatype;

	 if (Index < (u32)(sizeof(XSecure_MgfHashList) / sizeof(XSecure_MgfHashList[0]))) {
		 MgfHashPtr = &XSecure_MgfHashList[Index];
	 }

	 return MgfHashPtr;
 }

#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
/*****************************************************************************/
/**
 * @brief	Starts the SHA engine for MGF.
 *
 * @param	Shatype		SHA algorithm identifier.
 * @param	InstancePtr	Pointer to the SHA instance.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfStart(XSecure_ShaMode Shatype, void *InstancePtr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ShaStart((XSecure_Sha *)InstancePtr, Shatype);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Updates input data into the SHA engine for MGF.
 *
 * @param	Shatype		SHA algorithm identifier.
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	InDataAddr	Input data address.
 * @param	Size		Input data size in bytes.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;

	(void)Shatype;
	Status = XSecure_ShaUpdate((XSecure_Sha *)InstancePtr, InDataAddr, Size);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Finishes the SHA operation and writes the hash for MGF.
 *
 * @param	Shatype		SHA algorithm identifier.
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	HashAddr	Address where the hash is stored.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	(void)Shatype;
	Status = XSecure_ShaFinish((XSecure_Sha *)InstancePtr, HashAddr,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Computes the SHA digest of the provided data for MGF.
 *
 * @param	Shatype		SHA algorithm identifier.
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	InputDataAddr	Input data address.
 * @param	Size		Input data size in bytes.
 * @param	HashAddr	Address where the hash is stored.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
static int XSecure_ShaMgfDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XSecure_ShaDigest((XSecure_Sha *)InstancePtr, Shatype,
		InputDataAddr, Size, HashAddr, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Starts software SHA2-384 for Versal Net key wrap MGF.
 *
 ******************************************************************************/
static int XSecure_ShaMgfSoftStart(XSecure_ShaMode Shatype, void *InstancePtr)
{
	(void)Shatype;
	(void)InstancePtr;
	XSecure_Sha384Start();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	Updates software SHA2-384 for Versal Net key wrap MGF.
 *
 ******************************************************************************/
static int XSecure_ShaMgfSoftUpdate(XSecure_ShaMode Shatype, void *InstancePtr, u64 InDataAddr, u32 Size)
{
	(void)Shatype;
	(void)InstancePtr;

	return XSecure_Sha384Update((u8 *)(UINTPTR)InDataAddr, Size);
}

/*****************************************************************************/
/**
 * @brief	Finishes software SHA2-384 for Versal Net key wrap MGF.
 *
 ******************************************************************************/
static int XSecure_ShaMgfSoftFinish(XSecure_ShaMode Shatype, void *InstancePtr, u64 HashAddr)
{
	(void)Shatype;
	(void)InstancePtr;

	return XSecure_Sha384Finish((XSecure_Sha2Hash *)(UINTPTR)HashAddr);
}

/*****************************************************************************/
/**
 * @brief	Computes a software SHA2-384 digest for Versal Net key wrap MGF.
 *
 ******************************************************************************/
static int XSecure_ShaMgfSoftDigest(XSecure_ShaMode Shatype, void *InstancePtr, u64 InputDataAddr, u32 Size, u64 HashAddr)
{
	(void)Shatype;
	(void)InstancePtr;

	return XSecure_Sha384Digest((u8 *)(UINTPTR)InputDataAddr, Size,
		(u8 *)(UINTPTR)HashAddr);
}
#endif

/*****************************************************************************/
/**
 * @brief	Hashes seed concatenated with the MGF counter.
 *
 * @param	ShaType		SHA algorithm identifier.
 * @param	InstancePtr	Pointer to the SHA instance.
 * @param	SeedAddr	Seed address.
 * @param	SeedLen		Seed length in bytes.
 * @param	Counter		Four-byte MGF counter.
 * @param	Hash		Buffer that stores the digest.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_INVALID_PARAM if the SHA algorithm is not supported.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
static inline int XSecure_MgfHashCalculate(XSecure_ShaMode ShaType, void *InstancePtr,
	u64 SeedAddr, u32 SeedLen, const u8 *Counter, u8 *Hash)
{
	int Status = XST_FAILURE;
#if defined(VERSAL_NET) && !defined(VERSAL_2VE_2VM)
	const XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(ShaType);

	if ((HashPtr == NULL) || (HashPtr->ShaStart == NULL)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	Status = HashPtr->ShaStart(ShaType, InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = HashPtr->ShaUpdate(ShaType, InstancePtr, SeedAddr, SeedLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = HashPtr->ShaUpdate(ShaType, InstancePtr, (u64)(UINTPTR)Counter,
		XSECURE_WORD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = HashPtr->ShaFinish(ShaType, InstancePtr, (u64)(UINTPTR)Hash);
#else
	if (ShaType != XSECURE_SHA3_384) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_ShaStart((XSecure_Sha *)InstancePtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaUpdate((XSecure_Sha *)InstancePtr, SeedAddr, SeedLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaUpdate((XSecure_Sha *)InstancePtr, (u64)(UINTPTR)Counter,
		XSECURE_WORD_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_ShaFinish((XSecure_Sha *)InstancePtr, (u64)(UINTPTR)Hash,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Converts a non-negative integer to an octet string.
 *
 * @param	Integer	Integer value to convert.
 * @param	Size	Required octet-string length in bytes.
 * @param	Convert	Buffer that stores the octet string.
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
 * @brief	Generates a fixed-length mask from a variable-length seed using MGF1.
 *
 * @param		ShaType		SHA algorithm identifier.
 * @param		InstancePtr	Pointer to the SHA instance.
 * @param	MgfInput	Pointer to the MGF input and output parameters.
 *
 * @return
 *		 - XST_SUCCESS on success.
 *		 - XST_INVALID_PARAM if any input parameter is invalid.
 *		 - XST_FAILURE on failure.
 *
 ******************************************************************************/
int XSecure_MaskGenFunc(XSecure_ShaMode ShaType, void *InstancePtr, XSecure_MgfInput *MgfInput)
{
	int Status = XST_FAILURE;
	u32 Counter = 0U;
	u8 Hash[XSECURE_SHA3_HASH_LENGTH_IN_BYTES];
	u8 Bytes[XSECURE_WORD_SIZE] = {0U};
	u32 Size = 0U;
	u32 NoOfIterations = 0U;
	u32 LastIteration;
	u32 LastSize;
	u8 *OutputPtr;
	const u32 HashLen = XSECURE_SHA_384_HASH_SIZE_IN_BYTES;

	if ((MgfInput == NULL) || (MgfInput->Seed == NULL) ||
		(MgfInput->Output == NULL) || (MgfInput->OutputLen == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	Size = MgfInput->OutputLen;
	if (Size > HashLen) {
		Size = HashLen;
	}

	OutputPtr = MgfInput->Output;
	NoOfIterations = XIL_SCEILDIV(u32, MgfInput->OutputLen, HashLen);
	LastIteration = NoOfIterations - 1U;
	LastSize = MgfInput->OutputLen - (LastIteration * HashLen);
	while (Counter < NoOfIterations) {
		XSecure_I2Osp(Counter, XSECURE_WORD_SIZE, Bytes);

		Status = XSecure_MgfHashCalculate(ShaType, InstancePtr, (UINTPTR)MgfInput->Seed,
			MgfInput->SeedLen, Bytes, Hash);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemCpy(OutputPtr, Size, Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		OutputPtr = &OutputPtr[Size];
		Counter = Counter + 1U;
		if (Counter == LastIteration) {
			Size = LastSize;
		}
	}

END:
	return Status;
}
/** @} */
