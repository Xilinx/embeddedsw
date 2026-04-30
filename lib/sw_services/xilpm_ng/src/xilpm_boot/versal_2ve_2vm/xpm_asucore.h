/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_ASUCORE_H_
#define XPM_ASUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif
/** ASU MB soft reset deassert value */
#define XPM_ASU_SOFT_RST_DEASSERT	(0U)
/** Index of ASU base address within BaseAddress[] passed to init */
#define XPM_ASU_BASEADDR_IDX		(0U)

typedef struct XPm_AsuCore XPm_AsuCore;
struct XPm_AsuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 AsuBaseAddr; /**< Base address of ASU module */
};

/************************** Function Prototypes ******************************/
XStatus XPmAsuCore_Init(XPm_AsuCore *AsuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);
XStatus XPmAsuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address);
XStatus XPmAsuCore_PwrDwn(XPm_Core *Core);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
