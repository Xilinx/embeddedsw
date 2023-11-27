/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_PLAT_H_
#define XPM_RAIL_PLAT_H_

#include "xstatus.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MODES 18U

typedef enum {
	XPM_RAILTYPE_MODE_PMBUS = 1,
	XPM_RAILTYPE_PGOOD,
	XPM_RAILTYPE_TEMPVOLTADJ,
	XPM_RAILTYPE_MODE_GPIO,
} XPm_RailType;

typedef enum {
	XPM_PGOOD_SYSMON = 1,
} XPm_PgoodSource;

typedef struct {
	u32 UpperTempThresh;
	u32 LowerTempThresh;
	u8 UpperVoltMode;
	u8 LowerVoltMode;
	u8 CurrentVoltMode;
} XPmRail_TempVoltAdj;

typedef struct {
	u8 ModeNumber;		/** Power mode number */
	u16 Offset;		/** Register offset to change the state of GPIO */
	u32 Mask;		/** Mask used in read-modify-write to change only the targeted GPIOs */
	u32 Value;		/** Value to set the targeted GPIOs */
} XPm_GPIOCmd;

typedef struct {
	XPm_Power Power;
	XPm_PgoodSource Source;
	u32 ParentId;
	XPm_RailType ControlType[MAX_MODES];
	XPm_I2cCmd I2cModes[MAX_MODES];	  /** Modes information if parent regulator is controlled by I2C */
	XPm_GPIOCmd GPIOModes[MAX_MODES]; /** Modes information if parent regulator is controlled by GPIO */
	SAVE_REGION(
	u8 VIDAdjusted;
	)
	XPmRail_TempVoltAdj *TempVoltAdj;
} XPm_Rail;

/************************** Function Prototypes ******************************/
XStatus XPmRail_AdjustVID(XPm_Rail *Rail);

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_PLAT_H_ */
