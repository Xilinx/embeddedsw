/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xpm_device.h"
#include "xpm_debug.h"
#include "xpm_requirement.h"

static XPm_Device *PmDevices[(u32)XPM_NODEIDX_DEV_MAX];
static u32 PmNumDevices;
static XPm_DeviceOps PmDeviceOps;

XStatus XPmDevice_GetStatus(const u32 SubsystemId,
			const u32 DeviceId,
			XPm_DeviceStatus *const DeviceStatus)
{
	//changed to support minimum boot time xilpm
	PmDbg("DeviceId %x\n",DeviceId);
	(void)SubsystemId;
	(void)DeviceId;
	(void)DeviceStatus;
	//this service is not supported at boot time
	PmErr("unsupported service\n");
	return XST_FAILURE;

}

XStatus XPmDevice_AddClock(XPm_Device *Device, XPm_ClockNode *Clock)
{
	XStatus Status = XST_FAILURE;
	XPm_ClockHandle *ClkHandle;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	if (NULL == Clock) {
		Status = XST_SUCCESS;
		goto done;
	}

	ClkHandle = (XPm_ClockHandle *)XPm_AllocBytes(sizeof(XPm_ClockHandle));
	if (NULL == ClkHandle) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	ClkHandle->Clock = Clock;
	ClkHandle->Device = Device;

	/* Prepend the new handle to the device's clock handle list */
	ClkHandle->NextClock = Device->ClkHandles;
	Device->ClkHandles = ClkHandle;

	/* Prepend the new handle to the clock's device handle list */
	ClkHandle->NextDevice = Clock->ClkHandles;
	Clock->ClkHandles = ClkHandle;

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

XStatus XPmDevice_AddReset(XPm_Device *Device, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;
	XPm_ResetHandle *RstHandle;

	if (NULL == Device) {
		Status = XPM_ERR_DEVICE;
		goto done;
	}

	if (NULL == Reset) {
		Status = XST_SUCCESS;
		goto done;
	}

	RstHandle = (XPm_ResetHandle *)XPm_AllocBytes(sizeof(XPm_ResetHandle));
	if (NULL == RstHandle) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	RstHandle->Reset = Reset;
	RstHandle->Device = Device;

	/* Prepend the new handle to the device's reset handle list */
	RstHandle->NextReset = Device->RstHandles;
	Device->RstHandles = RstHandle;

	/* Prepend the new handle to the reset's device handle list */
	RstHandle->NextDevice = Reset->RstHandles;
	Reset->RstHandles = RstHandle;

	Status = XST_SUCCESS;

done:
	if (XST_SUCCESS != Status) {
		PmErr("0x%x\n\r", Status);
	}
	return Status;
}

static XStatus SetDeviceNode(u32 Id, XPm_Device *Device)
{
	XStatus Status = XST_INVALID_PARAM;
	u32 NodeIndex = NODEINDEX(Id);

	/*
	 * We assume that the Node ID class, subclass and type has _already_
	 * been validated before, so only check bounds here against index
	 */
	if ((NULL != Device) && ((u32)XPM_NODEIDX_DEV_MAX > NodeIndex)) {
		PmDevices[NodeIndex] = Device;
		PmNumDevices++;
		Status = XST_SUCCESS;
	}

	return Status;
}

XStatus XPmDevice_Init(XPm_Device *Device,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode * Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XPM_ERR_DEVICE_INIT;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if ((NULL != XPmDevice_GetById(Id)) &&
	    (((u32)XPM_NODESUBCL_DEV_PL != NODESUBCLASS(Id)) &&
	    ((u32)XPM_NODESUBCL_DEV_AIE != NODESUBCLASS(Id)))){
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	XPmNode_Init(&Device->Node, Id, (u8)XPM_DEVSTATE_UNUSED, BaseAddress);
	/* Add requirement for each requestable device on PMC subsystem */
	if (1U == XPmDevice_IsRequestable(Id)) {
		u32 Flags = REQUIREMENT_FLAGS(0U, (u32)REQ_ACCESS_SECURE_NONSECURE, (u32)REQ_NO_RESTRICTION);
		Status = XPmRequirement_Add(XPmSubsystem_GetByIndex((u32)XPM_NODEIDX_SUBSYS_PMC),
					    Device, Flags, 0U, XPM_DEF_QOS);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_ADD_REQUIREMENT;
			goto done;
		}
	}

	Device->Power = Power;
	Device->PendingReqm = NULL;
	Device->WfDealloc = 0;
	Device->WfPwrUseCnt = 0;

	Status = XPmDevice_AddClock(Device, Clock);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ADD_CLK;
		goto done;
	}

	Status = XPmDevice_AddReset(Device, Reset);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ADD_RST;
		goto done;
	}

	/*TBD: set HandleEvent*/
	/*TBD: set DeviceOps */
	Device->HandleEvent = NULL;
	PmDeviceOps.Request = NULL;
	PmDeviceOps.SetRequirement = NULL;
	PmDeviceOps.Release = NULL;
	Device->DeviceOps = NULL;
	if (NULL == Device->DeviceFsm) {
		/*TBD: add devicefsm */
		Device->DeviceFsm = NULL;
	}

		Status = SetDeviceNode(Id, Device);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SET_DEV_NODE;
			goto done;
		}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/****************************************************************************/
/**
 * @brief	Get handle to requested device node by "complete" Node ID
 *
 * @param DeviceId	Device Node ID
 *
 * @return	Pointer to requested XPm_Device
 *              NULL otherwise
 *
 * @note	Requires Complete Node ID
 *
 ****************************************************************************/
XPm_Device *XPmDevice_GetById(const u32 DeviceId)
{
	XPm_Device *Device = NULL;
	XPm_Device **DevicesHandle = NULL;

	if ((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) {
		goto done;
	}

	if ((u32)XPM_NODEIDX_DEV_MAX <= NODEINDEX(DeviceId)) {
		goto done;
	}
	DevicesHandle = PmDevices;

	if (NULL == DevicesHandle) {
		goto done;
	}

	/* Retrieve the device */
	Device = DevicesHandle[NODEINDEX(DeviceId)];
	/* Check that Device's ID is same as given ID or not. */
	if ((NULL != Device) && (DeviceId != Device->Node.Id)) {
		Device = NULL;
	}

done:
	return Device;
}

XStatus XPmDevice_AddParent(u32 Id, const u32 *Parents, u32 NumParents)
{
	XStatus Status = XST_FAILURE;
	u32 i = 0;
	XPm_Device *DevPtr = XPmDevice_GetById(Id);

	if ((DevPtr == NULL) || (NumParents == 0U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	for (i = 0; i < NumParents; i++){
		if ((u32)XPM_NODECLASS_CLOCK == NODECLASS(Parents[i])) {
			XPm_ClockNode *Clk = XPmClock_GetById(Parents[i]);
			if (NULL == Clk) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = XPmDevice_AddClock(DevPtr, Clk);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else if ((u32)XPM_NODECLASS_RESET == NODECLASS(Parents[i])) {
			XPm_ResetNode *Rst = XPmReset_GetById(Parents[i]);
			if (NULL == Rst) {
				Status = XST_INVALID_PARAM;
				goto done;
			}

			Status = XPmDevice_AddReset(DevPtr, Rst);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else if ((u32)XPM_NODECLASS_POWER == NODECLASS(Parents[i])) {
			if (DevPtr->Power != NULL) {
				Status = XST_INVALID_PARAM;
				goto done;
			} else {
				DevPtr->Power = XPmPower_GetById(Parents[i]);
				if (NULL == DevPtr->Power) {
					Status = XST_DEVICE_NOT_FOUND;
					goto done;
				}
				Status = XST_SUCCESS;
			}
		} else {
			Status = XST_INVALID_PARAM;
			goto done;
		}
	}
done:
	return Status;
}