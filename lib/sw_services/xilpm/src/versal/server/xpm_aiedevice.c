/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_aiedevice.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_requirement.h"
#include "xpm_regs.h"

#define AIE_DIVISOR0_MASK 0x0003FF00U
#define AIE_DIVISOR0_SHIFT 8U
/* TODO: Remove hardcoded AIE clock address when topology is supported */
#define ME_CORE_REF_CTRL_OFFSET 0x00000138U

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
	XPm_Device *Device;

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

	/* Assign AIE device dependency */
	AieDevice->BaseDev = Device;

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
static XStatus AieDeviceInitFinish(XPm_AieDevice *AieDevice, const u32 *Args, u32 NumArgs)
{
	u32 ClkDivider;
	u32 BaseAddress = AieDevice->BaseDev->Node.BaseAddress;

	(void)Args;
	(void)NumArgs;

	/* Store initial clock devider value */
	/* TODO: Get clock address from clock topology */
	ClkDivider =  XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	ClkDivider = ClkDivider >> AIE_DIVISOR0_SHIFT;

	AieDevice->DefaultClockDiv = ClkDivider;

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
	u32 BaseAddress = AieDev->BaseDev->Node.BaseAddress;

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

	/* If the divider value is 1 or any number smaller than the initial divider
	 * set at boot, it is assumed the subsystem would like the maximum
	 * frequency allowed, which is set at boot time.
	 */
	if ((1U == Divider) || (Divider <= AieDev->DefaultClockDiv)) {
		TempDiv = AieDev->DefaultClockDiv;
	} else {
		/* Check requirements placed on this device by any other subsystem */
		NextReqm = Reqm->NextSubsystem;
		while (NULL != NextReqm) {
			if (1U == NextReqm->Allocated) {
				if ((TempDiv > NextReqm->Curr.QoS) && (AieDev->DefaultClockDiv <= NextReqm->Curr.QoS)) {
					TempDiv = NextReqm->Curr.QoS;
				}
			}

			NextReqm = NextReqm->NextSubsystem;
		}

		/* Check for any other AIE devices within this subsystem */
		NextReqm = Reqm->NextDevice;
		while (NULL != NextReqm) {
			if ((1U == NextReqm->Allocated) && (IS_DEV_AIE(Device->Node.Id))) {
				if ((TempDiv > NextReqm->Curr.QoS) && (AieDev->DefaultClockDiv <= NextReqm->Curr.QoS)) {
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
				if ((TempDiv > NextReqm->Curr.QoS) && (AieDev->DefaultClockDiv <= NextReqm->Curr.QoS)) {
					TempDiv = NextReqm->Curr.QoS;
				}
			}

			NextReqm = NextReqm->NextSubsystem;
		}
	}

	TempDiv = TempDiv << AIE_DIVISOR0_SHIFT;

	/* Unlock NPI space */
	XPm_Out32(BaseAddress + NPI_PCSR_LOCK_OFFSET, NPI_PCSR_UNLOCK_VAL);
	/* Update clock divider with new value */
	/* TODO: Get clock address from topology */
	XPm_RMW32(BaseAddress + ME_CORE_REF_CTRL_OFFSET, AIE_DIVISOR0_MASK, TempDiv);
	/* Lock NPI space */
	XPm_Out32(BaseAddress + NPI_PCSR_LOCK_OFFSET, 0U);

	Status = XST_SUCCESS;

done:
	return Status;
}

void XPmAieDevice_QueryDivider(const XPm_Device *Device, u32 *Response)
{
	const XPm_AieDevice *AieDev = (const XPm_AieDevice *)Device;
	u32 BaseAddress = AieDev->BaseDev->Node.BaseAddress;
	u32 DefaultDivider;
	u32 Divider;

	DefaultDivider = AieDev->DefaultClockDiv;
	Divider = XPm_In32(BaseAddress + ME_CORE_REF_CTRL_OFFSET) & AIE_DIVISOR0_MASK;
	Divider = Divider >> AIE_DIVISOR0_SHIFT;

	Response[0] = DefaultDivider;
	Response[1] = Divider;
}
