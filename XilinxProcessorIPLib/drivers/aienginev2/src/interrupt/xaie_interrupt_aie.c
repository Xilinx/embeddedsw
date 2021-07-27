/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_aie.c
* @{
*
* This file contains AIE specific interrupt routines which are not exposed to
* the user.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_interrupt_aie.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API computes first level IRQ broadcast ID.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For a shim tile, value
* 			could be XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
*
* @return	IrqId: IRQ broadcast ID.
*
* @note		IRQ ID for each switch block starts from 0, every block on the
*		left will increase by 1 until it reaches the first Shim NoC
*		column. The IRQ ID restarts from 0 on the switch A of the
*		second shim NoC column. For the shim PL columns after the
*		second Shim NoC, if there is no shim NoC further right, the
*		column will use the shim NoC on the left. That is the L1 IRQ
*		broadcast ID pattern,
*		For column from 0 to 43 is: 0 1 2 3 4 5 0 1
*		For column from 44 to 49 is: 0 1 2 3 4 5 0 1 2 3 4 5
*
*		Internal Only.
******************************************************************************/
u8 _XAie_IntrCtrlL1IrqId(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch)
{
	u8 IrqId = (((Loc.Col % 4) % 3) * 2) + Switch;

	if(Loc.Col + 3 > DevInst->NumCols)
		IrqId += 2;

	return IrqId;
}

/** @} */
