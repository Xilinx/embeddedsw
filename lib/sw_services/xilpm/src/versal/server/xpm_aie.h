/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_AIE_H_
#define XPM_AIE_H_

#include "xpm_powerdomain.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_ipi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * AI Engine domain node class.
 */
typedef struct XPm_AieDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_AieDomain;

/************************** Function Prototypes ******************************/
void XPmAieDomain_UnlockPcsr(u32 BaseAddress);
void XPmAieDomain_LockPcsr(u32 BaseAddress);
XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_AIE_H_ */
