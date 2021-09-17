/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
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
* 1.00  ba   08/10/14 Initial release
* 2.0   vns  01/28/17 Added API to read SHA3 hash.
* 2.2   vns  07/06/17 Added doxygen tags
* 3.0   vns  01/23/18 Added NIST SHA3 support.
*                     Added SSS configuration before every CSU DMA transfer
* 3.2   ka   04/30/18 Modified SHa3 hash calculation fuctionality to
* 		      support the following features:
*                     - To support byte aligned data,
*                     - To support non-word aligned address
*                     - And also fixed limitation of input data,
*                     	now size of input can be of any size.
*                     	not limitted to 512MB.
* 4.0	arc  12/18/18 Fixed MISRA-C violations.
*       arc  03/06/19 Added asserts to validate input params.
*       vns  03/12/19 Modified as part of XilSecure code re-arch.
*       arc  03/20/19 Added time outs and status info for API's.
*       mmd  03/15/19 Refactored the code.
*       psl  03/26/19 Fixed MISRA-C violation
*       vns  03/30/19 Added error condition in XSecure_Sha3Finish for
*                     for wrong pad selection
* 4.1   kal  05/20/19 Updated doxygen tags
*       psl  07/02/19 Fixed Coverity warnings.
*       mmd  07/05/19 Optimized the code
*       psl  07/31/19 Fixed MISRA-C violation
* 4.2   har  01/06/20 Removed asserts to validate zero size of data as per
*                     CR-1049217 since hashing of zero size data is valid
*       mmd  02/03/20 optimized XSecure_Sha3DataUpdate function
*       kpt  02/03/20 Enhanced the Code for non-aligned data and
*                     aligned address i.e CR-1052152
*       har  03/23/20 Moved to zynqmp directory and removed versal related code
*       kpt  04/06/20 Updated Sha3 State variable in Xsecure_Sha3Update
*       vns  04/13/20 Improved SHA3Finish to handle if last update is set
*       kot  04/21/20 Fixed MISRA C violations
* 4.6   kal  08/11/21 Added EXPORT CONTROL eFuse check in Sha3Initialize
*       am   09/17/21 Resolved compiler warnings
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sha_hw.h"
#include "xsecure_sha.h"
#include "xil_util.h"
#include "xil_assert.h"
#include "xsecure_utils.h"
#include "xsecure_cryptochk.h"

/************************** Constant Definitions *****************************/
#define XSECURE_CSU_SHA3_HASH_LENGTH_IN_BITS	(384U)
#define XSECURE_CSU_SHA3_HASH_LENGTH_IN_WORDS	\
									(XSECURE_CSU_SHA3_HASH_LENGTH_IN_BITS / 32U)

/* Keccak and Nist padding masks */
#define XSECURE_CSU_SHA3_START_KECCAK_PADDING_MASK    (0x01U)
#define XSECURE_CSU_SHA3_END_KECCAK_PADDING_MASK      (0x80U)
#define XSECURE_CSU_SHA3_START_NIST_PADDING_MASK      (0x06U)
#define XSECURE_CSU_SHA3_END_NIST_PADDING_MASK        (0x80U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
 * @brief
 * This inline function waits till SHA3 completes its operation.
 *
 * @param	InstancePtr Pointer to the XSecure_Sha3 instance.
 *
 * @return	XST_SUCCESS if the SHA3 completes its operation.
 * 		XST_FAILURE if a timeout has occurred.
 *
 ******************************************************************************/
inline u32 XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr)
{
	return Xil_WaitForEvent((InstancePtr)->BaseAddress + XSECURE_CSU_SHA3_DONE_OFFSET,
	                XSECURE_CSU_SHA3_DONE_DONE,
	                XSECURE_CSU_SHA3_DONE_DONE,
	                XSECURE_SHA_TIMEOUT_MAX);
}

/************************** Function Prototypes ******************************/

static u32 XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size, u8 IsLast);
static u32 XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr, const u8 *Data,
					const u32 Size, u8 IsLastUpdate);
static void XSecure_Sha3KeccakPadd(XSecure_Sha3 *InstancePtr, u8 *Dst,
					u32 MsgLen);
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
* @param	CsuDmaPtr 	Pointer to the XCsuDma instance.
*
* @return	XST_SUCCESS if initialization was successful
*
* @note		The base address is initialized directly with value from
* 		xsecure_hw.h
*		The default is NIST SHA3 padding, to change to KECCAK
*		padding call XSecure_Sha3PadSelection() after
*		XSecure_Sha3Initialize().
*
*****************************************************************************/

s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma* CsuDmaPtr)
{
	int Status = XST_FAILURE;

	Status = (int)XSecure_CryptoCheck();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CsuDmaPtr != NULL);

	InstancePtr->BaseAddress = XSECURE_CSU_SHA3_BASE;
	InstancePtr->Sha3Len = 0U;
	InstancePtr->CsuDmaPtr = CsuDmaPtr;
	InstancePtr->Sha3PadType = XSECURE_CSU_NIST_SHA3;
	InstancePtr->IsLastUpdate = FALSE;

	XSecure_SssInitialize(&(InstancePtr->SssInstance));

	InstancePtr->Sha3State = XSECURE_SHA3_INITIALIZED;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function provides an option to select the SHA-3 padding type to be used
 * while calculating the hash.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Sha3Type 	Type of SHA3 padding to be used.
 * 			 - For NIST SHA-3 padding - XSECURE_CSU_NIST_SHA3
 * 			 - For KECCAK SHA-3 padding - XSECURE_CSU_KECCAK_SHA3
 *
 * @return	XST_SUCCESS if pad selection is successful.
 * 		XST_FAILURE if pad selecction is failed.
 *
 * @note	The default provides support for NIST SHA-3. If a user wants
 * 			to change the padding to Keccak SHA-3, this function
 * 			should be called after XSecure_Sha3Initialize()
 *
 ******************************************************************************/
 s32 XSecure_Sha3PadSelection(XSecure_Sha3 *InstancePtr,
		 XSecure_Sha3PadType Sha3PadType)
{
	s32 Status;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Sha3PadType == XSECURE_CSU_NIST_SHA3)
			|| (Sha3PadType == XSECURE_CSU_KECCAK_SHA3));
	Xil_AssertNonvoid((InstancePtr->Sha3State == XSECURE_SHA3_INITIALIZED) ||
					(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED));

	/* If operation is in between can't be modified */
	if (InstancePtr->Sha3Len != 0x00U) {
		Status = (s32)XST_FAILURE;
		goto END;
	}
	InstancePtr->Sha3PadType = Sha3PadType;
	Status = XST_SUCCESS;
END:
	return Status;
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
s32 XSecure_Sha3LastUpdate(XSecure_Sha3 *InstancePtr)
{

	 /* Assert validates the input arguments */
	 Xil_AssertNonvoid(InstancePtr != NULL);
	 Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	InstancePtr->IsLastUpdate = TRUE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function generates padding for the SHA-3 engine.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Dst 	Pointer to location where padding is to be applied.
 * @param	MsgLen	Length of padding in bytes.
 *
 * @return	None
 *
 ******************************************************************************/
static void XSecure_Sha3KeccakPadd(XSecure_Sha3 *InstancePtr, u8 *Dst,
		u32 MsgLen)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MsgLen != 0U);

	(void)memset(Dst, 0, MsgLen);
	Dst[0] = XSECURE_CSU_SHA3_START_KECCAK_PADDING_MASK;
	Dst[MsgLen -1U] |= XSECURE_CSU_SHA3_END_KECCAK_PADDING_MASK;
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
	Dst[0] =  XSECURE_CSU_SHA3_START_NIST_PADDING_MASK;
	Dst[MsgLen -1U] |= XSECURE_CSU_SHA3_END_NIST_PADDING_MASK;
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
			XSECURE_CSU_SHA3_RESET_OFFSET);

	/* Start SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_START_OFFSET,
			XSECURE_CSU_SHA3_START_START);
	InstancePtr->Sha3State = XSECURE_SHA3_ENGINE_STARTED;
}

/*****************************************************************************/
/**
 * @brief
 * This function updates the SHA3 engine with the input data.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	Data 		Pointer to the input data for hashing.
 * @param	Size 		Size of the input data in bytes.
 *
 * @return	XST_SUCCESS if the update is successful
 * 		XST_FAILURE if there is a failure in SSS config
 *
 ******************************************************************************/
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size)
{
	u32 DataSize;
	u32 TransferredBytes;
	u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	InstancePtr->Sha3Len += Size;
	DataSize = Size;
	TransferredBytes = 0U;
	/*
 	 * CSU DMA can transfer Max 0x7FFFFFF no of words(0x1FFFFFFC bytes)
	 * at a time .So if the data sent more than that will be handled
	 * in the next update internally
	 */
	while (DataSize > XSECURE_CSU_DMA_MAX_TRANSFER) {
		Status = XSecure_Sha3DataUpdate(InstancePtr,
				(Data + TransferredBytes),
				XSECURE_CSU_DMA_MAX_TRANSFER, 0);
		if (Status != (u32)XST_SUCCESS){
			goto END;
		}
		DataSize = DataSize - XSECURE_CSU_DMA_MAX_TRANSFER;
		TransferredBytes = TransferredBytes +
			XSECURE_CSU_DMA_MAX_TRANSFER;
	}
	Status = XSecure_Sha3DataUpdate(InstancePtr, (Data + TransferredBytes),
				DataSize, (u8)InstancePtr->IsLastUpdate);
END:
	if (Status != XST_SUCCESS) {
		/* Set SHA under reset on failure condition */
		XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET);
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
 * @param	Hash		Pointer to location where resulting hash will
 *		be written
 *
 * @return	XST_SUCCESS if finished without any errors
 *		XST_FAILURE if Sha3PadType is other than KECCAK or NIST
 *
 *****************************************************************************/
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 PadLen;
	u32 Status = (u32)XST_FAILURE;
	u32 Size;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Hash != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	PadLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;

	if (InstancePtr->IsLastUpdate != TRUE) {
		PadLen = (PadLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
			(XSECURE_SHA3_BLOCK_LEN - PadLen);

		if (InstancePtr->Sha3PadType == XSECURE_CSU_NIST_SHA3) {
			XSecure_Sha3NistPadd(InstancePtr,
				&InstancePtr->PartialData[InstancePtr->PartialLen],
									PadLen);
		}
		else {
			 XSecure_Sha3KeccakPadd(InstancePtr,
				&InstancePtr->PartialData[InstancePtr->PartialLen],
									PadLen);
		}

		Size = PadLen + InstancePtr->PartialLen;
		Status = XSecure_Sha3DmaTransfer(InstancePtr,
					(u8*)InstancePtr->PartialData,
					Size, 1U);
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
	/* Check the SHA3 DONE bit. */
	Status = XSecure_Sha3WaitForDone(InstancePtr);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* If requested, read out the Hash in reverse order.  */
	if (Hash != NULL)
	{
		XSecure_Sha3_ReadHash(InstancePtr, Hash);
	}
END:
	if (Size > 0X0U) {
		(void)memset((void*)InstancePtr->PartialData, 0, Size);
	}
	/* Set SHA under reset */
	XSecure_SetReset(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET);

	InstancePtr->Sha3State = XSECURE_SHA3_INITIALIZED;
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the SHA-3 digest on the given input data.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	In		Pointer to the input data for hashing
 * @param	Size		Size of the input data
 * @param	Out		Pointer to location where resulting hash will
 *		be written.
 *
 * @return	XST_SUCCESS if digest calculation done successfully
 *		XST_FAILURE if any error from Sha3Update or Sha3Finish.
 *
 ******************************************************************************/
u32 XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In, const u32 Size,
								u8 *Out)
{
	u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Out != NULL);

	XSecure_Sha3Start(InstancePtr);
	Status = XSecure_Sha3Update(InstancePtr, In, Size);
	if (Status != (u32)XST_SUCCESS){
		goto END;
	}
	Status = XSecure_Sha3Finish(InstancePtr, Out);
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
 * @param	Hash		Pointer to a buffer in which read hash will be
 *		stored.
 *
 * @return	None
 *
 ******************************************************************************/
void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 Index;
	u32 RegVal;
	u32 *HashPtr = (u32 *)Hash;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Hash != NULL);
	Xil_AssertVoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	for (Index = 0U; Index < XSECURE_CSU_SHA3_HASH_LENGTH_IN_WORDS; Index++)
	{
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (u16)(Index * 4U));
		HashPtr[XSECURE_CSU_SHA3_HASH_LENGTH_IN_WORDS - Index - 1] = RegVal;
	}
}
/*****************************************************************************/
/**
 * @brief
 * This function Transfers Data through Dma
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	Data 		Pointer to the input data need to be transferred.
 * @param	Size 		Size of the input data in bytes.
 *
 * @return	None
 *
 *
 ******************************************************************************/
static u32 XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr, const u8 *Data,
								const u32 Size, u8 IsLast)
{
	u32 Status = (u32)XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Configure the SSS for SHA3 hashing. */
	Status = XSecure_SssSha(&(InstancePtr->SssInstance),
				InstancePtr->CsuDmaPtr->Config.DeviceId);
	if (Status != (u32)XST_SUCCESS){
		goto ENDF;
	}
	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)Data, (u32)Size/4U, IsLast);

	/* Checking the CSU DMA done bit should be enough. */
	Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		goto ENDF;
	}
	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
ENDF:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function updates hash for data block of size <= 512MB.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	Data 		Pointer to the input data for hashing.
 * @param	Size 		Size of the input data in bytes.
 *
 * @return	None
 *
 *
 ******************************************************************************/
static u32 XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr, const u8 *Data,
		const u32 Size, u8 IsLastUpdate)
{
	u32 RemainingDataLen;
	u32 DmableDataLen;
	const u8 *DmableData;
	u8 IsLast;
	u32 Status = (u32)XST_FAILURE;
	u32 PrevPartialLen;
	u8 *PartialData;

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
		    (((UINTPTR)Data & XCSUDMA_ADDR_LSB_MASK) != 0U)) {
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

		Status = XSecure_Sha3DmaTransfer(InstancePtr, DmableData,
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
