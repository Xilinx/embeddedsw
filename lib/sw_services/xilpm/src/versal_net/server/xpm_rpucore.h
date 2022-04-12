/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RPUCORE_H_
#define XPM_RPUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_RPU_A_0_PWR_CTRL_MASK	BIT(20)
#define XPM_RPU_A_1_PWR_CTRL_MASK	BIT(21)
#define XPM_RPU_B_0_PWR_CTRL_MASK	BIT(22)
#define XPM_RPU_B_1_PWR_CTRL_MASK	BIT(23)
#define XPM_RPU_A_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_RPU_A_1_CPUPWRDWNREQ_MASK  BIT(1)
#define XPM_RPU_B_0_CPUPWRDWNREQ_MASK  BIT(2)
#define XPM_RPU_B_1_CPUPWRDWNREQ_MASK  BIT(3)

typedef struct XPm_RpuCore XPm_RpuCore;

/**
 * The RPU core class.
 */
struct XPm_RpuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 ResumeCfg;
	u32 RpuBaseAddr; /**< Base address of RPU module */
};

/************************** Function Prototypes ******************************/
XStatus XPmRpuCore_Init(XPm_RpuCore *RpuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
