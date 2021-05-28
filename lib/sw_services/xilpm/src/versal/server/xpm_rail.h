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

#define MAX_MODES 2U

typedef enum {
	XPM_RAILTYPE_MODE_PMBUS = 1,
	XPM_RAILTYPE_PGOOD,
} XPm_RailType;

typedef enum {
	XPM_PGOOD_SYSMON = 1,
} XPm_PgoodSource;

typedef struct XPmRail {
	XPm_Power Power;
	XPm_PgoodSource Source;
	u32 ParentId;
	u8 NumModes; 		/** Num of modes supported */
	struct XPm_I2cCmd I2cModes[MAX_MODES]; /** Modes information if parent regulator is controlled over i2c */
} XPm_Rail;

/************************** Function Prototypes ******************************/
XStatus XPmRail_Control(XPm_Rail *Rail, u8 State);
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, const u32 *Args, u32 NumArgs);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_H_ */
