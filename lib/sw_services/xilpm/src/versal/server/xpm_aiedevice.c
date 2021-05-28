/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"
#include "xpm_power.h"
#include "xpm_debug.h"

/****************************************************************************/
/**
 * @brief  Start Node initialization for AIE
 *
 * @param  AieDevice: Pointer to AIE Device Node
 * @param  Args: Arguments for AIE
 * @param  NumArgs: Number of arguments for AIE
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Arguments consist of Power Domain Node Id that AIE depends on
 *
 ****************************************************************************/
static XStatus AieDeviceInitStart(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDevice *Parent;
	const XPm_Device *Device;

	if ((1U != NumArgs) || (PM_POWER_ME != Args[0])) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		goto done;
	}

	/*
	 * Check the implicit device, PM_DEV_AIE is running. Device represents the
	 * AIE Device Array as whole (includes resets, clocks, power)
	 */
	Device = XPmDevice_GetById(PM_DEV_AIE);
	if ((NULL == Device) ||
	    ((u32)XPM_DEVSTATE_RUNNING != Device->Node.State)) {
		DbgErr = XPM_INT_ERR_DEV_AIE;
		goto done;
	}

	/*
	 * AIE CDO cannot run if parent is assigned or in unused or initializing
	 * state
	 */
	Parent = AieDevice->Parent;
	if ((NULL == Parent)||
	    ((u8)XPM_DEVSTATE_RUNNING != Parent->Device.Node.State)) {
		DbgErr = XPM_INT_ERR_INVALID_AIE_PARENT;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief  Finish Node initialization for AIE
 *
 * @param  AieDevice: Pointer to AIE Device Node
 * @param  Args: Arguments for AIE
 * @param  NumArgs: Number of arguments for AIE
 *
 * @return XST_SUCCESS
 *
 * @note Arguments consist of Power Domain Node Id that AIE depends on
 *
 ****************************************************************************/
static XStatus AieDeviceInitFinish(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;
	return XST_SUCCESS;
}

static struct XPm_AieInitNodeOps AieDeviceOps = {
	.InitStart = AieDeviceInitStart,
	.InitFinish = AieDeviceInitFinish,
};

/****************************************************************************/
/**
 * @brief  Initialize AIE Device node base class
 *
 * @param  AieDevice: Pointer to an uninitialized AieDevice struct
 * @param  NodeId: Node Id assigned to an AieDevice node
 * @param  BaseAddress: Baseaddress that is passed from topology
 * @param  Power: Power Node dependency
 * @param  Clock: Clocks that AieDevice is dependent on
 * @param  Reset: AieDevice reset dependency
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAieDevice_Init(XPm_AieDevice *AieDevice, u32 NodeId,
			  u32 BaseAddress, XPm_Power *Power,
			  XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmDevice_Init(&AieDevice->Device, NodeId, BaseAddress, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	AieDevice->Parent = NULL;
	AieDevice->Ops = &AieDeviceOps;

done:
	return Status;

}
