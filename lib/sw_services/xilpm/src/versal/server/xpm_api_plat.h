/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_API_PLAT_H_
#define XPM_API_PLAT_H_

#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xpm_common.h"
#include "xplmi_ipi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xF1110050U)

typedef struct XPm_Subsystem XPm_Subsystem;
typedef struct XPm_Core XPm_Core;

int XPm_RegisterWakeUpHandlers(void);

XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs);
XStatus XPm_HookAfterPlmCdo(void);
XStatus XPm_HookAfterBootPdi(void);
XStatus XPm_NocClockEnable(u32 NodeId, const u32 *Args, u32 NumArgs);
XStatus XPm_IfNocClockEnable(XPlmi_Cmd *Cmd, const u32 *Args, u32 NumArgs);

int XPm_PlatProcessCmd(XPlmi_Cmd * Cmd);
XStatus XPm_PlatInit(void);
XStatus XPm_PlatAddNode(const u32 *Args, u32 NumArgs);
XStatus XPm_PlatAddNodePower(const u32 *Args, u32 NumArgs);
XStatus XPm_PlatAddDevice(const u32 *Args, u32 NumArgs);
XStatus XPm_PlatFeatureCheck(const u32 ApiId, u32 *const Version);
XStatus IsOnSecondarySLR(u32 SubsystemId);
XStatus XPm_PlatAddNodePeriph(const u32 *Args, u32 PowerId);
void XPm_DisableSkipHC(void);

maybe_unused static inline XStatus XPm_PlatRequestDevice(const u32 DeviceId)
{
	(void)DeviceId;
	return XST_SUCCESS;
}
maybe_unused static inline void XPm_ClearScanClear(void)
{
}
maybe_unused static inline XStatus XPm_PlatCmnFlush(const u32 SubsystemId)
{
	(void)SubsystemId;
	return XST_SUCCESS;
}
XStatus XPm_EnableDdrSr(const u32 SubsystemId);
XStatus XPm_DisableDdrSr(const u32 SubsystemId);
XStatus XPm_ForceHouseClean(u32 NodeId);
void XPmPlDevice_SetStlInitFinishCb(void (*Handler)(void));
void XPm_PlatChangeCoreState(XPm_Core *Core, const u32 State);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_PLAT_H_ */
