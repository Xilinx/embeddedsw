/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_SUBSYSTEM_H_
#define XPM_SUBSYSTEM_H_

#include "xpm_api.h"
#include "xpm_defs.h"
#include "xstatus.h"
#include "xpm_list.h"
#include "xpm_requirement.h"

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
#define SUBSYSTEM_SUSCB_PRIORITIZE              ((u8)1U << 3U)

/**
 * Helper macros to check subsystem specific flags.
 */
#define IS_SUBSYS_CONFIGURED(Flags)		(SUBSYSTEM_IS_CONFIGURED == ((Flags) & SUBSYSTEM_IS_CONFIGURED))
#define IS_SUBSYS_INIT_FINALIZED(Flags)		(SUBSYSTEM_INIT_FINALIZED == ((Flags) & SUBSYSTEM_INIT_FINALIZED))

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

/**
 * Subsystem Ops Types
 */
typedef enum {
	SUBSYS_OPS_GENERIC = 0U,
	SUBSYS_OPS_MAX	/* Always keep this as the last element */
} XPm_SubsysOpsType;

typedef struct XPm_FrcPwrDwnReq {
	u32 AckType;
	u32 InitiatorIpiMask;
} XPm_FrcPwrDwnReq;

// /* Forward declaration */
typedef struct XPm_Subsystem XPm_Subsystem;
typedef struct XPm_SubsystemMgr XPm_SubsystemMgr;
typedef struct XPm_RequirementList XPm_RequirementList;

CREATE_LIST(XPm_Subsystem);

typedef struct XPm_Permissions {
	/*
	 *  bits[0:15] non secure operations
	 *  bits[16:31] secure operations
	 */
	u32 WakeupPerms;
	u32 SuspendPerms;
	u32 PowerdownPerms;
} XPm_Permissions;

/**
 * The Pending suspend callback
 */
typedef struct XPm_PendSuspCb {
	u32 Reason;
	u32 Latency;
	u32 State;
	int (*SuspendCb)(u32 IpiMask, u32 Event, u32 *Payload);
} XPm_PendSuspCb;

/**
 * Subsystem Ops
 */
typedef struct XPm_SubsystemOps {
    XStatus (*Activate)(XPm_Subsystem *Subsystem);
    XStatus (*SetState)(XPm_Subsystem *Subsystem, u32 State);
    XStatus (*InitFinalize)(XPm_Subsystem *Subsystem);
    XStatus (*GetStatus)(XPm_Subsystem *Subsystem, XPm_DeviceStatus *const DeviceStatus);
    XStatus (*AddPermissions)(XPm_Subsystem *Subsystem, u32 TargetId, u32 Operations);
    XStatus (*ShutDown)(XPm_Subsystem *Subsystem);
    XStatus (*WakeUp)(XPm_Subsystem *Subsystem);
    XStatus (*Suspend)(XPm_Subsystem *Subsystem);
    XStatus (*Idle)(XPm_Subsystem *Subsystem);
    XStatus (*IsAccessAllowed)(XPm_Subsystem *Subsystem, u32 NodeId);
    XStatus (*StartBootTimer)(XPm_Subsystem *Subsystem);
    XStatus (*StopBootTimer)(XPm_Subsystem *Subsystem);
    XStatus (*StartRecoveryTimer)(XPm_Subsystem *Subsystem);
    XStatus (*StopRecoveryTimer)(XPm_Subsystem *Subsystem);
} XPm_SubsystemOps;

/**
 * The subsystem class
 */
struct XPm_Subsystem {
	u32 Id; /**< Subsystem ID */
	u32 IpiMask; /**< IPI mask associated with the subsystem */
	u16 State; /**< Subsystem state */
	u16 Flags; /**< Subsystem specific flags */
	XPm_Permissions Perms; /**< Subsystem permissions */
	XPm_PendSuspCb PendCb; /**< Pending suspend callback */
	XPm_FrcPwrDwnReq FrcPwrDwnReq; /** Force power down request */
	XPm_RequirementList *Requirements; /**< Head of the requirement list for all devices within the subsystem */
	XPm_SubsystemOps *Ops; 		/**< Subsystem operations */
	XPm_SubsystemList *AllSubsystems; /**< Head of the list of all subsystems */
};

/**
 * The subsystem manager
 */
struct XPm_SubsystemMgr {
	XPm_SubsystemList Subsystems; /**< List of subsystem objects */
	XPm_SubsystemOps SubsysOps[SUBSYS_OPS_MAX]; /**< Subsystem operations supported */
	u32 NumSubsystems; /**< Total number of subsystems */
};

/************************** Function Prototypes ******************************/

XStatus XPmSubsystem_ModuleInit(void);
u32 XPmSubsystem_GetIPIMask(u32 SubsystemId);
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType);
XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId);
XStatus XPmSubsystem_Add(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetById(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetByIndex(u32 SubSysIdx);
u32 XPmSubsystem_GetMaxSubsysIdx(void);
XStatus XPmSubsystem_ForcePwrDwn(u32 SubsystemId);
XStatus XPmSubsystem_NotifyHealthyBoot(const u32 SubsystemId);
XStatus XPmSubsystem_IsOperationAllowed(const u32 HostId, const u32 TargetId,
					const u32 Operation, const u32 CmdType);
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId, u32 CmdType);

#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_H_ */
