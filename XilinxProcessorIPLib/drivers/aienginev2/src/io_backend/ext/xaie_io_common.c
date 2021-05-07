/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io_common.c
* @{
*
* This file contains routines for common io backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 03/08/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_io.h"
#include "xaie_helper.h"
#include "xaie_rsc_internal.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_BROADCAST_CHANNEL_MASK     0xFFFF

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API finds free resource after checking static and runtime allocated
* resource status in bitmap.
*
* @param	Bitmap: Bitmap of the resource
* @param	StaticBitmapOffset: Offset for static bitmap
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	MaxRscVal: Number of resource per tile
* @param	Index: Pointer to store free resource found
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_FindAvailableRsc(u32 *Bitmap, u32 StaticBitmapOffset,
		u32 StartBit, u32 MaxRscVal, u32 *Index)
{
	for(u32 i = StartBit; i < StartBit + MaxRscVal; i++) {
		if(!((CheckBit(Bitmap, i)) |
				CheckBit(Bitmap, (i + StaticBitmapOffset)))) {
			*Index = i - StartBit;
			return XAIE_OK;
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API finds free resource after checking static and runtime allocated
* resource status in bitmap. This API checks for contiguous free bits.
*
* @param	Bitmap: Bitmap of the resource
* @param	SBmOff: Offset for static bitmap
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	MaxRscVal: Number of resource per tile
* @param	Index: Pointer to store free resource found
* @param	NumContigRscs: Number of conitguous resource required
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_FindAvailableRscContig(u32 *Bitmap, u32 SBmOff,
		u32 StartBit, u32 MaxRscVal, u32 *Index, u8 NumContigRscs,
		XAie_RscType RscType)
{
	for(u32 i = StartBit; i < StartBit + MaxRscVal; i += NumContigRscs) {
		u32 j;

		if(!((CheckBit(Bitmap, i)) |
				CheckBit(Bitmap, (i + SBmOff)))) {
			*Index = i - StartBit;
			for(j = i + 1; j < i + NumContigRscs; j++) {
				if(CheckBit(Bitmap, j) ||
						CheckBit(Bitmap, (j + SBmOff)))
					break;
			}

			if(j == (i + NumContigRscs)) {
				if ((RscType == XAIE_PC_EVENTS_RSC ||
					  RscType == XAIE_COMBO_EVENTS_RSC) &&
					  *Index % 2 == 0) {
					return XAIE_OK;
				}
			}
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API grants resource based on availibility for the given location and
* marks that rsc as in use in the relevant bitmap.
*
* @param	Bitmap: Bitmap of the resource
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	StaticBitmapOffset: Offset for static bitmap
* @param	NumRscPerTile: Number of resource requested per tile
* @param	MaxRscVal: Maximum number of resource per tile
* @param	RscArrPerTile: Pointer to store available resource
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_RequestRsc(u32 *Bitmap, u32 StartBit,
		u32 StaticBitmapOffset, u32 NumRscPerTile, u32 MaxRscVal,
		u32 *RscArrPerTile)
{
	AieRC RC;

	/* Check for the requested resource in the bitmap locally */
	for(u32 i = 0; i < NumRscPerTile; i++) {
		u32 Index;
		RC = _XAie_FindAvailableRsc(Bitmap, StaticBitmapOffset,
				StartBit, MaxRscVal, &Index);
		if(RC != XAIE_OK) {
			/* Clear bitmap if any resource request failed */
			for(u32 j = 0; j < i; j++)
				_XAie_ClrBitInBitmap(Bitmap, RscArrPerTile[j]
				+ StartBit, 1U);
			XAIE_WARN("Unable to find free resource\n");

			return XAIE_ERR;
		}

		/* Set the bit as allocated if the request was successful*/
		_XAie_SetBitInBitmap(Bitmap, Index + StartBit, 1U);
		RscArrPerTile[i] = Index;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API grants resource based on availibility for the given location and
* marks that rsc as in use in the relevant bitmap. The API checks for contiguous
* bits available in the bitmap.
*
* @param	Bitmap: Bitmap of the resource
* @param	StartBit: Index for the resource start bit in the bitmap
* @param	StaticBitmapOffset: Offset for static bitmap
* @param	NumRscPerTile: Number of resource requested per tile
* @param	MaxRscVal: Maximum number of resource per tile
* @param	RscArrPerTile: Pointer to store available resource
* @param	NumContigRscs: Number of conitguous resource required
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestRscContig(u32 *Bitmaps, u32 StartBit,
		u32 StaticBitmapOffset, u32 NumRscPerTile, u32 MaxRscVal,
		u32 *RscArrPerTile, u8 NumContigRscs, XAie_RscType RscType)
{
	AieRC RC;

	/* Check for the requested resource in the bitmap locally */
	for(u32 i = 0U; i < NumRscPerTile; i += NumContigRscs) {
		u32 Index;

		RC = _XAie_FindAvailableRscContig(Bitmaps, StaticBitmapOffset,
				StartBit, MaxRscVal, &Index, NumContigRscs,
				RscType);
		if(RC != XAIE_OK)
			return XAIE_ERR;

		for(u8 j = 0U; j < NumContigRscs; j++)
			RscArrPerTile[i + j] = Index + j;
	}

	/* Set the bit as allocated if the request was successful*/
	for(u32 i = 0U; i < NumRscPerTile; i++) {
		_XAie_SetBitInBitmap(Bitmaps, RscArrPerTile[i] + StartBit, 1U);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API gets broadcast channel allocation status for the given start bit
* based on location and module from relevant bitmap.
*
*
* @param        Bitmap: BC channel bitmap
* @param        StaticBitmapOffset: Bitmap offset for static bitmap
* @param        StartBit: bit of the rsc in bitmap based on loc and module
* @param        StaticAllocCheckFlag: 1 - To check static and runtime bitmap
*                                     0 - To check runtime bitmap only
*
* @return       Channel allocation status.
*
* @note         Internal only.
*
*******************************************************************************/
static u32 _XAie_GetChannelStatusPerMod(u32 *Bitmap, u32 StaticBitmapOffset,
		u32 StartBit, u8 StaticAllocCheckFlag)
{
	u32 Size = sizeof(Bitmap[0]) * 8U;
	u32 ChannelStatus = (XAIE_BROADCAST_CHANNEL_MASK &
			(Bitmap[StartBit / Size] >> (StartBit % Size)));

	if(StaticAllocCheckFlag)
		return ChannelStatus | (XAIE_BROADCAST_CHANNEL_MASK &
			(Bitmap[(StartBit  + StaticBitmapOffset) / Size] >>
			((StartBit + StaticBitmapOffset) % Size)));

	return ChannelStatus;
}

/*****************************************************************************/
/**
* This API gets common broadcast channel allocation status for all modules.
*
*
* @param        DevInst: Device Instance
* @param        UserRscNum: Size of UserRsc array
* @param        Rscs: pointer to UserRsc array
* @param        StaticAllocCheckFlag: 1 - To check static and runtime bitmap
*                                     0 - To check runtime bitmap only
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
*******************************************************************************/
static u32 _XAie_GetCommonChannelStatus(XAie_DevInst *DevInst, u32 *UserRscNum,
		XAie_UserRsc *Rscs, u8 StaticAllocCheckFlag)
{
	u8 TileType;
	u32 ChannelStatus = 0, TotalRscs = *UserRscNum;
	u32 *Bitmap;
	XAie_BitmapOffsets Offsets;

	for(u32 i = 0; i < TotalRscs; i++) {

		TileType = _XAie_GetTileTypefromLoc(DevInst, Rscs[i].Loc);
		Bitmap = DevInst->RscMapping[TileType].
				Bitmaps[XAIE_BCAST_CHANNEL_RSC];
		_XAie_RscMgr_GetBitmapOffsets(DevInst, XAIE_BCAST_CHANNEL_RSC,
				Rscs[i].Loc, Rscs[i].Mod, &Offsets);
		ChannelStatus |= _XAie_GetChannelStatusPerMod(Bitmap,
				Offsets.StaticBitmapOffset, Offsets.StartBit,
				StaticAllocCheckFlag);
	}

	return ChannelStatus;
}

/*****************************************************************************/
/**
* This API finds common broadcast channel from ChannelStatus.
*
*
* @param        MaxRscVal: Max number of channels
* @param        ChannelStatus: Common broadcast channel status
* @param        ChannelIndex: Pointer to store common broadcast channel
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
*******************************************************************************/
static AieRC _XAie_FindCommonChannel(u32 MaxRscVal, u32 ChannelStatus,
		                u32 *ChannelIndex)
{
	u32 i;

	for(i = 0; i < MaxRscVal; i++) {
		if(ChannelStatus & 1U) {
			ChannelStatus >>= 1U;
		} else {
			*ChannelIndex = i;
			return XAIE_OK;
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This API checks if the specific broadcast channel is free for given tiles,
* If free, it allocates that channel and marks the resource status in
* channel bitmap and populates return rsc id.
*
* @param        DevInst: Device Instance
* @param        Args: Contains arguments for backend operation
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestSpecificBroadcastChannel(XAie_DevInst *DevInst,
		                XAie_BackendTilesRsc *Args)
{
	u32 ChannelStatus;

	ChannelStatus = _XAie_GetCommonChannelStatus(DevInst, Args->UserRscNum,
			                        Args->Rscs, XAIE_DISABLE);
	if(ChannelStatus & (1U << Args->RscId)) {
		 XAIE_ERROR("Broadcast Channel:%d busy\n", Args->RscId);
		 return XAIE_ERR;
	}

	/* Mark ChannelIndex in Bitmap for all tiles in the Rscs */
	_XAie_MarkChannelBitmapAndRscId(DevInst, *(Args->UserRscNum),
			Args->Rscs, Args->RscId);

	return XAIE_OK;

}

/*****************************************************************************/
/**
* This API finds common channel for given tiles, marks the resource status in
* channel bitmap and populates return rsc id.
*
* @param        DevInst: Device Instance
* @param        Args: Contains arguments for backend operation
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
*******************************************************************************/
static AieRC _XAie_RequestBroadcastChannelRscCommon(XAie_DevInst *DevInst,
		                XAie_BackendTilesRsc *Args)
{
	AieRC RC;
	u32 ChannelStatus, ChannelIndex;

	ChannelStatus = _XAie_GetCommonChannelStatus(DevInst, Args->UserRscNum,
			                Args->Rscs, XAIE_ENABLE);
	RC = _XAie_FindCommonChannel(XAIE_NUM_BROADCAST_CHANNELS,
			                ChannelStatus, &ChannelIndex);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to find common channel for broadcast\n");
		return RC;
	}

	/* Mark ChannelIndex in Bitmap for all tiles in the Rscs */
	_XAie_MarkChannelBitmapAndRscId(DevInst, *(Args->UserRscNum),
			                        Args->Rscs, ChannelIndex);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* The API grants resource based on availibility and marks that
* resource status in relevant bitmap.
*
*
* @param	DevInst: Device Instance
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestRscCommon(XAie_DevInst *DevInst, XAie_BackendTilesRsc *Args)
{
	AieRC RC;
	u32 RscArrPerTile[Args->NumRscPerTile];

	if(Args->RscType == XAIE_BCAST_CHANNEL_RSC)
		return _XAie_RequestBroadcastChannelRscCommon(DevInst, Args);

	if(Args->Flags == XAIE_RSC_MGR_CONTIG_FLAG) {
		RC = _XAie_RequestRscContig(Args->Bitmap, Args->StartBit,
				Args->StaticBitmapOffset, Args->NumRscPerTile,
				Args->MaxRscVal, RscArrPerTile,
				Args->NumContigRscs, Args->RscType);
		if(RC == XAIE_OK) {
			/*
			 * Return resource granted to caller by populating
			 * UserRsc
			 */
			for(u32 j = 0; j < Args->NumRscPerTile; j++)
				Args->Rscs[j].RscId = RscArrPerTile[j];
		}

		return RC;
	}

	RC = _XAie_RequestRsc(Args->Bitmap,
		Args->StartBit, Args->StaticBitmapOffset, Args->NumRscPerTile,
		Args->MaxRscVal, RscArrPerTile);
	if(RC == XAIE_OK) {
		/* Return resource granted to caller by populating UserRsc */
		for(u32 j = 0; j < Args->NumRscPerTile; j++) {
			Args->Rscs[j].RscId = RscArrPerTile[j];
		}
	}

	return RC;
}

/*****************************************************************************/
/**
* The API releases resouce statically and dynamically.
*
*
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_ReleaseRscCommon(XAie_BackendTilesRsc *Args)
{
	/* Clear resource from run-time bitmap */
	_XAie_ClrBitInBitmap(Args->Bitmap, Args->RscId + Args->StartBit, 1U);
	/* Clear resource from static bitmap */
	_XAie_ClrBitInBitmap(Args->Bitmap, Args->RscId + Args->StartBit +
		Args->StaticBitmapOffset, 1U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* The API releases resouce dynamically.
*
*
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_FreeRscCommon(XAie_BackendTilesRsc *Args)
{

	/* Clear resource from run-time bitmap */
	_XAie_ClrBitInBitmap(Args->Bitmap, Args->RscId + Args->StartBit, 1U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* The API requests statically allocated resource.
*
*
* @param	DevInst: Device Instance
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestAllocatedRscCommon(XAie_DevInst *DevInst,
		XAie_BackendTilesRsc *Args)
{
	if(Args->RscType == XAIE_BCAST_CHANNEL_RSC)
		return _XAie_RequestSpecificBroadcastChannel(DevInst, Args);

	/* Check if rsc is not allocated in runtime */
	if(!(CheckBit(Args->Bitmap, Args->StartBit))) {
		/* Mark the resource granted in the runtime bitmap */
		_XAie_SetBitInBitmap(Args->Bitmap, Args->StartBit, 1U);
		return XAIE_OK;
	}

	XAIE_ERROR("Resource in use\n");
	return XAIE_INVALID_ARGS;
}

/** @} */
