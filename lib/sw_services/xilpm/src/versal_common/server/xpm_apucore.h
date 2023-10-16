/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_APUCORE_H_
#define XPM_APUCORE_H_

#include "xpm_core.h"
#include "xpm_apucore_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The APU core class.
 */
struct XPm_ApuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 FpdApuBaseAddr; /**< Base address of FPD_APU module */
	u32 PcilPwrDwnReg; /*< Base Address of APU pwr dwn/ctrl reg*/
	SAVE_REGION()
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
