/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_PLAT_H_
#define XPM_APUCORE_PLAT_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_ApuCore XPm_ApuCore;

/************************** Function Prototypes ******************************/
XStatus XPmApuCore_AssignRegisterMask(struct XPm_ApuCore *ApuCore, const u32 Id);
XStatus XPm_ApuSetOperMode(const u32 ClusterId, const u32 Mode);
XStatus XPm_ApuGetOperMode(const u32 ClusterId, u32 *Mode);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_PLAT_H_ */
