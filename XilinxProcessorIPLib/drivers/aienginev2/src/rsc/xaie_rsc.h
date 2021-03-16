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

/************************** Enum *********************************************/
/************************** Function Prototypes  *****************************/
/* Performance counter resource management APIs */
AieRC XAie_RequestPerfcnt(XAie_DevInst *DevInst, u32 NumTiles,
		XAie_UserRscReq *RscReq, u32 UserRscNum, XAie_UserRsc *Rscs);
AieRC XAie_ReleasePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_FreePerfcnt(XAie_DevInst *DevInst, u32 UserRscNum,
		XAie_UserRsc *Rscs);
AieRC XAie_RequestAllocatedPerfcnt(XAie_DevInst *DevInst, u32 NumReq,
		XAie_UserRsc *RscReq);

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

#endif		/* end of protection macro */
