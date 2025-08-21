/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"
#include "xpm_power.h"
#include "xpm_aie.h"
#include "xpm_debug.h"
#include "xpm_regs.h"

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

done:
	return Status;

}

/****************************************************************************/
/**
 * @brief  Perform initialization start operations for AIE Device
 *
 * @param  AieDevice: Pointer to an AieDevice struct
 * @param  Args: Arguments passed from topology
 * @param  NumArgs: Number of arguments
 *
 * @return XST_SUCCESS (stub implementation)
 *
 * @note This is a stub implementation. Add specific initialization
 *       operations as needed.
 *
 ****************************************************************************/
XStatus XPmAieDevice_InitStart(XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;

	/* TODO: Add device-specific initialization start operations */

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Perform initialization finish operations for AIE Device
 *
 * @param  AieDevice: Pointer to an AieDevice struct
 * @param  Args: Arguments passed from topology
 * @param  NumArgs: Number of arguments
 *
 * @return XST_SUCCESS (stub implementation)
 *
 * @note This is a stub implementation. Add specific initialization
 *       finish operations as needed.
 *
 ****************************************************************************/
XStatus XPmAieDevice_InitFinish(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;

	/* TODO: Add device-specific initialization finish operations */

	return XST_SUCCESS;
}
