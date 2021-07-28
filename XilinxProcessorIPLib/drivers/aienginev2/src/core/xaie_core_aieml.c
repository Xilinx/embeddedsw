/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core_aieml.c
* @{
*
* This file contains routines for AIEML core control.
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
#include "xaie_core_aieml.h"
#include "xaie_feature_config.h"

#ifdef XAIE_FEATURE_CORE_ENABLE

/************************** Constant Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
*
* This API is not supported for AIE-ML.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the aie tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAieMl_CoreConfigureDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod)
{
	(void)DevInst;
	(void)Loc;
	(void)CoreMod;

	return XAIE_FEATURE_NOT_SUPPORTED;
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
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAieMl_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_CoreMod *CoreMod)
{
	u32 Mask, Value;
	u64 RegAddr;

	Mask = CoreMod->CoreCtrl->CtrlEn.Mask;
	Value = 1U << CoreMod->CoreCtrl->CtrlEn.Lsb;
	RegAddr = CoreMod->CoreCtrl->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, Value);
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
AieRC _XAieMl_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut, const struct XAie_CoreMod *CoreMod)
{
	u32 Mask, Value;
	u64 RegAddr;

	Mask = CoreMod->CoreSts->Done.Mask;
	Value = 1U << CoreMod->CoreSts->Done.Lsb;

	RegAddr = CoreMod->CoreSts->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	if(XAie_MaskPoll(DevInst, RegAddr, Mask, Value, TimeOut) !=
			XAIE_OK) {
		XAIE_DBG("Status poll time out\n");
		return XAIE_CORE_STATUS_TIMEOUT;
	}

	return XAIE_OK;
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
AieRC _XAieMl_CoreReadDoneBit(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 *DoneBit, const struct XAie_CoreMod *CoreMod)
{
	AieRC RC;
	u64 RegAddr;
	u32 Data;

	RegAddr = CoreMod->CoreSts->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	RC = XAie_Read32(DevInst, RegAddr, &Data);
	if(RC != XAIE_OK) {
		return RC;
	}

	*DoneBit = (Data & CoreMod->CoreSts->Done.Mask) ? 1U : 0U;

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_CORE_ENABLE */
/** @} */
