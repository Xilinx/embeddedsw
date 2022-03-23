/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_NPDOMAIN_H_
#define XPM_NPDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The NOC power domain node class.
 */
typedef struct XPm_NpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_NpDomain;

/************************** Function Prototypes ******************************/
XStatus XPmNpDomain_Init(XPm_NpDomain *Npd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NPDOMAIN_H_ */
