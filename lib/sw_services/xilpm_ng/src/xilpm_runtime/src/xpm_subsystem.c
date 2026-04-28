/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_subsystem.h"
#include "xpm_alloc.h"
#include "xpm_update.h"

/*
 * Subsystem Manager - Single unified object
 * Always defined here, regardless of build configuration
 */
XPm_SubsystemMgr SubsysMgr XPM_INIT_DATA(SubsysMgr) = {
	.Subsystems = { .Root = NULL },
	.NumSubsystems = 0U,
	.MaxSubsystemIdx = 0U,
	.SubsysOps = {
		/*
		 * SubsystemOpsTable - Subsystem Operations
		 * Points to:
		 *   - Common Ops & weak implementations from xpm_subsystem_ops.c
		 *   - Full implementations are in xpm_runtime_subsys.c (SUBSYS module enabled)
		 */
		[SUBSYS_OPS_GENERIC] = {
			.Activate = XPmSubsystemOp_Activate,               /* Common from xpm_subsystem.c */
			.GetStatus = XPmSubsystemOp_GetStatus,             /* Common from xpm_subsystem.c */
			.Idle = XPmSubsystemOp_Idle,                       /* Common from xpm_subsystem.c */
			.IsAccessAllowed = XPmSubsystemOp_IsAccessAllowed, /* Common from xpm_subsystem.c */
			.SetState = XPmSubsystemOp_SetState,               /* Common from xpm_subsystem.c */
			.Suspend = XPmSubsystemOp_Suspend,                 /* Common from xpm_subsystem.c */
			.WakeUp = XPmSubsystemOp_WakeUp,                   /* Common from xpm_subsystem.c */
			.AddRequirement = XPmSubsystemOp_AddRequirement,   /* Stub implementation */
			.InitFinalize = XPmSubsystemOp_InitFinalize,       /* Stub implementation */
			.NotifyHealthyBoot = XPmSubsystemOp_NotifyHealthyBoot, /* Stub implementation */
			.ShutDown = XPmSubsystemOp_ShutDown,               /* Stub implementation */
			.StartRecoveryTimer = XPmSubsystemOp_StartRecoveryTimer, /* Stub implementation */
		},
	},
};

/* Get subsystem operations for a given subsystem */
static XPm_SubsystemOps* XPmSubsystem_GetOps(XPm_Subsystem *Subsystem)
{
	/* Only support generic subsystems for now - SUBSYS_OPS_GENERIC */
	return &SubsysMgr.SubsysOps[Subsystem->OpsType];
}

/****************************************************************************/
/**
 * @brief  Get subsystem by ID
 *
 * @param SubsystemId     Subsystem ID
 *
 * @return Pointer to XPm_Subsystem if successful else NULL
 *
 * @note
 * This function serves as the abstraction layer between runtime core
 * and subsystem implementation. It always uses the unified SubsysMgr
 * regardless of build configuration.
 *
 ****************************************************************************/
XPm_Subsystem *XPmSubsystem_GetById(u32 SubsystemId)
{
	XPm_Subsystem *Subsystem = NULL;
	XPm_SubsystemList* PmSubsystems = &SubsysMgr.Subsystems;

	if ((INVALID_SUBSYSID == SubsystemId) ||
	    (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId))) {
		return NULL;
	}

	LIST_FOREACH(PmSubsystems, SsNode) {
		if (SsNode->Data->Id == SubsystemId) {
			Subsystem = SsNode->Data;
			break;
		}
	}

	return Subsystem;
}

/****************************************************************************/
/**
 * @brief  Get the maximum subsystem index currently present in the system
 *
 * @return Maximum subsystem index (0-based), or 0 if no subsystems exist
 *
 * @note   This value is updated when subsystems are added and is used for
 *         efficient iteration over subsystem indices.
 *
 ****************************************************************************/
u32 XPmSubsystem_GetMaxSubsysIdx(void)
{
	return SubsysMgr.MaxSubsystemIdx;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem has permission to access a node (public API wrapper)
 *
 * @param SubsystemId   Subsystem ID requesting access
 * @param NodeId        Node ID to check access for
 *
 * @return XST_SUCCESS if allowed, XPM_INVALID_SUBSYSID if subsystem not found,
 *         error code if denied
 *
 * @note   Public API wrapper that retrieves the subsystem by ID and calls
 *         XPmSubsystem_IsAccessAllowed. This function provides a convenient
 *         interface when only the subsystem ID is available.
 *
 ****************************************************************************/
XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId)
{
	volatile XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	Status = XPmSubsystem_IsAccessAllowed(Subsystem, NodeId);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function gives Subsystem from Subsystem "INDEX".
 *
 * @param SubSysIdx     Subsystem Index
 *
 * @return Pointer to XPm_Subsystem if successful else NULL
 *
 * @note
 * This is a less strict version of XPmSubsystem_GetByIndex(),
 * and mainly is implemented due to other modules such as xpm_device
 * needs to access the subsystem database and iterate over it using
 * indexes only, without the need to use the complete subsystem ID.
 * Use this function where it is absolutely necessary.
 *
 ****************************************************************************/
XPm_Subsystem *XPmSubsystem_GetByIndex(u32 SubSysIdx)
{
	XPm_Subsystem *Subsystem = NULL;
	XPm_SubsystemList* PmSubsystems = &SubsysMgr.Subsystems;
	/*
	 * We assume that Subsystem class, subclass and type have been
	 * validated before, so just validate index against bounds here
	 */
	LIST_FOREACH(PmSubsystems, SsNode){
		Subsystem = SsNode->Data;
		if (SubSysIdx == NODEINDEX(Subsystem->Id)) {
			break;
		}
	}
	return Subsystem;
}

/****************************************************************************/
/**
 * @brief  Get subsystem ID by matching IPI mask
 *
 * @param IpiMask     IPI mask to match against subsystem's IPI mask
 *
 * @return Subsystem ID if found and subsystem is not offline,
 *         INVALID_SUBSYSID otherwise
 *
 * @note   Searches through all subsystems to find one where the provided
 *         IPI mask matches the subsystem's IPI mask. Only returns subsystems
 *         that are not in OFFLINE state.
 *
 ****************************************************************************/
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	u32 SubsystemId = INVALID_SUBSYSID;
	XPm_Subsystem *Subsystem;
	XPm_SubsystemList* PmSubsystems = &SubsysMgr.Subsystems;
	/*
	 * Subsystem with least one IPI channel
	 * will have non-zero IPI mask.
	 */
	if (0U == IpiMask) {
		goto done;
	}
	LIST_FOREACH(PmSubsystems, SsNode){
		Subsystem = SsNode->Data;
		if (((Subsystem->IpiMask & IpiMask) == IpiMask) &&
		    ((u8)OFFLINE != Subsystem->State)) {
			SubsystemId = Subsystem->Id;
			break;
		}
	}

done:
	return SubsystemId;
}


/*
 * ============================================================================
 * Public API Implementations to abstract Subsystem Operations
 * ============================================================================
 * Following functions serve as the abstraction layer between runtime core and
 * subsystem module.
 * - xpm_runtime_eemi.c (EEMI-only builds)
 * - xpm_runtime_eemi_subsys.c (full subsystem builds)
 */

/****************************************************************************/
/**
 * @brief  Add a new subsystem to the power management framework
 *
 * @param SubsystemId     Subsystem ID to add
 *
 * @return XST_SUCCESS on success, XST_DEVICE_BUSY if subsystem already exists,
 *         XST_BUFFER_TOO_SMALL on allocation failure
 *
 * @note   Creates and initializes a new subsystem object with default state
 *         (ONLINE), adds it to the subsystem list, and updates the maximum
 *         subsystem index tracker.
 *
 ****************************************************************************/
XStatus XPmSubsystem_Add(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_SubsystemList *SubsysList = &SubsysMgr.Subsystems;
	XPm_Subsystem *Subsystem = NULL;

	/* Check if subsystem already exists */
	if (NULL != XPmSubsystem_GetById(SubsystemId)) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	/* Create subsystem */
	Subsystem = (XPm_Subsystem *)XPm_AllocBytesSubsys(sizeof(XPm_Subsystem));
	if (NULL == Subsystem) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Initialize subsystem */
	Subsystem->OpsType = SUBSYS_OPS_GENERIC;
	Subsystem->Id = SubsystemId;
	Subsystem->IpiMask = 0U;
	Subsystem->State = (u16)ONLINE;
	Subsystem->AllSubsystems = SubsysList;	/* Pointer to the list */
	Subsystem->Requirements = Make_XPm_RequirementList();
	if (NULL == Subsystem->Requirements){
		PmErr("There's no space to allocate RequirementList");
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Add the subsystem to the list */
	LIST_PREPEND(SubsysList, Subsystem);
	SubsysMgr.NumSubsystems++;
	if (SubsysMgr.MaxSubsystemIdx < NODEINDEX(Subsystem->Id)) {
		SubsysMgr.MaxSubsystemIdx = NODEINDEX(Subsystem->Id);
	}

	Status = XST_SUCCESS;
done:
	if (XST_SUCCESS != Status) {
		PmErr("Failed to add Subsystem 0x%x: 0x%x\r\n", SubsystemId, Status);
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief  Handler for idle subsystem and force down cleanup
 *
 * @param SubsystemId     Target Subsystem ID
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code
 * or a reason code
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmSubsystem_ForcePwrDwn(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->ShutDown(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Activate a subsystem by requesting all pre-allocated devices
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate Activate
 *         implementation via the SubsystemOps table. The actual implementation
 *         may be a common function or a build-specific override.
 *
 ****************************************************************************/
XStatus XPmSubsystem_Activate(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->Activate(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Set the state of a subsystem
 *
 * @param Subsystem     Pointer to subsystem object
 * @param State         New state value (must be less than MAX_STATE)
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate SetState
 *         implementation via the SubsystemOps table.
 *
 ****************************************************************************/
XStatus XPmSubsystem_SetState(XPm_Subsystem *Subsystem, u32 State)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->SetState(Subsystem, State);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Get the current status/state of a subsystem
 *
 * @param Subsystem     Pointer to subsystem object
 * @param DeviceStatus  Pointer to device status structure to populate
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem or
 *         DeviceStatus is NULL, error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate GetStatus
 *         implementation via the SubsystemOps table.
 *
 ****************************************************************************/
XStatus XPmSubsystem_GetStatus(XPm_Subsystem *Subsystem, XPm_DeviceStatus *const DeviceStatus)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem || NULL == DeviceStatus) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->GetStatus(Subsystem, DeviceStatus);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Add a requirement (device/reset/subsystem) to a subsystem
 *
 * @param Subsystem     Pointer to subsystem object
 * @param Payload       Requirement payload containing node-specific arguments
 * @param PayloadLen    Length of payload array
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem, Payload
 *         is NULL or PayloadLen is 0, error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate AddRequirement
 *         implementation via the SubsystemOps table. The payload format depends
 *         on the node type being added as a requirement.
 *
 ****************************************************************************/
XStatus XPmSubsystem_AddRequirement(XPm_Subsystem *Subsystem, u32 *Payload, u32 PayloadLen)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem || NULL == Payload || 0U == PayloadLen) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->AddRequirement(Subsystem, Payload, PayloadLen);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Wake up a subsystem from suspended state
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate WakeUp
 *         implementation via the SubsystemOps table.
 *
 ****************************************************************************/
XStatus XPmSubsystem_WakeUp(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->WakeUp(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Suspend a subsystem (enter low-power state)
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate Suspend
 *         implementation via the SubsystemOps table.
 *
 ****************************************************************************/
XStatus XPmSubsystem_Suspend(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->Suspend(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Idle a subsystem (prepare for low-power state)
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate Idle
 *         implementation via the SubsystemOps table.
 *
 ****************************************************************************/
XStatus XPmSubsystem_Idle(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->Idle(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Finalize subsystem initialization after successful boot
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate InitFinalize
 *         implementation via the SubsystemOps table. Typically notifies healthy
 *         boot to stop healthy boot monitors.
 *
 ****************************************************************************/
XStatus XPmSubsystem_InitFinalize(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->InitFinalize(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Check if subsystem has permission to access a node
 *
 * @param Subsystem     Pointer to subsystem object
 * @param NodeId        Node ID to check access for
 *
 * @return XST_SUCCESS if allowed, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code if denied
 *
 * @note   Wrapper function that dispatches to the appropriate IsAccessAllowed
 *         implementation via the SubsystemOps table. Checks permissions based
 *         on node class (device, clock, reset, pin, etc.).
 *
 ****************************************************************************/
XStatus XPmSubsystem_IsAccessAllowed(XPm_Subsystem *Subsystem, u32 NodeId)
{
	volatile XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->IsAccessAllowed(Subsystem, NodeId);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Notify that subsystem has booted successfully (releases healthy boot monitors)
 *
 * @param Subsystem     Pointer to subsystem object
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate NotifyHealthyBoot
 *         implementation via the SubsystemOps table. Releases healthy boot monitor
 *         devices allocated to the subsystem.
 *
 ****************************************************************************/
XStatus XPmSubsystem_NotifyHealthyBoot(XPm_Subsystem *Subsystem)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->NotifyHealthyBoot(Subsystem);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Start recovery timer for subsystem (healthy boot monitor)
 *
 * @param Subsystem     Pointer to subsystem object
 * @param CmdType       Command type (secure/non-secure)
 *
 * @return XST_SUCCESS on success, XPM_INVALID_SUBSYSID if subsystem is NULL,
 *         error code on failure
 *
 * @note   Wrapper function that dispatches to the appropriate StartRecoveryTimer
 *         implementation via the SubsystemOps table. Requests runtime healthy
 *         boot monitor device to start recovery timer.
 *
 ****************************************************************************/
XStatus XPmSubsystem_StartRecoveryTimer(XPm_Subsystem *Subsystem, u32 CmdType)
{
	XStatus Status = XST_FAILURE;
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	XPm_SubsystemOps *Ops = XPmSubsystem_GetOps(Subsystem);
	Status = Ops->StartRecoveryTimer(Subsystem, CmdType);
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  Initialize the subsystem module
 *
 * @return XST_SUCCESS
 *
 * @note   This function is reserved for future module initialization needs.
 *         Currently returns success as a placeholder.
 *
 ****************************************************************************/
XStatus XPmSubsystem_ModuleInit(void)
{
	/* Keep this function for module initialization if needed in future */
	return XST_SUCCESS;
}
