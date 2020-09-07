/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_pldevice.h"
#include "xpm_debug.h"
#include "xpm_defs.h"

/****************************************************************************/
/**
 * @brief  Initialize rail node base class
 *
 * @param  PlDevice: Pointer to an uninitialized PlDevice struct
 * @param  PldId: Node Id assigned to a Pld node
 * @param  Args: Arguments for pld node
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmPlDevice_Init(XPm_PlDevice *PlDevice,
		u32 PldId,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	PlDevice->Parent = NULL;
	PlDevice->NextPeer = NULL;
	PlDevice->Child = NULL;
	PlDevice->PowerBitMask = (u8)0x0U;
	PlDevice->WfPowerBitMask = (u8)0x0U;

	Status = XPmDevice_Init(&PlDevice->Device, PldId, BaseAddress, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLDEVICE_INIT;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
