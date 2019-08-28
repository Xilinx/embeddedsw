/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_H_
#define XPM_SUBSYSTEM_H_

#include "xpm_defs.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	INVALID_SUBSYSID			(0xFFFFFFFFU)

/**
 * Subsystem specific flags.
 */
#define SUBSYSTEM_INIT_FINALIZED		(1U << 0U)

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
	MAX_STATE
} XPm_SubsysState;

typedef struct XPm_Subsystem XPm_Subsystem;

/**
 * The subsystem class.
 */
struct XPm_Subsystem {
	u32 Id; /**< Subsystem ID */
	u8 State; /**< Subsystem state */
	u8 Flags; /**< Subsystem specific flags */
	u32 IpiMask;
	struct XPm_Reqm *Requirements;
		/**< Head of the requirement list for all devices. */
	void (*NotifyCb)(u32 SubsystemId, const u32 EventId);
	XPm_Subsystem *NextSubsystem;
};

/************************** Function Prototypes ******************************/

u32 XPmSubsystem_GetIPIMask(u32 SubsystemId);
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask);
XStatus XPm_IsWakeAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPm_IsAccessAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_SetState(const u32 SubsystemId, const u32 State);
XStatus XPmSubsystem_Add(u32 SubsystemId);
XStatus XPmSubsystem_Destroy(u32 SubsystemId);
XStatus XPmSubsystem_IsAllProcDwn(u32 SubsystemId);
XStatus XPm_IsForcePowerDownAllowed(u32 SubsystemId, u32 NodeId);
XStatus XPmSubsystem_ForceDownCleanup(u32 SubsystemId);
int XPmSubsystem_InitFinalize(const u32 SubsystemId);
int XPmSubsystem_Idle(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetById(u32 SubsystemId);
XPm_Subsystem *XPmSubsystem_GetByIndex(u32 SubSysIdx);
XStatus XPmSubsystem_SetCurrent(u32 SubsystemId);
u32 XPmSubsystem_GetCurrent(void);
XStatus XPmSubsystem_Restart(u32 SubsystemId);
XStatus XPmSubsystem_GetStatus(const u32 SubsystemId, const u32 DeviceId,
			       XPm_DeviceStatus *const DeviceStatus);
u32 XPmSubsystem_GetMaxSubsysIdx(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_H_ */
