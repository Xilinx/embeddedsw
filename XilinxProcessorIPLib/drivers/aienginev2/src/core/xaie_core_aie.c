/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core_aie.c
* @{
*
* This file contains routines for AIE core control..
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/02/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_core_aie.h"
#include "xaie_events_aie.h"

/************************** Constant Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
*
* This API configures the enable events register with disable event. This
* configuration will be used to check if the core has triggered the disable
* event.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod)
{
	u32 Value, Mask;
	u64 RegAddr;

	Mask = CoreMod->CoreEvent->DisableEvent.Mask;
	Value = XAIE_EVENTS_CORE_INSTR_EVENT_2 <<
		CoreMod->CoreEvent->DisableEvent.Lsb;
	RegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);

	return XAIE_OK;
}

/** @} */
