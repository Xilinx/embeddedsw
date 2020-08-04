/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_H_
#define XPM_RAIL_H_

#include "xstatus.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	XPM_RAILTYPE_MODE=1,
	XPM_RAILTYPE_PGOOD,
} XPm_RailType;

typedef enum {
	XPM_PGOOD_SYSMON=1,
} XPm_PgoodSource;

typedef struct XPmRail {
	XPm_Power Power;
	XPm_PgoodSource Source;
} XPm_Rail;

/************************** Function Prototypes ******************************/
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, u32 *Args);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_H_ */
