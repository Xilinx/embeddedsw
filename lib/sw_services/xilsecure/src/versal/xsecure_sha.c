/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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
*		      XSecure_Sha3Hash structure
*       kpt  08/15/20 Updated status value before reuse
*                     Replaced magic number with macro's
*                     and removed redundant check
*       kpt  08/15/20 Added validation for input arguments
*            08/27/20 Added 64 bit support for SHA3
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_error.h"
#include "xsecure_sha_hw.h"
#include "xil_assert.h"
#include "xsecure_sha.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/
#define XSECURE_SHA3_HASH_LENGTH_IN_BITS	(384U)
#define XSECURE_SHA3_HASH_LENGTH_IN_WORDS	\
				(XSECURE_SHA3_HASH_LENGTH_IN_BITS / 32U)

/* Nist padding masks */
#define XSECURE_SHA3_START_NIST_PADDING_MASK      (0x06U)
#define XSECURE_SHA3_END_NIST_PADDING_MASK        (0x80U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief
 * This inline function waits till SHA3 completes its operation.
 *
 * @param	InstancePtr Pointer to the XSecure_Sha3 instance.
 *
 * @return
 *		- XST_SUCCESS if the SHA3 completes its operation.
 * 		- XST_FAILURE if a timeout has occurred.
 *
 ******************************************************************************/
inline u32 XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr)
{
	return Xil_WaitForEvent((InstancePtr)->BaseAddress + XSECURE_SHA3_DONE_OFFSET,
			XSECURE_SHA3_DONE_DONE,
			XSECURE_SHA3_DONE_DONE,
			XSECURE_SHA_TIMEOUT_MAX);
}

/************************** Function Prototypes ******************************/

static u32 XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr,
					const UINTPTR InDataAddr, const u32 Size, u8 IsLast);
static u32 XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr,
					const UINTPTR InDataAddr, const u32 Size, u8 IsLastUpdate);
static void XSecure_Sha3NistPadd(XSecure_Sha3 *InstancePtr, u8 *Dst,
					u32 MsgLen);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/****************************************************************************/
/**
* @brief
* This function initializes a XSecure_Sha3 structure with the default values
* required for operating the SHA3 cryptographic engine.
*
* @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
* @param	DmaPtr 	Pointer to the XPmcDma instance.
*
* @return	XST_SUCCESS if initialization was successful
*
* @note		The base address is initialized directly with value from
* 		xsecure_hw.h
*
*****************************************************************************/
u32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XPmcDma* DmaPtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DmaPtr != NULL);

	InstancePtr->BaseAddress = XSECURE_SHA3_BASE;
	InstancePtr->Sha3Len = 0U;
	InstancePtr->DmaPtr = DmaPtr;
	InstancePtr->IsLastUpdate = FALSE;

	XSecure_SssInitialize(&(InstancePtr->SssInstance));

	InstancePtr->Sha3State = XSECURE_SHA3_INITIALIZED;

	return XST_SUCCESS;
}

 /****************************************************************************/
 /**
 * @brief
 * This function is to notify this is the last update of data where sha padding
 * is also been included along with the data in the next update call.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 *
 * @return	XST_SUCCESS if last update can be accepted
 *
 *
 *****************************************************************************/
u32 XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	InstancePtr->IsLastUpdate = TRUE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Generate padding for the NIST SHA-3
 *
 * @param	InstancePtr is a pointer to the XSecure_Sha3 instance.
 * @param	Dst is the pointer to location where padding is to be applied
 * @param	MsgLen is the length of padding in bytes
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
static void XSecure_Sha3NistPadd(XSecure_Sha3 *InstancePtr, u8 *Dst, u32 MsgLen)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsgLen != 0U);

	(void)memset(Dst, 0, MsgLen);
	Dst[0] =  XSECURE_SHA3_START_NIST_PADDING_MASK;
	Dst[MsgLen -1U] |= XSECURE_SHA3_END_NIST_PADDING_MASK;
}

/*****************************************************************************/
/**
 * @brief
 * This function configures Secure Stream Switch and starts the SHA-3 engine.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
void XSecure_Sha3Start(XSecure_Sha3 *InstancePtr)
{
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Sha3State == XSECURE_SHA3_INITIALIZED);

	InstancePtr->Sha3Len = 0U;
	InstancePtr->IsLastUpdate = FALSE;
	InstancePtr->PartialLen = 0U;
	(void)memset(InstancePtr->PartialData, 0, XSECURE_SHA3_BLOCK_LEN);

	/* Reset SHA3 engine. */
	XSecure_ReleaseReset(InstancePtr->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);

	/* Start SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_SHA3_START_OFFSET,
			XSECURE_SHA3_START_START);
	InstancePtr->Sha3State = XSECURE_SHA3_ENGINE_STARTED;
}

/*****************************************************************************/
/**
 * @brief
 * This function updates the SHA3 engine with the input data.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	InDataAddr 		Starting address of the data which has to be
 *			                updated to SHA engine.
 * @param	Size 		    Size of the input data in bytes.
 *
 * @return
 *		- XST_SUCCESS if the update is successful
 * 		- XST_FAILURE if there is a failure in SSS config
 *
 ******************************************************************************/
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
						const u32 Size)
{
	u32 DataSize;
	u32 TransferredBytes;
	u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);
	Xil_AssertNonvoid(((InDataAddr != 0x00U) && (Size > 0x00U)) ||
					  ((InDataAddr == 0x00U) && (Size == 0x00U)));

	InstancePtr->Sha3Len += Size;
	DataSize = Size;
	TransferredBytes = 0U;
	/*
	 * PMC DMA can transfer Max 0x7FFFFFF no of words(0x1FFFFFFC bytes)
	 * at a time .So if the data sent more than that will be handled
	 * in the next update internally
	 */
	while (DataSize > XSECURE_PMC_DMA_MAX_TRANSFER) {
		Status = XSecure_Sha3DataUpdate(InstancePtr,
				(InDataAddr + TransferredBytes),
				XSECURE_PMC_DMA_MAX_TRANSFER, FALSE);
		if (Status != (u32)XST_SUCCESS){
			goto END;
		}
		DataSize = DataSize - XSECURE_PMC_DMA_MAX_TRANSFER;
		TransferredBytes = TransferredBytes +
			XSECURE_PMC_DMA_MAX_TRANSFER;
	}
	Status = XSecure_Sha3DataUpdate(InstancePtr,
				(InDataAddr + TransferredBytes),
				DataSize, (u8)InstancePtr->IsLastUpdate);
END:
	if (Status != XST_SUCCESS) {
		/* Set SHA under reset on failure condition */
		XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);
		InstancePtr->Sha3State = XSECURE_SHA3_INITIALIZED;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function updates SHA3 engine with final data which includes SHA3
 * padding and reads final hash on complete data.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure,
 *                      where output hash is stored into Hash
 *                      which is member of the XSecure_Sha3Hash structure
 *
 * @return
 *		- XST_SUCCESS if finished without any errors
 *		- XST_FAILURE if Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, XSecure_Sha3Hash *Sha3Hash)
{
	u32 PadLen;
	volatile u32 Status = (u32)XST_FAILURE;
	u32 Size;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Sha3Hash != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	PadLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;

	if (InstancePtr->IsLastUpdate != TRUE) {
		PadLen = (PadLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
			(XSECURE_SHA3_BLOCK_LEN - PadLen);

		XSecure_Sha3NistPadd(InstancePtr,
					&InstancePtr->PartialData[InstancePtr->PartialLen],
					PadLen);

		Size = PadLen + InstancePtr->PartialLen;
		Status = XSecure_Sha3DmaTransfer(InstancePtr,
					(UINTPTR)InstancePtr->PartialData,
					Size, TRUE);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Size = InstancePtr->PartialLen;
		if (Size != 0x0U) {
			Status = XST_FAILURE;
			goto END;
		}
	}

	/* Status Reset */
	Status = (u32)XST_FAILURE;

	/* Check the SHA3 DONE bit. */
	Status = XSecure_Sha3WaitForDone(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Read out the Hash in reverse order.  */
	XSecure_Sha3ReadHash(InstancePtr, Sha3Hash);

END:
	if (Size > 0x0U) {
		(void)memset((void*)InstancePtr->PartialData, 0, Size);
	}
	/* Set SHA under reset */
	XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_SHA3_RESET_OFFSET);

	InstancePtr->Sha3State = XSECURE_SHA3_INITIALIZED;
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the SHA-3 digest on the given input data.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	InDataAddr 	Starting address of the data on which sha3
 * 			            hash should be calculated.
 * @param	Size		Size of the input data
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure,
 *                      where output hash is stored into Hash
 *                      which is member of the XSecure_Sha3Hash structure
 *
 * @return
 *		- XST_SUCCESS if digest calculation done successfully
 *		- XST_FAILURE if any error from Sha3Update or Sha3Finish.
 *
 ******************************************************************************/
u32 XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const UINTPTR InDataAddr,
					const u32 Size, XSecure_Sha3Hash *Sha3Hash)
{
	volatile u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Sha3Hash != NULL);
	Xil_AssertNonvoid(((InDataAddr != 0x00U) && (Size > 0x00U)) ||
					  ((InDataAddr == 0x00U) && (Size == 0x00U)));

	XSecure_Sha3Start(InstancePtr);
	Status = XSecure_Sha3Update(InstancePtr, InDataAddr, Size);
	if (Status != (u32)XST_SUCCESS){
		goto END;
	}

	/* Status Reset */
	Status = (u32)XST_FAILURE;

	Status = XSecure_Sha3Finish(InstancePtr, Sha3Hash);
	if (Status != (u32)XST_SUCCESS){
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reads the SHA3 hash of the data and it can be called
 * between calls to XSecure_Sha3Update.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Sha3Hash	Pointer to XSecure_Sha3Hash structure,
 *				where output hash is stored into Hash
 *				which is member of the XSecure_Sha3Hash structure
 *
 * @return	None
 *
 ******************************************************************************/
void XSecure_Sha3ReadHash(XSecure_Sha3 *InstancePtr, XSecure_Sha3Hash *Sha3Hash)
{
	u32 Index;
	u32 RegVal;
	u32 *HashPtr = (u32 *)Sha3Hash->Hash;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Sha3Hash != NULL);
	Xil_AssertVoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	for (Index = 0U; Index < XSECURE_SHA3_HASH_LENGTH_IN_WORDS; Index++)
	{
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_SHA3_DIGEST_0_OFFSET + (Index * XSECURE_WORD_SIZE));
		HashPtr[XSECURE_SHA3_HASH_LENGTH_IN_WORDS - Index - 1] = RegVal;
	}
}

/*****************************************************************************/
/**
 * @brief
 * This function Transfers Data through Dma
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	InDataAddr 		Starting address of the data which has to be
 *			                updated to SHA engine.
 * @param	Size 		    Size of the input data in bytes.
 * @param	IsLastUpdate	Flag to indicate whether this is the last update
 *			                or not.
 *
 * @return
 *		- XST_SUCCESS if the update is successful
 * 		- XST_FAILURE if there an error occurs
 *
 ******************************************************************************/
static u32 XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr,
						const UINTPTR InDataAddr, const u32 Size, u8 IsLast)
{
	u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Configure the SSS for SHA3 hashing. */
	Status = XSecure_SssSha(&(InstancePtr->SssInstance),
				InstancePtr->DmaPtr->Config.DeviceId);
	if (Status != (u32)XST_SUCCESS){
		goto ENDF;
	}
	XPmcDma_Transfer(InstancePtr->DmaPtr, XPMCDMA_SRC_CHANNEL,
					InDataAddr, (u32)Size/XSECURE_WORD_SIZE, IsLast);

	/* Checking the PMC DMA done bit should be enough. */
	Status = XPmcDma_WaitForDoneTimeout(InstancePtr->DmaPtr,
						XPMCDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
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
 * @brief
 * This function updates hash for data block of size <= 512MB.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	InDataAddr 		Starting address of the data, which has to be
 *			                updated to SHA engine.
 * @param	Size 	 	    Size of the input data in bytes.
 * @param	IsLastUpdate	Flag to indicate whether this is the last update
 *			                or not.
 *
 * @return
 *		- XST_SUCCESS if the update is successful
 * 		- XST_FAILURE if there is a failure
 *
 ******************************************************************************/
static u32 XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr,
			const UINTPTR InDataAddr, const u32 Size, u8 IsLastUpdate)
{
	u32 RemainingDataLen;
	u32 DmableDataLen;
	const u8 *DmableData;
	u8 IsLast;
	u32 Status = (u32)XST_FAILURE;
	u32 PrevPartialLen;
	u8 *PartialData;
	u8 *Data = (u8 *)InDataAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	PrevPartialLen = InstancePtr->PartialLen;
	PartialData = InstancePtr->PartialData;

	RemainingDataLen = Size + PrevPartialLen;
	IsLast = FALSE;
	while(RemainingDataLen >= XSECURE_SHA3_BLOCK_LEN)
	{
		/* Handle Partial data and non dword aligned data address */
		if ((PrevPartialLen != 0U) ||
		    (((UINTPTR)Data & XPMCDMA_ADDR_LSB_MASK) != 0U)) {
			XSecure_MemCpy((void *)&PartialData[PrevPartialLen],
				(void *)Data,
				XSECURE_SHA3_BLOCK_LEN - PrevPartialLen);
			DmableData = PartialData;
			DmableDataLen = XSECURE_SHA3_BLOCK_LEN;
			Data += XSECURE_SHA3_BLOCK_LEN - PrevPartialLen;
			RemainingDataLen = RemainingDataLen - DmableDataLen;
		}
		else {
			/* Process data of size in multiple of dwords */
			DmableData = Data;
			DmableDataLen = RemainingDataLen -
				(RemainingDataLen % XSECURE_SHA3_BLOCK_LEN);
			Data += DmableDataLen;
			RemainingDataLen -= DmableDataLen;
		}

		if ((RemainingDataLen == 0U) && (IsLastUpdate == TRUE)) {
			IsLast = TRUE;
		}

		Status = XSecure_Sha3DmaTransfer(InstancePtr, (UINTPTR)DmableData,
						DmableDataLen, IsLast);
		if (Status != (u32)XST_SUCCESS){
			(void)memset(&InstancePtr->PartialData, 0,
				    sizeof(InstancePtr->PartialData));
			goto END;
		}
		PrevPartialLen = 0U;
	}

	/* Handle remaining data during processing of next data chunk or during
	   data padding */
	if(RemainingDataLen > 0U) {
		XSecure_MemCpy((void *)(PartialData + PrevPartialLen), (void *)Data,
				     (RemainingDataLen - PrevPartialLen));
	}
	InstancePtr->PartialLen = RemainingDataLen;
	(void)memset(&InstancePtr->PartialData[RemainingDataLen], 0,
		    sizeof(InstancePtr->PartialData) - RemainingDataLen);
	Status = (u32) XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function performs known answer test(KAT) on SHA crypto engine.
 *
 * @param	SecureSha3	Pointer to the XSecure_Sha3 instance.
 *
 * @return
 *			- XST_SUCCESS when KAT Pass
 *			- Error code on failure
 *
 ******************************************************************************/
u32 XSecure_Sha3Kat(XSecure_Sha3 *SecureSha3)
{
	volatile u32 Status = (u32) XSECURE_SHA3_KAT_FAILED_ERROR;
	u32 Index;
	XSecure_Sha3Hash OutVal = {0U};

	const u8 ExpectedHash[XSECURE_HASH_SIZE_IN_BYTES] = {
			0x86U, 0x89U, 0xACU, 0xE3U, 0xA5U, 0xF9U, 0xF5U, 0x71U, 0xD6U,
			0xBBU, 0xCDU, 0x1CU, 0xE2U, 0xD4U, 0x18U, 0xD8U, 0xF6U, 0xCFU,
			0x76U, 0x82U, 0x56U, 0xDDU, 0x35U, 0x6DU, 0xB9U, 0xD6U, 0x1DU,
			0x58U, 0xCFU, 0xCBU, 0x96U, 0xEBU, 0x49U, 0xC6U, 0xB9U, 0xDDU,
			0xE3U, 0xA1U, 0x6EU, 0x63U, 0x5EU, 0x4BU, 0x61U, 0xB7U, 0x79U,
			0xB1U, 0xFEU, 0x8EU
	};
	const u32 DataValue[XSECURE_SHA3_BLOCK_LEN/4U] = {
			0xA1D1199EU, 0xB9278FF8U, 0xCA22EDA5U, 0xB51272AAU,
			0xA583A2F7U, 0x9513A099U, 0x1380DF32U, 0x4305F9A6U,
			0x26E7DF18U, 0x1C4B3315U, 0xA84AF20EU, 0x7447560CU,
			0x9580FB9FU, 0x0FE44017U, 0x9C25F0F7U, 0xA90D22A7U,
			0xA2155D69U, 0x6F34008EU, 0x3FF5E1EAU, 0x84CC3585U,
			0x3EAAB093U, 0x7DCFEDA7U, 0x21E00F23U, 0xE539A9F3U,
			0x9C84DB7BU, 0x801ABECDU
	};

	XSecure_Sha3Start(SecureSha3);
	Status = XSecure_Sha3LastUpdate(SecureSha3);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_LAST_UPDATE_ERROR;
		goto END;
	}

	Status = (u32) XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Update(SecureSha3, (UINTPTR)DataValue,
			XSECURE_SHA3_BLOCK_LEN);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_PMC_DMA_UPDATE_ERROR;
		goto END;
	}

	Status = (u32) XSECURE_SHA3_KAT_FAILED_ERROR;

	Status = XSecure_Sha3Finish(SecureSha3, &OutVal);
	if (Status != (u32)XST_SUCCESS) {
		Status = XSECURE_SHA3_FINISH_ERROR;
		goto END;
	}

	Status = (u32) XSECURE_SHA3_KAT_FAILED_ERROR;
	for(Index = 0U; Index < XSECURE_HASH_SIZE_IN_BYTES; Index++) {
		if (OutVal.Hash[Index] != ExpectedHash[Index]) {
			Status = (u32) XSECURE_SHA3_KAT_FAILED_ERROR;
			goto END;
		}
	}

	if (Index == XSECURE_HASH_SIZE_IN_BYTES) {
		Status = (u32)XST_SUCCESS;
	}

END:
	XSecure_SetReset(SecureSha3->BaseAddress,
			XSECURE_SHA3_RESET_OFFSET);
	return Status;
}
