/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_NPDOMAIN_H_
#define XPM_NPDOMAIN_H_

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

XStatus NpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
XStatus NpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
XStatus NpdAmsTrim(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NPDOMAIN_H_ */
