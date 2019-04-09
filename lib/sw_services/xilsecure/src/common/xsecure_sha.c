/******************************************************************************
*
* Copyright (C) 2014 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sha.h"
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
* This function initializes a specific Xsecure_Sha3 instance so that it is
* ready to be used.
*
* @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
* @param	CsuDmaPtr 	Pointer to the XCsuDma instance.
*
* @return	XST_SUCCESS if initialization was successful
*
* @note		The base address is initialized directly with value from
* 		xsecure_hw.h
*		By default uses NIST SHA3 padding, to change to KECCAK
*		padding call XSecure_Sha3PadSelection() after
*		XSecure_Sha3Initialize().
*
*****************************************************************************/

s32 XSecure_Sha3Initialize(XSecure_Sha3 *InstancePtr, XCsuDma* CsuDmaPtr)
{
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

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function provides an option to select the SHA-3 padding type to be used
 * while calculating the hash.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Sha3Type 	Type of the sha3 padding to be used.
 * 			 - For NIST SHA-3 padding - XSECURE_CSU_NIST_SHA3
 * 			 - For KECCAK SHA-3 padding - XSECURE_CSU_KECCAK_SHA3
 *
 * @return	By default provides support for NIST SHA-3, if wants to change for
 * 			Keccak SHA-3 this function should be called after
 * 			XSecure_Sha3Initialize()
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
	Dst[0] =  XSECURE_CSU_SHA3_START_NIST_PADDING_MASK;;
	Dst[MsgLen -1U] |= XSECURE_CSU_SHA3_END_NIST_PADDING_MASK;
}
/*****************************************************************************/
/**
 * @brief
 * This function configures the SSS and starts the SHA-3 engine.
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
 * This function updates hash for new input data block.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 * @param	Data 		Pointer to the input data for hashing.
 * @param	Size 		Size of the input data in bytes.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size)
{
	u32 DataSize;
	u32 TransferredBytes;
	u32 Status;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size > (u32)0x00U);
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
	if (Status != (u32)XST_SUCCESS){
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function waits till SHA3 completes its action.
 *
 * @param	InstancePtr 	Pointer to the XSecure_Sha3 instance.
 *
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr)
{
	volatile u32 RegStatus;
	u32 Status = (u32)XST_FAILURE;
	u32 TimeOut = XSECURE_SHA_TIMEOUT_MAX;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	while (TimeOut != 0U) {
		RegStatus = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_DONE_OFFSET);
		if (XSECURE_CSU_SHA3_DONE_DONE ==
				((u32)RegStatus & XSECURE_CSU_SHA3_DONE_DONE)) {
			Status = (u32)XST_SUCCESS;
			goto END;
		}
		TimeOut = TimeOut - 1U;
	}
END:
	return Status;
}


/*****************************************************************************/
/**
 * @brief
 * This function sends the last data and padding when blocksize is not
 * multiple of 104 bytes.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Hash		Pointer to location where resulting hash will
 *		be written
 *
 * @return	None
 *
 *
 *****************************************************************************/
u32 XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 PartialLen;
	u32 Status;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Hash != NULL);
	Xil_AssertNonvoid(InstancePtr->Sha3State == XSECURE_SHA3_ENGINE_STARTED);

	PartialLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;

	PartialLen = (PartialLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
		(XSECURE_SHA3_BLOCK_LEN - PartialLen);

	if (InstancePtr->Sha3PadType == XSECURE_CSU_NIST_SHA3) {
		XSecure_Sha3NistPadd(InstancePtr,
			&InstancePtr->PartialData[InstancePtr->PartialLen],
								PartialLen);
	}
	else if (InstancePtr->Sha3PadType == XSECURE_CSU_KECCAK_SHA3) {
		 XSecure_Sha3KeccakPadd(InstancePtr,
			&InstancePtr->PartialData[InstancePtr->PartialLen],
								PartialLen);
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	/* Configure the SSS for SHA3 hashing. */
	Status = XSecure_SssSha(&(InstancePtr->SssInstance),
			InstancePtr->CsuDmaPtr->Config.DeviceId);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)InstancePtr->PartialData,
				(PartialLen + (InstancePtr->PartialLen))/4U, 1);

	/* Check for CSU DMA done bit */
	Status = XCsuDma_WaitForDoneTimeout(InstancePtr->CsuDmaPtr,
						XCSUDMA_SRC_CHANNEL);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

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
 * @return	None
 *
 *
 ******************************************************************************/
u32 XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In, const u32 Size,
								u8 *Out)
{
	u32 Status;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size > (u32)0x00U);
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
 * Reads the SHA3 hash of the data. It can be called intermediately of updates
 * also to read hashs.
 *
 * @param	InstancePtr	Pointer to the XSecure_Sha3 instance.
 * @param	Hash		Pointer to a buffer in which read hash will be
 *		stored.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSecure_Sha3_ReadHash(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	u32 Index;
	u32 RegVal;
	u32 *HashPtr = (u32 *)Hash;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Hash != NULL);

	for (Index = 0U; Index < XSECURE_CSU_SHA3_HASH_LENGTH_IN_WORDS; Index++)
	{
		RegVal = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4U));
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
	Xil_AssertNonvoid(Size > (u32)0x00U);

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
	u32 CurrentPartialLen;
	u32 PrevPartialLen;
	u32 TotalLen ;
	u32 TransferredBytes ;
	u32 DataSize;
	u8 IsLast;
	u32 Status = XST_FAILURE;

	/* Asserts validate the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Size > (u32)0x00U);

	CurrentPartialLen = (Size % 4U);
	PrevPartialLen = InstancePtr->PartialLen;
	TotalLen = Size + PrevPartialLen;
	TransferredBytes = 0U;

	/* If always Word Aligned Data and Word aligned address */
	if ((CurrentPartialLen == 0U) && (PrevPartialLen == 0U) &&
			(((UINTPTR)Data & XCSUDMA_ADDR_LSB_MASK) == 0U)) {
		Status = XSecure_Sha3DmaTransfer(InstancePtr, Data, Size, IsLastUpdate);
		if (Status != (u32)XST_SUCCESS){
			goto END;
		}
	}
	/* For Non-Word Aligned Data and for Non-Word aligned Address*/
	else {
		if (TotalLen < XSECURE_SHA3_BLOCK_LEN) {

			/*
  			 * Copy the data to buffer when combined length
			 * does not exceed SHA3_BLOCK_LEN
			 */

			(void)XSecure_MemCpy(&InstancePtr->PartialData[PrevPartialLen],
					(void *)(UINTPTR)Data, Size);
			InstancePtr->PartialLen = TotalLen;
		}
		else if (TotalLen == XSECURE_SHA3_BLOCK_LEN) {

			/*
 			 * Copy and transfer the data when combined length
			 * is equal to SHA3_BLOCK_LEN
			 */

			(void)XSecure_MemCpy(&InstancePtr->PartialData[PrevPartialLen],
					(void *)(UINTPTR)Data, Size);
			Status = XSecure_Sha3DmaTransfer(InstancePtr,
						InstancePtr->PartialData,
						XSECURE_SHA3_BLOCK_LEN, IsLastUpdate);
			if (Status != (u32)XST_SUCCESS){
				goto END;
			}
			InstancePtr->PartialLen = 0U ;
			(void)memset(&InstancePtr->PartialData, 0,
				sizeof(InstancePtr->PartialData));
		}
		else {
			DataSize = Size;

			/*
 			 * Perform Multiple Dma transfers until
			 * Data Size < SHA3_BLOCK_LEN
			 */
			IsLast = FALSE;
			while (DataSize > XSECURE_SHA3_BLOCK_LEN) {
				(void)XSecure_MemCpy(&InstancePtr->PartialData[PrevPartialLen],
						(void *)(UINTPTR)(Data + TransferredBytes),
						(XSECURE_SHA3_BLOCK_LEN -
						PrevPartialLen ));
				DataSize = DataSize - (XSECURE_SHA3_BLOCK_LEN -
								PrevPartialLen);
				if ((DataSize == 0U) && (IsLastUpdate == TRUE)) {
					IsLast = TRUE;
				}
				Status = XSecure_Sha3DmaTransfer(InstancePtr,
						InstancePtr->PartialData,
						XSECURE_SHA3_BLOCK_LEN, IsLast);
				if (Status != (u32)XST_SUCCESS){
					goto END;
				}
				(void)memset(&InstancePtr->PartialData, 0,
					sizeof(InstancePtr->PartialData));
				TransferredBytes = TransferredBytes +
							XSECURE_SHA3_BLOCK_LEN -
							PrevPartialLen;
				PrevPartialLen = 0U;
			}
			/*
 			 * Update PartialData and PartialLen based on
			 * size of the data remaining
			 */
			if (DataSize == 0U) {
				InstancePtr->PartialLen = 0U;
				(void)memset(&InstancePtr->PartialData, 0,
						sizeof(InstancePtr->PartialData));
			}
			else {
				(void)XSecure_MemCpy(InstancePtr->PartialData,
					Data + TransferredBytes,
							DataSize);
				InstancePtr->PartialLen = DataSize;
			}
		}
	}
END:
	return Status;
}
