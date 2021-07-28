/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_rsc.h
* @{
*
* Header file for resource manager implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   Dishita  01/11/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_RSC_H
#define XAIE_RSC_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaie_feature_config.h"

/***************************** Macro Definitions *****************************/
/**************************** Type Definitions *******************************/
/*
 * This structure is used to return resource as per availibility from the
 * driver.
 */
typedef struct {
	XAie_LocType Loc;
	u32 Mod;
	u32 RscType;
	u32 RscId;
} XAie_UserRsc;

/* This structure is used to request a resource by the user. */
typedef struct {
	XAie_LocType Loc;
	XAie_ModuleType Mod;
	u32 NumRscPerTile;
} XAie_UserRscReq;

/************************** Enum *********************************************/
/* This enum is used to capture all resource types managed by resource manager*/
typedef enum {
	XAIE_PERFCNT_RSC,
	XAIE_USER_EVENTS_RSC,
	XAIE_TRACE_CTRL_RSC,
	XAIE_PC_EVENTS_RSC,
	XAIE_SS_EVENT_PORTS_RSC,
	XAIE_BCAST_CHANNEL_RSC,
	XAIE_COMBO_EVENTS_RSC,
	XAIE_GROUP_EVENTS_RSC,
	XAIE_MAX_RSC,
} XAie_RscType;

/*
 * This structure is used to request the statistics of a resource type of a
 * module of a tile.
 */
typedef struct {
	XAie_LocType Loc;
	u8 Mod;
	u8 RscType;
	u8 NumRscs;
} __attribute__((packed, aligned(4))) XAie_UserRscStat;

/************************** Function Prototypes  *****************************/
#ifndef XAIE_FEATURE_RSC_ENABLE
/* Performance counter resource management APIs */
static inline AieRC XAie_RequestPerfcnt(XAie_DevInst *DevInst, u32 NumTiles,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumTiles;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleasePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedPerfcnt(XAie_DevInst *DevInst,
		u32 NumReq, XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_SaveAllocatedRscsToFile(XAie_DevInst *DevInst,
		const char *File) {
	(void)DevInst;
	(void)File;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* User Events resource management APIs */
static inline AieRC XAie_RequestUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleaseUserEvents(XAie_DevInst *DevInst,
		u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreeUserEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedUserEvents(XAie_DevInst *DevInst,
		u32 NumReq, XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* PC Events resource management APIs */
static inline AieRC XAie_RequestPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleasePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedPCEvents(XAie_DevInst *DevInst,
		u32 NumReq, XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestPCRangeEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* Stream switch event port selection resource management APIs */
static inline AieRC XAie_RequestSSEventPortSelect(XAie_DevInst *DevInst,
		u32 NumReq, XAie_UserRscReq *RscReq, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleaseSSEventPortSelect(XAie_DevInst *DevInst,
		u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreeSSEventPortSelect(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedSSEventPortSelect(XAie_DevInst *DevInst,
		u32 NumReq, XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* Trace control resource management API */
static inline AieRC XAie_RequestTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleaseTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreeTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* Group Events Resource management APIs */
static inline AieRC XAie_RequestAllocatedGroupEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreeGroupEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* Combo Events resource management APIs */
static inline AieRC XAie_RequestComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleaseComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_FreeComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestAllocatedComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq) {
	(void)DevInst;
	(void)NumReq;
	(void)RscReq;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/* Broadcast channel resource management APIs */
static inline AieRC XAie_RequestBroadcastChannel(XAie_DevInst *DevInst, u32 *UserRscNum,
		XAie_UserRsc *Rscs, u8 BroadcastAllFlag) {
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	(void)BroadcastAllFlag;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_RequestSpecificBroadcastChannel(XAie_DevInst *DevInst, u32 BcId,
		u32 *UserRscNum, XAie_UserRsc *Rscs, u8 BroadcastAllFlag) {
	(void)DevInst;
	(void)BcId;
	(void)UserRscNum;
	(void)Rscs;
	(void)BroadcastAllFlag;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
static inline AieRC XAie_ReleaseBroadcastChannel(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs)
{
	(void)DevInst;
	(void)UserRscNum;
	(void)Rscs;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

static inline AieRC XAie_GetStaticRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats)
{
	(void)DevInst;
	(void)NumRscStat;
	(void)RscStats;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

static inline AieRC XAie_GetAvailRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats)
{
	(void)DevInst;
	(void)NumRscStat;
	(void)RscStats;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
#else /* !XAIE_FEATURE_RSC_ENABLE */

/* Performance counter resource management APIs */
AieRC XAie_RequestPerfcnt(XAie_DevInst *DevInst, u32 NumTiles,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleasePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedPerfcnt(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);
AieRC XAie_SaveAllocatedRscsToFile(XAie_DevInst *DevInst, const char *File);

/* User Events resource management APIs */
AieRC XAie_RequestUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleaseUserEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreeUserEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedUserEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);

/* PC Events resource management APIs */
AieRC XAie_RequestPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleasePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreePCEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedPCEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);
AieRC XAie_RequestPCRangeEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);

/* Stream switch event port selection resource management APIs */
AieRC XAie_RequestSSEventPortSelect(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleaseSSEventPortSelect(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreeSSEventPortSelect(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedSSEventPortSelect(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);

/* Trace control resource management API */
AieRC XAie_RequestTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleaseTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreeTraceCtrl(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedTraceCtrl(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);

/* Group Events Resource management APIs */
AieRC XAie_RequestAllocatedGroupEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);
AieRC XAie_FreeGroupEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);

/* Combo Events resource management APIs */
AieRC XAie_RequestComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleaseComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreeComboEvents(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedComboEvents(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);

/* Broadcast channel resource management APIs */
AieRC XAie_RequestBroadcastChannel(XAie_DevInst *DevInst, u32 *UserRscNum,
		XAie_UserRsc *Rscs, u8 BroadcastAllFlag);
AieRC XAie_RequestSpecificBroadcastChannel(XAie_DevInst *DevInst, u32 BcId,
		u32 *UserRscNum, XAie_UserRsc *Rscs, u8 BroadcastAllFlag);
AieRC XAie_ReleaseBroadcastChannel(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);

/*****************************************************************************/
/*
*
* This API sets XAie_UserRscReq struct fields - loc, module and number of
* resource and returns struct XAie_UserRscReq.
*
* @param        Loc: location of the tile
* @param        Mod: module type
* @param        NumRsc: number of resource
*
* @return       RscReq: structure containing Loc, Mod and NumRsc
*
* @note         None.
*
******************************************************************************/
static inline XAie_UserRscReq XAie_SetupRscRequest(XAie_LocType Loc,
		XAie_ModuleType Mod, u32 NumRsc)
{
	XAie_UserRscReq RscReq = {Loc, Mod, NumRsc};
	return RscReq;
}

AieRC XAie_LoadStaticRscfromMem(XAie_DevInst *DevInst, const char *MetaData);
AieRC XAie_GetStaticRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats);
AieRC XAie_GetAvailRscStat(XAie_DevInst *DevInst, u32 NumRscStat,
		XAie_UserRscStat *RscStats);
#endif /* XAIE_FEATURE_RSC_ENABLE */
#endif		/* end of protection macro */
