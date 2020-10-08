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
#include <unistd.h>

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
* @param	CoreMod: Pointer to the core module data structure.
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

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to enable the core.
* Any gracefulness required in enabling/disabling the core are required to be
* handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	CoreMod: Pointer to the core module data structure.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod)
{
	u32 Mask, Value;
	u64 RegAddr;

	/* Clear the disable event occurred bit */
	Mask = CoreMod->CoreEvent->DisableEventOccurred.Mask;
	Value = 1U << CoreMod->CoreEvent->DisableEventOccurred.Lsb;
	RegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);

	/* Enable the core */
	Mask = CoreMod->CoreCtrl->CtrlEn.Mask;
	Value = 1U << CoreMod->CoreCtrl->CtrlEn.Lsb;
	RegAddr = CoreMod->CoreCtrl->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);

	return XAIE_OK;
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* done state for a AIE tile. API comes out of the loop when core status
* changes to done or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @param	CoreMod: Pointer to the core module data structure.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut, const struct XAie_CoreMod *CoreMod)
{
	AieRC RC = XAIE_CORE_STATUS_TIMEOUT;
	u32 Count, MinTimeOut, StatusReg, DisEventReg;
	u64 StatusRegAddr, EventRegAddr;

	StatusRegAddr = CoreMod->CoreSts->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	EventRegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	MinTimeOut = 200;
	Count = ((u64)TimeOut + MinTimeOut - 1) / MinTimeOut;

	while (Count > 0U) {
		StatusReg = XAie_Read32(DevInst, StatusRegAddr);
		StatusReg = XAie_GetField(StatusReg, CoreMod->CoreSts->Done.Lsb,
				CoreMod->CoreSts->Done.Mask);

		if(StatusReg == 1U) {
			RC = XAIE_OK;
			break;
		}

		DisEventReg = XAie_Read32(DevInst, EventRegAddr);
		DisEventReg = XAie_GetField(DisEventReg,
				CoreMod->CoreEvent->DisableEventOccurred.Lsb,
				CoreMod->CoreEvent->DisableEventOccurred.Mask);

		if(DisEventReg == 1U) {
			RC = XAIE_OK;
			break;
		}

		usleep(MinTimeOut);
		Count--;
	}

	return RC;
}

/*****************************************************************************/
/*
*
* This API reads the Done bit value in the core status register.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	DoneBit: Pointer to store the value of Done bit. Returns 1 if
*		Done bit is set, 0 otherwise.
* @param	CoreMod: Pointer to the core module data structure.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_CoreReadDoneBit(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *DoneBit, const struct XAie_CoreMod *CoreMod)
{
	u64 RegAddr;
	u32 StatusReg, EventReg;

	/* Read core status register */
	RegAddr = CoreMod->CoreSts->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	StatusReg = XAie_Read32(DevInst, RegAddr);
	StatusReg = XAie_GetField(StatusReg, CoreMod->CoreSts->Done.Lsb,
			CoreMod->CoreSts->Done.Mask);
	if(StatusReg == 1U) {
		*DoneBit = 1U;
		return XAIE_OK;
	}

	/* Read enable events register */
	RegAddr = CoreMod->CoreEvent->EnableEventOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	EventReg = XAie_Read32(DevInst, RegAddr);

	EventReg = XAie_GetField(EventReg,
			CoreMod->CoreEvent->DisableEventOccurred.Lsb,
			CoreMod->CoreEvent->DisableEventOccurred.Mask);

	*DoneBit = EventReg == 1U;
	return XAIE_OK;
}

/** @} */
