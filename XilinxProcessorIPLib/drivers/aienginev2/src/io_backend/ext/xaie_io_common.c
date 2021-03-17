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
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
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
			XAIE_ERROR("Unable to find free resource\n");

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
* The API grants resource based on availibility and marks that
* resource status in relevant bitmap.
*
*
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestRscCommon(XAie_BackendTilesRsc *Args)
{
	AieRC RC;
	u32 RscArrPerTile[Args->NumRscPerTile];

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
* @param	Args: Contains arguments for backend operation
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie_RequestAllocatedRscCommon(XAie_BackendTilesRsc *Args)
{
	/* Check if rsc is not allocated statically */
	if(CheckBit(Args->Bitmap, (Args->StartBit + Args->StaticBitmapOffset))) {
		/* Mark the resource granted in the runtime bitmap */
		_XAie_SetBitInBitmap(Args->Bitmap, Args->StartBit, 1U);
		return XAIE_OK;
	}

	return XAIE_INVALID_ARGS;
}

/** @} */
