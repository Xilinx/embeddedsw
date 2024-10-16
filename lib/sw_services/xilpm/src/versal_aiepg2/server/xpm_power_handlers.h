/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_IOMODULE_H
#define XPM_IOMODULE_H

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSXC_LPX_SLCR_PMC_IRQ_PWR_MB_IRQ		(0xeb4606e0U)

#define PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_SHIFT		11U
#define PMC_IRQ_PWR_MB_IRQ_PWR_UP_REQ_MASK		((u32)0x00000800U)

#define PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_SHIFT		12U
#define PMC_IRQ_PWR_MB_IRQ_PWR_DWN_REQ_MASK		((u32)0x00001000U)

#define PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_SHIFT		14U
#define PMC_IRQ_PWR_MB_IRQ_WAKE_UP_REQ_MASK		((u32)0x00004000U)

#define PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_SHIFT		15U
#define PMC_IRQ_PWR_MB_IRQ_PWR_CTRL_REQ_MASK		((u32)0x00008000U)

typedef void (*VoidFunction_t)(void);
struct HandlerTable {
	u32 Shift;
	u32 Mask;
	VoidFunction_t Handler;
};

/************************** Function Prototypes ******************************/
int XPm_PwrIntrHandler(void *IntrNumber);

#ifdef __cplusplus
}
#endif

#endif /* XPM_IOMODULE_H */
