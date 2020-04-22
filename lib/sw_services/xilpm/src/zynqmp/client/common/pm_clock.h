/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 * @file pm_clock.h
 *
 * PM Definitions of clocks - for xilpm internal purposes only
 *****************************************************************************/

#ifndef PM_CLOCKS_H_
#define PM_CLOCKS_H_

#include <xil_types.h>
#include <xstatus.h>
#include "pm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPm_GetClockParentBySelect(const enum XPmClock clock,
				   const u32 select,
				   enum XPmClock* const parent);

XStatus XPm_GetSelectByClockParent(const enum XPmClock clock,
				   const enum XPmClock parent,
				   u32* const select);

u8 XPm_GetClockDivType(const enum XPmClock clock);

u8 XPm_MapDivider(const enum XPmClock clock,
		  const u32 div,
		  u32* const div0,
		  u32* const div1);

#ifdef __cplusplus
}
#endif

#endif /* PM_CLOCKS_H_ */
