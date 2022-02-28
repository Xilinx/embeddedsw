/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_nodeid.h"
#include "xpm_common.h"
#include "xpm_err.h"
#include "xpm_subsystem.h"

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

u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask)
{
	//changed to support minimum boot time xilpm
	PmDbg("%s IpiMask %x\n",__func__,IpiMask);
	(void)IpiMask;
	//this service is not supported at boot time
	PmDbg("supports default subsystem only\n");
	return PM_SUBSYS_DEFAULT;
}
