/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All rights reserved.
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
	XPm_AieNode *AieNode;
	const XPm_AieDomain *AieDomain;

	if ((1U != NumArgs) || (PM_POWER_ME != Args[0]) || (PM_POWER_ME2 != Args[0])) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		goto done;
	}

	/* TODO: Remove AIE device dependency on PM_DEV_AIE once it has been
	 * fully deprecated */
	/*
	 * Check the implicit device, PM_DEV_AIE is running. Device represents the
	 * AIE Device Array as whole (includes resets, clocks, power)
	 */
	AieNode = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieNode) {
		DbgErr = XPM_INT_ERR_DEV_AIE;
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

	/* Assign AIE device dependency */
	AieDevice->BaseDev = AieNode;

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
 * @param  Device: Pointer to Device
 * @param  Subsystem: Current subsystem making the request
 * @param  Divider: Requested divider value
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note None
 *
 *****************************************************************************/
XStatus XPmAieDevice_UpdateClockDiv(const XPm_Device *Device, const XPm_Subsystem *Subsystem,
		const u32 Divider)
{
	XStatus Status = XST_FAILURE;
	const XPm_AieDevice *AieDev = (const XPm_AieDevice *)Device;
	const XPm_Requirement *Reqm = NULL;
	const XPm_Requirement *NextReqm = NULL;
	u32 TempDiv = Divider;
	const XPm_AieNode *AieNode = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	u32 BaseAddress = AieDev->BaseDev->Device.Node.BaseAddress;

	Reqm = XPmDevice_FindRequirement(Device->Node.Id, Subsystem->Id);
	if ((NULL == Reqm) || (1U != Reqm->Allocated)) {
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

	/* Check if requested divider is higher than max possible divider value */
	if (AIE_MAX_DIVIDER < Divider) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* If the divider value is 1 or any number smaller than the initial divider
	 * set at boot, it is assumed the subsystem would like the maximum
	 * frequency allowed, which is set at boot time.
	 */
	if ((1U == Divider) || (Divider <= AieNode->DefaultClockDiv)) {
		TempDiv = AieNode->DefaultClockDiv;
	} else {
		/* Check requirements placed on this device by any other subsystem */
		NextReqm = Reqm->NextSubsystem;
		while (NULL != NextReqm) {
			if (1U == NextReqm->Allocated) {
				if ((TempDiv > NextReqm->Curr.QoS) && (AieNode->DefaultClockDiv <= NextReqm->Curr.QoS)) {
					TempDiv = NextReqm->Curr.QoS;
				}
			}

			NextReqm = NextReqm->NextSubsystem;
		}

		/* Check for any other AIE devices within this subsystem */
		NextReqm = Reqm->NextDevice;
		while (NULL != NextReqm) {
			if ((1U == NextReqm->Allocated) && (IS_DEV_AIE(NextReqm->Device->Node.Id))) {
				if ((TempDiv > NextReqm->Curr.QoS) && (AieNode->DefaultClockDiv <= NextReqm->Curr.QoS)) {
					TempDiv = NextReqm->Curr.QoS;
				}
			}

			NextReqm = NextReqm->NextDevice;
		}

		/* Check for any other AIE devices in a separate subsystem. If there
		 * are any other AIE devices check the QoS (divider) values.
		 */
		NextReqm = Device->Requirements;
		while (NULL != NextReqm) {
			if ((IS_DEV_AIE(NextReqm->Device->Node.Id)) && (NextReqm->Device != Device)
					&& (NextReqm->Subsystem != Subsystem)) {
				if ((TempDiv > NextReqm->Curr.QoS) && (AieNode->DefaultClockDiv <= NextReqm->Curr.QoS)) {
					TempDiv = NextReqm->Curr.QoS;
				}
			}

			NextReqm = NextReqm->NextSubsystem;
		}
	}

	TempDiv = TempDiv << AIE_DIVISOR0_SHIFT;

	/* Unlock NPI space */
	XPm_UnlockPcsr(BaseAddress);

	/* Update clock divider with new value */
	/* TODO: Get clock address from topology */
	XPm_RMW32(BaseAddress + ME_CORE_REF_CTRL_OFFSET, AIE_DIVISOR0_MASK, TempDiv);

	/* Lock NPI space */
	XPm_LockPcsr(BaseAddress);

	Status = XST_SUCCESS;

done:
	return Status;
}

void XPmAieDevice_QueryDivider(const XPm_Device *Device, u32 *Response)
{
	const XPm_AieDevice *AieDev = (const XPm_AieDevice *)Device;
	const XPm_AieNode *AieNode = (XPm_AieNode *)XPmDevice_GetById(PM_DEV_AIE);
	u32 BaseAddress = AieDev->BaseDev->Device.Node.BaseAddress;
	u32 DefaultDivider;
	u32 Divider;

	DefaultDivider = AieNode->DefaultClockDiv;
	Divider = XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	Divider = Divider >> AIE_DIVISOR0_SHIFT;

	Response[0] = DefaultDivider;
	Response[1] = Divider;
}
