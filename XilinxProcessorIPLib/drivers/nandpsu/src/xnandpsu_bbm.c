/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_bbm.c
* @addtogroup Overview
* @{
*
* This file implements the Bad Block Management (BBM) functionality.
* See xnandpsu_bbm.h for more details.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date        Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 2.0   sb     01/12/2015  Added support for writing BBT signature and version
*			   in page section by enabling XNANDPSU_BBT_NO_OOB.
*			   Modified Bbt Signature and Version Offset value for
*			   Oob and No-Oob region.
* 1.1	nsk    11/07/16    Change memcpy to Xil_MemCpy to handle word aligned
*	                   data access.
* 1.4	nsk    04/10/18    Added ICCARM compiler support.
* 1.10	akm    01/05/22    Remove assert checks form static and internal APIs.
* 1.13  akm    02/13/24    Update BBT writing logic.
* 1.14  sb     05/28/24    Fix BBT mapping functions.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>	/**< For Xil_MemCpy and memset */
#include "xil_types.h"
#include "xnandpsu.h"
#include "xnandpsu_bbm.h"
#include "xil_mem.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 XNandPsu_ReadBbt(XNandPsu *InstancePtr, u32 Target);

static s32 XNandPsu_SearchBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
			      u32 Target);

static void XNandPsu_CreateBbt(XNandPsu *InstancePtr, u32 Target);

static void XNandPsu_ConvertBbt(XNandPsu *InstancePtr, u8 *Buf, u32 Target);

static s32 XNandPsu_WriteBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
			     XNandPsu_BbtDesc *MirrorDesc, u32 Target);

static s32 XNandPsu_MarkBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
			    u32 Target);

static s32 XNandPsu_UpdateBbt(XNandPsu *InstancePtr, u32 Target);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes the Bad Block Table(BBT) descriptors with a
* predefined pattern for searching Bad Block Table(BBT) in flash.
*
* @param	InstancePtr is a pointer to the XNandPsu instance.
*
* @return
*		- NONE
*
******************************************************************************/
void XNandPsu_InitBbtDesc(XNandPsu *InstancePtr)
{
	u32 Index;

	/* Initialize primary Bad Block Table(BBT) */
	for (Index = 0U; Index < XNANDPSU_MAX_TARGETS; Index++) {
		InstancePtr->BbtDesc.PageOffset[Index] =
			XNANDPSU_BBT_DESC_PAGE_OFFSET;
	}
	if (InstancePtr->EccMode == XNANDPSU_ONDIE) {
		InstancePtr->BbtDesc.SigOffset = XNANDPSU_ONDIE_SIG_OFFSET;
		InstancePtr->BbtDesc.VerOffset = XNANDPSU_ONDIE_VER_OFFSET;
	} else {
		InstancePtr->BbtDesc.SigOffset = XNANDPSU_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtDesc.VerOffset = XNANDPSU_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtDesc.SigLength = XNANDPSU_BBT_DESC_SIG_LEN;
	InstancePtr->BbtDesc.MaxBlocks = XNANDPSU_BBT_DESC_MAX_BLOCKS;
	(void)strcpy(&InstancePtr->BbtDesc.Signature[0], "Bbt0");
	for (Index = 0U; Index < XNANDPSU_MAX_TARGETS; Index++) {
		InstancePtr->BbtDesc.Version[Index] = 0U;
	}
	InstancePtr->BbtDesc.Valid = 0U;

	/* Assuming that the flash device will have at least 4 blocks. */
	if (InstancePtr->Geometry.NumTargetBlocks <= InstancePtr->
	    BbtDesc.MaxBlocks) {
		InstancePtr->BbtDesc.MaxBlocks = 4U;
	}

	/* Initialize mirror Bad Block Table(BBT) */
	for (Index = 0U; Index < XNANDPSU_MAX_TARGETS; Index++) {
		InstancePtr->BbtMirrorDesc.PageOffset[Index] =
			XNANDPSU_BBT_DESC_PAGE_OFFSET;
	}
	if (InstancePtr->EccMode == XNANDPSU_ONDIE) {
		InstancePtr->BbtMirrorDesc.SigOffset =
			XNANDPSU_ONDIE_SIG_OFFSET;
		InstancePtr->BbtMirrorDesc.VerOffset =
			XNANDPSU_ONDIE_VER_OFFSET;
	} else {
		InstancePtr->BbtMirrorDesc.SigOffset =
			XNANDPSU_BBT_DESC_SIG_OFFSET;
		InstancePtr->BbtMirrorDesc.VerOffset =
			XNANDPSU_BBT_DESC_VER_OFFSET;
	}
	InstancePtr->BbtMirrorDesc.SigLength = XNANDPSU_BBT_DESC_SIG_LEN;
	InstancePtr->BbtMirrorDesc.MaxBlocks = XNANDPSU_BBT_DESC_MAX_BLOCKS;
	(void)strcpy(&InstancePtr->BbtMirrorDesc.Signature[0], "1tbB");
	for (Index = 0U; Index < XNANDPSU_MAX_TARGETS; Index++) {
		InstancePtr->BbtMirrorDesc.Version[Index] = 0U;
	}
	InstancePtr->BbtMirrorDesc.Valid = 0U;

	/* Assuming that the flash device will have at least 4 blocks. */
	if (InstancePtr->Geometry.NumTargetBlocks <= InstancePtr->
	    BbtMirrorDesc.MaxBlocks) {
		InstancePtr->BbtMirrorDesc.MaxBlocks = 4U;
	}

	/* Initialize Bad block search pattern structure */
	if (InstancePtr->Geometry.BytesPerPage > 512U) {
		/* For flash page size > 512 bytes */
		InstancePtr->BbPattern.Options = XNANDPSU_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPSU_BB_PTRN_OFF_LARGE_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPSU_BB_PTRN_LEN_LARGE_PAGE;
	} else {
		InstancePtr->BbPattern.Options = XNANDPSU_BBT_SCAN_2ND_PAGE;
		InstancePtr->BbPattern.Offset =
			XNANDPSU_BB_PTRN_OFF_SML_PAGE;
		InstancePtr->BbPattern.Length =
			XNANDPSU_BB_PTRN_LEN_SML_PAGE;
	}
	for (Index = 0U; Index < XNANDPSU_BB_PTRN_LEN_LARGE_PAGE; Index++) {
		InstancePtr->BbPattern.Pattern[Index] = XNANDPSU_BB_PATTERN;
	}
}

/*****************************************************************************/
/**
* This function scans the NAND flash for factory marked bad blocks and creates
* a RAM based Bad Block Table(BBT).
*
* @param	InstancePtr is a pointer to the XNandPsu instance.
*
* @return
*		- NONE
*
******************************************************************************/
static void XNandPsu_CreateBbt(XNandPsu *InstancePtr, u32 Target)
{
	u32 BlockIndex;
	u32 PageIndex;
	u32 Length;
	u32 BlockOffset;
	u8 BlockShift;
	u32 NumPages;
	u32 Page;
#ifdef __ICCARM__
#pragma pack(push, 1)
	u8 Buf[XNANDPSU_MAX_SPARE_SIZE] = {0U};
#pragma pack(pop)
#else
	u8 Buf[XNANDPSU_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
#endif
	u32 StartBlock = Target * InstancePtr->Geometry.NumTargetBlocks;
	u32 NumBlocks = InstancePtr->Geometry.NumTargetBlocks;
	s32 Status;

	/* Number of pages to search for bad block pattern */
	if ((InstancePtr->BbPattern.Options & XNANDPSU_BBT_SCAN_2ND_PAGE) != 0U) {
		NumPages = 2U;
	} else {
		NumPages = 1U;
	}
	/* Scan all the blocks for factory marked bad blocks */
	for (BlockIndex = StartBlock; BlockIndex < (StartBlock + NumBlocks);
	     BlockIndex++) {
		/* Block offset in Bad Block Table(BBT) entry */
		BlockOffset = BlockIndex >> XNANDPSU_BBT_BLOCK_SHIFT;
		/* Block shift value in the byte */
		BlockShift = XNandPsu_BbtBlockShift(BlockIndex);
		Page = BlockIndex * InstancePtr->Geometry.PagesPerBlock;
		/* Search for the bad block pattern */
		for (PageIndex = 0U; PageIndex < NumPages; PageIndex++) {
			Status = XNandPsu_ReadSpareBytes(InstancePtr,
							 (Page + PageIndex), &Buf[0]);

			if (Status != XST_SUCCESS) {
				/* Marking as bad block */
				InstancePtr->Bbt[BlockOffset] |=
					(u8)(XNANDPSU_BLOCK_FACTORY_BAD <<
					     BlockShift);
				break;
			}
			/*
			 * Read the spare bytes to check for bad block
			 * pattern
			 */
			for (Length = 0U; Length <
			     InstancePtr->BbPattern.Length; Length++) {
				if (Buf[InstancePtr->BbPattern.Offset + Length]
				    !=
				    InstancePtr->BbPattern.Pattern[Length]) {
					/* Bad block found */
					InstancePtr->Bbt[BlockOffset] |=
						(u8)
						(XNANDPSU_BLOCK_FACTORY_BAD <<
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
* @param	InstancePtr is a pointer to the XNandPsu instance.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPsu_ScanBbt(XNandPsu *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY)

	s32 Status;
	u32 Index;
	u32 BbtLen;

	/* Zero the RAM based Bad Block Table(BBT) entries */
	BbtLen = InstancePtr->Geometry.NumBlocks >>
		 XNANDPSU_BBT_BLOCK_SHIFT;
	(void)memset(&InstancePtr->Bbt[0], 0, BbtLen);

	for (Index = 0U; Index < InstancePtr->Geometry.NumTargets; Index++) {

		if (XNandPsu_ReadBbt(InstancePtr, Index) != XST_SUCCESS) {
			/* Create memory based Bad Block Table(BBT) */
			XNandPsu_CreateBbt(InstancePtr, Index);
			/* Write the Bad Block Table(BBT) to the flash */
			Status = XNandPsu_WriteBbt(InstancePtr,
						   &InstancePtr->BbtDesc,
						   &InstancePtr->BbtMirrorDesc, Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/* Write the Mirror Bad Block Table(BBT) to the flash */
			Status = XNandPsu_WriteBbt(InstancePtr,
						   &InstancePtr->BbtMirrorDesc,
						   &InstancePtr->BbtDesc, Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/*
			 * Mark the blocks containing Bad Block Table
			 * (BBT) as Reserved
			 */
			Status = XNandPsu_MarkBbt(InstancePtr,
						  &InstancePtr->BbtDesc,
						  Index);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			Status = XNandPsu_MarkBbt(InstancePtr,
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
* @param	InstancePtr is a pointer to the XNandPsu instance.
* @param	Buf is the buffer which contains BBT read from flash.
*
* @return
*		- NONE.
*
******************************************************************************/
static void XNandPsu_ConvertBbt(XNandPsu *InstancePtr, u8 *Buf, u32 Target)
{
	u32 BbtOffset = Target * InstancePtr->Geometry.NumTargetBlocks /
			XNANDPSU_BBT_ENTRY_NUM_BLOCKS;
	u32 BbtLen = InstancePtr->Geometry.NumTargetBlocks >>
		     XNANDPSU_BBT_BLOCK_SHIFT;

	for(u32 BbtIndex = 0; BbtIndex < BbtLen; BbtIndex++) {
		/* Invert the byte to convert from in-flash BBT to in-memory BBT */
		InstancePtr->Bbt[BbtIndex + BbtOffset] = ~Buf[BbtIndex];
	}
}

/*****************************************************************************/
/**
* This function searches the Bad Bloock Table(BBT) in flash and loads into the
* memory based Bad Block Table(BBT).
*
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPsu_ReadBbt(XNandPsu *InstancePtr, u32 Target)
{
	u64 Offset;
#ifdef __ICCARM__
#pragma pack(push, 1)
	u8 Buf[XNANDPSU_BBT_BUF_LENGTH] = {0U};
#pragma pack(pop)
#else
	u8 Buf[XNANDPSU_BBT_BUF_LENGTH] __attribute__ ((aligned(64))) = {0U};
#endif
	s32 Status1;
	s32 Status2;
	s32 Status;
	u32 BufLen;

	XNandPsu_BbtDesc *Desc = &InstancePtr->BbtDesc;
	XNandPsu_BbtDesc *MirrorDesc = &InstancePtr->BbtMirrorDesc;
	BufLen = InstancePtr->Geometry.NumTargetBlocks >>
		 XNANDPSU_BBT_BLOCK_SHIFT;
	/* Search the Bad Block Table(BBT) in flash */
	Status1 = XNandPsu_SearchBbt(InstancePtr, Desc, Target);
	Status2 = XNandPsu_SearchBbt(InstancePtr, MirrorDesc, Target);
	if ((Status1 != XST_SUCCESS) && (Status2 != XST_SUCCESS)) {
#ifdef XNANDPSU_DEBUG
		xil_printf("%s: Bad block table not found\r\n", __func__);
#endif
		Status = XST_FAILURE;
		goto Out;
	}
#ifdef XNANDPSU_DEBUG
	xil_printf("%s: Bad block table found\r\n", __func__);
#endif
	/* Bad Block Table found */
	if ((Desc->Valid != 0U) && (MirrorDesc->Valid != 0U)) {
		/* Valid BBT & Mirror BBT found */
		if (Desc->Version[Target] > MirrorDesc->Version[Target]) {
			Offset = (u64)Desc->PageOffset[Target] *
				 (u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPsu_Read(InstancePtr, Offset, BufLen,
					       &Buf[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/* Convert flash BBT to memory based BBT */
			XNandPsu_ConvertBbt(InstancePtr, &Buf[0], Target);
			MirrorDesc->Version[Target] = Desc->Version[Target];

			/* Write the BBT to Mirror BBT location in flash */
			Status = XNandPsu_WriteBbt(InstancePtr, MirrorDesc,
						   Desc, Target);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
		} else if (Desc->Version[Target] <
			   MirrorDesc->Version[Target]) {
			Offset = (u64)MirrorDesc->PageOffset[Target] *
				 (u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPsu_Read(InstancePtr, Offset, BufLen,
					       &Buf[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/* Convert flash BBT to memory based BBT */
			XNandPsu_ConvertBbt(InstancePtr, &Buf[0], Target);
			Desc->Version[Target] = MirrorDesc->Version[Target];

			/* Write the Mirror BBT to BBT location in flash */
			Status = XNandPsu_WriteBbt(InstancePtr, Desc,
						   MirrorDesc, Target);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
		} else {
			/* Both are up-to-date */
			Offset = (u64)Desc->PageOffset[Target] *
				 (u64)InstancePtr->Geometry.BytesPerPage;
			Status = XNandPsu_Read(InstancePtr, Offset, BufLen,
					       &Buf[0]);
			if (Status != XST_SUCCESS) {
				goto Out;
			}
			/* Convert flash BBT to memory based BBT */
			XNandPsu_ConvertBbt(InstancePtr, &Buf[0], Target);
		}
	} else if (Desc->Valid != 0U) {
		/* Valid Primary BBT found */
		Offset = (u64)Desc->PageOffset[Target] *
			 (u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPsu_Read(InstancePtr, Offset, BufLen, &Buf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		/* Convert flash BBT to memory based BBT */
		XNandPsu_ConvertBbt(InstancePtr, &Buf[0], Target);
		MirrorDesc->Version[Target] = Desc->Version[Target];

		/* Write the BBT to Mirror BBT location in flash */
		Status = XNandPsu_WriteBbt(InstancePtr, MirrorDesc, Desc,
					   Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	} else {
		/* Valid Mirror BBT found */
		Offset = (u64)MirrorDesc->PageOffset[Target] *
			 (u64)InstancePtr->Geometry.BytesPerPage;
		Status = XNandPsu_Read(InstancePtr, Offset, BufLen, &Buf[0]);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
		/* Convert flash BBT to memory based BBT */
		XNandPsu_ConvertBbt(InstancePtr, &Buf[0], Target);
		Desc->Version[Target] = MirrorDesc->Version[Target];

		/* Write the Mirror BBT to BBT location in flash */
		Status = XNandPsu_WriteBbt(InstancePtr, Desc, MirrorDesc,
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
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @param	Desc is the BBT descriptor pattern to search.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPsu_SearchBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
			      u32 Target)
{
	u32 StartBlock;
	u32 SigOffset;
	u32 VerOffset;
	u32 MaxBlocks;
	u32 PageOff;
	u32 SigLength;
#ifdef __ICCARM__
#pragma pack(push, 1)
	u8 Buf[XNANDPSU_MAX_SPARE_SIZE] = {0U};
#pragma pack(pop)
#else
	u8 Buf[XNANDPSU_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
#endif
	u32 Block;
	u32 Offset;
	s32 Status;

	StartBlock = ((Target + (u32)1) *
		      InstancePtr->Geometry.NumTargetBlocks) - (u32)1;
	SigOffset = Desc->SigOffset;
	VerOffset = Desc->VerOffset;
	MaxBlocks = Desc->MaxBlocks;
	SigLength = Desc->SigLength;
	Desc->Valid = 0U;

	/* Read the last 4 blocks for Bad Block Table(BBT) signature */
	for (Block = 0U; Block < MaxBlocks; Block++) {
		PageOff = (StartBlock - Block) *
			  InstancePtr->Geometry.PagesPerBlock;

		Status = XNandPsu_ReadSpareBytes(InstancePtr, PageOff, &Buf[0]);
		if (Status != XST_SUCCESS) {
			continue;
		}
		/* Check the Bad Block Table(BBT) signature */
		for (Offset = 0U; Offset < SigLength; Offset++) {
			if (Buf[Offset + SigOffset] !=
			    (u8)(Desc->Signature[Offset])) {
				break; /* Check the next blocks */
			}
		}
		if (Offset >= SigLength) {
			/* Bad Block Table(BBT) found */
			Desc->PageOffset[Target] = PageOff;
			Desc->Version[Target] = Buf[VerOffset];
			Desc->Valid = 1U;

			Status = XST_SUCCESS;
			goto Out;
		}
	}
	/* Bad Block Table(BBT) not found */
	Status = XST_FAILURE;
Out:
	return Status;
}

/*****************************************************************************/
/**
* This function writes Bad Block Table(BBT) from RAM to flash.
*
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @param	Desc is the BBT descriptor to be written to flash.
* @param	MirrorDesc is the mirror BBT descriptor.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPsu_WriteBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
			     XNandPsu_BbtDesc *MirrorDesc, u32 Target)
{
	u64 Offset;
	u32 Block = {0U};
	u32 EndBlock = ((Target + (u32)1) *
			InstancePtr->Geometry.NumTargetBlocks) - (u32)1;
#ifdef __ICCARM__
#pragma pack(push, 1)
	u8 Buf[XNANDPSU_BBT_BUF_LENGTH] = {0U};
	u8 SpareBuf[XNANDPSU_MAX_SPARE_SIZE] = {0U};
#pragma pack(pop)
#else
	u8 Buf[XNANDPSU_BBT_BUF_LENGTH] __attribute__ ((aligned(64))) = {0U};
	u8 SpareBuf[XNANDPSU_MAX_SPARE_SIZE] __attribute__ ((aligned(64))) = {0U};
#endif

	s32 Status;
	u32 Index;
	u32 BbtLen = InstancePtr->Geometry.NumTargetBlocks >>
		     XNANDPSU_BBT_BLOCK_SHIFT;
	/* Find a valid block to write the Bad Block Table(BBT) */
	if ((!Desc->Valid) != 0U) {
		for (Index = 0U; Index < Desc->MaxBlocks; Index++) {
			Block  = (EndBlock - Index);
			if (XNandPsu_IsBlockBad(InstancePtr, Block) != XST_FAILURE) {
				continue;
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


		/* Block not found for writing Bad Block Table(BBT) */
		if (Index >= Desc->MaxBlocks) {
#ifdef XNANDPSU_DEBUG
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
	/* Convert the memory based BBT to flash based table */
	(void)memset(Buf, 0xff, BbtLen);

	u32 BbtTargetOffset = BbtLen * Target;
	/* Loop through the BBT entries */
	for(u32 BbtIndex = 0U; BbtIndex < BbtLen; BbtIndex++) {
		/* Invert byte to convert from in-memory BBT to in-flash BBT */
		Buf[BbtIndex] = ~InstancePtr->Bbt[BbtIndex + BbtTargetOffset];
	}
	/* Write the Bad Block Table(BBT) to flash */
	Status = XNandPsu_EraseBlock(InstancePtr, Target,
			Block % InstancePtr->Geometry.NumTargetBlocks);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	/* Write the BBT to page offset */
	Offset = (u64)Desc->PageOffset[Target] *
		 (u64)InstancePtr->Geometry.BytesPerPage;
	Status = XNandPsu_Write(InstancePtr, Offset, BbtLen, &Buf[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}
	/* Write the signature and version in the spare data area */
	(void)memset(SpareBuf, 0xff, InstancePtr->Geometry.SpareBytesPerPage);
	Status = XNandPsu_ReadSpareBytes(InstancePtr, Desc->PageOffset[Target],
					 &SpareBuf[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	(void)Xil_MemCpy(SpareBuf + Desc->SigOffset, &Desc->Signature[0],
			 Desc->SigLength);
	(void)memcpy(SpareBuf + Desc->VerOffset, &Desc->Version[Target], 1U);

	Status = XNandPsu_WriteSpareBytes(InstancePtr,
					  Desc->PageOffset[Target], &SpareBuf[0]);
	if (Status != XST_SUCCESS) {
		goto Out;
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
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPsu_UpdateBbt(XNandPsu *InstancePtr, u32 Target)
{
	s32 Status;
	u8 Version;

	/* Update the version number */
	Version = InstancePtr->BbtDesc.Version[Target];
	InstancePtr->BbtDesc.Version[Target] = (u8)(((u16)Version +
					       (u16)1) % (u16)256U);

	Version = InstancePtr->BbtMirrorDesc.Version[Target];
	InstancePtr->BbtMirrorDesc.Version[Target] = (u8)(((u16)Version +
			(u16)1) % (u16)256);
	/* Update the primary Bad Block Table(BBT) in flash */
	Status = XNandPsu_WriteBbt(InstancePtr, &InstancePtr->BbtDesc,
				   &InstancePtr->BbtMirrorDesc,
				   Target);
	if (Status != XST_SUCCESS) {
		goto Out;
	}

	/* Update the mirrored Bad Block Table(BBT) in flash */
	Status = XNandPsu_WriteBbt(InstancePtr, &InstancePtr->BbtMirrorDesc,
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
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @param	Desc is the BBT descriptor pointer.
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
static s32 XNandPsu_MarkBbt(XNandPsu *InstancePtr, XNandPsu_BbtDesc *Desc,
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

	/* Mark the last four blocks as Reserved */
	BlockIndex = ((Target + (u32)1) * InstancePtr->Geometry.NumTargetBlocks) -
		     Desc->MaxBlocks;

	for (Index = 0U; Index < Desc->MaxBlocks; Index++) {

		BlockOffset = BlockIndex >> XNANDPSU_BBT_BLOCK_SHIFT;
		BlockShift = XNandPsu_BbtBlockShift(BlockIndex);
		OldVal = InstancePtr->Bbt[BlockOffset];
		NewVal = (u8) (OldVal | (XNANDPSU_BLOCK_RESERVED <<
					 BlockShift));
		InstancePtr->Bbt[BlockOffset] = NewVal;

		if (OldVal != NewVal) {
			UpdateBbt = 1U;
		}
		BlockIndex++;
	}

	/* Update the BBT to flash */
	if (UpdateBbt != 0U) {
		Status = XNandPsu_UpdateBbt(InstancePtr, Target);
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
* @param	InstancePtr is the pointer to the XNandPsu instance.
*
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPsu_IsBlockBad(XNandPsu *InstancePtr, u32 Block)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY)
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	u8 Data;
	u8 BlockShift;
	u8 BlockType;
	u32 BlockOffset;
	s32 Status;

	BlockOffset = Block >> XNANDPSU_BBT_BLOCK_SHIFT;
	BlockShift = XNandPsu_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */
	BlockType = (Data >> BlockShift) & XNANDPSU_BLOCK_TYPE_MASK;

	if ((BlockType != XNANDPSU_BLOCK_GOOD) &&
	    (BlockType != XNANDPSU_BLOCK_RESERVED)) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
* This function marks a block as bad in the RAM based Bad Block Table(BBT). It
* also updates the Bad Block Table(BBT) in the flash.
*
* @param	InstancePtr is the pointer to the XNandPsu instance.
* @param	Block is the block number.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail.
*
******************************************************************************/
s32 XNandPsu_MarkBlockBad(XNandPsu *InstancePtr, u32 Block)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY)
	Xil_AssertNonvoid(Block < InstancePtr->Geometry.NumBlocks);

	u8 Data;
	u8 BlockShift;
	u32 BlockOffset;
	u8 OldVal;
	u8 NewVal;
	s32 Status;
	u32 Target;

	Target = Block / InstancePtr->Geometry.NumTargetBlocks;

	BlockOffset = Block >> XNANDPSU_BBT_BLOCK_SHIFT;
	BlockShift = XNandPsu_BbtBlockShift(Block);
	Data = InstancePtr->Bbt[BlockOffset];	/* Block information in BBT */

	/* Mark the block as bad in the RAM based Bad Block Table */
	OldVal = Data;
	Data &= ~(XNANDPSU_BLOCK_TYPE_MASK << BlockShift);
	Data |= (XNANDPSU_BLOCK_BAD << BlockShift);
	NewVal = Data;
	InstancePtr->Bbt[BlockOffset] = Data;

	/* Update the Bad Block Table(BBT) in flash */
	if (OldVal != NewVal) {
		Status = XNandPsu_UpdateBbt(InstancePtr, Target);
		if (Status != XST_SUCCESS) {
			goto Out;
		}
	}

	Status = XST_SUCCESS;
Out:
	return Status;
}
/** @} */
