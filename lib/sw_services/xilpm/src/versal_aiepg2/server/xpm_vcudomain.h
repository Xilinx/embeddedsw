/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_VCUDOMAIN_H_
#define XPM_VCUDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The VCU power domain node class.
 */
typedef struct XPm_VcuDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_VcuDomain;

/************************** Function Prototypes ******************************/
XStatus XPmVcuDomain_Init(XPm_VcuDomain *VcuDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_VCUDOMAIN_H_ */
