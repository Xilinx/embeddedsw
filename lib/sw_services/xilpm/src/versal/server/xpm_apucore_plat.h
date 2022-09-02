/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_PLAT_H_
#define XPM_APUCORE_PLAT_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_ACPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_ACPU_1_CPUPWRDWNREQ_MASK  BIT(1)
#define XPM_ACPU_0_PWR_CTRL_MASK	BIT(0)
#define XPM_ACPU_1_PWR_CTRL_MASK	BIT(1)

typedef struct XPm_ApuCore XPm_ApuCore;
/************************** Function Prototypes ******************************/
XStatus XPmApuCore_AssignRegisterMask(XPm_ApuCore *ApuCore, const u32 Id);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_PLAT_H_ */
