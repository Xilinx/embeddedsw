/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aieml.h
* @{
*
* This file contains the apis for device specific operations of aieml.
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
#ifndef XAIE_DEVICE_AIEML
#define XAIE_DEVICE_AIEML

/***************************** Include Files *********************************/
/************************** Function Prototypes  *****************************/
u8 _XAieMl_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAieMl_SetPartColShimReset(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAieMl_SetPartColClockAfterRst(XAie_DevInst *DevInst, u8 Enable);
AieRC _XAieMl_SetPartIsolationAfterRst(XAie_DevInst *DevInst);
AieRC _XAieMl_PartMemZeroInit(XAie_DevInst *DevInst);
AieRC _XAieMl_RequestTiles(XAie_DevInst *DevInst, XAie_BackendTilesArray *Args);

#endif /* XAIE_DEVICE_AIEML */
/** @} */
