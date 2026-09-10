/******************************************************************************
* Copyright (C)  2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_power.h"
#include "xpm_debug.h"
#include "xpm_fsm.h"
#include "xpm_ddr_fsm.h"
#include "xpm_device_fsm.h"
#include "xpm_subsystem.h"

static const XPm_Fsm* XPmFsmTable[XPM_FSM_TYPE_MAX] = {
	[XPM_FSM_TYPE_GENERIC_DEVICE] = &XPmGenericDeviceFsm,
	/* Will be retrieved at runtime */
	[XPM_FSM_TYPE_HB_MON] = NULL,
	[XPM_FSM_TYPE_DDR] = &XPmDDRDeviceFsm,
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

/****************************************************************************/
/**
 * @brief Resolve an initialized state machine from the FSM table.
 *
 * @param FsmType FSM type to resolve.
 * @param Fsm Pointer to receive the selected state machine.
 *
 * @return XST_SUCCESS on success, or XST_FAILURE when the FSM is unavailable.
 *
 ****************************************************************************/
static XStatus XPmFsm_Resolve(XPm_FsmType FsmType, XPm_Fsm **Fsm)
{
	XStatus Status = XST_FAILURE;

	/* Initialize the heartbeat-monitor entry on its first lookup. */
	if (XPM_FSM_TYPE_HB_MON == FsmType) {
		XPmFsm_SetHbMonFsm();
	}

	/* Resolve the requested entry only after any lazy initialization. */
	*Fsm = (XPm_Fsm*)XPmFsmTable[FsmType];
	if (NULL == *Fsm) {
		PmErr("Fsm is not initialized for type %d\n", FsmType);
		goto done;
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Retrieve the state machine registered for a given FSM type.
 *
 * The heartbeat-monitor state machine is obtained from the subsystem module on
 * its first lookup.
 *
 * @param  FsmType  FSM type to retrieve
 * @param  Fsm      Pointer to receive the registered state machine
 *
 * @return XST_SUCCESS on success, XST_INVALID_PARAM for an invalid type or
 *         NULL output pointer, or XST_FAILURE when the type is not initialized
 *
 ****************************************************************************/
XStatus XPmFsm_GetFsmByType(XPm_FsmType FsmType, XPm_Fsm** Fsm)
{
	XStatus Status = XST_FAILURE;

	/* Reject the type before using it as an index into the FSM table. */
	if (FsmType >= XPM_FSM_TYPE_MAX) {
		PmErr("Invalid FSM type %d\n", FsmType);
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Require storage for the selected FSM before performing lazy initialization. */
	if (NULL == Fsm) {
		PmErr("Fsm pointer is NULL\n");
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Resolve the validated type and propagate its initialization status. */
	Status = XPmFsm_Resolve(FsmType, Fsm);
done:
	return Status;
}
