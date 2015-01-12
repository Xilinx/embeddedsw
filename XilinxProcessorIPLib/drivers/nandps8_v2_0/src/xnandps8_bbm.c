/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xnandps8_bbm.c
*
* This file implements the Bad Block Management (BBM) functionality.
* See xnandps8_bbm.h for more details.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date        Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 2.0   sb     11/04/2014  Added support for writing BBT signature and version
*			   in page section by enabling XNANDPS8_BBT_NO_OOB.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>	/**< For memcpy and memset */
#include "xil_types.h"
#include "xnandps8.h"
#include "xnandps8_bbm.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 XNandPs8_ReadBbt(XNandPs8 *InstancePtr, u32 Target);

static s32 XNandPs8_SearchBbt(XNandPs8 *InstancePtr, XNandPs8_BbtDesc *Desc,
							u32 Target);

static void XNandPs8_CreateBbt(XNandPs8 *InstancePtr, u32 Target);

static void XNandPs8_ConvertBbt(XNandPs8 *InstancePtr, u8 *Buf, u32 Target);

static s32 XNandPs8_WriteBbt(XNandPs8 *InstancePtr, XNandPs8_BbtDesc *Desc,
				XNandPs8_BbtDesc *MirrorDesc, u32 Target);

static s32 XNandPs8_MarkBbt(XNandPs8* InstancePtr, XNandPs8_BbtDesc *Desc,
							u32 Target);

static s32 XNandPs8_UpdateBbt(XNandPs8 *InstancePtr, u32 Target);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes the Bad Block Table(BBT) descriptors with a
* predefined pattern for searching Bad Block Table(BBT) in flash.
*
* @param	InstancePtr is a pointer to the XNandPs8 instance.
*
* @return
*		- NONE
*
******************************************************************************/
void XNandPs8_InitBbtDesc(XNandPs8 *InstancePtr)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Initialize primary Bad Block Table(BBT)
	 */
	for (Index = 0U; Index < XNANDPS8_MAX_TARGETS; Index++) {
		InstancePtr->BbtDesc.PageOffset[Index] =
						XNANDPS8_BBT_DESC_PAGE_OFFSET;
	}
	if (InstancePtr->EccMode == XNANDPS8_ONDIE) {
		InstancePtr->BbtDesc.SigOffset = XNANDPS8_ONDIE_SIG_OFFSET;
		InstancePtr->BbtDesc.VerOffset = XNANDPS8_ONDIE_VER_OFFSET;
	} else {
		InstancePtr->BbtDesc.SigOffset = XNANDPS8_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtDesc.VerOffset = XNANDPS8_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtDesc.SigLength = XNANDPS8_BBT_DESC_SIG_LEN;
	InstancePtr->BbtDesc.MaxBlocks = XNANDPS8_BBT_DESC_MAX_BLOCKS;
	(void)strcpy(&InstancePtr->BbtDesc.Signature[0], "Bbt0");
	for (Index = 0U; Index < XNANDPS8_MAX_TARGETS; Index++) {
		InstancePtr->BbtDesc.Version[Index] = 0U;
	}
	InstancePtr->BbtDesc.Valid = 0U;
	InstancePtr->BbtDesc.Option = XNANDPS8_BBT_OOB;

	/*
	 * Initialize mirror Bad Block Table(BBT)
	 */
	for (Index = 0U; Index < XNANDPS8_MAX_TARGETS; Index++) {
		InstancePtr->BbtMirrorDesc.PageOffset[Index] =
						XNANDPS8_BBT_DESC_PAGE_OFFSET;
	}
	if (InstancePtr->EccMode == XNANDPS8_ONDIE) {
		InstancePtr->BbtMirrorDesc.SigOffset =
						XNANDPS8_ONDIE_SIG_OFFSET;
		InstancePtr->BbtMirrorDesc.VerOffset =
						XNANDPS8_ONDIE_VER_OFFSET;
	} else {
		InstancePtr->BbtMirrorDesc.SigOffset =
						XNANDPS8_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtMirrorDesc.VerOffset =
						XNANDPS8_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtMirrorDesc.SigLength = XNANDPS8_BBT_DESC_SIG_LEN;
	InstancePtr->BbtMirrorDesc.MaxBlocks = XNANDPS8_BBT_DESC_MAX_BLOCKS;
	(void)strcpy(&InstancePtr->BbtMirrorDesc.Signature[0], "1tbB");
	for (Index = 0U; Index < XNANDPS8_MAX_TARGETS; Index++) {
		InstancePtr->BbtMirrorDesc.Version[Index] = 0U;
	}
	InstancePtr->BbtMirrorDesc.Valid = 0U;
	InstancePtr->BbtMirrorDesc.Option = XNANDPS8_BBT_OOB;

	/*
	 * Initialize Bad block search pattern structure
	 */
	if (InstancePtr->Geometry.BytesPerPage > 512U) {
		/* For flash page size > 512 bytes */
		InstancePtr->BbPattern.Options = XNANDPS8_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPS8_BB_PTRN_OFF_LARGE_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPS8_BB_PTRN_LEN_LARGE_PAGE;
	} else {
		InstancePtr->BbPattern.Options = XNANDPS8_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPS8_BB_PTRN_OFF_SML_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPS8_BB_PTRN_LEN_SML_PAGE;
	}
	for(Index = 0U; Index < XNANDPS8_BB_PTRN_LEN_LARGE_PAGE; Index++) {
		InstancePtr->BbPattern.Pattern[Index] = XNANDPS8_BB_PATTERN;
	}
}

/*****************************************************************************/
/**
* This function scans the NAND flash for factory marked bad blocks and creates
* a RAM based Bad Block Table(BBT).
*
* @param	InstancePtr is a pointer to the XNandPs8 instance.
*
* @return
*		- NONE
*
******************************************************************************/
static void XNandPs8_CreateBbt(XNandPs8 *InstancePtr, u32 Target)
{
	u32 BlockIndex;
	u32 PageIndex;
	u32 Length;
	u32 BlockOffset;
	u8 BlockShift;
	u32 NumPages;
	u32 Page;
	u8 Buf[XNANDPS8_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
	u32 StartBlock = Target * InstancePtr->Geometry.NumTargetBlocks;
	u32 NumBlocks = InstancePtr->Geometry.NumTargetBlocks;
	s32 Status;

	/*
	 * Number of pages to search for bad block pattern
	 */
	if ((InstancePtr->BbPattern.Options & XNANDPS8_BBT_SCAN_2ND_PAGE) != 0U)
	{
		NumPages = 2U;
	} else {
		NumPages = 1U;
	}
	/*
	 * Scan all the blocks for factory marked bad blocks
	 */
	for(BlockIndex = StartBlock; BlockIndex < (StartBlock + NumBlocks);
							BlockIndex++) {
		/*
		 * Block offset in Bad Block Table(BBT) entry
		 */
		BlockOffset = BlockIndex >> XNANDPS8_BBT_BLOCK_SHIFT;
		/*
		 * Block shift value in the byte
		 */
		BlockShift = XNandPs8_BbtBlockShift(BlockIndex);
		Page = BlockIndex * InstancePtr->Geometry.PagesPerBlock;
		/*
		 * Search for the bad block pattern
		 */
		for(PageIndex = 0U; PageIndex < NumPages; PageIndex++) {
			Status = XNandPs8_ReadSpareBytes(InstancePtr,
					(Page + PageIndex), &Buf[0]);

			if (Status != XST_SUCCESS) {
				/* Marking as bad block */
				InstancePtr->Bbt[BlockOffset] |=
					(u8)(XNANDPS8_BLOCK_FACTORY_BAD <<
					 BlockShift);
				break;
			}
			/*
			 * Read the spare bytes to check for bad block
			 * pattern
			 */
			for(Length = 0U; Length <
				InstancePtr->BbPattern.Length; Length++) {
				if (Buf[InstancePtr->BbPattern.Offset + Length]
						!=
					InstancePtr->BbPattern.Pattern[Length])
				{
					/* Bad block found */
					InstancePtr->Bbt[BlockOffset] |=
						(u8)
						(XNANDPS8_BLOCK_FACTORY_BAD <<
						 BlockShift);
					break;
				}
			}
		}
	}
}

/*****************************************************************************/
/**
* This function reads the Bad Block Table(BBT) if present in flash. If not it
* scans the flash for detecting factory marked bad blocks and creates a bad
* block table and write the Bad Block Table(BBT) into the flash.
*
* @param	InstancePtr is a pointer to the XNandPs8 instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPs8_ScanBbt(XNandPs8 *InstancePtr)
{
	s32 Status;
	u32 Index;
	u32 BbtLen;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Zero the RAM based Bad Block Table(BBT) entries
	 */
	BbtLen = InstancePtr->Geometry.NumBlocks >>
					XNANDPS8_BBT_BLOCK_SHIFT;
	memset(&InstancePtr->Bbt[0], 0, BbtLen);

	for (Index = 0U; Index < InstancePtr->Geometry.NumTargets; Index++) {

		if (XNandPs8_ReadBbt(InstancePtr, Index) != XST_SUCCESS) {
			/*
			 * Create memory based Bad Block Table(BBT)
			 */
			XNandPs8_CreateBbt(InstancePtr, Index);
			/*
			 * Write the Bad Block Table(BBT) to the flash
			 */
			Status = XNandPs8_WriteBbt(InstancePtr,
					&InstancePtr->BbtDesc,
					&InstancePtr->BbtMirrorDesc, Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/*
			 * Write the Mirror Bad Block Table(BBT) to the flash
			 */
			Status = XNandPs8_WriteBbt(InstancePtr,
					&InstancePtr->BbtMirrorDesc,
					&InstancePtr->BbtDesc, Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/*
			 * Mark the blocks containing Bad Block Table
			 * (BBT) as Reserved
			 */
			Status = XNandPs8_MarkBbt(InstancePtr,
							&InstancePtr->BbtDesc,
							Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			Status = XNandPs8_MarkBbt(InstancePtr,
						&InstancePtr->BbtMirrorDesc,
							Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function converts the Bad Block Table(BBT) read from the flash to the
* RAM based Bad Block Table(BBT).
*
* @param	InstancePtr is a pointer to the XNandPs8 instance.
* @param	Buf is the buffer which contains BBT read from flash.
*
* @return
*		- NONE.
*
******************************************************************************/
static void XNandPs8_ConvertBbt(XNandPs8 *InstancePtr, u8 *Buf, u32 Target)
{
	u32 BlockOffset;
	u8 BlockShift;
	u32 Data;
	u8 BlockType;
	u32 BlockIndex;
	u32 BbtLen = InstancePtr->Geometry.NumTargetBlocks >>
					XNANDPS8_BBT_BLOCK_SHIFT;
	u32 StartBlock = Target * InstancePtr->Geometry.NumTargetBlocks;

	for(BlockOffset = StartBlock; BlockOffset < (StartBlock + BbtLen);
						BlockOffset++) {
		Data = *(Buf + BlockOffset);
		/*
		 * Clear the RAM based Bad Block Table(BBT) contents
		 */
		InstancePtr->Bbt[BlockOffset] = 0x0U;
		/*
		 * Loop through the every 4 blocks in the bitmap
		 */
		for(BlockIndex = 0U; BlockIndex < XNANDPS8_BBT_ENTRY_NUM_BLOCKS;
				BlockIndex++) {
			BlockShift = XNandPs8_BbtBlockShift(BlockIndex);
			BlockType = (u8) ((Data >> BlockShift) &
				XNANDPS8_BLOCK_TYPE_MASK);
			switch(BlockType) {
				case XNANDPS8_FLASH_BLOCK_FAC_BAD:
					/* Factory bad block */
					InstancePtr->Bbt[BlockOffset] |=
						(u8)
						(XNANDPS8_BLOCK_FACTORY_BAD <<
						BlockShift);
					break;
				case XNANDPS8_FLASH_BLOCK_RESERVED:
					/* Reserved block */
					InstancePtr->Bbt[BlockOffset] |=
						(u8)
						(XNANDPS8_BLOCK_RESERVED <<
						BlockShift);
					break;
				case XNANDPS8_FLASH_BLOCK_BAD:
					/* Bad block due to wear */
					InstancePtr->Bbt[BlockOffset] |=
						(u8)(XNANDPS8_BLOCK_BAD <<
						BlockShift);
					break;
				default:
					/* Good block */
					/* The BBT entry already defaults to
					 * zero */
					break;
			}
		}
	}
}

/*****************************************************************************/
/**
* This function searches the Bad Bloock Table(BBT) in flash and loads into the
* memory based Bad Block Table(BBT).
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPs8_ReadBbt(XNandPs8 *InstancePtr, u32 Target)
{
	u64 Offset;
	u8 Buf[XNANDPS8_BBT_BUF_LENGTH]
					 __attribute__ ((aligned(64))) = {0U};
	s32 Status1;
	s32 Status2;
	s32 Status;
	u32 BufLen;
	u8 * BufPtr = Buf;

	XNandPs8_BbtDesc *Desc = &InstancePtr->BbtDesc;
	XNandPs8_BbtDesc *MirrorDesc = &InstancePtr->BbtMirrorDesc;
	BufLen = XNANDPS8_BBT_BUF_LENGTH;
	/*
	 * Search the Bad Block Table(BBT) in flash
	 */
	Status1 = XNandPs8_SearchBbt(InstancePtr, Desc, Target);
	Status2 = XNandPs8_SearchBbt(InstancePtr, MirrorDesc, Target);
	if ((Status1 != XST_SUCCESS) && (Status2 != XST_SUCCESS)) {
#ifdef XNANDPS8_DEBUG
		xil_printf("%s: Bad block table not found\r\n",__func__);
#endif
		Status = XST_FAILURE;
		goto Out;
	}
#ifdef XNANDPS8_DEBUG
	xil_printf("%s: Bad block table found\r\n",__func__);
#endif
	/*
	 * Bad Block Table found
	 */
	if ((Desc->Valid != 0U) && (MirrorDesc->Valid != 0U)) {
		/*
		 * Valid BBT & Mirror BBT found
		 */
		if (Desc->Version[Target] > MirrorDesc->Version[Target]) {
			Offset = (u64)Desc->PageOffset[Target] *
				(u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPs8_Read(InstancePtr, Offset, BufLen,
								&BufPtr[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}

			if (Desc->Option == XNANDPS8_BBT_NO_OOB){
				BufPtr = BufPtr + Desc->VerOffset +
						XNANDPS8_BBT_VERSION_LENGTH;
			}
			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs8_ConvertBbt(InstancePtr, &BufPtr[0], Target);
			MirrorDesc->Version[Target] = Desc->Version[Target];

			/*
			 * Write the BBT to Mirror BBT location in flash
			 */
			Status = XNandPs8_WriteBbt(InstancePtr, MirrorDesc,
							Desc, Target);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
		} else if (Desc->Version[Target] <
						MirrorDesc->Version[Target]) {
			Offset = (u64)MirrorDesc->PageOffset[Target] *
				(u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPs8_Read(InstancePtr, Offset, BufLen,
								&BufPtr[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			if(Desc->Option == XNANDPS8_BBT_NO_OOB){
				BufPtr = BufPtr + Desc->VerOffset +
						XNANDPS8_BBT_VERSION_LENGTH;
			}
			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs8_ConvertBbt(InstancePtr, &BufPtr[0], Target);
			Desc->Version[Target] = MirrorDesc->Version[Target];

			/*
			 * Write the Mirror BBT to BBT location in flash
			 */
			Status = XNandPs8_WriteBbt(InstancePtr, Desc,
							MirrorDesc, Target);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
		} else {
			/* Both are up-to-date */
			Offset = (u64)Desc->PageOffset[Target] *
				(u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPs8_Read(InstancePtr, Offset, BufLen,
								&BufPtr[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}

			if(Desc->Option == XNANDPS8_BBT_NO_OOB){
				BufPtr = BufPtr + Desc->VerOffset +
						XNANDPS8_BBT_VERSION_LENGTH;
			}

			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs8_ConvertBbt(InstancePtr, &BufPtr[0], Target);
		}
	} else if (Desc->Valid != 0U) {
		/*
		 * Valid Primary BBT found
		 */
		Offset = (u64)Desc->PageOffset[Target] *
			(u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPs8_Read(InstancePtr, Offset, BufLen, &BufPtr[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		if(Desc->Option == XNANDPS8_BBT_NO_OOB){
			BufPtr = BufPtr + Desc->VerOffset +
				XNANDPS8_BBT_VERSION_LENGTH;
		}
		/*
		 * Convert flash BBT to memory based BBT
		 */
		XNandPs8_ConvertBbt(InstancePtr, &BufPtr[0], Target);
		MirrorDesc->Version[Target] = Desc->Version[Target];

		/*
		 * Write the BBT to Mirror BBT location in flash
		 */
		Status = XNandPs8_WriteBbt(InstancePtr, MirrorDesc, Desc,
								Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	} else {
		/*
		 * Valid Mirror BBT found
		 */
		Offset = (u64)MirrorDesc->PageOffset[Target] *
			(u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPs8_Read(InstancePtr, Offset, BufLen, &BufPtr[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		if(Desc->Option == XNANDPS8_BBT_NO_OOB){
			BufPtr = BufPtr + Desc->VerOffset +
				XNANDPS8_BBT_VERSION_LENGTH;
		}

		/*
		 * Convert flash BBT to memory based BBT
		 */
		XNandPs8_ConvertBbt(InstancePtr, &BufPtr[0], Target);
		Desc->Version[Target] = MirrorDesc->Version[Target];

		/*
		 * Write the Mirror BBT to BBT location in flash
		 */
		Status = XNandPs8_WriteBbt(InstancePtr, Desc, MirrorDesc,
								Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function searches the BBT in flash.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @param	Desc is the BBT descriptor pattern to search.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPs8_SearchBbt(XNandPs8 *InstancePtr, XNandPs8_BbtDesc *Desc,
								u32 Target)
{
	u32 StartBlock;
	u32 SigOffset;
	u32 VerOffset;
	u32 MaxBlocks;
	u32 PageOff;
	u32 SigLength;
	u8 Buf[XNANDPS8_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
	u32 Block;
	u32 Offset;
	s32 Status;
	u64 BlockOff;

	StartBlock = ((Target + (u32)1) *
				InstancePtr->Geometry.NumTargetBlocks) - (u32)1;
	SigOffset = Desc->SigOffset;
	VerOffset = Desc->VerOffset;
	MaxBlocks = Desc->MaxBlocks;
	SigLength = Desc->SigLength;

	/*
	 * Read the last 4 blocks for Bad Block Table(BBT) signature
	 */
	for(Block = 0U; Block < MaxBlocks; Block++) {
		PageOff = (StartBlock - Block) *
			InstancePtr->Geometry.PagesPerBlock;

		if(Desc->Option == XNANDPS8_BBT_NO_OOB){
			BlockOff = (u64)PageOff * (u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPs8_Read(InstancePtr, BlockOff,
				Desc->SigLength + Desc->SigOffset , &Buf[0]);
		}else{
			Status = XNandPs8_ReadSpareBytes(InstancePtr, PageOff, &Buf[0]);
		}
		if (Status != XST_SUCCESS) {
			continue;
		}
		/*
		 * Check the Bad Block Table(BBT) signature
		 */
		for(Offset = 0U; Offset < SigLength; Offset++) {
			if (Buf[Offset + SigOffset] !=
				(u8)(Desc->Signature[Offset]))
			{
				break; /* Check the next blocks */
			}
		}
		if (Offset >= SigLength) {
			/*
			 * Bad Block Table(BBT) found
			 */
			Desc->PageOffset[Target] = PageOff;
			Desc->Version[Target] = Buf[VerOffset];
			Desc->Valid = 1U;

			Status = XST_SUCCESS;
			goto Out;
		}
	}
	/*
	 * Bad Block Table(BBT) not found
	 */
	Status = XST_FAILURE;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function writes Bad Block Table(BBT) from RAM to flash.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @param	Desc is the BBT descriptor to be written to flash.
* @param	MirrorDesc is the mirror BBT descriptor.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPs8_WriteBbt(XNandPs8 *InstancePtr, XNandPs8_BbtDesc *Desc,
				XNandPs8_BbtDesc *MirrorDesc, u32 Target)
{
	u64 Offset;
	u32 Block = {0U};
	u32 EndBlock = ((Target + (u32)1) *
			InstancePtr->Geometry.NumTargetBlocks) - (u32)1;
	u8 Buf[XNANDPS8_BBT_BUF_LENGTH]
					 __attribute__ ((aligned(64))) = {0U};
	u32 BufLen = XNANDPS8_BBT_BUF_LENGTH;
	u8 SpareBuf[XNANDPS8_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
	u8 Mask[4] = {0x00U, 0x01U, 0x02U, 0x03U};
	u8 Data;
	u32 BlockOffset;
	u8 BlockShift;
	s32 Status;
	u32 BlockIndex;
	u32 Index;
	u8 BlockType;
	u32 BbtLen = InstancePtr->Geometry.NumBlocks >>
						XNANDPS8_BBT_BLOCK_SHIFT;
	u8* BufPtr = Buf;
	/*
	 * Find a valid block to write the Bad Block Table(BBT)
	 */
	if ((!Desc->Valid) != 0U) {
		for(Index = 0U; Index < Desc->MaxBlocks; Index++) {
			Block  = (EndBlock - Index);
			BlockOffset = Block >> XNANDPS8_BBT_BLOCK_SHIFT;
			BlockShift = XNandPs8_BbtBlockShift(Block);
			BlockType = (InstancePtr->Bbt[BlockOffset] >>
					BlockShift) & XNANDPS8_BLOCK_TYPE_MASK;
			switch(BlockType)
			{
				case XNANDPS8_BLOCK_BAD:
				case XNANDPS8_BLOCK_FACTORY_BAD:
					continue;
				default:
					/* Good Block */
					break;
			}
			Desc->PageOffset[Target] = Block *
				InstancePtr->Geometry.PagesPerBlock;
			if (Desc->PageOffset[Target] !=
					MirrorDesc->PageOffset[Target]) {
				/* Free block found */
				Desc->Valid = 1U;
				break;
			}
		}

		/*
		 * Block not found for writing Bad Block Table(BBT)
		 */
		if (Index >= Desc->MaxBlocks) {
#ifdef XNANDPS8_DEBUG
		xil_printf("%s: Blocks unavailable for writing BBT\r\n",
								__func__);
#endif
			Status = XST_FAILURE;
			goto Out;
		}
	} else {
		Block = Desc->PageOffset[Target] /
					InstancePtr->Geometry.PagesPerBlock;
	}
	/*
	 * Convert the memory based BBT to flash based table
	 */
	memset(Buf, 0xff, BufLen);

	if(Desc->Option == XNANDPS8_BBT_NO_OOB){
		BufPtr = BufPtr + Desc->VerOffset + XNANDPS8_BBT_VERSION_LENGTH;
	}
	/*
	 * Loop through the number of blocks
	 */
	for(BlockOffset = 0U; BlockOffset < BufLen; BlockOffset++) {
		Data = InstancePtr->Bbt[BlockOffset];
		/*
		 * Calculate the bit mask for 4 blocks at a time in loop
		 */
		for(BlockIndex = 0U; BlockIndex < XNANDPS8_BBT_ENTRY_NUM_BLOCKS;
				BlockIndex++) {
			BlockShift = XNandPs8_BbtBlockShift(BlockIndex);
			BufPtr[BlockOffset] &= ~(Mask[Data &
					XNANDPS8_BLOCK_TYPE_MASK] <<
					BlockShift);
			Data >>= XNANDPS8_BBT_BLOCK_SHIFT;
		}
	}
	/*
	 * Write the Bad Block Table(BBT) to flash
	 */
	Status = XNandPs8_EraseBlock(InstancePtr, 0U, Block);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	if(Desc->Option == XNANDPS8_BBT_NO_OOB){
		/*
		 * Copy the signature and version to the Buffer
		 */
		memcpy(Buf + Desc->SigOffset, &Desc->Signature[0],
							Desc->SigLength);
		memcpy(Buf + Desc->VerOffset, &Desc->Version[Target], 1U);
		/*
		 * Write the Buffer to page offset
		 */
		Offset = (u64)Desc->PageOffset[Target] *
				(u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPs8_Write(InstancePtr, Offset, BufLen, &Buf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}else{
		/*
		 * Write the BBT to page offset
		 */
		Offset = (u64)Desc->PageOffset[Target] *
				(u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPs8_Write(InstancePtr, Offset, BbtLen, &Buf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		/*
		 * Write the signature and version in the spare data area
		 */
		memset(SpareBuf, 0xff, InstancePtr->Geometry.SpareBytesPerPage);
		Status = XNandPs8_ReadSpareBytes(InstancePtr, Desc->PageOffset[Target],
				&SpareBuf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}

		memcpy(SpareBuf + Desc->SigOffset, &Desc->Signature[0],
							Desc->SigLength);
		memcpy(SpareBuf + Desc->VerOffset, &Desc->Version[Target], 1U);

		Status = XNandPs8_WriteSpareBytes(InstancePtr,
				Desc->PageOffset[Target], &SpareBuf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function updates the primary and mirror Bad Block Table(BBT) in the
* flash.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPs8_UpdateBbt(XNandPs8 *InstancePtr, u32 Target)
{
	s32 Status;
	u8 Version;

	/*
	 * Update the version number
	 */
	Version = InstancePtr->BbtDesc.Version[Target];
	InstancePtr->BbtDesc.Version[Target] = (u8)(((u16)Version +
							(u16)1) % (u16)256U);

	Version = InstancePtr->BbtMirrorDesc.Version[Target];
	InstancePtr->BbtMirrorDesc.Version[Target] = (u8)(((u16)Version +
							(u16)1) % (u16)256);
	/*
	 * Update the primary Bad Block Table(BBT) in flash
	 */
	Status = XNandPs8_WriteBbt(InstancePtr, &InstancePtr->BbtDesc,
						&InstancePtr->BbtMirrorDesc,
						Target);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	/*
	 * Update the mirrored Bad Block Table(BBT) in flash
	 */
	Status = XNandPs8_WriteBbt(InstancePtr, &InstancePtr->BbtMirrorDesc,
						&InstancePtr->BbtDesc,
						Target);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function marks the block containing Bad Block Table as reserved
* and updates the BBT.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @param	Desc is the BBT descriptor pointer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPs8_MarkBbt(XNandPs8* InstancePtr, XNandPs8_BbtDesc *Desc,
								u32 Target)
{
	u32 BlockIndex;
	u32 BlockOffset;
	u8 BlockShift;
	u8 OldVal;
	u8 NewVal;
	s32 Status;
	u32 UpdateBbt = 0U;
	u32 Index;

	/*
	 * Mark the last four blocks as Reserved
	 */
	BlockIndex = ((Target + (u32)1) * InstancePtr->Geometry.NumTargetBlocks) -
						Desc->MaxBlocks - (u32)1;

	for(Index = 0U; Index < Desc->MaxBlocks; Index++) {

		BlockOffset = BlockIndex >> XNANDPS8_BBT_BLOCK_SHIFT;
		BlockShift = XNandPs8_BbtBlockShift(BlockIndex);
		OldVal = InstancePtr->Bbt[BlockOffset];
		NewVal = (u8) (OldVal | (XNANDPS8_BLOCK_RESERVED <<
							BlockShift));
		InstancePtr->Bbt[BlockOffset] = NewVal;

		if (OldVal != NewVal) {
			UpdateBbt = 1U;
		}
		BlockIndex++;
	}

	/*
	 * Update the BBT to flash
	 */
	if (UpdateBbt != 0U) {
		Status = XNandPs8_UpdateBbt(InstancePtr, Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}

/*****************************************************************************/
/**
*
* This function checks whether a block is bad or not.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
*
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPs8_IsBlockBad(XNandPs8 *InstancePtr, u32 Block)
{
	u8 Data;
	u8 BlockShift;
	u8 BlockType;
	u32 BlockOffset;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	BlockOffset = Block >> XNANDPS8_BBT_BLOCK_SHIFT;
	BlockShift = XNandPs8_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */
	BlockType = (Data >> BlockShift) & XNANDPS8_BLOCK_TYPE_MASK;

	if ((BlockType != XNANDPS8_BLOCK_GOOD) &&
		(BlockType != XNANDPS8_BLOCK_RESERVED)) {
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
* This function marks a block as bad in the RAM based Bad Block Table(BBT). It
* also updates the Bad Block Table(BBT) in the flash.
*
* @param	InstancePtr is the pointer to the XNandPs8 instance.
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPs8_MarkBlockBad(XNandPs8 *InstancePtr, u32 Block)
{
	u8 Data;
	u8 BlockShift;
	u32 BlockOffset;
	u8 OldVal;
	u8 NewVal;
	s32 Status;
	u32 Target;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	Target = Block % InstancePtr->Geometry.NumTargetBlocks;

	BlockOffset = Block >> XNANDPS8_BBT_BLOCK_SHIFT;
	BlockShift = XNandPs8_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */

	/*
	 * Mark the block as bad in the RAM based Bad Block Table
	 */
	OldVal = Data;
	Data &= ~(XNANDPS8_BLOCK_TYPE_MASK << BlockShift);
	Data |= (XNANDPS8_BLOCK_BAD << BlockShift);
	NewVal = Data;
	InstancePtr->Bbt[BlockOffset] = Data;

	/*
	 * Update the Bad Block Table(BBT) in flash
	 */
	if (OldVal != NewVal) {
		Status = XNandPs8_UpdateBbt(InstancePtr, Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}
