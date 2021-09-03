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

/** @cond xilpm_internal */
XStatus XPm_GetClockParentBySelect(const enum XPmClock clockId,
				   const u32 select,
				   enum XPmClock* const parentId);

XStatus XPm_GetSelectByClockParent(const enum XPmClock clockId,
				   const enum XPmClock parentId,
				   u32* const select);

u8 XPm_GetClockDivType(const enum XPmClock clockId);

u8 XPm_MapDivider(const enum XPmClock clockId,
		  const u32 div_val,
		  u32* const div0,
		  u32* const div1);
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* PM_CLOCKS_H_ */
