/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"

/****************************************************************************/
/**
 * @brief  Start Node initialization for AIE
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
static XStatus AieInitStart(XPm_AieDevice *AieDevice, u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;
	return XST_SUCCESS;
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
static XStatus AieInitFinish(XPm_AieDevice *AieDevice, u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;
	return XST_SUCCESS;
}

static struct XPm_AieInitNodeOps AieOps = {
	.InitStart = AieInitStart,
	.InitFinish = AieInitFinish,
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
	AieDevice->Ops = &AieOps;

done:
	return Status;

}
