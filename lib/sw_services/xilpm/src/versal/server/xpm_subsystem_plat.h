/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_PLAT_H_
#define XPM_SUBSYSTEM_PLAT_H_

#include "xpm_common.h"
#include "xpm_node.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

maybe_unused static XStatus IsDevExcluded(const u32 DevId)
{
	XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();
	u32 PlatformVersion = XPm_GetPlatformVersion();

	if ((((u32)PM_DEV_L2_BANK_0 == DevId) ||
	    ((u32)XPM_NODETYPE_DEV_SOC == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_CORE_PMC == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_EFUSE == NODETYPE(DevId)) ||
	    ((u32)XPM_NODETYPE_DEV_XRAM == NODETYPE(DevId)) ||
	    ((u32)XPM_NODESUBCL_DEV_PHY == NODESUBCLASS(DevId)) ||
	    ((u32)XPM_NODEIDX_DEV_AMS_ROOT == NODEINDEX(DevId))) ||
	    (((u32)PM_DEV_GPIO == DevId) &&
	    ((u32)PLATFORM_VERSION_SILICON == Platform) &&
	    ((u32)PLATFORM_VERSION_SILICON_ES1 == PlatformVersion))) {
		Status = XST_SUCCESS;
	}

	return Status;
};

/************************** Function Prototypes ******************************/

XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId);
XStatus XPm_PinCheckPermission(const XPm_Subsystem *Subsystem, u32 NodeId);
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType);

#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_PLAT_H_ */
