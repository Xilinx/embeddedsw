/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPFW_MOD_OVERTEMP_H_
#define XPFW_MOD_OVERTEMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_config.h"
#include "xil_types.h"

#ifdef ENABLE_RUNTIME_OVERTEMP
#define OT_LIMIT_MIN		(s32)(-55)
#define OT_LIMIT_MAX		(s32)(125)

s32 OverTempCfgInit(void);
s32 OverTempCfgDeInit(void);
void SetOverTempLimit(u32 DegCel);
u32 GetOverTempLimit(void);
#else
void ModOverTempInit(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* XPFW_MOD_OVERTEMP_H_ */
