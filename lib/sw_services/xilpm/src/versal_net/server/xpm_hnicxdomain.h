/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_HNICXDOMAIN_H_
#define XPM_HNICXDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The HNICX power domain node class.
 */
typedef struct XPm_HnicxDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
	SAVE_REGION()
} XPm_HnicxDomain;

/************************** Function Prototypes ******************************/
XStatus XPmHnicxDomain_Init(XPm_HnicxDomain *Hnicxd, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_HNICXDOMAIN_H_ */
