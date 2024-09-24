/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_PLDOMAIN_PLAT_H_
#define XPM_PLDOMAIN_PLAT_H_

#include "xpm_powerdomain.h"
#include "xcframe.h"
#include "xcfupmc.h"
#ifdef SDT
#include "xpm_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern u32 HcleanDone;
/**
 * The PL power domain node class.
 */
typedef struct XPm_PlDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	u32 CfuApbBaseAddr; /**< CFU APB base address */
	u32 Cframe0RegBaseAddr; /**< CFRAME0 Register base address */
} XPm_PlDomain;


/************************** Function Prototypes ******************************/
XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, const u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_PLDOMAIN_PLAT_H_ */
