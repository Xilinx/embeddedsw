/******************************************************************************
*
* Copyright (C) 2014 - 18 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_sha.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static void XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr, const u8 *Data,
							const u32 Size);
static void XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr, const u8 *Data,
							const u32 Size);
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
		 XSecure_Sha3PadType Sha3Type)
{
	/* Assert validates the input arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Sha3Type <= XSECURE_CSU_KECCAK_SHA3);

	/* If operation is in between can't be modified */
	if (InstancePtr->Sha3Len != 0x00U) {
		return XST_FAILURE;
	}
	InstancePtr->Sha3PadType = Sha3Type;

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
void XSecure_Sha3Padd(XSecure_Sha3 *InstancePtr, u8 *Dst, u32 MsgLen)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	memset(Dst, 0, MsgLen);
	Dst[0] = 0x1U;
	Dst[MsgLen -1U] |= 0x80U;
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
static void XSecure_NistSha3Padd(XSecure_Sha3 *InstancePtr, u8 *Dst, u32 MsgLen)
{
	/* Assert validates the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	memset(Dst, 0, MsgLen);
	Dst[0] =  0x6;
	Dst[MsgLen -1U] |= 0x80U;
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

	InstancePtr->Sha3Len = 0U;
	InstancePtr->PartialLen = 0U;
	memset(InstancePtr->PartialData, 0, XSECURE_SHA3_BLOCK_LEN);

	/* Reset SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_RESET_OFFSET,
			XSECURE_CSU_SHA3_RESET_RESET);

	XSecure_WriteReg(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_RESET_OFFSET, 0U);

	/* Start SHA3 engine. */
	XSecure_WriteReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_START_OFFSET,
			XSECURE_CSU_SHA3_START_START);
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
void XSecure_Sha3Update(XSecure_Sha3 *InstancePtr, const u8 *Data,
						const u32 Size)
{
	u32 DataSize;
	u32 TransferredBytes;

	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Size != (u32)0x00U);

	InstancePtr->Sha3Len += Size;
	DataSize = Size;
	TransferredBytes = 0;
	/*
 	 * CSU DMA can transfer Max 0x7FFFFFF no of words(0x1FFFFFFC bytes)
	 * at a time .So if the data sent more than that will be handled
	 * in the next update internally
	 */
	while (DataSize > XSECURE_CSU_DMA_MAX_TRANSFER) {
		XSecure_Sha3DataUpdate(InstancePtr,
				(Data + TransferredBytes),
				XSECURE_CSU_DMA_MAX_TRANSFER);
		DataSize = DataSize - XSECURE_CSU_DMA_MAX_TRANSFER;
		TransferredBytes = TransferredBytes +
			XSECURE_CSU_DMA_MAX_TRANSFER;
	}
	XSecure_Sha3DataUpdate(InstancePtr, (Data + TransferredBytes),
							DataSize);
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
void XSecure_Sha3WaitForDone(XSecure_Sha3 *InstancePtr)
{
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	volatile u32 Status;

	do
	{
		Status = XSecure_ReadReg(InstancePtr->BaseAddress,
					XSECURE_CSU_SHA3_DONE_OFFSET);
	} while (XSECURE_CSU_SHA3_DONE_DONE !=
			((u32)Status & XSECURE_CSU_SHA3_DONE_DONE));
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
void XSecure_Sha3Finish(XSecure_Sha3 *InstancePtr, u8 *Hash)
{
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Hash != NULL);

	u32 *HashPtr = (u32 *)Hash;
	u32 PartialLen = InstancePtr->Sha3Len % XSECURE_SHA3_BLOCK_LEN;

	PartialLen = (PartialLen == 0U)?(XSECURE_SHA3_BLOCK_LEN) :
		(XSECURE_SHA3_BLOCK_LEN - PartialLen);

	if (InstancePtr->Sha3PadType == XSECURE_CSU_NIST_SHA3) {
		XSecure_NistSha3Padd(InstancePtr,
			&InstancePtr->PartialData[InstancePtr->PartialLen],
								PartialLen);
	}
	 else {
		 XSecure_Sha3Padd(InstancePtr,
			&InstancePtr->PartialData[InstancePtr->PartialLen],
								PartialLen);
	 }


	/* Configure the SSS for SHA3 hashing. */
	XSecure_SssSetup(XSecure_SssInputSha3(XSECURE_CSU_SSS_SRC_SRC_DMA));

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)InstancePtr->PartialData,
				(PartialLen + (InstancePtr->PartialLen))/4, 1);

	/* Check for CSU DMA done bit */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
						XCSUDMA_IXR_DONE_MASK);

	/* Check the SHA3 DONE bit. */
	XSecure_Sha3WaitForDone(InstancePtr);

	/* If requested, read out the Hash in reverse order.  */
	if (Hash)
	{
		u32 Index = 0U;
		u32 Val = 0U;
		for (Index=0U; Index < 12U; Index++)
		{
			Val = XSecure_ReadReg(InstancePtr->BaseAddress,
				XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4));
			HashPtr[11U - Index] = Val;
		}
	}

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
void XSecure_Sha3Digest(XSecure_Sha3 *InstancePtr, const u8 *In, const u32 Size,
								u8 *Out)
{
	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Size != (u32)0x00U);
	Xil_AssertVoid(Out != NULL);

	XSecure_Sha3Start(InstancePtr);
	XSecure_Sha3Update(InstancePtr, In, Size);
	XSecure_Sha3Finish(InstancePtr, Out);
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
	u32 Index = 0U;
	u32 Val = 0U;
	u32 *HashPtr = (u32 *)Hash;

	for (Index=0U; Index < 12U; Index++)
	{
		Val = XSecure_ReadReg(InstancePtr->BaseAddress,
			XSECURE_CSU_SHA3_DIGEST_0_OFFSET + (Index * 4));
		HashPtr[11U - Index] = Val;
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
static void XSecure_Sha3DmaTransfer(XSecure_Sha3 *InstancePtr, const u8 *Data,
								const u32 Size)
{
	/* Configure the SSS for SHA3 hashing. */
	XSecure_SssSetup(XSecure_SssInputSha3(XSECURE_CSU_SSS_SRC_SRC_DMA));

	XCsuDma_Transfer(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				(UINTPTR)Data, (u32)Size/4, 0);

	/* Checking the CSU DMA done bit should be enough. */
	XCsuDma_WaitForDone(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL);

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(InstancePtr->CsuDmaPtr, XCSUDMA_SRC_CHANNEL,
				XCSUDMA_IXR_DONE_MASK);
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
static void XSecure_Sha3DataUpdate(XSecure_Sha3 *InstancePtr, const u8 *Data,
								const u32 Size)
{
	u32 CurrentPartialLen;
	u32 PrevPartialLen;
	u32 TotalLen ;
	u32 TransferredBytes ;
	u32 DataSize;

	/* Asserts validate the input arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Size != (u32)0x00U);

	CurrentPartialLen = (Size % 4);
	PrevPartialLen = InstancePtr->PartialLen;
	TotalLen = Size + PrevPartialLen;
	TransferredBytes = 0;

	/* If always Word Aligned Data and Word aligned address */
	if ((CurrentPartialLen == 0) && (PrevPartialLen == 0) &&
			(((UINTPTR)Data & XCSUDMA_ADDR_LSB_MASK) == 0)) {
		XSecure_Sha3DmaTransfer(InstancePtr, Data,Size);
	}
	/* For Non-Word Aligned Data and for Non-Word aligned Address*/
	else {
		if (TotalLen < XSECURE_SHA3_BLOCK_LEN) {

			/*
  			 * Copy the data to buffer when combined length
			 * does not exceed SHA3_BLOCK_LEN
			 */

			memcpy(&InstancePtr->PartialData[PrevPartialLen],
								Data, Size);
			InstancePtr->PartialLen = TotalLen;
		}
		else if (TotalLen == XSECURE_SHA3_BLOCK_LEN) {

			/*
 			 * Copy and transfer the data when combined length
			 * is equal to SHA3_BLOCK_LEN
			 */

			memcpy(&InstancePtr->PartialData[PrevPartialLen],
								Data, Size);
			XSecure_Sha3DmaTransfer(InstancePtr,
						InstancePtr->PartialData,
						XSECURE_SHA3_BLOCK_LEN);
			InstancePtr->PartialLen = 0 ;
			memset(&InstancePtr->PartialData, 0,
				sizeof(InstancePtr->PartialData));
			}
		else if (TotalLen > XSECURE_SHA3_BLOCK_LEN) {
			DataSize = Size;

			/*
 			 * Perform Multiple Dma transfers until
			 * Data Size < SHA3_BLOCK_LEN
			 */

			while (DataSize > XSECURE_SHA3_BLOCK_LEN) {
				memcpy(&InstancePtr->PartialData[PrevPartialLen],
						Data + TransferredBytes,
						(XSECURE_SHA3_BLOCK_LEN -
						PrevPartialLen ));
				XSecure_Sha3DmaTransfer(InstancePtr,
						InstancePtr->PartialData,
						XSECURE_SHA3_BLOCK_LEN);
				memset(&InstancePtr->PartialData, 0,
					sizeof(InstancePtr->PartialData));
				DataSize = DataSize - (XSECURE_SHA3_BLOCK_LEN -
						PrevPartialLen);
				TransferredBytes = TransferredBytes +
							XSECURE_SHA3_BLOCK_LEN -
							PrevPartialLen;
				PrevPartialLen = 0;
			}
			/*
 			 * Update PartialData and PartialLen based on
			 * size of the data remaining
			 */
			if (DataSize == 0) {
				InstancePtr->PartialLen = 0;
				memset(&InstancePtr->PartialData, 0,
						sizeof(InstancePtr->PartialData));
			}
			else if (DataSize < XSECURE_SHA3_BLOCK_LEN) {
				memcpy(InstancePtr->PartialData,
					Data + TransferredBytes,
							DataSize);
				InstancePtr->PartialLen = DataSize;
			}
		}
	}
}
