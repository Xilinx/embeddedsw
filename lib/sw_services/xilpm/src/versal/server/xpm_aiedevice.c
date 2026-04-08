/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"
#include "xpm_power.h"
#include "xpm_aie.h"
#include "xpm_debug.h"
#include "xpm_requirement.h"
#include "xpm_regs.h"

#define AIE_MAX_DIVIDER 1023U

static const XPm_StateCap AieDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = (u32)XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = (u32)PM_CAP_ACCESS,
	},
};

static const XPm_StateTran AieDeviceTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	},
};

/****************************************************************************/
/**
 * @brief	State machine for AIE Device state management
 *
 * @param Device	Device structure whose states need to be managed
 * @param NextState	Desired next state for the Device in consideration
 *
 * @return	XStatus	Returns XST_SUCCESS or appropriate error code
 *
 * @note	None
 *
 *****************************************************************************/
static XStatus HandleAieDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_AieDevice *AieDevice;
	const XPm_PlDevice *Parent = NULL;
	u8 CurrState;

	if (NULL == Device) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	if (!(IS_DEV_AIE(Device->Node.Id))) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	AieDevice = (XPm_AieDevice *)Device;
	Parent = AieDevice->Parent;

	if ((NULL != Parent) &&
			((u8)XPM_DEVSTATE_RUNNING != Parent->Device.Node.State)) {
		DbgErr = XPM_INT_ERR_INVALID_AIE_PARENT;
		goto done;
	}

	CurrState = Device->Node.State;

	if ((u8)NextState == CurrState) {
		Status = XST_SUCCESS;
		goto done;
	}

	PmDbg ("ID=0x%x FromState=0x%x ToState=0x%x\n\r", AieDevice->Device.Node.Id,
			CurrState, NextState);
	switch (CurrState) {
		case (u8)XPM_DEVSTATE_UNUSED:
			if ((u8)XPM_DEVSTATE_RUNNING == NextState) {
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_STATE_TRANS;
			}
			break;
		case (u8)XPM_DEVSTATE_RUNNING:
			if ((u8)XPM_DEVSTATE_UNUSED == NextState) {
				Status = XST_SUCCESS;
			} else {
				DbgErr = XPM_INT_ERR_INVALID_STATE_TRANS;
			}
			break;
		default:
			DbgErr = XPM_INT_ERR_INVALID_STATE;
			break;
	}

	if (XST_SUCCESS == Status) {
		Device->Node.State = (u8)NextState;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const XPm_DeviceFsm XPmAieDeviceFsm = {
	DEFINE_DEV_STATES(AieDeviceStates),
	DEFINE_DEV_TRANS(AieDeviceTransitions),
	.EnterState = &HandleAieDeviceState,
};

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
static XStatus AieDeviceInitStart(XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_PlDevice *Parent;
	const XPm_AieDomain *AieDomain;

	if ((1U != NumArgs) || ((PM_POWER_ME != Args[0]) && (PM_POWER_ME2 != Args[0]))) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		goto done;
	}

	AieDomain = (XPm_AieDomain*)XPmPower_GetById(Args[0]);
	if (NULL == AieDomain) {
		Status = XPM_PM_INVALID_NODE;
		goto done;
	}

	if ((u8)XPM_POWER_STATE_ON != AieDomain->Domain.Power.Node.State) {
		Status = XPM_PM_NO_ACCESS;
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
	.InitStart = &AieDeviceInitStart,
	.InitFinish = &AieDeviceInitFinish,
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
	AieDevice->Device.DeviceFsm = &XPmAieDeviceFsm;

done:
	return Status;

}

/****************************************************************************/
/**
 * @brief  Update AIE clock divider
 *
 * @param  Divider: Requested divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 *****************************************************************************/
XStatus XPmAieDevice_UpdateClockDiv(const u32 Divider)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain;

	AieDomain = XPmAie_GetDomain();
	if (NULL == AieDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* If 0 is passed as divider value, it is assumed the subsystem does not
	 * care what divider value is used or does not wish to change the current
	 * value.
	 */
	if (0U == Divider) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if requested divider is higher than max possible divider value
	 * or less than the default divider value.
	 */
	if ((AIE_MAX_DIVIDER < Divider) || (Divider < AieDomain->DefaultClockDiv)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Unlock NPI space */
	XPm_UnlockPcsr(AieDomain->AieNpiAddress);

	/* Update clock divider with new value */
	XPm_RMW32(AieDomain->AieNpiAddress + ME_CORE_REF_CTRL_OFFSET, AIE_DIVISOR0_MASK, Divider << AIE_DIVISOR0_SHIFT);

	/* Lock NPI space */
	XPm_LockPcsr(AieDomain->AieNpiAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

/**
 * @brief Query the current AIE clock divider value.
 *
 * Reads the current clock divider from the AIE NPI register and returns
 * both the current and default divider values in the response.
 *
 * @param Response Pointer to store the divider query result.
 *
 * @return XST_SUCCESS on success, error code otherwise.
 */
XStatus XPmAieDevice_QueryDivider(u32 *Response)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDomain *AieDomain;
	u32 BaseAddress;
	u32 DefaultDivider;
	u32 Divider;

	AieDomain = XPmAie_GetDomain();
	if (NULL == AieDomain) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	BaseAddress = AieDomain->AieNpiAddress;
	DefaultDivider = AieDomain->DefaultClockDiv;

	Divider = XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	Divider = Divider >> AIE_DIVISOR0_SHIFT;

	Response[0] = DefaultDivider;
	Response[1] = Divider;

	Status = XST_SUCCESS;

done:
	return Status;
}
