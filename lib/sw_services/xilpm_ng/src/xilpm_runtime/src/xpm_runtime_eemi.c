/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xplmi.h"
#include "xil_sutil.h"
#include "xstatus.h"
#include "xpm_nodeid.h"
#include "xpm_device.h"
#include "xpm_requirement.h"
#include "xpm_alloc.h"
#include "xpm_update.h"
#include "xpm_runtime_device.h"
#include "xpm_runtime_clock.h"
#include "xpm_runtime_reset.h"
#include "xpm_runtime_pin.h"
#include "xpm_subsystem.h"

/*
 * ============================================================================
 * EEMI-Specific Generic Operation Implementations (Baseline stubs & common functions)
 * ============================================================================
 * These implementations are common to both EEMI-only and full subsystem builds.
 * They are non-static so they can be referenced by SubsystemOps table in
 * top subsystem manager module (xpm_subsystem.c).
 *
 * Full subsystem-specific implementations are in xpm_runtime_subsys.c
 * EEMI-only stub implementations are in xpm_runtime_eemi.c
 */

/****************************************************************************/
/**
 * @brief Acknowledge a subsystem healthy-boot notification.
 *
 * This weak EEMI implementation does not inspect or modify the subsystem.
 *
 * @param Subsystem Subsystem reporting a healthy boot; unused by this stub.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in xpm_runtime_subsys.c.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_NotifyHealthyBoot(XPm_Subsystem *Subsystem)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)Subsystem;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Handle a request to start a subsystem recovery timer.
 *
 * This weak EEMI implementation does not start a timer or modify the subsystem.
 *
 * @param Subsystem Subsystem whose recovery timer was requested; unused.
 * @param CmdType Security type of the request; unused.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in xpm_runtime_subsys.c.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_StartRecoveryTimer(XPm_Subsystem *Subsystem, u32 CmdType)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)Subsystem;
	(void)CmdType;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Finalize subsystem initialization.
 *
 * This weak EEMI implementation does not inspect or modify the subsystem.
 *
 * @param Subsystem Subsystem to finalize; unused by this stub.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in xpm_runtime_subsys.c.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_InitFinalize(XPm_Subsystem *Subsystem)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)Subsystem;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Accept a subsystem shutdown request without changing subsystem state.
 *
 * @param Subsystem Subsystem to shut down; unused by this EEMI stub.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in xpm_runtime_subsys.c.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_ShutDown(XPm_Subsystem *Subsystem)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)Subsystem;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Accept a requirement without storing it in the subsystem.
 *
 * @param Subsystem Subsystem that would own the requirement; unused.
 * @param Payload Serialized requirement data; unused.
 * @param PayloadLen Number of payload words; unused.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in xpm_runtime_subsys.c.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)Subsystem;
	(void)Payload;
	(void)PayloadLen;
	return XST_SUCCESS;
}

/****************************************************************************/
/** @brief Return success without idling the subsystem. */
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Idle(XPm_Subsystem *Subsystem)
{
	/* This hook is an intentional no-op in the current subsystem operation table. */
	(void)Subsystem;
	return XST_SUCCESS;
}

/****************************************************************************/
/** @brief Return success without suspending the subsystem. */
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Suspend(XPm_Subsystem *Subsystem)
{
	/* This hook is an intentional no-op in the current subsystem operation table. */
	(void)Subsystem;
	return XST_SUCCESS;
}

/****************************************************************************/
/** @brief Return success without waking the subsystem. */
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_WakeUp(XPm_Subsystem *Subsystem)
{
	/* This hook is an intentional no-op in the current subsystem operation table. */
	(void)Subsystem;
	return XST_SUCCESS;
}


/****************************************************************************/
/**
 * @brief Set the software state recorded for a subsystem.
 *
 * Validates the pointer and state, stores the state, and verifies the stored
 * value to detect a fault during the update. No hardware registers are written.
 *
 * @param Subsystem Subsystem whose State field is updated.
 * @param State New state; must be less than MAX_STATE.
 *
 * @return XST_SUCCESS if the state is stored, XST_INVALID_PARAM for an invalid
 *         pointer or state, or XST_GLITCH_ERROR if verification fails.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_SetState(XPm_Subsystem *Subsystem, u32 State)
{
	volatile XStatus Status = XST_FAILURE;

	/* Reject out-of-range states and a NULL handle before writing */
	if (((u32)MAX_STATE <= State) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	Subsystem->State = (u16)State;
	Status = XST_SUCCESS;
	/* Redundant check: verify inputs and assignment against glitch */
	if (((u32)MAX_STATE <= State) || (NULL == Subsystem) ||
	    (Subsystem->State != (u16)State)) {
		Status = XST_GLITCH_ERROR;
	}
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Read the software state recorded for a subsystem.
 *
 * Copies Subsystem->State to DeviceStatus->Status.
 *
 * @param Subsystem Subsystem whose state is queried.
 * @param DeviceStatus Output structure that receives the state.
 *
 * @return XST_SUCCESS on success, or XST_INVALID_PARAM if either pointer is
 *         NULL.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_GetStatus(XPm_Subsystem *Subsystem,
				      XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;

	/* Both the subsystem and the output structure must be supplied */
	if ((NULL == Subsystem)|| (NULL == DeviceStatus)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/* Report the cached software state without reading hardware */
	DeviceStatus->Status = Subsystem->State;
	Status = XST_SUCCESS;
done:
	return Status;
}

typedef enum {
	XPM_PREALLOC_REQ_NONE,
	XPM_PREALLOC_REQ_INVALID_NODE,
	XPM_PREALLOC_REQ_INVALID_DEVICE,
	XPM_PREALLOC_REQ_REQUEST_FAILED
} XPm_PreallocReqResult;

/****************************************************************************/
/**
 * @brief Request every unallocated device marked for preallocation.
 *
 * The caller retains error reporting so its function name remains in each log
 * prefix. This helper identifies the failing condition and, for a rejected
 * request, returns the device ID needed by that report.
 *
 * @param Subsystem Subsystem whose requirement list is processed.
 * @param DeviceId Output device ID when a device request fails.
 *
 * @return XPM_PREALLOC_REQ_NONE after processing the full list, or the precise
 *         condition that stopped requirement processing.
 ****************************************************************************/
static XPm_PreallocReqResult XPmSubsystem_RequestPreallocDevices(
						   XPm_Subsystem *Subsystem,
						   u32 *DeviceId)
{
	XPm_PreallocReqResult Result = XPM_PREALLOC_REQ_NONE;
	XStatus Status = XST_FAILURE;
	const XPm_Requirement *Reqm;
	LIST_FOREACH(Subsystem->Requirements, ReqmNode) {
		/* Stop at the first malformed list entry, as activation cannot continue. */
		if ((NULL == ReqmNode) || (NULL == ReqmNode->Data)) {
			Result = XPM_PREALLOC_REQ_INVALID_NODE;
			goto done;
		}
		Reqm = ReqmNode->Data;

		/* Ignore allocated entries and requirements not marked for preallocation. */
		if ((1U != Reqm->Allocated) && (1U == PREALLOC((u32)Reqm->Flags))) {
			/* A preallocation cannot be requested without its resolved device. */
			if (NULL == Reqm->Device) {
				Result = XPM_PREALLOC_REQ_INVALID_DEVICE;
				goto done;
			}
			/* Retain the ID for the caller before issuing the matching request. */
			*DeviceId = Reqm->Device->Node.Id;
			Status = XPmDevice_Request(Subsystem->Id, *DeviceId,
							   Reqm->PreallocCaps,
							   Reqm->PreallocQoS,
							   XPLMI_CMD_SECURE);
			/* Preserve the first request failure for the caller to report. */
			if (XST_SUCCESS != Status) {
				Result = XPM_PREALLOC_REQ_REQUEST_FAILED;
				goto done;
			}
		}
	}
done:
	return Result;
}

/****************************************************************************/
/**
 * @brief Activate and configure a subsystem's preallocated devices.
 *
 * Marks a powered-off subsystem online, requests each unallocated preallocated
 * device, and sets SUBSYSTEM_IS_CONFIGURED after all requests succeed.
 *
 * @param Subsystem Subsystem to activate and configure.
 *
 * @return XST_SUCCESS if already configured or all preallocations succeed;
 *         XST_INVALID_PARAM for a NULL subsystem; XST_FAILURE for malformed
 *         requirement data; or XPM_ERR_DEVICE_REQ if a device request fails.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_Activate(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	u32 DeviceId;
	XPm_PreallocReqResult Result = XPM_PREALLOC_REQ_NONE;

	/* A subsystem handle is required before activation can begin */
	if (NULL == Subsystem) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Consider request as success if subsystem is already configured */
	if (IS_SUBSYS_CONFIGURED(Subsystem->Flags)) {
		PmInfo("Subsystem 0x%x is already configured\r\n", Subsystem->Id);
		Status = XST_SUCCESS;
		goto done;
	}

	/* Device requests require an online owner, so restore the state before preallocation. */
	/* Set subsystem to online if powered off */
	if (Subsystem->State == (u16)POWERED_OFF) {
		Subsystem->State = (u16)ONLINE;
	}
	PmDbg("Configuring Subsystem: 0x%x, reqlist\r\n", Subsystem->Id, Subsystem->Requirements);
	/*
	 * An empty requirement list is a legitimate configuration - a subsystem
	 * that preallocates nothing - so it completes as SUCCESS rather than as
	 * an error.  The diagnostic stays in this function because the helper
	 * below never sees the empty case.
	 */
	/* Keep the NULL-list diagnostic in this function before delegating traversal. */
	if (NULL == Subsystem->Requirements) {
		PmErr("List is NULL\n");
		Status = XST_SUCCESS;
	} else {
		/* Request each device marked for preallocation. */
		Result = XPmSubsystem_RequestPreallocDevices(Subsystem, &DeviceId);
	}
	/* Translate helper diagnostics here to retain this function in log prefixes. */
	if (XPM_PREALLOC_REQ_INVALID_NODE == Result) {
		PmErr("ReqmNode is NULL\r\n");
		Status = XST_FAILURE;
		goto done;
	} else if (XPM_PREALLOC_REQ_INVALID_DEVICE == Result) {
		PmErr("Reqm->Device is NULL\r\n");
		Status = XST_FAILURE;
		goto done;
	} else if (XPM_PREALLOC_REQ_REQUEST_FAILED == Result) {
		PmErr("Requesting prealloc device 0x%x failed.\n\r", DeviceId);
		Status = XPM_ERR_DEVICE_REQ;
		goto done;
	} else {
		Status = XST_SUCCESS;
	}
	/* Mark the subsystem configured after all requests succeed */
	Subsystem->Flags |= SUBSYSTEM_IS_CONFIGURED;

	PmInfo("Subsystem 0x%x configured successfully\r\n", Subsystem->Id);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Dispatch a node access check to its class-specific permission checker.
 *
 * Power and unsupported classes remain denied through the failure initializer.
 * Supported classes retain temporal redundancy around their permission checks.
 *
 * @param Subsystem Subsystem requesting access.
 * @param NodeId Node whose access permission is checked.
 *
 * @return XST_FAILURE for an unsupported class, or the selected permission
 *         checker's status.
 ****************************************************************************/
static XStatus XPmSubsystem_CheckNodePermissions(XPm_Subsystem *Subsystem,
						 u32 NodeId)
{
	volatile XStatus Status = XST_FAILURE;

	/* Select the permission checker without granting any unhandled node class. */
	switch (NODECLASS(NodeId)) {
	case (u32)XPM_NODECLASS_POWER:
		/* Leave unimplemented power-node access denied. */
		/* TODO: Check if an implementation is needed for this case */
		break;
	case (u32)XPM_NODECLASS_CLOCK:
		/* Repeat each implemented permission check to detect transient disagreement. */
		XSECURE_TEMPORAL_CHECK(done, Status,
			XPmClock_CheckPermissions, NODEINDEX(Subsystem->Id), NodeId);
		break;
	case (u32)XPM_NODECLASS_RESET:
		/*
		 * Reset and device permissions are held per subsystem OBJECT, so
		 * these two checkers take the pointer rather than an identifier.
		 */
		XSECURE_TEMPORAL_CHECK(done, Status,
			XPmReset_CheckPermissions, Subsystem, NodeId);
		break;
	case (u32)XPM_NODECLASS_DEVICE:
		XSECURE_TEMPORAL_CHECK(done, Status,
			XPmDevice_CheckPermissions, Subsystem, NodeId);
		break;
	case (u32)XPM_NODECLASS_STMIC:
		/*
		 * Pin permissions are keyed on the WHOLE subsystem ID, while the
		 * clock checker above takes only NODEINDEX() of it.  The two are
		 * not interchangeable; passing the wrong form silently checks
		 * against the wrong subsystem.
		 */
		XSECURE_TEMPORAL_CHECK(done, Status,
			XPmPin_CheckPerms, Subsystem->Id, NodeId);
		break;
	default:
		/* Unsupported classes retain the deny-by-default status. */
		break;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Validate the PMC identity with two independent comparisons.
 *
 * A stable non-PMC identity retains the failure status so a fault in the caller
 * cannot turn the privileged bypass into a grant. If the first comparison
 * identifies PMC, a second volatile read must independently identify PMC.
 *
 * @param Subsystem Subsystem whose ID is checked.
 *
 * @return XST_SUCCESS for PMC, XST_FAILURE for a stable non-PMC identity, or
 *         XST_GLITCH_ERROR when the two PMC decisions disagree.
 ****************************************************************************/
static XStatus XPmSubsystem_CheckPmcIdentity(const XPm_Subsystem *Subsystem)
{
	volatile XStatus Status = XST_FAILURE;
	const volatile XPm_Subsystem *SubsystemCheck = Subsystem;
	volatile u32 SubsysIdCheck;
	volatile u32 SubsysIdCheckTmp;

	/* Keep both PMC decisions independent for fault-injection detection. */
	SubsysIdCheck = SubsystemCheck->Id;
	if (PM_SUBSYS_PMC != SubsysIdCheck) {
		goto done;
	}

	SubsysIdCheckTmp = SubsystemCheck->Id;
	if (PM_SUBSYS_PMC != SubsysIdCheckTmp) {
		Status = XST_GLITCH_ERROR;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief Check whether a subsystem may access a node.
 *
 * PMC access is accepted directly. Clock, reset, device, and pin requests are
 * dispatched to their permission checkers; power and unsupported node classes
 * remain denied. A denial is logged.
 *
 * @param Subsystem Subsystem requesting access.
 * @param NodeId Node whose access permission is checked.
 *
 * @return XST_SUCCESS when access is allowed, XPM_INVALID_SUBSYSID for a NULL
 *         subsystem, XST_GLITCH_ERROR if the redundant PMC-ID check disagrees,
 *         XST_FAILURE for a power or unsupported node class, or the selected
 *         permission checker's failure status.
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystemOp_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId)
{
	volatile XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	/* Grant PMC access only when both independent comparisons agree. */
	Status = XPmSubsystem_CheckPmcIdentity(Subsystem);
	if (XST_SUCCESS == Status) {
		goto done;
	}
	/* Stop on a disagreement before evaluating ordinary node permissions. */
	if (XST_GLITCH_ERROR == Status) {
		goto done;
	}

	/* Dispatch non-PMC requests without moving the denial log. */
	Status = XPmSubsystem_CheckNodePermissions(Subsystem, NodeId);
done:
	/* Log denials while handling the NULL-subsystem path separately */
	if (Status != XST_SUCCESS) {
		if (NULL != Subsystem) {
			PmErr("Permission denied 0x%x, Subsys 0x%x NodeId 0x%x\r\n",
				Status, Subsystem->Id, NodeId);
		} else {
			PmErr("Permission denied 0x%x, Subsys NULL NodeId 0x%x\r\n",
				Status, NodeId);
		}
	}
	return Status;
}

/*
 * ============================================================================
 * Utility Functions - EEMI-Only Stub Implementations
 * ============================================================================
 */

/****************************************************************************/
/**
 * @brief  Check if host subsystem has permission to perform operation on target subsystem
 *
 * @param HostId        Host subsystem ID requesting the operation
 * @param TargetId      Target subsystem ID on which operation is requested
 * @param Operation     Operation type (wake/powerdown)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, error code if denied
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         returns success. Full permission checking implementation available
 *         in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)HostId;
	(void)TargetId;
	(void)Operation;
	(void)CmdType;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to wake up a node
 *
 * @param SubsystemId   Subsystem ID requesting wakeup
 * @param NodeId        Node ID to wake (subsystem or core device)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS if allowed, error code if denied
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         returns success. Full permission checking implementation available
 *         in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem is allowed to force power down a node
 *
 * @param SubsystemId   Subsystem ID requesting force powerdown
 * @param NodeId        Node ID to power down (subsystem node)
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_INVALID_PARAM (stub always denies)
 *
 * @note   Weak symbol - stub implementation for EEMI-only builds. Always
 *         denies force powerdown requests. Full permission checking implementation
 *         available in subsystem builds via strong symbol override.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType)
{
	/* The strong subsystem implementation replaces this weak EEMI default. */
	(void)SubsystemId;
	(void)NodeId;
	(void)CmdType;
	return XST_INVALID_PARAM;
}

/****************************************************************************/
/**
 * @brief Handle a request to flush a subsystem's CMN cache.
 *
 * This weak EEMI implementation performs no cache or register operation.
 *
 * @param SubsystemId Subsystem whose cache would be flushed; unused.
 *
 * @return XST_SUCCESS.
 *
 * @note A subsystem build overrides this definition in
 *       xpm_runtime_mem_subsys.c.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XStatus XPmSubsystem_CmnFlush(u32 SubsystemId)
{
	/* The strong memory-subsystem implementation replaces this weak EEMI default. */
	(void)SubsystemId;
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * @brief Get the healthy-boot monitor finite state machine.
 *
 * This weak EEMI implementation logs that boot and recovery timers are not
 * supported and does not access a finite state machine.
 *
 * @return NULL.
 *
 * @note A subsystem build overrides this definition in
 *       xpm_runtime_hbmon_subsys.c.
 *
 ****************************************************************************/
__attribute__((weak, noinline))
XPm_Fsm *XPmSubsystem_GetHbMonFsm(void)
{
	/* The strong healthy-boot monitor implementation replaces this weak default. */
	PmWarn("Boot/recovery timer feature is not supported in EEMI\r\n");
	return NULL;
}
