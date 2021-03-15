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
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"
#include "xaie_helper.h"
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
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
