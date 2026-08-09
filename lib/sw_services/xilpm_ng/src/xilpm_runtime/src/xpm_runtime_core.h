/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_CORE_H
#define XPM_RUNTIME_CORE_H
#include "xstatus.h"
#include "xil_types.h"
#include "xpm_subsystem.h"
#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct XPmRuntime_Core XPmRuntime_Core;
CREATE_LIST(XPmRuntime_Core)
struct XPmRuntime_Core {
	XPm_Core * Core;
	u8 IsCoreIdleSupported; /**< Flag for core idle is supported */
	struct XPm_FrcPwrDwnReq FrcPwrDwnReq;
	/** TODO: enable these  members when they're actually usedd */
	// u16 PwrUpLatency;
	// u16 PwrDwnLatency;
	// u8 DebugMode; DebugMode: Debugger is connected
};

XStatus XPmCore_GetCoreIdleSupport(const XPm_Core* Core, u8 *IsCoreIdleSupported);
XStatus XPmCore_SetCoreIdleSupport(XPm_Core* Core, const u32 Value);
XStatus XPmCore_ReleaseFromSubsys(XPm_Core *Core);
XStatus ResetAPUGic(const u32 DeviceId);
#ifdef __cplusplus
}
#endif
#endif /* XPM_RUNTIME_CORE_H */
