/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_mem.c
* @{
*
* This file contains read and write routines for AIE data memory.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Reorder range apis
* 1.3   Tejus   03/20/2020  Make internal functions static
* 1.4   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.5   Tejus   06/10/2020  Switch to new io backend apis.
* 1.6   Nishad  07/30/2020  Add API to read and write block of data from tile
*			    data memory.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_mem.h"

#ifdef XAIE_FEATURE_DATAMEM_ENABLE

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API writes a 32-bit value to the specified data memory location for
* the selected tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: 32-bit value to be written.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemWrWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 Data)
{
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAIE_ERROR("Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_Write32(DevInst, RegAddr, Data);
}

/*****************************************************************************/
/**
*
* This API read a 32-bit value from the specified data memory location for
* the selected tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: Pointer to store 32-bit value read from memory.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemRdWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 *Data)
{
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Data == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAIE_ERROR("Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_Read32(DevInst, RegAddr, Data);
}

/*****************************************************************************/
/**
*
* This API writes a block of data to the specified data memory location of
* the selected tile. Byte-level writes are supported by this API. For unaligned
* data memory offsets, this API implements read-modify-write operation.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Src - Source to write data.
* @param	Size - Size in bytes to write.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemBlockWrite(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		const void *Src, u32 Size)
{
	AieRC RC;
	u64 DmAddrRoundDown, DmAddrRoundUp;
	u32 BytePtr = 0;
	u32 Mask = 0, TempWord = 0;
	u32 RemBytes = Size;
	u8 FirstWriteOffset = Addr & XAIE_MEM_WORD_ALIGN_MASK;
	u8 TileType;
	unsigned char *CharSrc = (unsigned char *)Src;
	const XAie_MemMod *MemMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY) || (Src == NULL))
	{
		XAIE_ERROR("Invalid device instance or source pointer\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;

	/* Check for any size overflow */
	if((u64)Addr + Size > MemMod->Size) {
		XAIE_ERROR("Size of source block overflows tile data memory\n");
		return XAIE_ERR_OUTOFBOUND;
	}

	/* Absolute 4-byte aligned AXI-MM address to write */
	DmAddrRoundDown =  MemMod->MemAddr + XAIE_MEM_WORD_ROUND_DOWN(Addr) +
				_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Round-up unaligned Addr */
	DmAddrRoundUp = MemMod->MemAddr + XAIE_MEM_WORD_ROUND_UP(Addr) +
				_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Unaligned start bytes */
	if(FirstWriteOffset) {
		/*
		 * Construct 4-byte word along with bit mask to read-modify
		 * write at unaligned offset
		 */
		for(u32 UnalignedByte = FirstWriteOffset;
			UnalignedByte < XAIE_MEM_WORD_ALIGN_SIZE && RemBytes;
			UnalignedByte++, RemBytes--) {
			TempWord |= CharSrc[BytePtr++] << (UnalignedByte * 8);
			Mask |= 0xFF << (UnalignedByte * 8);
		}
		RC = XAie_MaskWrite32(DevInst, DmAddrRoundDown, Mask, TempWord);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	/* Aligned bytes */
	RC = XAie_BlockWrite32(DevInst, DmAddrRoundUp,
			(u32 *)(CharSrc + BytePtr),
			(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE));
	if(RC != XAIE_OK) {
		return RC;
	}
	/* Remaining unaligned bytes */
	if(RemBytes % XAIE_MEM_WORD_ALIGN_SIZE) {
		DmAddrRoundDown = DmAddrRoundUp + XAIE_MEM_WORD_ALIGN_SIZE *
					(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE);
		BytePtr += XAIE_MEM_WORD_ALIGN_SIZE *
				(RemBytes / XAIE_MEM_WORD_ALIGN_SIZE);
		TempWord = 0;
		Mask = 0;
		for (u32 UnalignedByte = 0;
			 UnalignedByte < RemBytes % XAIE_MEM_WORD_ALIGN_SIZE;
			 UnalignedByte++) {
			TempWord |= CharSrc[BytePtr++] << (UnalignedByte * 8);
			Mask |= 0xFF << (UnalignedByte * 8);
		}
		RC = XAie_MaskWrite32(DevInst, DmAddrRoundDown, Mask, TempWord);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API reads a block of data from the specified data memory location of
* the selected tile. Byte-level reads are supported by this API. For unaligned
* data memory offsets, this API implements read-modify-write operation.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Dst - Destination to store read data.
* @param	Size - Size in bytes to read.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemBlockRead(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Addr,
		void *Dst, u32 Size)
{
	AieRC RC;
	u64 DmAddrRoundDown, DmAddrRoundUp;
	u32 BytePtr = 0;
	u32 RemBytes = Size;
	u32 TempWord;
	u8 FirstReadOffset = Addr & XAIE_MEM_WORD_ALIGN_MASK;
	u8 TileType;
	unsigned char *CharDst = (unsigned char *)Dst;
	const XAie_MemMod *MemMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY) || (Dst == NULL))
	{
		XAIE_ERROR("Invalid device instance or destination pointer\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;

	/* Check for any size overflow */
	if((u64)Addr + Size > MemMod->Size) {
		XAIE_ERROR("Size of read block overflows tile data memory\n");
		return XAIE_ERR_OUTOFBOUND;
	}

	/* Absolute 4-byte aligned AXI-MM address to write */
	DmAddrRoundDown = MemMod->MemAddr + XAIE_MEM_WORD_ROUND_DOWN(Addr) +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Round-up unaligned Addr */
	DmAddrRoundUp = MemMod->MemAddr + XAIE_MEM_WORD_ROUND_UP(Addr) +
				_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* First unaligned byte read into destination block */
	if(FirstReadOffset) {
		RC = XAie_Read32(DevInst, DmAddrRoundDown, &TempWord);
		if(RC != XAIE_OK) {
			return RC;
		}

		for(u32 UnalignedByte = FirstReadOffset;
			UnalignedByte < XAIE_MEM_WORD_ALIGN_SIZE && RemBytes;
			UnalignedByte++, RemBytes--)
			CharDst[BytePtr++] = TempWord >> (UnalignedByte * 8) &
									0xFF;
	}

	/* Aligned bytes */
	for(u32 AlignedWord = 0; AlignedWord < RemBytes / 4;
		AlignedWord++, BytePtr += 4, DmAddrRoundUp += 4) {
		RC = XAie_Read32(DevInst, DmAddrRoundUp,
				(u32 *)(CharDst + BytePtr));
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	/* Remaining bytes */
	if(RemBytes % XAIE_MEM_WORD_ALIGN_SIZE) {
		RC = XAie_Read32(DevInst, DmAddrRoundUp, &TempWord);
		if(RC != XAIE_OK) {
			return RC;
		}

		for(u32 UnalignedByte = 0;
			UnalignedByte < RemBytes % XAIE_MEM_WORD_ALIGN_SIZE;
			UnalignedByte++)
			CharDst[BytePtr++] = TempWord >> (UnalignedByte * 8) &
									0xFF;
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_DATAMEM_ENABLE */
/** @} */
