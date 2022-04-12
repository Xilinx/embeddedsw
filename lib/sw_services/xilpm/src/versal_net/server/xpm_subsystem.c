/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi.h"
#include "xplmi_scheduler.h"
#include "xpm_subsystem.h"
#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_reset.h"
#include "xpm_debug.h"
#include "xpm_device.h"
#include "xpm_regs.h"

static XPm_Subsystem *PmSubsystems;
static u32 MaxSubsysIdx;

XStatus XPmSubsystem_Configure(u32 SubsystemId)
{
	//changed to support minimum boot time xilpm
	XStatus Status = XST_FAILURE;
	PmDbg("SubsystemId %x\n",SubsystemId);
	if(PM_SUBSYS_DEFAULT == SubsystemId){
		Status = XST_SUCCESS;
	}else{
		PmErr("Invalid Subsystem %x\n",SubsystemId);
		Status = XPM_INVALID_SUBSYSID;
	}

	return Status;

}

/****************************************************************************/
/**
 * @brief  This function gives Subsystem from SubsystemId.
 *
 * @param SubsystemId	Subsystem ID
 *
 * @return XPm_Subsystem if successful else NULL
 *
 * @note
 *  If the ID is greater than MAX_NUM_SUBSYSTEMS+2 then the ID is  outside of
 *  supported IDs for subsystem permissions logic. The +2 is for PMC and
 *  default subsystem.
 *
 ****************************************************************************/
XPm_Subsystem * XPmSubsystem_GetById(u32 SubsystemId)
{
	XPm_Subsystem *SubSystem = NULL;

	if ((INVALID_SUBSYSID == SubsystemId) ||
	    (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId))) {
		goto done;
	}

	SubSystem = PmSubsystems;
	while (NULL != SubSystem) {
		if (SubSystem->Id == SubsystemId) {
			break;
		}
		SubSystem = SubSystem->NextSubsystem;
	}

done:
	return SubSystem;
}

/****************************************************************************/
/**
 * @brief  This function gives Subsystem from Subsystem "INDEX".
 *
 * @param SubSysIdx	Subsystem Index
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
	XPm_Subsystem *Subsystem = PmSubsystems;

	/*
	 * We assume that Subsystem class, subclass and type have been
	 * validated before, so just validate index against bounds here
	 */
	while (NULL != Subsystem) {
		if (SubSysIdx == NODEINDEX(Subsystem->Id)) {
			break;
		}
		Subsystem = Subsystem->NextSubsystem;
	}

	return Subsystem;
}

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	//changed to support minimum boot time xilpm
	PmDbg("%s IpiMask %x\n",__func__,IpiMask);
	(void)IpiMask;
	//this service is not supported at boot time
	PmDbg("supports default subsystem only\n");
	return PM_SUBSYS_DEFAULT;
}

XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State)
{
	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem = XPmSubsystem_GetById(SubsystemId);

	if (((u32)MAX_STATE <= State) || (NULL == Subsystem)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (((u32)POWERED_OFF == State) || ((u32)OFFLINE == State)) {
		Subsystem->Flags &= (u8)(~SUBSYSTEM_IS_CONFIGURED);
	}

	Subsystem->State = (u8)State;

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmSubsystem_Add(u32 SubsystemId)
{

	XStatus Status = XST_FAILURE;
	XPm_Subsystem *Subsystem;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	if (((u32)XPM_NODECLASS_SUBSYSTEM != NODECLASS(SubsystemId)) ||
	    ((u32)XPM_NODESUBCL_SUBSYSTEM != NODESUBCLASS(SubsystemId)) ||
	    ((u32)XPM_NODETYPE_SUBSYSTEM != NODETYPE(SubsystemId))) {
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		goto done;
	}
	/*
	 * Ensure the subsystem being added is within the range of supported
	 * subsystem IDs for the subsystem permissions logic.
	 */
	if (MAX_NUM_SUBSYSTEMS <= NODEINDEX(SubsystemId)) {
		DbgErr = XPM_INT_ERR_INVALID_SUBSYSTEMID;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/*
	 * Check if subsystem is being re-added, skip for default
	 * subsystem since it is already validated before
	 */
	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if ((NULL != Subsystem) && ((u8)OFFLINE != Subsystem->State)) {
		DbgErr = XPM_INT_ERR_SUBSYS_ADDED;
		Status = XST_FAILURE;
		goto done;
	}

	Subsystem = (XPm_Subsystem *)XPm_AllocBytes(sizeof(XPm_Subsystem));
	if (NULL == Subsystem) {
		DbgErr = XPM_INT_ERR_BUFFER_TOO_SMALL;
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	Subsystem->NextSubsystem = PmSubsystems;
	Subsystem->Id = SubsystemId;
	Subsystem->PendCb.Reason = 0U;
	Subsystem->PendCb.Latency = 0U;
	Subsystem->PendCb.State = 0U;
	if (PM_SUBSYS_PMC == SubsystemId) {
		Subsystem->Flags = SUBSYSTEM_INIT_FINALIZED;
		Subsystem->IpiMask = PMC_IPI_MASK;
	} else {
		Subsystem->Flags = 0U;
		Subsystem->IpiMask = 0U;
	}
	PmSubsystems = Subsystem;

	if (NODEINDEX(SubsystemId) > MaxSubsysIdx) {
		MaxSubsysIdx = NODEINDEX(SubsystemId);
	}

	Status = XPmSubsystem_SetState(SubsystemId, (u32)ONLINE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_SUBSYS_SET_STATE;
		goto done;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}