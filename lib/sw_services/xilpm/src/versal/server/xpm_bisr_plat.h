/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_BISR_PLAT_H_
#define XPM_BISR_PLAT_H_

#include "xpm_common.h"
#ifdef SDT
#include "xpm_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

XStatus XPmBisr_Repair(u32 TagId);
XStatus XPmBisr_NidbLeftMostLaneRepair(void);
XStatus XPmBisr_NidbLaneRepair(void);
XStatus XPmBisr_TriggerLpd(void);

#ifdef __cplusplus
}
#endif

#endif /* XPM_BISR_PLAT_H_ */
