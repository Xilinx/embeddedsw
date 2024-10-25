/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_API_PLAT_H_
#define XPM_API_PLAT_H_

#include "xplmi_cmd.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xpm_defs.h"
#include "xpm_nodeid.h"
#include "xpm_common.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Persistent global general storage register base address */
#define PGGS_BASEADDR	(0xF1110050U)

typedef struct XPm_Subsystem XPm_Subsystem;
typedef struct XPm_Core XPm_Core;

int XPm_RegisterWakeUpHandlers(void);


XStatus XPm_PlatCmnFlush(const u32 SubsystemId);
XStatus XPm_InitNode(u32 NodeId, u32 Function, const u32 *Args, u32 NumArgs);
XStatus XPm_HookAfterPlmCdo(void);

XStatus XPm_PlatAddNode(const u32 *Args, u32 NumArgs);

XStatus XPm_PlatFeatureCheck(const u32 ApiId, u32 *const Version);

int XPm_PlatProcessCmd(XPlmi_Cmd *Cmd);

XStatus XPm_PlatAddNodePower(const u32 *Args, u32 NumArgs);

maybe_unused static inline XStatus XPm_PlatAddNodePeriph(const u32 *Args, u32 PowerId)
{
	/*unused function in versal net*/
	(void)Args;
	(void)PowerId;
	return XST_FAILURE;
}

maybe_unused static inline XStatus XPm_PlatAddDevice(const u32 *Args, u32 NumArgs)
{
	(void)Args;
	(void)NumArgs;
	return XST_INVALID_PARAM;
}

XStatus XPm_PlatInit(void);
maybe_unused static inline void XPm_DisableSkipHC(void)
{
	/*unused function in versal net*/
}

maybe_unused static inline XStatus XPm_EnableDdrSr(const u32 SubsystemId)
{
	/*
	 * TODO: If subsystem is using DDR and NOC Power Domain is idle,
	 * enable self-refresh as post suspend requirement
	 */
	(void)SubsystemId;
	return XST_SUCCESS;
}
maybe_unused static inline XStatus XPm_DisableDdrSr(const u32 SubsystemId)
{
	/* TODO: If subsystem is using DDR, disable self-refresh */
	(void)SubsystemId;
	return XST_SUCCESS;
}
maybe_unused static inline void XPm_ClearScanClear(void)
{
	XPm_Out32(PSMX_GLOBAL_SCAN_CLEAR_TRIGGER, 0U);
	XPm_Out32(PSMX_GLOBAL_MEM_CLEAR_TRIGGER, 0U);
}
maybe_unused static inline XStatus IsOnSecondarySLR(u32 SubsystemId)
{
	(void)SubsystemId;
	return XST_FAILURE;
}
void XPm_PlatChangeCoreState(XPm_Core *Core, const u32 State);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_API_PLAT_H_ */
