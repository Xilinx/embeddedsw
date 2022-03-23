/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PMCDOMAIN_H_
#define XPM_PMCDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_PmcDomain XPm_PmcDomain;

/**
 * The PMC power domain node class.
 */
struct XPm_PmcDomain {
	XPm_PowerDomain Domain; /**< Power: Power domain base class */
};

/************************** Function Prototypes ******************************/
XStatus XPmPmcDomain_Init(XPm_PmcDomain *PmcDomain, u32 Id, XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_PMCDOMAIN_H_ */
