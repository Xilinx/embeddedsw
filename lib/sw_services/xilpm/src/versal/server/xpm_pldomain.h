/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PLDOMAIN_H_
#define XPM_PLDOMAIN_H_

#include "xpm_powerdomain.h"
#include "xcframe.h"
#include "xcfupmc.h"

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

/* TRIM Types */
#define XPM_PL_TRIM_VGG          (0x1U)
#define XPM_PL_TRIM_CRAM         (0x2U)
#define XPM_PL_TRIM_BRAM         (0x3U)
#define XPM_PL_TRIM_URAM         (0x4U)

/************************** Function Prototypes ******************************/
XStatus XPmPlDomain_InitandHouseclean(void);
XStatus XPmPlDomain_Init(XPm_PlDomain *PlDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent, u32 *OtherBaseAddresses,
			 u32 OtherBaseAddressCnt);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* XPM_PLDOMAIN_H_ */
