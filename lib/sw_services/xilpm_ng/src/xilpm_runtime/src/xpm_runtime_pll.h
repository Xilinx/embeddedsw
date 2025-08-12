/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RUNTIME_PLL_H_
#define XPM_RUNTIME_PLL_H_

#include "xpm_clock.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct XPm_PllClockNode XPm_PllClockNode;


/************************** Function Prototypes ******************************/
XStatus XPmClockPll_SetMode(XPm_PllClockNode *Pll, u32 Mode);
XStatus XPmClockPll_GetMode(XPm_PllClockNode *Pll, u32 *Mode);
XStatus XPmClockPll_SetParam(XPm_PllClockNode *Pll, u32 Param,u32 Value);
XStatus XPmClockPll_GetParam(const XPm_PllClockNode *Pll, u32 Param,u32 *Val);
XStatus XPmClockPll_QueryMuxSources(u32 Id, u32 Index, u32 *Resp);
XStatus XPmClockPll_GetWakeupLatency(const u32 Id, u32 *Latency);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RUNTIME_PLL_H_ */
