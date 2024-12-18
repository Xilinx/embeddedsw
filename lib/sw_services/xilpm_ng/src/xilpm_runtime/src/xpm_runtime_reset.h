/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_RESET_H_
#define XPM_RUNTIME_RESET_H_

#include "xpm_node.h"
#include "xpm_common.h"
#include "xpm_subsystem.h"
#include "xpm_reset.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RESET_PERM_SHIFT_NS	(0U)
#define RESET_PERM_SHIFT_S	(0U + MAX_NUM_SUBSYSTEMS)
typedef struct XPmRuntime_Reset XPmRuntime_Reset;

CREATE_LIST(XPmRuntime_Reset);

struct XPmRuntime_Reset {
        XPm_ResetNode *Device;
        u32 AllowedSubsystems;
};

/************************** Function Prototypes ******************************/
XStatus XPmReset_SetAllowedSubsystems(XPm_ResetNode *Reset, const u32 AllowSubsystems);
XStatus XPmReset_GetAllowedSubsystems(const XPm_ResetNode *Reset, u32 *AllowSubsystems);
XStatus XPmReset_AssertbyId(u32 ResetId, const u32 Action);
XStatus XPmReset_CheckPermissions(const XPm_Subsystem *Subsystem, u32 ResetId);
XStatus XPmReset_SystemReset(void);
XStatus XPmReset_IsPermissionReset(const u32 ResetId);
XStatus XPmReset_AddPermForGlobalResets(const XPm_Subsystem *Subsystem);
 XStatus XPmReset_IsOperationAllowed(const u32 SubsystemId,
				    const XPm_ResetNode *Rst,
				    const u32 CmdType);
 XStatus XPmReset_AddPermission(XPm_ResetNode *Rst,
			       const XPm_Subsystem *Subsystem,
			       const u32 Operations);
// void XPmReset_MakeCpmPorResetCustom(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_RESET_H_ */
