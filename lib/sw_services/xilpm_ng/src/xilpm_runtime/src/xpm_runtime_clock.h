/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_CLOCK_H_
#define XPM_RUNTIME_CLOCK_H_

#include "xpm_node.h"
#include "xpm_common.h"
#include "xpm_subsystem.h"
#include "xpm_clock.h"
#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmClock_CheckPermissions(u32 SubsystemIdx, u32 ClockId);
XStatus XPmClock_Release(const XPm_ClockHandle *ClkHandle);
/** TODO: implement these functions */
XStatus XPmClock_Request(const XPm_ClockHandle *ClkHandle);
XStatus XPmClock_SetGate(XPm_OutClockNode *Clk, u32 Enable);
XStatus XPmClock_SetDivider(const XPm_OutClockNode *Clk, u32 Divider);
XStatus XPmClock_QueryName(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryTopology(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryFFParams(u32 ClockId, u32 *Resp);
XStatus XPmClock_QueryMuxSources(u32 ClockId, u32 Index, u32 *Resp);
XStatus XPmClock_QueryAttributes(u32 ClockIndex, u32 *Resp);
XStatus XPmClock_GetNumClocks(u32 *Resp);
XStatus XPmClock_GetMaxDivisor(u32 ClockId, u32 DivType, u32 *Resp);
XStatus XPmClock_SetParent(XPm_OutClockNode *Clk, u32 ParentIdx);
#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_CLOCK_H_ */
