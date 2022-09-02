/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_CLOCK_PLAT_H_
#define XPM_CLOCK_PLAT_H_

#include "xpm_common.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISCPMCLK(idx)	(((idx) == (u32)XPM_NODEIDX_CLK_CPM_LSBUS_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_PLL) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_PRESRC) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_POSTCLK) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_PLL_OUT) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_CORE_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_DBG_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_AUX0_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_AUX1_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_TOPSW_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_DMA_ALT_REF) ||	\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_AUX2_REF))
#define CLK_DUMMY_PARENT		(-2)

void XPmClock_SetPlClockAsReadOnly(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_CLOCK_PLAT_H_ */
