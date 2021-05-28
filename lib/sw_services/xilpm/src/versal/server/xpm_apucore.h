/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_H_
#define XPM_APUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPM_ACPU_0_CPUPWRDWNREQ_MASK  BIT(0)
#define XPM_ACPU_1_CPUPWRDWNREQ_MASK  BIT(1)
#define XPM_ACPU_0_PWR_CTRL_MASK	BIT(0)
#define XPM_ACPU_1_PWR_CTRL_MASK	BIT(1)

typedef struct XPm_ApuCore XPm_ApuCore;

/**
 * The APU core class.
 */
struct XPm_ApuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 FpdApuBaseAddr; /**< Base address of FPD_APU module */
};

/************************** Function Prototypes ******************************/
XStatus XPmApuCore_Init(XPm_ApuCore *ApuCore,
	u32 Id,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_APUCORE_H_ */
