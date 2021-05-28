/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
	struct XPm_PeriphOps *PeriphOps; /**< Core operations */
	u32 GicProxyMask; /**< GIC Proxy Mask */
	u32 GicProxyGroup; /**< GIC Proxy Group */
	u32 WakeProcId; /**< ID of processor which needs to wake on GIC interrupt */
};

/************************** Function Prototypes ******************************/
XStatus XPmPeriph_Init(XPm_Periph *Periph, u32 Id, u32 BaseAddress,
		       XPm_Power *Power, XPm_ClockNode *Clock,
		       XPm_ResetNode *Reset, u32 GicProxyMask,
		       u32 GicProxyGroup);
XStatus XPmHbMonDev_Init(XPm_Device *Device, u32 Id, XPm_Power *Power);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PERIPH_H_ */
