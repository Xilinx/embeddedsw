/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PSFPDOMAIN_H_
#define XPM_PSFPDOMAIN_H_

#include "xpm_powerdomain.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The PS Full power domain node class.
 */
typedef struct XPm_PsFpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 FpdSlcrBaseAddr; /**< FPD SLCR base address */
	u32 FpdSlcrSecureBaseAddr; /**< FPD SLCR SECURE base address */
} XPm_PsFpDomain;

/************************** Function Prototypes ******************************/
XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressCnt);

XStatus FpdInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
XStatus FpdInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);
XStatus FpdAmsTrim(const XPm_PowerDomain *PwrDomain, const u32 *Args, u32 NumOfArgs);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSFPDOMAIN_H_ */
