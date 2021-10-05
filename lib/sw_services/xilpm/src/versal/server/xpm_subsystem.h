/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_H_
#define XPM_SUBSYSTEM_H_

#include "xpm_api.h"
#include "xpm_defs.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	INVALID_SUBSYSID			(0xFFFFFFFFU)
#define MAX_NUM_SUBSYSTEMS			(16U)
#define SUBSYS_TO_NS_BITPOS(x)			(NODE_INDEX_MASK & ((x)))
#define SUBSYS_TO_S_BITPOS(x)			(NODE_INDEX_MASK & ((x) + MAX_NUM_SUBSYSTEMS))

/**
 * Subsystem specific flags.
 */
#define SUBSYSTEM_INIT_FINALIZED		((u8)1U << 0U)
#define SUBSYSTEM_IS_CONFIGURED			((u8)1U << 1U)
#define SUBSYSTEM_IDLE_SUPPORTED		((u8)1U << 2U)

/**
 * Helper macros to check subsystem specific flags.
 */
#define IS_SUBSYS_CONFIGURED(Flags)	(SUBSYSTEM_IS_CONFIGURED ==	\
					 ((Flags) & SUBSYSTEM_IS_CONFIGURED))
#define IS_SUBSYS_INIT_FINALIZED(Flags)	(SUBSYSTEM_INIT_FINALIZED ==	\
					 ((Flags) & SUBSYSTEM_INIT_FINALIZED))

#define SUB_PERM_WAKE_MASK			(0x1U << 0U)
#define SUB_PERM_PWRDWN_MASK			(0x1U << 1U)
#define SUB_PERM_SUSPEND_MASK			(0x1U << 2U)

#define SUB_PERM_WAKE_SHIFT_NS			(0U)
#define SUB_PERM_PWRDWN_SHIFT_NS		(1U)
#define SUB_PERM_SUSPEND_SHIFT_NS		(2U)

#define SUB_PERM_WAKE_SHIFT_S			(0U + MAX_NUM_SUBSYSTEMS)
#define SUB_PERM_PWRDWN_SHIFT_S			(1U + MAX_NUM_SUBSYSTEMS)
#define SUB_PERM_SUSPEND_SHIFT_S		(2U + MAX_NUM_SUBSYSTEMS)

#define PERM_BITMASK(Op, OpShift, SubShift)	((1U & ((Op) >> (OpShift))) << (SubShift))

/**
 * Subsystem creation states.
 */
typedef enum {
	OFFLINE,
	RESERVED,
	ONLINE,
	SUSPENDING,
	SUSPENDED,
	POWERED_OFF,
	PENDING_POWER_OFF,
	PENDING_RESTART,
	MAX_STATE
} XPm_SubsysState;

struct XPm_Permissions {
	/*
	 *  bits[0:15] non secure operations
	 *  bits[16:31] secure operations
	 */
	u32 WakeupPerms;
	u32 SuspendPerms;
	u32 PowerdownPerms;
};

typedef struct XPm_PendSuspCb XPm_PendSuspCb;

/**
 * The Pending suspend callback
 */
struct XPm_PendSuspCb {
	u32 Reason;
	u32 Latency;
	u32 State;
};

typedef struct XPm_Subsystem XPm_Subsystem;

/**
 * The subsystem class.
 */
struct XPm_Subsystem {
	u32 Id; /**< Subsystem ID */
	u8 State; /**< Subsystem state */
	u8 Flags; /**< Subsystem specific flags */
	u32 IpiMask;
	struct XPm_Permissions Perms;
	struct XPm_PendSuspCb PendCb;
	struct XPm_Reqm *Requirements;
		/**< Head of the requirement list for all devices. */
	struct XPm_FrcPwrDwnReq FrcPwrDwnReq;
	void (*NotifyCb)(u32 SubsystemId, const u32 EventId);
	XPm_Subsystem *NextSubsystem;
};

/************************** Function Prototypes ******************************/

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId);
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask);
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType);
XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State);
XStatus XPmSubsystem_Configure(u32 SubsystemId);
XStatus XPmSubsystem_Add(u32 SubsystemId);
XStatus XPmSubsystem_Destroy(u32 SubsystemId);
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType);
XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId);
XStatus XPmSubsystem_InitFinalize(const u32 SubsystemId);
XStatus XPmSubsystem_Idle(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetById(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetByIndex(u32 SubSysIdx);
XStatus XPmSubsystem_GetStatus(const u32 SubsystemId, const u32 DeviceId,
			       XPm_DeviceStatus *const DeviceStatus);
u32 XPmSubsystem_GetMaxSubsysIdx(void);
XStatus XPmSubsystem_AddPermission(const XPm_Subsystem *Host,
				   XPm_Subsystem *Target,
				   const u32 Operations);
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType);
XStatus XPmSubsystem_ForcePwrDwn(u32 SubsystemId);

#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_H_ */
