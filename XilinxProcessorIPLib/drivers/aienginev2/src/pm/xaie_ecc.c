/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ecc.c
* @{
*
* This file contains routines for AIE ECC Scrubbing
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 07/26/2020  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_ecc.h"
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaie_perfcnt.h"

#if defined(XAIE_FEATURE_PRIVILEGED_ENABLE) && \
	defined(XAIE_FEATURE_PERFCOUNT_ENABLE) && \
	defined(XAIE_FEATURE_EVENTS_ENABLE) && \
	defined(XAIE_FEATURE_RSC_ENABLE)

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_BROADCAST_CHANNEL_6		6U
#define XAIE_ECC_SCRUB_CLOCK_COUNT		1000000U
#define XAIE_ECC_PERFCOUNTER_ID			0U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API configures performance counter 0 of core module to trigger ECC On
* for Data and Program memory of the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*
*
******************************************************************************/
static AieRC _XAie_EccPerfCntConfig(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	AieRC RC;

	/* Reserve perf counter 0 of Core Module for ECC */
	XAie_UserRsc ReturnRsc = {Loc, XAIE_CORE_MOD, XAIE_PERFCNT_RSC,
		XAIE_ECC_PERFCOUNTER_ID};
	RC = XAie_RequestAllocatedPerfcnt(DevInst, 1U, &ReturnRsc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to reserve perf counter for ECC\n");
		return XAIE_ERR;
	}

	/*
	* Configure perf count event value register for core module's
	* perf counter 0 with the decided ECC scrub clock count.
	* This value is chosen such that its long enough not to trigger ECC
	* as often because core is stalled when ECC scrubber runs and it is
	* short enough to capture ECC reasonably.
	*/
	RC = XAie_PerfCounterEventValueSet(DevInst, Loc, XAIE_CORE_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_ECC_SCRUB_CLOCK_COUNT);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to set performance counter event value\n");
		return RC;
	}

	/* Set reset perf counter control reg with perf cnt 0 as reset event */
	RC = XAie_PerfCounterResetControlSet(DevInst, Loc, XAIE_CORE_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_EVENT_PERF_CNT_0_CORE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter control"
				" with reset event\n");
		return RC;
	}

	/* Set start stop perf counter control with true event */
	RC = XAie_PerfCounterControlSet(DevInst, Loc, XAIE_CORE_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_EVENT_TRUE_CORE,
		XAIE_EVENT_TRUE_CORE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter control"
				" with start stop event\n");
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures registers to turn ECC On for Data memory of the given
* tile.
*
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               To turn ECC On for Data Memory of a given tile, the performance
*               counter 0 event is broadcasted from core module to memory module
*               on broadcast channel 6 to trigger ECC ON for data
*               memory.ECC scrubbing event register of mem module is configured
*               with that generated event.
*
******************************************************************************/
AieRC _XAie_EccOnDM(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	AieRC RC;
	u8 Dir, TileType;
	u32 RegVal, CheckTileEccStatus;
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	/* Check if tile is shim noc or shim pl */
	if((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) ||
		(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("ECC cannot be enabled for this tile.\n");
		return XAIE_INVALID_ARGS;
	}

	/* Check bitmap for data memory already in use */
	CheckTileEccStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc);
	if(CheckBit(DevInst->MemInUse, CheckTileEccStatus)) {
		return XAIE_OK;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_MEM_MOD];
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			MemMod->EccEvntRegOff;
	/*
	 * The performance counter 0 event is broadcasted from core module to
	 * memory module on broadcast channel 6 to trigger ECC ON for data
	 * memory. Configure ECC scrubbing event register for mem module
	 * with broadcast 6 event.
	 */
	RegVal = EvntMod->XAie_EventNumber[XAIE_EVENT_BROADCAST_6_MEM -
			EvntMod->EventMin];
	RC = XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	/*
	 * Block event broadcast in all direction except east because core
	 * Module east broadcast event interface is internally connected to
	 * memory module west broadcast event interface.
	 */
	Dir = XAIE_EVENT_BROADCAST_SOUTH | XAIE_EVENT_BROADCAST_WEST |
		XAIE_EVENT_BROADCAST_NORTH;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_CORE_MOD,
		XAIE_EVENT_SWITCH_A, XAIE_BROADCAST_CHANNEL_6, Dir);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to block broadcast from core module\n");
		return RC;
	}

	/* Block broadcast of event in all direction from Mem module */
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_MEM_MOD,
		XAIE_EVENT_SWITCH_A, XAIE_BROADCAST_CHANNEL_6,
		XAIE_EVENT_BROADCAST_ALL);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to block broadcast from mem module\n");
		return RC;
	}

	/* Broadcast core perf counter 0 event to mem module of that tile */
	RC = XAie_EventBroadcast(DevInst, Loc, XAIE_CORE_MOD,
		XAIE_BROADCAST_CHANNEL_6, XAIE_EVENT_PERF_CNT_0_CORE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to broadcast event from core module\n");
		return RC;
	}

	/*
	 * Skip perf counter 0 configuration in core module if program memory
	 * already in use.
	 */
	if(CheckBit(DevInst->CoreInUse, CheckTileEccStatus)) {
		_XAie_SetBitInBitmap(DevInst->MemInUse,
				CheckTileEccStatus, 1U);
		return XAIE_OK;
	}

	/* Configure Performance counter 0 to generate event to trigger ECC */
	RC = _XAie_EccPerfCntConfig(DevInst, Loc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter for ECC\n");
		return RC;
	}

	/* Set bit corresponding to tile in MemInUse bitmap */
	_XAie_SetBitInBitmap(DevInst->MemInUse, CheckTileEccStatus, 1U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures registers to turn ECC On for Program memory of the given
* tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of tile
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               To turn ECC On for program memory of a given tile, the
*               performance counter 0 event is generated every 10^6 clock
*               cycle to trigger ECC ON for program memory. ECC scrubbing event
*               register of core module is configured with that generated event.
*
******************************************************************************/
AieRC _XAie_EccOnPM(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	AieRC RC;
	u8 TileType;
	u32 RegVal, CheckTileEccStatus;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	/* Check if tile is shim noc or shim pl */
	if((TileType == XAIEGBL_TILE_TYPE_SHIMNOC) ||
		(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("ECC cannot be enabled for this tile.\n");
		return XAIE_INVALID_ARGS;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[XAIE_CORE_MOD];

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			CoreMod->EccEvntRegOff;
	RegVal = EvntMod->XAie_EventNumber[XAIE_EVENT_PERF_CNT_0_CORE -
			EvntMod->EventMin];
	RC = XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Before configuring performance counter check if the DM in use */
	CheckTileEccStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc);
	if(CheckBit(DevInst->MemInUse, CheckTileEccStatus)) {
		return XAIE_OK;
	}

	/* Configure Performance counter 0 to generate event to trigger ECC */
	RC = _XAie_EccPerfCntConfig(DevInst, Loc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter for ECC\n");
		return RC;
	}

	/* Set bit corresponding to tile in CoreInUse bitmap */
	_XAie_SetBitInBitmap(DevInst->CoreInUse, CheckTileEccStatus, 1U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures the register to turn ECC Off for Program memory of the
* given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of tile
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               This API resets the Ecc scrubbing event register for program
*               memory of the given tile. It does not release performance
*               counter 0.
*
******************************************************************************/
void _XAie_EccEvntResetPM(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	u64 RegAddr;
	const XAie_CoreMod *CoreMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			CoreMod->EccEvntRegOff;
	XAie_Write32(DevInst, RegAddr, 0U);
}

/*****************************************************************************/
/**
* This API configures performance counter 0 of Mem tile module to trigger ECC On
* for memory of the given Mem tile location.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of Mem tile
*
* @return       XAIE_OK on success
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*               The performance counter 0 of Mem Tile module is used by driver
*               to turn ECC on with the decided ECC scrub clock count.
*               This value is chosen such that its long enough not to trigger
*               ECC as often because core is stalled when ECC scrubber runs and
*               it is short enough to capture ECC reasonably.
*
******************************************************************************/
static AieRC _XAie_EccPerfCntConfigMemTile(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	AieRC RC;

	RC = XAie_PerfCounterEventValueSet(DevInst, Loc, XAIE_MEM_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_ECC_SCRUB_CLOCK_COUNT);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to set performance counter event value\n");
		return RC;
	}

	/* Set reset perf counter control reg with perf cnt 0 as reset event */
	RC = XAie_PerfCounterResetControlSet(DevInst, Loc, XAIE_MEM_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_EVENT_PERF_CNT0_EVENT_MEM_TILE +
		XAIE_ECC_PERFCOUNTER_ID);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter control"
				" with reset event\n");
		return RC;
	}

	/* Set start stop perf counter control with true event */
	RC = XAie_PerfCounterControlSet(DevInst, Loc, XAIE_MEM_MOD,
		XAIE_ECC_PERFCOUNTER_ID, XAIE_EVENT_TRUE_MEM_TILE,
		XAIE_EVENT_TRUE_MEM_TILE);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter control"
				" with start stop event\n");
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API configures registers to turn ECC On for Mem tile memory of the given
* Mem tile location
*
* @param        DevInst: Device Instance
* @param        Loc: Location of Mem tile
*
* @return       XAIE_OK on success
*
* @note         This API is internal.
*               To turn ECC On for mem tile memory, the performance counter 0
*               of Mem tile is used to generate event every 10^6 clock cycle to
*               trigger ECC ON. ECC scrubbing event register of mem tile module
*               is configured with that generated event.
*
******************************************************************************/
AieRC _XAie_EccOnMemTile(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	AieRC RC;
	u8 TileType;
	u32 RegVal;
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	/* Check if tile type is Mem tile */
	if(TileType != XAIEGBL_TILE_TYPE_MEMTILE) {
		XAIE_ERROR("ECC cannot be enabled for this tile.\n");
		return XAIE_INVALID_ARGS;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	EvntMod = DevInst->DevProp.DevMod[TileType].EvntMod;

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
			MemMod->EccEvntRegOff;
	RegVal = EvntMod->XAie_EventNumber[XAIE_EVENT_PERF_CNT0_EVENT_MEM_TILE +
			XAIE_ECC_PERFCOUNTER_ID - EvntMod->EventMin];
	RC = XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	/* Configure Performance counter 0 to generate event to trigger ECC */
	RC = _XAie_EccPerfCntConfigMemTile(DevInst, Loc);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Unable to configure performance counter for ECC\n");
		return RC;
	}

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE && XAIE_FEATURE_PERFCOUNT_ENABLE &&
	* XAIE_FEATURE_EVENTS_ENABLE && XAIE_FEATURE_RSC_ENABLE */
