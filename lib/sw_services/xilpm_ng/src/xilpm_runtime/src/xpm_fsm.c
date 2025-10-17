/******************************************************************************
* Copyright (C)  2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_fsm.h"
#include "xpm_device_fsm.h"
#include "xpm_subsystem.h"

static const XPm_Fsm* XPmFsmTable[] = {
	[XPM_FSM_TYPE_GENERIC_DEVICE] = &XPmGenericDeviceFsm,
	/* Will be retrieved at runtime */
	[XPM_FSM_TYPE_HB_MON] = NULL,
};

static void XPmFsm_SetHbMonFsm(void)
{
	const XPm_Fsm *HbMonFsm = NULL;

	HbMonFsm = XPmFsmTable[XPM_FSM_TYPE_HB_MON];
	if (NULL != HbMonFsm) {
		/* Already set */
		goto done;
	}

	/* Retrieve HB_MON FSM from Subsystem module */
	HbMonFsm = XPmSubsystem_GetHbMonFsm();
	if (NULL == HbMonFsm) {
		PmErr("Failed to retrieve HB_MON FSM\n");
		goto done;
	}
	/* Set it in the FSM table */
	XPmFsmTable[XPM_FSM_TYPE_HB_MON] = HbMonFsm;

done:
	return;
}

XStatus XPmFsm_GetFsmByType(XPm_FsmType FsmType, XPm_Fsm** Fsm)
{
	XStatus Status = XST_FAILURE;
	if (FsmType >= XPM_FSM_TYPE_MAX) {
		PmErr("Invalid FSM type %d\n", FsmType);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (NULL == Fsm) {
		PmErr("Fsm pointer is NULL\n");
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (FsmType == XPM_FSM_TYPE_HB_MON) {
		/* Retrieve and Set HB_MON FSM at runtime */
		XPmFsm_SetHbMonFsm();
	}

	*Fsm = (XPm_Fsm*)XPmFsmTable[FsmType];
	if (NULL == *Fsm) {
		PmErr("Fsm is not initialized for type %d\n", FsmType);
		Status = XST_FAILURE;
		goto done;
	}

	Status = XST_SUCCESS;
done:
	return Status;
}
