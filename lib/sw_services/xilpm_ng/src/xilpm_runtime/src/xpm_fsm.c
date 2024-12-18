/******************************************************************************
* Copyright (C)  2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_runtime_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_fsm.h"

XStatus HandleDeviceEvent(XPm_Device* Device, const u32 Event)
{
	XStatus Status = XST_FAILURE;
	XPmRuntime_DeviceOps *DevOps = XPm_GetDevOps_ById(Device->Node.Id);
	if (NULL == DevOps) {
		PmErr("Runtime Device Ops is not initalized. Device ID = 0x%x\n", Device->Node.Id);
		Status = XST_FAILURE;
		goto done;
	}
	if (NULL == DevOps->Fsm) {
		PmErr("Runtime Device FSM is not initalized. Device ID = 0x%x\n", Device->Node.Id);
		Status = XST_FAILURE;
		goto done;
	}

	const XPmFsm_Tran* EventTransitions = DevOps->Fsm->Trans;
	u8 TransCnt = DevOps->Fsm->TransCnt;

	// Find the transition for the current state and event
	for (u8 i = 0; i < TransCnt; i++) {
		if (EventTransitions[i].Event == Event && EventTransitions[i].FromState == Device->Node.State) {
			u8 NextState = EventTransitions[i].ToState;
			// Perform the necessary actions to transition to the next state
			if (EventTransitions[i].Action == NULL) {
				PmWarn("Action is NULL for transition FromState=%d ToState=%d Event=%d\n", EventTransitions[i].FromState, EventTransitions[i].ToState, EventTransitions[i].Event);
				Status = XST_SUCCESS;
				goto done;
			}
			if (EventTransitions[i].Action(Device) == XST_SUCCESS) {
				// Update the current state of the device
				Device->Node.State = NextState;
				return XST_SUCCESS;
		} else {
			return XST_FAILURE;
			}
		}
	}
done:
	return Status;
}