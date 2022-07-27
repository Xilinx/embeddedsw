/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSM_H_
#define XPM_PSM_H_

#include "xpm_core.h"
#include "xpm_psm_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Psm XPm_Psm;

/**
 * The PSM processor class.
 */
struct XPm_Psm {
	XPm_Core Core; /**< Processor core device */
	u32 PsmGlobalBaseAddr; /**< PSM Global register module base address */
	u32 CrlBaseAddr; /**< CRL module base address */
};

/************************** Function Prototypes ******************************/
XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset);
u32 XPmPsm_FwIsPresent(void);
void XPmPsm_RegWrite(const u32 Offset, const u32 Value);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSM_H_ */
