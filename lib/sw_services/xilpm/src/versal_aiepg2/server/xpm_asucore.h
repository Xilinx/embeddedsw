/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_ASUCORE_H_
#define XPM_ASUCORE_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XPm_AsuCore {
	XPm_Core Core; /**< Processor core devices */
	u32 AsuBaseAddr; /**< Base address of RPU module */
};

/************************** Function Prototypes ******************************/
XStatus XPmAsuCore_Init(struct XPm_AsuCore *AsuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_RPUCORE_H_ */
