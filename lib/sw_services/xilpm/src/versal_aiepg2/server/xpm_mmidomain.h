/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_MMIDOMAIN_H_
#define XPM_MMIDOMAIN_H_

#include "xpm_bisr.h"
#include "xpm_powerdomain.h"
#include "xpm_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The MMI power domain node class.
 */
typedef struct XPm_MmiDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_MmiDomain;

/************************** Function Prototypes ******************************/
XStatus XPmMmiDomain_Init(XPm_MmiDomain *Mmi, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_NPDOMAIN_H_ */
