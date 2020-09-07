/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_pldevice.h"
#include "xpm_debug.h"
#include "xpm_defs.h"

static XStatus PlInitStart(XPm_PlDevice *PlDevice, u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/*
	 * TBD: Init Start implementation
	 */
	(void)Args;
	(void)NumArgs;

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus PlInitFinish(XPm_PlDevice *PlDevice, u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/*
	 * TBD: Init Finish implementation
	 */
	(void)Args;
	(void)NumArgs;

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PldInitNodeOps PldOps = {
	.InitStart = PlInitStart,
	.InitFinish = PlInitFinish,
};

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
	PlDevice->Ops = &PldOps;

	Status = XPmDevice_Init(&PlDevice->Device, PldId, BaseAddress, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PLDEVICE_INIT;
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

XStatus XPmPlDevice_GetParent(u32 PldId, u32 *Resp)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	XPm_PlDevice *PlDevice = NULL;
	XPm_PlDevice *Parent = NULL;

	PlDevice = (XPm_PlDevice *)XPmDevice_GetById(PldId);
	if (NULL == PlDevice) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	/* For PLD0 returned value will be 0U */
	Parent = PlDevice->Parent;
	if (NULL != Parent) {
		*Resp = Parent->Device.Node.Id;
	} else {
		*Resp = 0U;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
