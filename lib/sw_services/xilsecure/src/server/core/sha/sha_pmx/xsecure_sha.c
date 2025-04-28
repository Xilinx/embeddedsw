/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sha.c
*
* This file contains the implementation of the interface functions for SHA
* driver. Refer to the header file xsecure_sha.h for more detailed information.
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   har  03/20/20 Initial release
* 4.2   har  03/20/20 Updated file version to sync with library version
*       ana  04/02/20 Added APIs for crypto KAT
*       bvi  04/07/20 Renamed csudma as pmcdma
*       kpt  04/08/20 Updated Sha3 State variable in Xsecure_Sha3Update
*       vns  04/13/20 Improved SHA3Finish to handle if last update is set
*       kpt  04/21/20 Fixed MISRA C violation
* 4.3   ana  06/04/20 Minor enhancement and Updated Sha3 hash variable with
*                     XSecure_Sha3Hash structure
*       kpt  08/15/20 Updated status value before reuse
*                     Replaced magic number with macro's
*                     and removed redundant check
*       kpt  08/15/20 Added validation for input arguments
*            08/27/20 Added 64 bit support for SHA3
*       rpo  09/10/20 Asserts are not compiled by default for secure libraries
*       rpo  09/21/20 New error code added for crypto state mismatch
*       am   09/24/20 Resolved MISRA C violations
*       har  10/12/20 Addressed security review comments
*       am   10/19/20 Resolved MISRA C violations
* 4.5   am   11/24/20 Resolved MISRA C violations
*       bm   01/13/21 Added 64-bit support
*       kpt  05/02/21 Added check to verify the DMA state in
*                     XSecure_Sha3Initialize
* 4.6   har  07/14/21 Fixed doxygen warnings
* 4.7   kpt  12/01/21 Replaced library specific,standard utility functions
*                     with xilinx maintained functions
*       am   03/08/22 Replaced memset() with Xil_SMemSet()
* 5.0   bm   07/06/22 Refactor versal and versal_net code
*       kpt  07/24/22 Moved XSecure_Sha3Kat into xsecure_kat.c
*       dc   08/26/22 Optimization of size, changed type of variables u8 to u32
* 5.2   vss  07/15/23 Removed check for sha3state in  XSecure_Sha3Initialize()
*       ng   07/15/23 Added support for system device tree flow
*	vss  09/11/2023 Fixed Coverity warning EXPRESSION_WITH_MAGIC_NUMBERS and MISRA-C Rule 10.1 violation
*	vss  09/21/23 Fixed doxygen warnings
*	vss  09/11/2023 Fixed MISRA-C Rule 10.3 and 10.4 violation
* 5.4   yog  04/29/24 Fixed doxygen warnings
*       kal  07/24/24 Code refactoring for versal_2ve_2vm
*       pre  03/02/25 Removed data context lost setting
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_sha_server_apis XilSecure SHA Server APIs
* @{
*/
/***************************** Include Files *********************************/
#include "xsecure_error.h"
#include "xsecure_sha_hw.h"
#include "xil_assert.h"
#include "xsecure_sha.h"
#include "xil_sutil.h"
#include "xsecure_plat.h"

/************************** Constant Definitions *****************************/
#define XSECURE_SHA3_START_NIST_PADDING_MASK		(0x06U)
						/**< Nist Start padding masks */
#define XSECURE_SHA3_END_NIST_PADDING_MASK		(0x80U)
						/**< Nist End padding masks */
#define XSECURE_TYPE_PMC_DMA0	(1U) /**< DMA0 type */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief	This inline function waits till SHA3 completes its operation
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  If the SHA3 completes its operation
 *		 - XST_FAILURE  If a timeout has occurred
 *
 ******************************************************************************/
static inline int XSecure_Sha3WaitForDone(const XSecure_Sha3 *InstancePtr)
{
	return (int)Xil_WaitForEvent((InstancePtr)->BaseAddress + XSECURE_SHA3_DONE_OFFSET,
			XSECURE_SHA3_DONE_DONE,
			XSECURE_SHA3_DONE_DONE,
			XSECURE_SHA_TIMEOUT_MAX);
}

/************************** Function Prototypes ******************************/
static int XSecure_Sha3DmaTransfer(const XSecure_Sha3 *InstancePtr,
	u64 InDataAddr, const u32 Size, u8 IsLastUpdate);
static int XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr,
	u64 InDataAddr, const u32 Size, u8 IsLastUpdate);
static int XSecure_Sha3NistPadd(u8 *Dst, u32 MsgLen);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
 * @brief	This function initializes a XSecure_Sha3 structure with the default
 * 		values required for operating the SHA3 cryptographic engine
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	DmaPtr		Pointer to the XPmcDma instance
 *
 * @return
 *		 - XST_SUCCESS  If initialization was successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure
 *
 *****************************************************************************/
int XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XPmcDma* DmaPtr)
{
	int Status = XST_FAILURE;

	Status = XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Validate the input arguments */
	if ((InstancePtr == NULL) || (DmaPtr == NULL) ||
		(DmaPtr->IsReady != (u32)XIL_COMPONENT_IS_READY)) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (DmaPtr->Config.DmaType == XPMCDMA_DMATYPEIS_INVALID) {
		/* Error out if the dma type is invalid. */
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->ShaConfig == NULL) {
		Status = XSecure_Sha3LookupConfig(InstancePtr,
				XSECURE_SHA3_0_DEVICE_ID);
		if (Status != XST_SUCCESS) {
			Status = (int)XSECURE_SHA3_INVALID_PARAM;
			goto END;
		}
	}

	InstancePtr->Sha3Len = 0U;
	InstancePtr->DmaPtr = DmaPtr;
	InstancePtr->IsLastUpdate = FALSE;

	/** Initialize SSS Instance */
	Status = XSecure_SssInitialize(&(InstancePtr->SssInstance));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

	Status = XST_SUCCESS;

END:
	return Status;
}

 /****************************************************************************/
 /**
 * @brief	This function notifies the SHA driver at the end of the SHA data
 *		update and last update includes padding also. This function is
 *		called before XSecure_Sha3Finish to prevent driver adding the
 *		padding.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  If last update can be accepted
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *
 *****************************************************************************/
int XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr)
{
	int Status = XST_FAILURE;

	/** Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
		Status = (int)XSECURE_SHA3_STATE_MISMATCH_ERROR;
		goto END;
	}

	/** Update IsLastUpdate flag to TRUE */
	InstancePtr->IsLastUpdate = TRUE;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Generate padding for the NIST SHA-3
 *
 * @param	Dst	Pointer to location where padding is to be applied
 * @param	MsgLen	Is the length of padding in bytes
 *
 * @return
 *		 - XST_SUCCESS  On successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
static int XSecure_Sha3NistPadd(u8 *Dst, u32 MsgLen)
{
	int Status = XST_FAILURE;

	/* Validates the input arguments */
	if (MsgLen == 0U) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	Status = Xil_SMemSet(Dst, MsgLen, 0U, MsgLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Dst[0] =  XSECURE_SHA3_START_NIST_PADDING_MASK;
	Dst[MsgLen -1U] |= XSECURE_SHA3_END_NIST_PADDING_MASK;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures Secure Stream Switch and starts the
 *		SHA-3 engine
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  On successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_Sha3Start(XSecure_Sha3 *InstancePtr)
{
	int Status = XST_FAILURE;

	/** Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if (InstancePtr->ShaState != XSECURE_SHA_INITIALIZED) {
		Status = (int)XSECURE_SHA3_STATE_MISMATCH_ERROR;
		goto END;
	}

	InstancePtr->Sha3Len = 0U;
	InstancePtr->IsLastUpdate = FALSE;
	InstancePtr->PartialLen = 0U;

	Status = Xil_SMemSet(InstancePtr->PartialData, XSECURE_SHA3_BLOCK_LEN, 0U,
			XSECURE_SHA3_BLOCK_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Reset SHA3 engine. */
	XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);

	/** Start SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_SHA3_START_OFFSET,
			XSECURE_SHA3_START_START);
	InstancePtr->ShaState = XSECURE_SHA_ENGINE_STARTED;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the SHA3 engine with the input data
 * located at a 64-bit address
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr	Starting 64 bit address of the data which has
 *				to be updated to SHA engine
 * @param	Size		Size of the input data in bytes
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  If there is a failure in SSS configuration
 *
 ******************************************************************************/
int XSecure_Sha3Update64Bit(XSecure_Sha3 *InstancePtr, u64 InDataAddr,
						const u32 Size)
{
	int Status = XST_FAILURE;
	u32 DataSize;
	u32 TransferredBytes;

	/* Validate the input arguments */
	if (InstancePtr == NULL) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED) &&
		(InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS)) {
		Status = (int)XSECURE_SHA3_STATE_MISMATCH_ERROR;
		goto END;
	}

	InstancePtr->Sha3Len += Size;
	DataSize = Size;
	TransferredBytes = 0U;

	/*
	 * PMC DMA can transfer Max 0x7FFFFFF no of words(0x1FFFFFFC bytes)
	 * at a time .So if the data sent more than that will be handled
	 * in the next update internally
	 */
	/** Update input data to SHA3 engine */
	while (DataSize > XSECURE_PMC_DMA_MAX_TRANSFER) {
		Status = XSecure_Sha3DataUpdate(InstancePtr,
				(InDataAddr + TransferredBytes),
				XSECURE_PMC_DMA_MAX_TRANSFER, FALSE);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
		DataSize = DataSize - XSECURE_PMC_DMA_MAX_TRANSFER;
		TransferredBytes = TransferredBytes +
			XSECURE_PMC_DMA_MAX_TRANSFER;
	}
	Status = XSecure_Sha3DataUpdate(InstancePtr,
				(InDataAddr + TransferredBytes),
				DataSize, (u8)InstancePtr->IsLastUpdate);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	InstancePtr->ShaState = XSECURE_SHA_UPDATE_IN_PROGRESS;
END_RST:
	if (Status != XST_SUCCESS) {
		/* Set SHA under reset on failure condition */
		XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);
		InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the SHA3 engine with the input data
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr	Starting address of the data which has to be updated
 *				to SHA engine
 * @param	Size		Size of the input data in bytes
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  If there is a failure in SSS configuration
 *
 ******************************************************************************/
int XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
						const u32 Size)
{
	/** Update input data to SHA3 engine */
	return XSecure_Sha3Update64Bit(InstancePtr, (u64)InDataAddr, Size);
}

/*****************************************************************************/
/**
 * @brief	This function updates SHA3 engine with final data which includes
 * 		SHA3 padding and reads final hash on complete data
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure, where
 * 				output hash is stored into Hash which is a member of
 * 				XSecure_Sha3Hash structure
 *
 * @return
 *		 - XST_SUCCESS  If finished without any errors
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  If Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
int XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, XSecure_Sha3Hash *Sha3Hash)
{
	volatile int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	u32 PadLen;
	u32 Size = 0U;

	/** Validate the input arguments */
	if ((InstancePtr == NULL) || (Sha3Hash == NULL)) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	if ((InstancePtr->ShaState != XSECURE_SHA_UPDATE_IN_PROGRESS) &&
		(InstancePtr->ShaState != XSECURE_SHA_ENGINE_STARTED)) {
			Status = (int)XSECURE_SHA3_STATE_MISMATCH_ERROR;
		goto END;
	}

	PadLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;
	/** Perform SHA3 padding based on IsLastUpdate flag */
	if (InstancePtr->IsLastUpdate != TRUE) {
		PadLen = (PadLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
			(XSECURE_SHA3_BLOCK_LEN - PadLen);

		Status = XSecure_Sha3NistPadd(&InstancePtr->PartialData[InstancePtr->PartialLen],
					PadLen);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}

		Size = PadLen + InstancePtr->PartialLen;
		Status = XSecure_Sha3DmaTransfer(InstancePtr,
					(u64)(UINTPTR)InstancePtr->PartialData,
					Size, TRUE);
		if (Status != XST_SUCCESS) {
			goto END_RST;
		}
	}
	else {
		Size = InstancePtr->PartialLen;
		if (Size != 0x0U) {
			Status = XST_FAILURE;
			goto END_RST;
		}
	}

	/* Status Reset */
	Status = XST_FAILURE;

	/* Check the SHA3 DONE bit. */
	Status = XSecure_Sha3WaitForDone(InstancePtr);
	if (Status != XST_SUCCESS) {
		goto END_RST;
	}

	/* Status Reset */
	Status = XST_FAILURE;

	/** Read out the Hash in reverse order.  */
	Status = XSecure_Sha3ReadHash(InstancePtr, Sha3Hash);

END_RST:
	if (Size > 0x0U) {
		SStatus = Xil_SMemSet(&InstancePtr->PartialData, Size, 0U, Size);
		if (Status == XST_SUCCESS) {
			Status = SStatus;
		}
	}
	/* Set SHA under reset */
	XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);

	InstancePtr->ShaState = XSECURE_SHA_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function calculates the SHA-3 digest on the given input data
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr 	Starting address of the data on which sha3 hash
 * 				should be calculated
 * @param	Size		Size of the input data
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure, where output
 * 				hash is stored into Hash which is a member of
 *				XSecure_Sha3Hash structure
 *
 * @return
 *		 - XST_SUCCESS  If digest calculation done successfully
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XST_FAILURE  If any error from Sha3Update or Sha3Finish
 *
 ******************************************************************************/
int XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
					const u32 Size, XSecure_Sha3Hash *Sha3Hash)
{
	volatile int Status = XST_FAILURE;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Sha3Hash == NULL)) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	/** Start SHA3 engine */
	Status = XSecure_Sha3Start(InstancePtr);
	if (Status != XST_SUCCESS){
		goto END;
	}

	/* Status Reset */
	Status = XST_FAILURE;
	/** Update SHA3 engine */
	Status = XSecure_Sha3Update(InstancePtr, InDataAddr, Size);
	if (Status != XST_SUCCESS){
		goto END;
	}

	/* Status Reset */
	Status = XST_FAILURE;
	/** Perform padding and read out the hash */
	Status = XSecure_Sha3Finish(InstancePtr, Sha3Hash);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the SHA3 hash of the data and it can be called
 * 		between calls to XSecure_Sha3Update
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure, where output
 *				hash is stored into Hash which is a member of
 *				XSecure_Sha3Hash structure
 *
 * @return
 *		 - XST_SUCCESS  On successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_Sha3ReadHash(const XSecure_Sha3 *InstancePtr,
	XSecure_Sha3Hash *Sha3Hash)
{
	int Status = XST_FAILURE;
	u32 Index;
	u32 RegVal;
	u32 *HashPtr;

	/* Validate the input arguments */
	if ((InstancePtr == NULL) || (Sha3Hash == NULL)) {
		Status = (int)XSECURE_SHA3_INVALID_PARAM;
		goto END;
	}

	HashPtr = (u32 *)Sha3Hash->Hash;

	/** Read the hash from the registers and store in the local buffer in reverse order */
	for (Index = 0U; Index < XSECURE_SHA3_HASH_LENGTH_IN_WORDS; Index++)
	{
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_SHA3_DIGEST_0_OFFSET + (u16)(Index * XSECURE_WORD_SIZE));
		HashPtr[XSECURE_SHA3_HASH_LENGTH_IN_WORDS - Index - 1U] = RegVal;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function transfers data through DMA
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr 	Starting address of the data which has to be updated
 *				to SHA engine
 * @param	Size 		Size of the input data in bytes
 * @param	IsLastUpdate	Flag to indicate whether this is the last update
 *				or not
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XST_FAILURE  In case of an error
 *
 ******************************************************************************/
static int XSecure_Sha3DmaTransfer(const XSecure_Sha3 *InstancePtr,
	u64 InDataAddr, const u32 Size, u8 IsLastUpdate)
{
	int Status = XST_FAILURE;

	/* Asserts validate the input arguments */
	XSecure_AssertNonvoid(InstancePtr != NULL);

	/* Configure the SSS for SHA3 hashing. */
	Status = XSecure_SssSha(&(InstancePtr->SssInstance),
					(u16)(InstancePtr->DmaPtr->Config.DmaType - XSECURE_TYPE_PMC_DMA0),
					InstancePtr->ShaConfig->SssShaCfg);
	if (Status != XST_SUCCESS) {
		goto ENDF;
	}
	XPmcDma_Transfer(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL,
		InDataAddr, (u32)Size/XSECURE_WORD_SIZE, IsLastUpdate);

	/* Checking the PMC DMA done bit should be enough. */
	Status = XPmcDma_WaitForDoneTimeout(InstancePtr->DmaPtr,
						XPMCDMA_SRC_CHANNEL);
	if (Status != XST_SUCCESS) {
		goto ENDF;
	}

	/* Acknowledge the transfer has completed */
	XPmcDma_IntrClear(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL,
				XPMCDMA_IXR_DONE_MASK);

ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates hash for data block of size <= 512MB
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr 	Starting address of the data, which has to be updated
 *				to SHA engine
 * @param	Size 	 	Size of the input data in bytes
 * @param	IsLastUpdate	Flag to indicate whether this is the last update
 *				or not
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
static int XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr,
			u64 InDataAddr, const u32 Size, u8 IsLastUpdate)
{
	int Status = XST_FAILURE;
	u32 RemainingDataLen;
	u32 DmableDataLen;
	u64 DmableDataAddr;
	u8 IsLast;
	u32 PrevPartialLen;
	u8 *PartialData;
	u64 DataAddr = InDataAddr;

	XSecure_AssertNonvoid(InstancePtr != NULL);

	PrevPartialLen = InstancePtr->PartialLen;
	PartialData = InstancePtr->PartialData;

	RemainingDataLen = Size + PrevPartialLen;
	IsLast = FALSE;
	while(RemainingDataLen >= XSECURE_SHA3_BLOCK_LEN)
	{
		/* Handle Partial data and non dword aligned data address */
		if ((PrevPartialLen != 0U) ||
		    ((InDataAddr & (u64)XPMCDMA_ADDR_LSB_MASK) != 0U)) {
			XSecure_MemCpy64((u64)(UINTPTR)&PartialData[PrevPartialLen], DataAddr,
				XSECURE_SHA3_BLOCK_LEN - PrevPartialLen);
			DmableDataAddr = (u64)(UINTPTR)PartialData;
			DmableDataLen = XSECURE_SHA3_BLOCK_LEN;
			DataAddr = DataAddr + XSECURE_SHA3_BLOCK_LEN - PrevPartialLen;
			RemainingDataLen = RemainingDataLen - DmableDataLen;
		}
		else {
			/* Process data of size in multiple of dwords */
			DmableDataAddr = DataAddr;
			DmableDataLen = RemainingDataLen -
				(RemainingDataLen % XSECURE_SHA3_BLOCK_LEN);
			DataAddr = DataAddr + (u64)DmableDataLen;
			RemainingDataLen -= DmableDataLen;
		}

		if ((RemainingDataLen == 0U) && (IsLastUpdate == TRUE)) {
			IsLast = TRUE;
		}

		Status = XSecure_Sha3DmaTransfer(InstancePtr, DmableDataAddr,
						DmableDataLen, IsLast);
		if (Status != XST_SUCCESS){
			Status |= Xil_SMemSet(&InstancePtr->PartialData,
					sizeof(InstancePtr->PartialData), 0U,
					sizeof(InstancePtr->PartialData));
			goto END;
		}
		PrevPartialLen = 0U;
	}

	/* Handle remaining data during processing of next data chunk or during
	   data padding */
	if(RemainingDataLen > 0U) {
		XSecure_MemCpy64((u64)(UINTPTR)&PartialData[PrevPartialLen],
			DataAddr, RemainingDataLen - PrevPartialLen);
	}
	InstancePtr->PartialLen = RemainingDataLen;
	Status = Xil_SMemSet(&InstancePtr->PartialData[RemainingDataLen],
			sizeof(InstancePtr->PartialData) - RemainingDataLen, 0U,
			sizeof(InstancePtr->PartialData) - RemainingDataLen);
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function assigns SHA3 configuration to the InstancePtr
 *
 * @param	InstancePtr	Pointer to XSecure_Sha3Instance
 * @param	DeviceId	Deviceid of the SHA3
 *
 * @return
 *		 - XST_SUCCESS  On successful initialization
 *		 - XST_FAILURE  On failure
 *
 ******************************************************************************/
int XSecure_Sha3LookupConfig(XSecure_Sha3 *InstancePtr, u32 DeviceId) {
	int Status = XST_FAILURE;
	const XSecure_ShaConfig *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XSECURE_SHA3_NUM_OF_INSTANCES; Index++) {
		if (ShaConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &ShaConfigTable[Index];
			break;
		}
	}

	if ((CfgPtr == NULL) || (InstancePtr == NULL)) {
		goto END;
	}

	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	InstancePtr->ShaConfig = CfgPtr;
	Status = XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	This function initializes a XSecure_Sha3 structure with the default
 * 		values required for operating the SHA3 cryptographic engine
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	DmaPtr		Pointer to the XPmcDma instance
 * @param	Mode		SHA mode
 *
 * @return
 *		 - XST_SUCCESS  If initialization was successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *
 *****************************************************************************/
int XSecure_ShaInitialize(XSecure_Sha *InstancePtr, XPmcDma* DmaPtr)
{
	return XSecure_Sha3Initialize(InstancePtr, DmaPtr);
}

/*****************************************************************************/
/**
 * @brief	This function configures Secure Stream Switch and starts the
 *		SHA-3 engine
 *
 * @param	InstancePtr Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  On successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *
 ******************************************************************************/
int XSecure_ShaStart(XSecure_Sha *InstancePtr, XSecure_ShaMode Mode)
{
	(void)Mode;

	return XSecure_Sha3Start(InstancePtr);
}

/*****************************************************************************/
/**
 * @brief	This function updates the SHA3 engine with the input data
 * 		located at a 64-bit address
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	InDataAddr	Starting 64 bit address of the data which has
 *				to be updated to SHA engine
 * @param	Size		Size of the input data in bytes
 *
 * @return
 *		 - XST_SUCCESS  If the update is successful
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  If there is a failure in SSS configuration
 *
 ******************************************************************************/
int XSecure_ShaUpdate(XSecure_Sha *InstancePtr, u64 InDataAddr, const u32 DataSize)
{
	return XSecure_Sha3Update64Bit(InstancePtr, InDataAddr, DataSize);
}

/****************************************************************************/
 /**
 * @brief	This function notifies the SHA driver at the end of the SHA data
 *		update and last update includes padding also. Typically called
 *		before XSecure_Sha3Finish to prevent driver adding the padding.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 *
 * @return
 *		 - XST_SUCCESS  If last update can be accepted
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *
 *****************************************************************************/
int XSecure_ShaLastUpdate(XSecure_Sha *InstancePtr)
{
	return XSecure_Sha3LastUpdate(InstancePtr);
}

/*****************************************************************************/
/**
 * @brief	This function updates SHA3 engine with final data which includes
 * 		SHA3 padding and reads final hash on complete data
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure, where
 * 				output hash is stored into Hash which is a member of
 * 				XSecure_Sha3Hash structure
 *
 * @return
 *		 - XST_SUCCESS  If finished without any errors
 *		 - XSECURE_SHA3_INVALID_PARAM  On invalid parameter
 *		 - XSECURE_SHA3_STATE_MISMATCH_ERROR  If State mismatch is occurred
 *		 - XST_FAILURE  If Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
int XSecure_ShaFinish(XSecure_Sha *InstancePtr, u64 HashAddr, const u32 HashSize)
{
	(void)HashSize;

	return XSecure_Sha3Finish(InstancePtr, (XSecure_Sha3Hash *)(UINTPTR)HashAddr);
}

/*****************************************************************************/
/**
 * @brief	This wrapper function calls SHA3 digest on the given input data
 *
 * @param	InstancePtr	Pointer to the void
 * @param	InDataAddr 	Starting address of the data on which sha3 hash
 * 				should be calculated
 * @param	Size		Size of the input data
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure, where output
 * 				hash is stored into Hash which is a member of
 *				XSecure_Sha3Hash structure
 *
 * @return
 *		 - XST_SUCCESS  If digest calculation done successfully
 *		 - XST_FAILURE  If any error from Sha3Update or Sha3Finish
 *
 ******************************************************************************/
int XSecure_ShaDigest(XSecure_Sha *InstancePtr, XSecure_ShaMode Mode,
			const u64 InDataAddr, const u32 DataSize, u64 HashAddr,
			const u32 HashSize)
{
	(void)HashSize;
	(void)Mode;

	return XSecure_Sha3Digest(InstancePtr, (UINTPTR)InDataAddr,
				DataSize, (XSecure_Sha3Hash *)(UINTPTR)HashAddr);
}

/*****************************************************************************/
/**
 * @brief	This function assigns SHA configuration to the InstancePtr
 *
 * @param	InstancePtr	Pointer to the void instance
 * @param	DeviceId	DeviceId of Sha engine
 *
 * @return
 *		 - XST_SUCCESS  Upon Success
 *		 - XST_FAILURE  Upon Failure
 *
 ******************************************************************************/
int XSecure_ShaLookupConfig(XSecure_Sha *InstancePtr, u32 DeviceId)
{
	return XSecure_Sha3LookupConfig(InstancePtr, DeviceId);
}
/** @} */
