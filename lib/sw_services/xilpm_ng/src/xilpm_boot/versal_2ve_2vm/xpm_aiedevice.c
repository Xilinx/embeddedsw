/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"
#include "xpm_power.h"
#include "xpm_aie.h"
#include "xpm_debug.h"
#include "xpm_regs.h"

/****************************************************************************/
/**
 * @brief Initialize an AIE device and its partition bounds.
 *
 * Initializes the embedded generic device with its power, clock, and reset
 * dependencies. On success, clears the AIE parent link and records the column
 * range and partition flag supplied by the topology.
 *
 * @param AieDevice Pointer to the AIE device object to initialize.
 * @param NodeId Node ID assigned to the AIE device.
 * @param BaseAddress Device register base address supplied by the topology.
 * @param Power Pointer to the device's power dependency.
 * @param Clock Pointer to the device's clock dependency.
 * @param Reset Pointer to the device's reset dependency.
 * @param StartCol Inclusive first column of the partition, or 0 if unconstrained.
 * @param EndCol Inclusive last column of the partition, or 0 if unconstrained.
 * @param IsPartitioned Nonzero when the supplied column bounds constrain the
 *                      device, and zero when the device is unconstrained.
 *
 * @return XST_SUCCESS when the generic device and partition data are
 *         initialized, or the error returned by XPmDevice_Init(). Partition
 *         fields are not updated when generic device initialization fails.
 ****************************************************************************/
XStatus XPmAieDevice_Init(XPm_AieDevice *AieDevice, u32 NodeId,
			  u32 BaseAddress, XPm_Power *Power,
			  XPm_ClockNode *Clock, XPm_ResetNode *Reset,
			  u16 StartCol, u16 EndCol, u8 IsPartitioned)
{
	XStatus Status = XST_FAILURE;

	/* Initialize the generic device dependencies before AIE-specific state. */
	Status = XPmDevice_Init(&AieDevice->Device, NodeId, BaseAddress, Power, Clock, Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Record the partition relationship and topology bounds. */
	AieDevice->Parent = NULL;
	AieDevice->StartCol = StartCol;
	AieDevice->EndCol = EndCol;
	AieDevice->IsPartitioned = IsPartitioned;

done:
	return Status;

}

/****************************************************************************/
/**
 * @brief  Check if requested column range is within device partition bounds
 *
 * @param  DeviceId: AIE device node ID
 * @param  ReqStartCol: Requested start column
 * @param  ReqEndCol: Requested end column
 *
 * @return XST_SUCCESS if within bounds or unconstrained, XST_INVALID_PARAM otherwise
 *
 * @note None
 *
 ****************************************************************************/
XStatus XPmAieDevice_CheckPartitionBounds(u32 DeviceId, u32 ReqStartCol, u32 ReqEndCol)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDevice *AieDevice;

	/* Look up the AIE device by node ID */
	AieDevice = (const XPm_AieDevice *)XPmDevice_GetById(DeviceId);
	if (NULL == AieDevice) {
		Status = XST_DEVICE_NOT_FOUND;
		goto done;
	}

	/* No constraint if device is unconstrained (no partition info provided) */
	if (0U == AieDevice->IsPartitioned) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Requested range must be within the assigned partition bounds */
	if ((ReqStartCol < (u32)AieDevice->StartCol) ||
	    (ReqEndCol > (u32)AieDevice->EndCol)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Status = XST_SUCCESS;

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
maybe_unused static XStatus XPmAieDevice_InitStart(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
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
maybe_unused static XStatus XPmAieDevice_InitFinish(const XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	(void)AieDevice;
	(void)Args;
	(void)NumArgs;

	/* TODO: Add device-specific initialization finish operations */

	return XST_SUCCESS;
}
