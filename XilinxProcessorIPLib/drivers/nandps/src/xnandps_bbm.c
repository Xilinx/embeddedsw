/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_bbm.c
* @addtogroup nandps Overview
* @{
* This file implements the Bad Block Management (BBM) functionality.
* See xnandps_bbm.h for more details.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.00a nm     12/10/2010  First release
* 1.03a nm     10/22/2012  Fixed CR# 683787.
* 2.4   nsk    06/20/2018  Fixed BBT offset overflow in XNandPs_WriteBbt()
*			   and XNandPs_ReadBbt(), overflow causes incorrect
*			   BBT writes.
* 2.9   sb     01/03/2024  Use FlashBbt array of nandps structure instead
*                          of local Buf array in XNandPs_WriteBbt() and XNandPs_ReadBbt(),
*                          to avoid stack overflow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>	/**< For memcpy and memset */
#include "xil_types.h"
#include "xnandps_bbm.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XNandPs_ReadBbt(XNandPs *InstancePtr);

static int XNandPs_SearchBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc);

static void XNandPs_CreateBbt(XNandPs *InstancePtr);

static void XNandPs_ConvertBbt(XNandPs *InstancePtr, u8 *Buf);

static int XNandPs_WriteBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc,
			    XNandPs_BbtDesc *MirrorDesc);

static int XNandPs_MarkBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc);

static int XNandPs_UpdateBbt(XNandPs *InstancePtr);

extern int XNandPs_ReadSpareBytes(XNandPs *InstancePtr, u32 Page, u8 *Buf);
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes the Bad Block Table(BBT) descriptors with a
* predefined pattern for searching Bad Block Table(BBT) in flash.
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- NONE
*
******************************************************************************/
void XNandPs_InitBbtDesc(XNandPs *InstancePtr)
{
	int Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Initialize primary Bad Block Table(BBT)
	 */
	InstancePtr->BbtDesc.PageOffset = XNANDPS_BBT_DESC_PAGE_OFFSET;
	if (InstancePtr->EccMode == XNANDPS_ECC_ONDIE) {
		InstancePtr->BbtDesc.SigOffset = 0x4;
		InstancePtr->BbtDesc.VerOffset = 0x14;
	} else {
		InstancePtr->BbtDesc.SigOffset = XNANDPS_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtDesc.VerOffset = XNANDPS_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtDesc.SigLength = XNANDPS_BBT_DESC_SIG_LEN;
	InstancePtr->BbtDesc.MaxBlocks = XNANDPS_BBT_DESC_MAX_BLOCKS;
	strcpy(&InstancePtr->BbtDesc.Signature[0], "Bbt0");
	InstancePtr->BbtDesc.Version = 0;
	InstancePtr->BbtDesc.Valid = 0;

	/*
	 * Initialize mirror Bad Block Table(BBT)
	 */
	InstancePtr->BbtMirrorDesc.PageOffset = XNANDPS_BBT_DESC_PAGE_OFFSET;
	if (InstancePtr->EccMode == XNANDPS_ECC_ONDIE) {
		InstancePtr->BbtMirrorDesc.SigOffset = 0x4;
		InstancePtr->BbtMirrorDesc.VerOffset = 0x14;
	} else {
		InstancePtr->BbtMirrorDesc.SigOffset = XNANDPS_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtMirrorDesc.VerOffset = XNANDPS_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtMirrorDesc.SigLength = XNANDPS_BBT_DESC_SIG_LEN;
	InstancePtr->BbtMirrorDesc.MaxBlocks = XNANDPS_BBT_DESC_MAX_BLOCKS;
	strcpy(&InstancePtr->BbtMirrorDesc.Signature[0], "1tbB");
	InstancePtr->BbtMirrorDesc.Version = 0;
	InstancePtr->BbtMirrorDesc.Valid = 0;

	/*
	 * Initialize Bad block search pattern structure
	 */
	if (InstancePtr->Geometry.BytesPerPage > 512) {
		/* For flash page size > 512 bytes */
		InstancePtr->BbPattern.Options = XNANDPS_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPS_BB_PATTERN_OFFSET_LARGE_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPS_BB_PATTERN_LENGTH_LARGE_PAGE;
	} else {
		InstancePtr->BbPattern.Options = XNANDPS_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPS_BB_PATTERN_OFFSET_SMALL_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPS_BB_PATTERN_LENGTH_SMALL_PAGE;
	}
	for (Index = 0; Index < XNANDPS_BB_PATTERN_LENGTH_LARGE_PAGE; Index++) {
		InstancePtr->BbPattern.Pattern[Index] = XNANDPS_BB_PATTERN;
	}
}
/*****************************************************************************/
/**
* This function scans the NAND flash for factory marked bad blocks and creates
* a RAM based Bad Block Table(BBT).
*
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- NONE
*
******************************************************************************/
static void XNandPs_CreateBbt(XNandPs *InstancePtr)
{
	u32 BlockIndex;
	u32 PageIndex;
	u32 Length;
	u32 BlockOffset;
	u32 BlockShift;
	u32 NumPages;
	u32 Page;
	u8 Buf[XNANDPS_MAX_SPARE_SIZE];
	u32 BbtLen = InstancePtr->Geometry.NumBlocks >>
		     XNANDPS_BBT_BLOCK_SHIFT;
	int Status;

	/*
	 * Number of pages to search for bad block pattern
	 */
	if (InstancePtr->BbPattern.Options & XNANDPS_BBT_SCAN_2ND_PAGE) {
		NumPages = 2;
	} else {
		NumPages = 1;
	}

	/*
	 * Zero the RAM based Bad Block Table(BBT) entries
	 */
	memset(&InstancePtr->Bbt[0], 0, BbtLen);

	/*
	 * Scan all the blocks for factory marked bad blocks
	 */
	for (BlockIndex = 0; BlockIndex <
	     InstancePtr->Geometry.NumBlocks; BlockIndex++) {
		/*
		 * Block offset in Bad Block Table(BBT) entry
		 */
		BlockOffset = BlockIndex >> XNANDPS_BBT_BLOCK_SHIFT;
		/*
		 * Block shift value in the byte
		 */
		BlockShift = XNandPs_BbtBlockShift(BlockIndex);
		Page = BlockIndex * InstancePtr->Geometry.PagesPerBlock;

		/*
		 * Search for the bad block pattern
		 */
		for (PageIndex = 0; PageIndex < NumPages; PageIndex++) {
			Status = XNandPs_ReadSpareBytes(InstancePtr,
							(Page + PageIndex), &Buf[0]);

			if (Status != XST_SUCCESS) {
				/* Marking as bad block */
				InstancePtr->Bbt[BlockOffset] |=
					(XNANDPS_BLOCK_FACTORY_BAD <<
					 BlockShift);
				break;
			}

			/*
			 * Read the spare bytes to check for bad block
			 * pattern
			 */
			for (Length = 0; Length <
			     InstancePtr->BbPattern.Length; Length++) {
				if (Buf[InstancePtr->BbPattern.Offset + Length]
				    !=
				    InstancePtr->BbPattern.Pattern[Length]) {
					/* Bad block found */
					InstancePtr->Bbt[BlockOffset] |=
						(XNANDPS_BLOCK_FACTORY_BAD <<
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
* @param	InstancePtr is a pointer to the XNandPs instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
int XNandPs_ScanBbt(XNandPs *InstancePtr)
{
	int Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (XNandPs_ReadBbt(InstancePtr) != XST_SUCCESS) {
		/*
		 * Create memory based Bad Block Table(BBT)
		 */
		XNandPs_CreateBbt(InstancePtr);

		/*
		 * Write the Bad Block Table(BBT) to the flash
		 */
		Status = XNandPs_WriteBbt(InstancePtr,
					  &InstancePtr->BbtDesc,
					  &InstancePtr->BbtMirrorDesc);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Write the Mirror Bad Block Table(BBT) to the flash
		 */
		Status = XNandPs_WriteBbt(InstancePtr,
					  &InstancePtr->BbtMirrorDesc,
					  &InstancePtr->BbtDesc);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Mark the blocks containing Bad Block Table(BBT) as Reserved
		 */
		XNandPs_MarkBbt(InstancePtr, &InstancePtr->BbtDesc);
		XNandPs_MarkBbt(InstancePtr, &InstancePtr->BbtMirrorDesc);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function converts the Bad Block Table(BBT) read from the flash to the RAM
* based Bad Block Table(BBT).
*
* @param	InstancePtr is a pointer to the XNandPs instance.
* @param	Buf is the buffer which contains BBT read from flash.
*
* @return
*		- NONE.
*
******************************************************************************/
static void XNandPs_ConvertBbt(XNandPs *InstancePtr, u8 *Buf)
{
	u32 BlockOffset;
	u32 BlockShift;
	u32 Data;
	u8 BlockType;
	u32 BlockIndex;
	u32 BbtLen = InstancePtr->Geometry.NumBlocks >>
		     XNANDPS_BBT_BLOCK_SHIFT;

	for (BlockOffset = 0; BlockOffset < BbtLen; BlockOffset++) {
		Data = Buf[BlockOffset];

		/*
		 * Clear the RAM based Bad Block Table(BBT) contents
		 */
		InstancePtr->Bbt[BlockOffset] = 0x0;

		/*
		 * Loop through the every 4 blocks in the bitmap
		 */
		for (BlockIndex = 0; BlockIndex < XNANDPS_BBT_ENTRY_NUM_BLOCKS;
		     BlockIndex++) {
			BlockShift = XNandPs_BbtBlockShift(BlockIndex);
			BlockType = (Data >> BlockShift) &
				    XNANDPS_BLOCK_TYPE_MASK;
			switch (BlockType) {
				case XNANDPS_FLASH_BLOCK_FACTORY_BAD:
					/* Factory bad block */
					InstancePtr->Bbt[BlockOffset] |=
						XNANDPS_BLOCK_FACTORY_BAD <<
						BlockShift;
					break;
				case XNANDPS_FLASH_BLOCK_RESERVED:
					/* Reserved block */
					InstancePtr->Bbt[BlockOffset] |=
						XNANDPS_BLOCK_RESERVED <<
						BlockShift;
					break;
				case XNANDPS_FLASH_BLOCK_BAD:
					/* Bad block due to wear */
					InstancePtr->Bbt[BlockOffset] |=
						XNANDPS_BLOCK_BAD <<
						BlockShift;
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
* @param	InstancePtr is the pointer to the XNandPs instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static int XNandPs_ReadBbt(XNandPs *InstancePtr)
{
	u64 Offset;
	u32 Status1;
	u32 Status2;
	u32 Status;
	u32 BbtLen;

	XNandPs_BbtDesc *Desc = &InstancePtr->BbtDesc;
	XNandPs_BbtDesc *MirrorDesc = &InstancePtr->BbtMirrorDesc;
	BbtLen = InstancePtr->Geometry.NumBlocks >> XNANDPS_BBT_BLOCK_SHIFT;

	/*
	 * Search the Bad Block Table(BBT) in flash
	 */
	Status1 = XNandPs_SearchBbt(InstancePtr, Desc);
	Status2 = XNandPs_SearchBbt(InstancePtr, MirrorDesc);
	if ((Status1 != XST_SUCCESS) && (Status2 != XST_SUCCESS)) {
		return XST_FAILURE;
	}

	/*
	 * Bad Block Table found
	 */
	if (Desc->Valid && MirrorDesc->Valid) {
		/*
		 * Valid BBT & Mirror BBT found
		 */
		if (Desc->Version > MirrorDesc->Version) {
			Offset = ((u64)Desc->PageOffset) *
				 ((u64)InstancePtr->Geometry.BytesPerPage);
			XNandPs_Read(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, NULL);

			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs_ConvertBbt(InstancePtr, InstancePtr->FlashBbt);
			MirrorDesc->Version = Desc->Version;

			/*
			 * Write the BBT to Mirror BBT location in flash
			 */
			Status = XNandPs_WriteBbt(InstancePtr, MirrorDesc,
						  Desc);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		} else if (Desc->Version < MirrorDesc->Version) {
			Offset = ((u64)MirrorDesc->PageOffset) *
				 ((u64)InstancePtr->Geometry.BytesPerPage);
			XNandPs_Read(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, NULL);

			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs_ConvertBbt(InstancePtr, InstancePtr->FlashBbt);
			Desc->Version = MirrorDesc->Version;

			/*
			 * Write the Mirror BBT to BBT location in flash
			 */
			Status = XNandPs_WriteBbt(InstancePtr, Desc,
						  MirrorDesc);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		} else {
			/* Both are up-to-date */
			Offset = ((u64)Desc->PageOffset) *
				 ((u64)InstancePtr->Geometry.BytesPerPage);
			XNandPs_Read(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, NULL);

			/*
			 * Convert flash BBT to memory based BBT
			 */
			XNandPs_ConvertBbt(InstancePtr, InstancePtr->FlashBbt);
		}
	} else if (Desc->Valid) {
		/*
		 * Valid Primary BBT found
		 */
		Offset = ((u64)Desc->PageOffset) * ((u64)InstancePtr->Geometry.BytesPerPage);
		XNandPs_Read(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, NULL);

		/*
		 * Convert flash BBT to memory based BBT
		 */
		XNandPs_ConvertBbt(InstancePtr, InstancePtr->FlashBbt);
		MirrorDesc->Version = Desc->Version;

		/*
		 * Write the BBT to Mirror BBT location in flash
		 */
		Status = XNandPs_WriteBbt(InstancePtr, MirrorDesc, Desc);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	} else {
		/*
		 * Valid Mirror BBT found
		 */
		Offset = ((u64)MirrorDesc->PageOffset) *
			 ((u64)InstancePtr->Geometry.BytesPerPage);
		XNandPs_Read(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, NULL);

		/*
		 * Convert flash BBT to memory based BBT
		 */
		XNandPs_ConvertBbt(InstancePtr, InstancePtr->FlashBbt);
		Desc->Version = MirrorDesc->Version;

		/*
		 * Write the Mirror BBT to BBT location in flash
		 */
		Status = XNandPs_WriteBbt(InstancePtr, Desc, MirrorDesc);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function searches the BBT in flash.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Desc is the BBT descriptor pattern to search.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static int XNandPs_SearchBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc)
{
	u32 StartBlock;
	u32 SigOffset;
	u32 VerOffset;
	u32 MaxBlocks;
	u32 PageOff;
	u32 SigLength;
	u8 Buf[XNANDPS_MAX_SPARE_SIZE];
	u32 Block;
	u32 Offset;
	int Status;

	StartBlock = InstancePtr->Geometry.NumBlocks - 1;
	SigOffset = Desc->SigOffset;
	VerOffset = Desc->VerOffset;
	MaxBlocks = Desc->MaxBlocks;
	SigLength = Desc->SigLength;

	/*
	 * Read the last 4 blocks for Bad Block Table(BBT) signature
	 */
	for (Block = 0; Block < MaxBlocks; Block++) {
		PageOff = (StartBlock - Block) *
			  InstancePtr->Geometry.PagesPerBlock;

		Status = XNandPs_ReadSpareBytes(InstancePtr, PageOff, &Buf[0]);
		if (Status != XST_SUCCESS) {
			continue;
		}

		/*
		 * Check the Bad Block Table(BBT) signature
		 */
		for (Offset = 0; Offset < SigLength; Offset++) {
			if (Buf[Offset + SigOffset] != Desc->Signature[Offset]) {
				break; /* Check the next blocks */
			}
		}
		if (Offset >= SigLength) {
			/*
			 * Bad Block Table(BBT) found
			 */
			Desc->PageOffset = PageOff;
			Desc->Version = Buf[VerOffset];
			Desc->Valid = 1;
			return XST_SUCCESS;
		}
	}
	/*
	 * Bad Block Table(BBT) not found
	 */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
* This function writes Bad Block Table(BBT) from RAM to flash.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Desc is the BBT descriptor to be written to flash.
* @param	MirrorDesc is the mirror BBT descriptor.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static int XNandPs_WriteBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc,
			    XNandPs_BbtDesc *MirrorDesc)
{
	u64 Offset;
	u32 Block = InstancePtr->Geometry.NumBlocks - 1;
	u8 SpareBuf[XNANDPS_MAX_SPARE_SIZE];
	u8 Mask[4] = {0x00, 0x01, 0x02, 0x03};
	u8 Data;
	u32 BlockOffset;
	u32 BlockShift;
	u32 Status;
	u32 BlockIndex;
	u32 Index;
	u8 BlockType;
	u32 BbtLen = InstancePtr->Geometry.NumBlocks >>
		     XNANDPS_BBT_BLOCK_SHIFT;
	/*
	 * Find a valid block to write the Bad Block Table(BBT)
	 */
	if (!Desc->Valid) {
		for (Index = 0; Index < Desc->MaxBlocks; Index++) {
			Block  = (Block - Index);
			BlockOffset = Block >> XNANDPS_BBT_BLOCK_SHIFT;
			BlockShift = XNandPs_BbtBlockShift(Block);
			BlockType = (InstancePtr->Bbt[BlockOffset] >>
				     BlockShift) & XNANDPS_BLOCK_TYPE_MASK;
			switch (BlockType) {
				case XNANDPS_BLOCK_BAD:
				case XNANDPS_BLOCK_FACTORY_BAD:
					continue;
				default:
					/* Good Block */
					break;
			}
			Desc->PageOffset = Block *
					   InstancePtr->Geometry.PagesPerBlock;
			if (Desc->PageOffset != MirrorDesc->PageOffset) {
				/* Free block found */
				Desc->Valid = 1;
				break;
			}
		}

		/*
		 * Block not found for writing Bad Block Table(BBT)
		 */
		if (Index >= Desc->MaxBlocks) {
			return XST_FAILURE;
		}
	} else {
		Block = Desc->PageOffset / InstancePtr->Geometry.PagesPerBlock;
	}

	/*
	 * Convert the memory based BBT to flash based table
	 */
	memset(InstancePtr->FlashBbt, 0xff, BbtLen);

	/*
	 * Loop through the number of blocks
	 */
	for (BlockOffset = 0; BlockOffset < BbtLen; BlockOffset++) {
		Data = InstancePtr->Bbt[BlockOffset];
		/*
		 * Calculate the bit mask for 4 blocks at a time in loop
		 */
		for (BlockIndex = 0; BlockIndex < XNANDPS_BBT_ENTRY_NUM_BLOCKS;
		     BlockIndex++) {
			BlockShift = XNandPs_BbtBlockShift(BlockIndex);
			InstancePtr->FlashBbt[BlockOffset] &= ~(Mask[Data &
						   XNANDPS_BLOCK_TYPE_MASK] <<
					      BlockShift);
			Data >>= XNANDPS_BBT_BLOCK_SHIFT;
		}
	}

	/*
	 * Write the Bad Block Table(BBT) to flash
	 */
	Status = XNandPs_EraseBlock(InstancePtr, Block);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Write the signature and version in the spare data area
	 */
	memset(SpareBuf, 0xff, InstancePtr->Geometry.SpareBytesPerPage);
	memcpy(SpareBuf + Desc->SigOffset, &Desc->Signature[0],
	       Desc->SigLength);
	memcpy(SpareBuf + Desc->VerOffset, &Desc->Version, 1);

	/*
	 * Write the BBT to page offset
	 */
	Offset = ((u64)Desc->PageOffset) * ((u64)InstancePtr->Geometry.BytesPerPage);
	Status = XNandPs_Write(InstancePtr, Offset, BbtLen, InstancePtr->FlashBbt, SpareBuf);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function updates the primary and mirror Bad Block Table(BBT) in the
* flash.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static int XNandPs_UpdateBbt(XNandPs *InstancePtr)
{
	int Status;
	u8 Version;

	/*
	 * Update the version number
	 */
	Version = InstancePtr->BbtDesc.Version;
	InstancePtr->BbtDesc.Version = (Version + 1) % 256;
	Version = InstancePtr->BbtMirrorDesc.Version;
	InstancePtr->BbtMirrorDesc.Version = (Version + 1) % 256;

	/*
	 * Update the primary Bad Block Table(BBT) in flash
	 */
	Status = XNandPs_WriteBbt(InstancePtr, &InstancePtr->BbtDesc,
				  &InstancePtr->BbtMirrorDesc);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Update the mirrored Bad Block Table(BBT) in flash
	 */
	Status = XNandPs_WriteBbt(InstancePtr, &InstancePtr->BbtMirrorDesc,
				  &InstancePtr->BbtDesc);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function marks the block containing Bad Block Table as reserved
* and updates the BBT.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Desc is the BBT descriptor pointer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static int XNandPs_MarkBbt(XNandPs *InstancePtr, XNandPs_BbtDesc *Desc)
{
	u32 BlockIndex;
	u32 BlockOffset;
	u8 BlockShift;
	u8 OldVal;
	u8 NewVal;
	int Status;
	u32 UpdateBbt = 0;
	u32 Index;

	/*
	 * Mark the last four blocks as Reserved
	 */
	BlockIndex = InstancePtr->Geometry.NumBlocks - Desc->MaxBlocks - 1;

	for (Index = 0; Index < Desc->MaxBlocks; Index++, BlockIndex++) {

		BlockOffset = BlockIndex >> XNANDPS_BBT_BLOCK_SHIFT;
		BlockShift = XNandPs_BbtBlockShift(BlockIndex);
		OldVal = InstancePtr->Bbt[BlockOffset];
		NewVal = OldVal | (XNANDPS_BLOCK_RESERVED << BlockShift);
		InstancePtr->Bbt[BlockOffset] = NewVal;

		if (OldVal != NewVal) {
			UpdateBbt = 1;
		}
	}

	/*
	 * Update the BBT to flash
	 */
	if (UpdateBbt) {
		Status = XNandPs_UpdateBbt(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function checks whether a block is bad or not.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
*
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
int XNandPs_IsBlockBad(XNandPs *InstancePtr, u32 Block)
{
	u8 Data;
	u8 BlockShift;
	u8 BlockType;
	u32 BlockOffset;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	BlockOffset = Block >> XNANDPS_BBT_BLOCK_SHIFT;
	BlockShift = XNandPs_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */
	BlockType = (Data >> BlockShift) & XNANDPS_BLOCK_TYPE_MASK;

	if (BlockType != XNANDPS_BLOCK_GOOD) {
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
* This function marks a block as bad in the RAM based Bad Block Table(BBT). It
* also updates the Bad Block Table(BBT) in the flash.
*
* @param	InstancePtr is the pointer to the XNandPs instance.
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
int XNandPs_MarkBlockBad(XNandPs *InstancePtr, u32 Block)
{
	u8 Data;
	u8 BlockShift;
	u32 BlockOffset;
	u8 OldVal;
	u8 NewVal;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	BlockOffset = Block >> XNANDPS_BBT_BLOCK_SHIFT;
	BlockShift = XNandPs_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */

	/*
	 * Mark the block as bad in the RAM based Bad Block Table
	 */
	OldVal = Data;
	Data &= ~(XNANDPS_BLOCK_TYPE_MASK << BlockShift);
	Data |= (XNANDPS_BLOCK_BAD << BlockShift);
	NewVal = Data;
	InstancePtr->Bbt[BlockOffset] = Data;

	/*
	 * Update the Bad Block Table(BBT) in flash
	 */
	if (OldVal != NewVal) {
		Status = XNandPs_UpdateBbt(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
	return XST_SUCCESS;
}
/** @} */
