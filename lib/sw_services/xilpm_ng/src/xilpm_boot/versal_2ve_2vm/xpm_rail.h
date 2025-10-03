/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_H_
#define XPM_RAIL_H_

#include "xstatus.h"
#include "xpm_power.h"
#ifdef SDT
#include "xpm_config.h"
#endif
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
#include "xiicps.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MODES		20U
#define MAX_VID_RAILS		3U

typedef enum {
	XPM_RAILTYPE_MODE_PMBUS = 1,
	XPM_RAILTYPE_PGOOD,
	XPM_RAILTYPE_TEMPVOLTADJ,
	XPM_RAILTYPE_MODE_GPIO,
	XPM_RAILTYPE_VID,
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
	u32 RailId;
	u32 Performance[MAX_MODES];
	u8 VIDAdjusted;
} XPmRail_VIDAdj;

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
	XPm_I2cCmd I2cModes[MAX_MODES];   /** Modes information if parent regulator is controlled by I2C */
	XPm_GPIOCmd GPIOModes[MAX_MODES]; /** Modes information if parent regulator is controlled by GPIO */
	XPmRail_TempVoltAdj *TempVoltAdj;
	XPmRail_VIDAdj *VIDAdj;
} XPm_Rail;

/************************** Function Prototypes ******************************/
XStatus XPmRail_Control(XPm_Rail *Rail, u8 State, u8 Mode);
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, const u32 *Args, u32 NumArgs);
XStatus XPmRail_AdjustVID(XPm_Rail *Rail);
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
XIicPs *XPmRail_GetIicInstance(void);
XStatus I2CInitialize(XIicPs *Iic, const u32 ControllerID);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_H_ */
