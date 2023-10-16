/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PLL_PLAT_H_
#define XPM_PLL_PLAT_H_

#include "xpm_clock.h"
#include "xpm_pll.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif
void XPm_PllClearLockError(const XPm_PllClockNode* Pll);
void XPmClockPll_PlatReset(XPm_PllClockNode* Pll);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PLL_PLAT_H_ */
