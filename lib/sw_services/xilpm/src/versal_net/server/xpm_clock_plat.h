/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_CLOCK_PLAT_H_
#define XPM_CLOCK_PLAT_H_

#include "xpm_common.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISCPMCLK(idx)	(((idx) == (u32)XPM_NODEIDX_CLK_CPLL) ||			\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_PRESRC) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_PLL) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_POSTCLK) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_PLL_OUT) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_CORE_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_DBG_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_AUX0_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_AUX1_REF) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM_TOPSW) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_TOPSW_REF) ||	\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_DMA_ALT) ||		\
			((idx) == (u32)XPM_NODEIDX_CLK_CPM5N_AUX2_REF))
#define CLK_DUMMY_PARENT	(0xFFFFFFFFU)
typedef struct XPm_ClockNode XPm_ClockNode;
typedef struct XPm_ClockHandle XPm_ClockHandle;
XStatus XPmClock_SaveClockNode(XPm_ClockNode* ClockNodeData, u32** SavedData);
XStatus XPmClock_RestoreClockNode(u32* SavedData, XPm_ClockNode* ThisData, u32** NextSavedData);
XStatus XPmClock_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op);
#ifdef __cplusplus
}
#endif

#endif /* XPM_CLOCK_PLAT_H_ */
