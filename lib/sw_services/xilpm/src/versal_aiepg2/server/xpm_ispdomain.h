/******************************************************************************
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_ISPDOMAIN_H_
#define XPM_ISPDOMAIN_H_

#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  The ISP power domain node class.
 */
typedef struct XPm_IspDomain {
	XPm_PowerDomain Domain; /**< Power domain node base class */
} XPm_IspDomain;

/************************** Function Prototypes ******************************/
XStatus XPmIspDomain_Init(XPm_IspDomain *IspDomain, u32 Id, u32 BaseAddress,
			 XPm_Power *Parent);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_ISPDOMAIN_H_ */
