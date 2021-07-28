/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc_bcast.c
* @{
*
* This file contains routines for broadcast channel resource management
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 03/10/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_clock.h"
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_rsc.h"
#include "xaie_rsc_internal.h"

#ifdef XAIE_FEATURE_RSC_ENABLE
/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API populates ungated tiles of partition to UserRsc list.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to return reource such as
* 		      Location, Module, resource type.
*
*
* @return	None.
*
* @note		Internal only.
* 		UserRscNum pointer is used to indicate the size of Rscs as input
* 		when passed by the caller. The same pointer gets updated to
* 		indicate the return broadcast channel Rscs size by this API.
*
*******************************************************************************/
static AieRC _XAie_GetUngatedTilesInPartition(XAie_DevInst *DevInst,
		u32 *UserRscNum, XAie_UserRsc *Rscs)
{
	u8 TileType;
	u32 Index = 0;

	/* Add clock enabled tiles of the partition to Rscs */
	for(u32 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u32 Row = 0; Row < DevInst->NumRows; Row++) {
			XAie_LocType Loc = XAie_TileLoc(Col, Row);

			if(_XAie_PmIsTileRequested(DevInst, Loc)) {
				if(Index >= *UserRscNum) {
					XAIE_ERROR("Invalid UserRscNum: %d\n",
						*UserRscNum);
					return XAIE_INVALID_ARGS;
				}

				TileType = DevInst->DevOps->GetTTypefromLoc(
						DevInst, Loc);
				if((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) ||
					(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
					Rscs[Index].Mod = XAIE_PL_MOD;

				} else if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
					Rscs[Index].Loc = Loc;
					Rscs[Index].Mod = XAIE_CORE_MOD;
					Rscs[Index].RscType =
						XAIE_BCAST_CHANNEL_RSC;
					Index++;
					Rscs[Index].Mod = XAIE_MEM_MOD;

				} else {
					Rscs[Index].Mod = XAIE_MEM_MOD;
				}

				Rscs[Index].Loc = Loc;
				Rscs[Index].RscType = XAIE_BCAST_CHANNEL_RSC;
				Index++;
			}
		}
	}

	/* Update UserRscNum to size equal to ungated tiles in partition */
	*UserRscNum = Index;

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request broadcast channel. The API grants
* broadcast channel based on availibility and marks that resource status in
* relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to return reource such as
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	BroadcastAllFlag: 1 - Broadcast to all tiles
* 				  0 - Broadcast to list of tiles
*
* @return	XAIE_OK on success.
*
* @note		This API finds common channel for the entire partition or
* 		set of tiles depending on BroadcastAllFlag. If no common channel
* 		found, API returns XAIE_ERR.
* 		UserRscNum pointer is used to indicate the size of Rscs as input
* 		when passed by the caller. The same pointer gets updated to
* 		indicate the return broadcast channel Rscs size by this API.
*
*******************************************************************************/
AieRC XAie_RequestBroadcastChannel(XAie_DevInst *DevInst, u32 *UserRscNum,
		XAie_UserRsc *Rscs, u8 BroadcastAllFlag)
{
	AieRC RC;
	XAie_BackendTilesRsc TilesRsc = {};

	if((DevInst == XAIE_NULL) || (Rscs == NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	/*
	 * _XAie_GetUngatedTilesInPartition() function will change the
	 * UserRscNum. For kernel backend, it requires the user input number
	 * of resources, otherwise, the Linux kernel will not know how many
	 * resources user have allocated memory space for.
	 */
	TilesRsc.UserRscNumInput = *UserRscNum;
	if(BroadcastAllFlag) {
		RC = _XAie_GetUngatedTilesInPartition(DevInst, UserRscNum,Rscs);
		if(RC != XAIE_OK)
			return RC;
	}

	TilesRsc.RscType = XAIE_BCAST_CHANNEL_RSC;
	TilesRsc.MaxRscVal = XAIE_NUM_BROADCAST_CHANNELS;
	TilesRsc.UserRscNum = UserRscNum;
	TilesRsc.Rscs = Rscs;
	TilesRsc.Flags = BroadcastAllFlag;

	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_RESOURCE,
		(void *)&TilesRsc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to find free broadcast channel\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to request specific broadcast channel. The API grants
* the requested broadcast channel if available and marks that resource status in
* relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	BcId: ID of the broadcast channel to be requested
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to return reource such as
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
* @param	BroadcastAllFlag: 1 - Broadcast to all tiles
* 				  0 - Broadcast to list of tiles
*
* @return	XAIE_OK on success.
*
* @note		This API checks for specific channel for the entire partition or
* 		set of tiles depending on BroadcastAllFlag.
*		If channel is busy, API returns XAIE_ERR.
* 		UserRscNum pointer is used to indicate the size of Rscs as input
* 		when passed by the caller. The same pointer gets updated to
* 		indicate the return broadcast channel Rscs size by this API.
*
*******************************************************************************/
AieRC XAie_RequestSpecificBroadcastChannel(XAie_DevInst *DevInst, u32 BcId,
		u32 *UserRscNum, XAie_UserRsc *Rscs, u8 BroadcastAllFlag)
{
	AieRC RC;
	XAie_BackendTilesRsc TilesRsc = {};

	if((UserRscNum == XAIE_NULL) || (DevInst == XAIE_NULL) ||
			(Rscs == XAIE_NULL)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(BroadcastAllFlag == 0U) {
		RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, *UserRscNum, Rscs,
				XAIE_BCAST_CHANNEL_RSC);
		if(RC != XAIE_OK)
			return RC;
	}

	/*
	 * _XAie_GetUngatedTilesInPartition() function will change the
	 * UserRscNum. For kernel backend, it requires the user input number
	 * of resources, otherwise, the Linux kernel will not know how many
	 * resources user have allocated memory space for.
	 */
	TilesRsc.UserRscNumInput = *UserRscNum;
	if(BroadcastAllFlag) {
		RC = _XAie_GetUngatedTilesInPartition(DevInst, UserRscNum,Rscs);
		if(RC != XAIE_OK)
			return RC;
	}

	TilesRsc.RscType = XAIE_BCAST_CHANNEL_RSC;
	TilesRsc.MaxRscVal = XAIE_NUM_BROADCAST_CHANNELS;
	TilesRsc.UserRscNum = UserRscNum;
	TilesRsc.Rscs = Rscs;
	TilesRsc.RscId = BcId;
	TilesRsc.Flags = BroadcastAllFlag;

	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE,
		(void *)&TilesRsc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Broadcast channel:%d busy\n", BcId);
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API shall be used to release broadcast channel. The API deallocates the
* channel and marks that resource status as free in relevant bitmap.
*
* @param	DevInst: Device Instance
* @param	UserRscNum: Size of Rscs array.
* @param	Rscs: Contains parameters to return reource such as
* 		      Location, Module, resource type.
* 		      It needs to be allocated from user application.
*
* @return	XAIE_OK on success.
*
* @note
*
*******************************************************************************/
AieRC XAie_ReleaseBroadcastChannel(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	AieRC RC;

	RC = _XAie_RscMgrRscApi_CheckArgs(DevInst, UserRscNum, Rscs,
			XAIE_BCAST_CHANNEL_RSC);
	if(RC != XAIE_OK)
		return RC;

	return _XAie_RscMgr_ReleaseRscs(DevInst, UserRscNum, Rscs,
			XAIE_BCAST_CHANNEL_RSC);
}
#endif /* XAIE_FEATURE_RSC_ENABLE */

/** @} */
