/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPFW_MOD_EXTWDT_H_
#define XPFW_MOD_EXTWDT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_config.h"
#include "xil_types.h"

#ifdef ENABLE_RUNTIME_EXTWDT
#define EWDT_LIMIT_MIN 			(50U)
#define EWDT_LIMIT_MAX 			(2000U)

s32 ExtWdtCfgInit(void);
s32 ExtWdtCfgDeInit(void);
void SetExtWdtInterval(u32 Interval);
u32 GetExtWdtInterval(void);
#endif
void ModExtWdtInit(void);

#ifdef __cplusplus
}
#endif
#endif /* XPFW_MOD_EXTWDT_H_ */
