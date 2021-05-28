/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PMC_H_
#define XPM_PMC_H_

#include "xpm_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_Pmc XPm_Pmc;

/**
 * The PMC processor class.
 */
struct XPm_Pmc {
	XPm_Core Core; /**< Processor core device */
	u32 PmcIouSlcrBaseAddr; /**< PMC IOU SLCR register base address */
	u32 PmcGlobalBaseAddr; /**< PMC GLOBAL register base address */
	u32 PmcAnalogBaseAddr; /**< PMC Analog register base address */
};

/************************** Function Prototypes ******************************/
XStatus XPmPmc_Init(XPm_Pmc *Pmc, u32 DevcieId, u32 Ipi, const u32 *BaseAddress,
		    XPm_Power *Power,  XPm_ClockNode *Clock,
		    XPm_ResetNode *Reset);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PMC_H_ */
