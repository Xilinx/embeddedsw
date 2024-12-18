/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xpm_subsystem.h"
#include "xpm_device.h"
#include "xpm_requirement.h"
#include "xpm_runtime_device.h"
#include "xpm_notifier.h"

/* this needs to be global to run the unit tests independently*/
//static u32 MaxSubsysIdx = 1;
//static XPm_SubsystemList* PmSubsystems;

extern XPm_SubsystemMgr SubsysMgr;

u32 XPmSubsystem_GetMaxSubsysIdx(void)
{
	return SubsysMgr.NumSubsystems;
}

XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}
	Status = Subsystem->Ops->IsAccessAllowed(Subsystem, NodeId);
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

/****************************************************************************/
/**
 * @brief  Handler for idle subsystem and force down cleanup
 *
 * @param Subsystem     Target Subsystem
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
	Status = Subsystem->Ops->ShutDown(Subsystem);
done:
	return Status;
}

XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Subsystem *Subsystem;

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XPM_INVALID_SUBSYSID;
		goto done;
	}

	Status = XPmRequirement_Release(LIST_FIRST_DATA(Subsystem->Requirements), RELEASE_ALL);
		/* Todo: Cancel wakeup if scheduled
		 * Should be included with wakeup support
		XPm_WakeUpCancelScheduled(SubSysIdx);*/

	/* Unregister all notifiers for this subsystem */
	Status = XPmNotifier_UnregisterAll(Subsystem);

done:
	return Status;
}
maybe_unused static XStatus IsDevExcluded(const u32 DevId)
{
	XStatus Status = XST_FAILURE;

	if (((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_XRAM == NODETYPE(DevId)) ||
	    ((u32)XPM_NODESUBCL_DEV_PHY == NODESUBCLASS(DevId)) ||
	    ((u32)XPM_NODEIDX_DEV_AMS_ROOT == NODEINDEX(DevId))) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*
 * Handle the healthy boot notification from the subsystem
 */
XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;
	const XPm_Requirement *Reqm;
	u32 Idx;

	for (Idx = (u32)XPM_NODEIDX_DEV_HB_MON_0;
			Idx < (u32)XPM_NODEIDX_DEV_HB_MON_MAX; Idx++) {
		/*
		 * Iterate through available Healthy Boot Monitor nodes
		 * and release it, if it is part of the given subsystem
		 */
		Device = XPmDevice_GetHbMonDeviceByIndex(Idx);
		if (NULL != Device) {
			Reqm = XPmDevice_FindRequirement(Device->Node.Id, SubsystemId);
			if (NULL != Reqm) {
				Status = XPmDevice_Release(SubsystemId, Device->Node.Id,
							   XPLMI_CMD_SECURE);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

XStatus XPmSubsystem_ModuleInit(void)
{
	XStatus Status = XST_FAILURE;
	//if (NULL == PmSubsystems) {
	//	PmSubsystems = &SubsysMgr.Subsystems;
	//}
	Status = XST_SUCCESS;
	return Status;
}
