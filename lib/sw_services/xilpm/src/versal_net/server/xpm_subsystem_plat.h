/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_PLAT_H_
#define XPM_SUBSYSTEM_PLAT_H_

#include "xpm_node.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration */
typedef struct XPm_Subsystem XPm_Subsystem;

struct XPm_Permissions {
	/*
	 *  bits[0:15] non secure operations
	 *  bits[16:31] secure operations
	 */
	u32 WakeupPerms;
	u32 SuspendPerms;
	u32 PowerdownPerms;
};

/**
 * The Pending suspend callback
 */
struct XPm_PendSuspCb {
	u32 Reason;
	u32 Latency;
	u32 State;
};

/**
 * The subsystem class.
 */
struct XPm_Subsystem {
	u32 Id; /**< Subsystem ID */
SAVE_REGION(
	u8 State; /**< Subsystem state */
	u8 Flags; /**< Subsystem specific flags */
	u32 IpiMask;
	struct XPm_PendSuspCb PendCb;
	struct XPm_FrcPwrDwnReq FrcPwrDwnReq;
	struct XPm_Permissions Perms;
)
	struct XPm_Reqm *Requirements;
		/**< Head of the requirement list for all devices. */
	void (*NotifyCb)(u32 SubsystemId, const u32 EventId);
	XPm_Subsystem *NextSubsystem;
};

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

/************************** Function Prototypes ******************************/

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
