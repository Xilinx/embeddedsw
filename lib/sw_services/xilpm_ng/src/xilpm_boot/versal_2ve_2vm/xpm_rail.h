/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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

/**
 * @def XPM_RAIL_PERF_MODE_STATE_OFFSET
 * @brief Offset added to a Performance[] index to produce the
 *        XPmRail_Control() power-state argument. Performance-mode
 *        index 0 maps to power-state value 2 because
 *        XPM_POWER_STATE_OFF (0) and XPM_POWER_STATE_ON (1) are
 *        reserved.
 */
#define XPM_RAIL_PERF_MODE_STATE_OFFSET		(2U)

/*
 * XPmRail_InitVID() CDO argument layout:
 *   Args[0] = rail node id
 *   Args[1] = rail type (low byte = XPM_RAILTYPE_VID)
 *   Args[2] = (entries-count << 8) | speed-grade SPGD index byte
 *   Args[3 .. 3 + Entries - 1] = performance-mode entries
 */

/** @def XPM_VID_INIT_MIN_NUMARGS
 *  @brief Minimum NumArgs accepted by XPmRail_InitVID() (RailId, RailType,
 *         EntriesSpgd byte). Each performance-mode entry adds one Args[]
 *         slot above this minimum.
 */
#define XPM_VID_INIT_MIN_NUMARGS		(3U)

/** @def XPM_VID_INIT_PERF_ARG_BASE
 *  @brief Index of the first performance-mode entry in the InitVID Args[]
 *         vector (immediately after the 3 fixed-position fields).
 */
#define XPM_VID_INIT_PERF_ARG_BASE		(3U)

/** @def XPM_VID_RAILTYPE_BYTE_MASK
 *  @brief Byte mask used to extract the rail-type field from Args[1].
 */
#define XPM_VID_RAILTYPE_BYTE_MASK		(0xFFU)

/** @def XPM_VID_ENTRIES_SHIFT
 *  @brief Bit-shift used to encode the entry-count nibble in the high byte
 *         of Args[2] (entries-count << 8 | speed-grade SPGD byte).
 */
#define XPM_VID_ENTRIES_SHIFT			(8U)

/** @def XPM_VID_ENTRIES_MASK
 *  @brief Byte mask applied after right-shifting Args[2] by
 *         XPM_VID_ENTRIES_SHIFT to recover the entries-count value.
 */
#define XPM_VID_ENTRIES_MASK			(0xFFU)

/** @def XPM_VID_SPGD_BYTE_MASK
 *  @brief Byte mask used to extract the speed-grade SPGD index byte from
 *         the low byte of Args[2].
 */
#define XPM_VID_SPGD_BYTE_MASK			(0xFFU)

/** @def XPM_VID_ARG_RAIL_ID
 *  @brief Args[] index of the rail-node-id field in the InitVID payload.
 */
#define XPM_VID_ARG_RAIL_ID			(0U)

/** @def XPM_VID_ARG_RAIL_TYPE
 *  @brief Args[] index of the rail-type field in the InitVID payload.
 */
#define XPM_VID_ARG_RAIL_TYPE			(1U)

/** @def XPM_VID_ARG_ENTRIES_SPGD
 *  @brief Args[] index of the (entries-count << 8 | SPGD byte) field in
 *         the InitVID payload.
 */
#define XPM_VID_ARG_ENTRIES_SPGD		(2U)

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
