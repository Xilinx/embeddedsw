/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

/* PLL fractional mode constants */
#define FRAC_DIV                (1U << 16)   /**< Fractional divisor (2^16) */
#define PLL_FBDIV_MIN           25U          /**< Minimum feedback divider value */
#define PLL_FBDIV_MAX           125U         /**< Maximum feedback divider value */
#define PLL_VCO_MIN             1500000000U  /**< Minimum VCO frequency (1.5 GHz) */
#define PLL_VCO_MAX             3000000000U  /**< Maximum VCO frequency (3.0 GHz) */

/* Valid divider values for PLL output divider clocks */
#define PLL_NUM_DIVIDERS        4U           /**< Number of PLL output dividers */

#ifdef __cplusplus
}
#endif

#endif /* XPM_RUNTIME_PLL_H_ */
