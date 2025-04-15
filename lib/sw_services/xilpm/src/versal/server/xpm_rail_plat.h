/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_PLAT_H_
#define XPM_RAIL_PLAT_H_

#include "xstatus.h"
#include "xpm_power.h"
#ifdef SDT
#include "xpm_config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (RAIL_CONTROL)
#define MAX_MODES	4U
#define MAX_PARENTS	1U

#define XPM_RAILTYPE_MODE_PMBUS		1U
#if defined (VERSAL_DVS)
#define XPM_RAILTYPE_TEMPVOLTADJ	3U
#endif /* VERSAL_DVS */
#define XPM_RAILTYPE_MODE_GPIO		4U
#endif /* RAIL_CONTROL */

#define XPM_RAILTYPE_PGOOD		2U

typedef enum {
	XPM_PGOOD_SYSMON = 1,
} XPm_PgoodSource;

#if defined (VERSAL_DVS)
typedef struct {
	u32 UpperTempThresh;
	u32 LowerTempThresh;
	u8 UpperVoltMode;
	u8 LowerVoltMode;
	u8 CurrentVoltMode;
} XPmRail_TempVoltAdj;
#endif /* VERSAL_DVS */

#if defined (RAIL_CONTROL)
typedef struct {
	u8 ModeNumber;		/** Power mode number */
	u16 Offset;		/** Register offset to change the state of GPIO */
	u32 Mask;		/** Mask used in read-modify-write to change only the targeted GPIOs */
	u32 Value;		/** Value to set the targeted GPIOs */
} XPm_GPIOCmd;
#endif /* RAIL_CONTROL */

typedef struct {
	XPm_Power Power;
	XPm_PgoodSource Source;
#if defined (RAIL_CONTROL)
	u32 ParentIds[MAX_PARENTS];
	u8 ControlType[MAX_PARENTS][MAX_MODES];
	XPm_I2cCmd I2cModes[MAX_PARENTS][MAX_MODES];	  /** Modes information if parent regulator is controlled by I2C */
	XPm_GPIOCmd GPIOModes[MAX_MODES]; /** Modes information if parent regulator is controlled by GPIO */
	u8 ParentIndex;
#if defined (VERSAL_DVS)
	XPmRail_TempVoltAdj *TempVoltAdj;
#endif /*VERSAL_DVS */
#endif /* RAIL_CONTROL */
} XPm_Rail;

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_PLAT_H_ */
