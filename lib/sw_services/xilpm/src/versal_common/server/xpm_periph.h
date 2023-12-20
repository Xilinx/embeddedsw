/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PERIPH_H_
#define XPM_PERIPH_H_

#include "xpm_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Periph XPm_Periph;

#define HBMON_SCHED_PERIOD	(100U)

/* Core Operations */
struct XPm_PeriphOps {
	void (*SetWakeupSource)(const XPm_Periph *Periph, u8 Enable);
};

/**
 * The processor core class.  This is the base class for all processor cores.
 */
struct XPm_Periph {
	XPm_Device Device; /**< Device: Base class */
	u32 GicProxyMask; /**< GIC Proxy Mask */
	u32 GicProxyGroup; /**< GIC Proxy Group */
	SAVE_REGION(
	u32 WakeProcId; /**< ID of processor which needs to wake on GIC interrupt */
	)
	struct XPm_PeriphOps *PeriphOps; /**< Core operations */
};

/************************** Function Prototypes ******************************/
XStatus XPmPeriph_Init(XPm_Periph *Periph, u32 Id, u32 BaseAddress,
		       XPm_Power *Power, XPm_ClockNode *Clock,
		       XPm_ResetNode *Reset, u32 GicProxyMask,
		       u32 GicProxyGroup);
XStatus XPmHbMonDev_Init(XPm_Device *Device, u32 Id, XPm_Power *Power);
XStatus XPmPeriph_SsitTempPropInitTask(void);
#ifdef CPPUTEST
int XPmPeriph_TempPropTask(void *data);
#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PERIPH_H_ */
