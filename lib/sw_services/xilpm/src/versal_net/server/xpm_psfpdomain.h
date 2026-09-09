/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PSFPDOMAIN_H_
#define XPM_PSFPDOMAIN_H_

#include "xpm_powerdomain.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_ipi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Minimum number of base addresses that must be supplied before the optional
 * FPD SLCR SECURE base address can be read
 */
#define XPM_FPD_SECURE_BASE_ADDR_CNT	(2U)

/**
 * The PS Full power domain node class.
 */
typedef struct XPm_PsFpDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 FpdSlcrBaseAddr; /**< FPD SLCR base address */
	u32 FpdSlcrSecureBaseAddr; /**< FPD SLCR SECURE base address */
	SAVE_REGION()
} XPm_PsFpDomain;

/************************** Function Prototypes ******************************/
XStatus XPmPsFpDomain_Init(XPm_PsFpDomain *PsFpd, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent, const u32 *OtherBaseAddresses,
			   u32 OtherBaseAddressCnt);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PSFPDOMAIN_H_ */
