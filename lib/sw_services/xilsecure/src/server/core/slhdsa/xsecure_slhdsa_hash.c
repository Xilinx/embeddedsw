/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/slhdsa/xsecure_slhdsa_hash.c
*
* This file consists definitions for SLH-DSA hash operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_slhdsa_server_apis XilSecure SLHDSA Server APIs
* @{
*/
/*************************************** Include Files ********************************************/
#include "xsecure_slhdsa_hash.h"
#include "xsecure_sha.h"
#include "xsecure_utils.h"
#include "xil_io.h"

/************************************ Constant Definitions ****************************************/

/**************************************** Function Prototypes *************************************/

/************************************* Variable Definitions ***************************************/

/************************************* Function Definitions ***************************************/

/**************************************************************************************************/
/**
 *
 * @brief	This function computes the hash of the message as required by the SLH_DSA algorithm.
 *		Gets signature and public key addresses from the instance structure for IPC
 *		compatibility.
 *
 * @return
 *		- XST_SUCCESS if the operation was successful.
 *		- Error code otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaShake256sHashMsg(void)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaStart,
		InstancePtr->ShaInstance,	/* [in] SHA instance pointer */
		XSECURE_SHAKE_256);		/* [in] SHA mode (SHAKE-256) */

	/** - For R <- SIG.getR(), R = SIG[0 : n] - access from instance address */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,	/* [in] SHA instance pointer */
		InstancePtr->SignatureAddr,	/* [in] R value from signature (first n bytes) */
		(u32)InstancePtr->Param->n);	/* [in] Length of R in bytes */

	/** - For PK.seed [0:n] and PK.root [n:2n] - access from instance address */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,	/* [in] SHA instance pointer */
		InstancePtr->PublicKeyAddr,	/* [in] Public key (seed||root) */
		((u32)InstancePtr->Param->n * XSECURE_VALUE_TWO));
						/** - [in] Length of public key in bytes */

	/** - M' <- toByte(0,1) || toByte(|ctx|, 1) || ctx || M, here we send all prefix data before M */
	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)InstancePtr->Data1,	/* [in] Message prefix data */
		InstancePtr->Data1Len);			/* [in] Length of prefix data in bytes */

	if (InstancePtr->DataLen != (u32)XSECURE_ZERO) {
		/** - M, actual data - access from instance address */
		XSECURE_TEMPORAL_CHECK(END,
				Status,
				XSecure_ShaLastUpdate,
				InstancePtr->ShaInstance);	/* [in] SHA instance pointer */

		XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_ShaUpdate,
			InstancePtr->ShaInstance,	/* [in] SHA instance pointer */
			InstancePtr->DataAddr,		/* [in] Actual message data */
			InstancePtr->DataLen);		/* [in] Length of message in bytes */
	}

	XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_ExtendedShaFinish,
			InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
			(u64)(UINTPTR)InstancePtr->Data2,	/* [out] Message digest output buffer */
			InstancePtr->Param->m,			/* [in] Expected digest length in bytes */
			InstancePtr->Param->m);			/* [in] Required digest length in bytes */

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generates a hash value based on the provided public key and input
 *		data, and stores the result in the output buffer. It takes an n-byte message as
 *		input and produces an n-byte output.
 *
 * @param	PublicKeyAddr 64-bit address of the public key buffer.
 * @param	InputAddr 64-bit address of the input data buffer to be hashed.
 * @param	Output Pointer to the buffer where the computed hash will be stored.
 *
 * @return
 *		- XST_SUCCESS if the operation was successful.
 *		- Error code otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaShake256sHashF(const u64 PublicKeyAddr, const u64 InputAddr,
				 u8 * const Output)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaStart,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		XSECURE_SHAKE_256);			/* [in] SHA mode (SHAKE-256) */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		PublicKeyAddr,				/* [in] Public key seed address */
		(u32)InstancePtr->Param->n);		/* [in] Length of public key in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,			/* [in] SHA instance pointer */
		(u64)(UINTPTR)InstancePtr->Addr->AdrsByte,	/* [in] Address structure bytes */
		XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES);		/* [in] Size of address structure */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaLastUpdate,
		InstancePtr->ShaInstance);		/* [in] SHA instance pointer */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		InputAddr,				/* [in] Input data buffer address */
		(u32)InstancePtr->Param->n);		/* [in] Length of input data in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaFinish,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)Output,			/* [out] Hash output buffer */
		(u32)InstancePtr->Param->n);		/* [in] Expected output length in bytes */

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function performs chained SLH_DSA hash operations on the input data, starting
 *		from the specified index and performing the given number of steps.
 *
 * @param	InputAddr 64-bit address of the input data buffer.
 * @param	ChainConfig Pointer to structure containing chain configuration:
 *			- StartIdx: Index in the input buffer from where the chain operation should start
 *			- Steps: Number of hash chain steps to perform
 * @param	PublicKeyAddr 64-bit address of the public key used for verification.
 * @param	Output Pointer to the buffer where the resulting hash will be stored.
 *
 * @return
 *		- XST_SUCCESS if the operation was successful.
 *		- Error code otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaShake256sChain(const u64 InputAddr,
				 const XSecure_SlhdsaChainConfig * const ChainConfig,
				 const u64 PublicKeyAddr, u8 * const Output)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	XSecure_MemCpy64((u64)(UINTPTR)Output, InputAddr, (u32)InstancePtr->Param->n);

	if (ChainConfig->Steps == XSECURE_ZERO) {
		Status = XST_SUCCESS;
		goto END;
	}

	XSecure_SlhdsaSetHashAddress(InstancePtr->Addr, ChainConfig->StartIdx);

	InstancePtr->ShaInstance->DoChainConfig = (u32)TRUE;
	InstancePtr->ShaInstance->StartIdx = ChainConfig->StartIdx;
	InstancePtr->ShaInstance->ChainItr = ChainConfig->Steps;

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaStart,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		XSECURE_SHAKE_256_SLH_DSA_CHAIN);	/* [in] SHA mode (SHAKE-256 SLH-DSA chain) */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		PublicKeyAddr,				/* [in] Public key seed address */
		(u32)InstancePtr->Param->n);		/* [in] Length of public key in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,			/* [in] SHA instance pointer */
		(u64)(UINTPTR)InstancePtr->Addr->AdrsByte,	/* [in] Address structure bytes */
		XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES);		/* [in] Size of address structure */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaLastUpdate,
		InstancePtr->ShaInstance);		/* [in] SHA instance pointer */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		InputAddr,				/* [in] Input data address for chaining */
		(u32)InstancePtr->Param->n);		/* [in] Length of input data in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaFinish,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)Output,			/* [out] Chain output buffer */
		(u32)InstancePtr->Param->n);		/* [in] Expected output length in bytes */

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generates a hash value for the provided input data. The hash is
 *		computed with respect to the given public key and the result is stored in the output
 *		buffer.
 *
 * @param	PublicKeyAddr 64-bit address of the public key used for hashing.
 * @param	Input Pointer to the input data to be hashed.
 * @param	InputLen Length of the input data in bytes.
 * @param	Output Pointer to the buffer where the computed hash will be stored.
 *
 * @return
 *	- XST_SUCCESS if the hash was computed successfully.
 *	- Error code otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaShake256sHashTl(const u64 PublicKeyAddr, const u8 * const Input, u32 InputLen,
				  u8 * const Output)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaStart,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		XSECURE_SHAKE_256);			/* [in] SHA mode (SHAKE-256) */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		PublicKeyAddr,				/* [in] Public key seed address */
		(u32)InstancePtr->Param->n);		/* [in] Length of public key in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,			/* [in] SHA instance pointer */
		(u64)(UINTPTR)InstancePtr->Addr->AdrsByte,	/* [in] Address structure bytes */
		XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES);		/* [in] Size of address structure */

	XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_ShaLastUpdate,
			InstancePtr->ShaInstance);	/* [in] SHA instance pointer */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)Input,			/* [in] Input data buffer */
		InputLen);				/* [in] Length of input data in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaFinish,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)Output,			/* [out] Hash output buffer */
		(u32)InstancePtr->Param->n);		/* [in] Expected output length in bytes */

END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function generates a hash value for the provided input data. The hash is
 *		computed with respect to the given public key and the result is stored in the output
 *		buffer.
 *
 * @param	PublicKeyAddr 64-bit address of the public key used for hashing.
 * @param	Data1Addr 64-bit address of the first input data to be hashed.
 * @param	Data2Addr 64-bit address of the second input data to be hashed.
 * @param	Output Pointer to the buffer where the hash output will be stored.
 *
 * @return
 *		- XST_SUCCESS if the hash was computed successfully.
 *		- Error code otherwise.
 *
 **************************************************************************************************/
int XSecure_SlhdsaShake256sHashH(const u64 PublicKeyAddr, const u64 Data1Addr,
				 const u64 Data2Addr, u8 * const Output)
{
	volatile int Status = XST_FAILURE;
	const XSecure_SlhdsaInstance *InstancePtr = XSecure_SlhdsaGetInstance();

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaStart,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		XSECURE_SHAKE_256);			/* [in] SHA mode (SHAKE-256) */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		PublicKeyAddr,				/* [in] Public key seed address */
		(u32)InstancePtr->Param->n);		/* [in] Length of public key in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,			/* [in] SHA instance pointer */
		(u64)(UINTPTR)InstancePtr->Addr->AdrsByte,	/* [in] Address structure bytes */
		XSECURE_SLH_DSA_ADDR_SIZE_IN_BYTES);		/* [in] Size of address structure */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		Data1Addr,				/* [in] First data input address */
		(u32)InstancePtr->Param->n);		/* [in] Length of first data in bytes */

	XSECURE_TEMPORAL_CHECK(END,
			Status,
			XSecure_ShaLastUpdate,
			InstancePtr->ShaInstance);	/* [in] SHA instance pointer */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaUpdate,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		Data2Addr,				/* [in] Second data input address */
		(u32)InstancePtr->Param->n);		/* [in] Length of second data in bytes */

	XSECURE_TEMPORAL_CHECK(END,
		Status,
		XSecure_ShaFinish,
		InstancePtr->ShaInstance,		/* [in] SHA instance pointer */
		(u64)(UINTPTR)Output,			/* [out] Hash output buffer */
		(u32)InstancePtr->Param->n);		/* [in] Expected output length in bytes */

END:
	return Status;
}
/** @} */
