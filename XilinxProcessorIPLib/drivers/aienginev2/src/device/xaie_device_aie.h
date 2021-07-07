/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aie.h
* @{
*
* This file contains the apis for device specific operations of aie.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   05/03/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_DEVICE_AIE
#define XAIE_DEVICE_AIE

/***************************** Include Files *********************************/
/************************** Function Prototypes  *****************************/
u8 _XAie_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_SetPartColShimReset(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAie_SetPartColClockAfterRst(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAie_SetPartIsolationAfterRst(XAie_DevInst *DevInst);
AieRC _XAie_PartMemZeroInit(XAie_DevInst *DevInst);
AieRC _XAie_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args);

#endif /* XAIE_DEVICE_AIE */
/** @} */
