/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_PLAT_H_
#define XPM_SUBSYSTEM_PLAT_H_

#include "xpm_node.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Subsystem XPm_Subsystem;

maybe_unused static XStatus IsDevExcluded(const u32 DevId)
{
	XStatus Status = XST_FAILURE;

	if (((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(DevId))) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/************************** Function Prototypes ******************************/
maybe_unused static inline XStatus XPm_PinCheckPermission(const XPm_Subsystem *Subsystem, u32 NodeId)
{
        (void)Subsystem;
        (void)NodeId;

        return XST_FAILURE;
}

/*
 * Handle the healthy boot notification from the subsystem
 */
maybe_unused static inline XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId)
{
        (void)SubsystemId;

        return XST_SUCCESS;
}

maybe_unused static inline XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
                                        const u32 Operation, const u32 CmdType)
{
	(void)HostId;
	(void)TargetId;
	(void)Operation;
	(void)CmdType;

	return XST_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_PLAT_H_ */
