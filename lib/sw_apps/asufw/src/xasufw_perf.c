/**************************************************************************************************
* Copyright (c) 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_perf.c
 *
 * This file contains implementation of performance measurement for ASUFW modules.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   02/18/26 Initial release
 *       kp   04/08/26 Added same-module nesting guard for split and composite APIs
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_perf_server_apis ASUFW perf Server APIs
* @{
*/
/***************************** Include Files *****************************************************/
#include "xasufw_perf.h"
/************************** Constant Definitions *************************************************/

/************************** Macros Definitions ***************************************************/
#ifdef XASU_PERF_MEASUREMENT_ENABLE

/**************************** Type Definitions ***************************************************/

/************************** Variable Definitions *************************************************/
/** @brief Global performance monitor object. */
XAsufw_PerfObj XAsufw_PerfMonitor = {
	.PerfMonState = XASUFW_PERF_MON_RESUMED,
	.NestingDepth = 0U,
	.ModuleId = XASU_MAX_MODULES,
	.StartTime = 0U
};

/************************** Inline Function Definitions ******************************************/

/************************** Function Prototypes **************************************************/

/*************************************************************************************************/
/**
 * @brief	This function starts performance measurement for a module.
 *
 * 		If monitoring is paused, returns immediately. If no measurement is
 * 		active (NestingDepth == 0), this records the module ID, sets
 * 		NestingDepth to 1, and captures the start time. If a measurement
 * 		is already in progress for the same module ID (same-module nesting,
 * 		e.g. PssSignGenerate -> PvtExp both with RSA_ID), NestingDepth is
 * 		incremented so the inner STOP will not terminate the outer
 * 		measurement. Cross-module calls (different ID) are suppressed
 * 		without incrementing depth.
 *
 * @param	ModuleId	XASU_MODULE_*_ID of the calling module.
 *
 *************************************************************************************************/
void XAsufw_PerfStart(u8 ModuleId)
{
	/** If monitoring is paused (e.g. during KATs), skip. */
	if (XAsufw_PerfMonitor.PerfMonState != XASUFW_PERF_MON_PAUSED) {
		/** If a measurement is already in progress, handle nesting. */
		if (XAsufw_PerfMonitor.NestingDepth > 0U) {
			/** Same module ID: increment depth to track nested call. */
			if (XAsufw_PerfMonitor.ModuleId == ModuleId) {
				XAsufw_PerfMonitor.NestingDepth++;
			}
			/** Different module ID: suppress without depth change. */
		} else {
			/** First call: record module ID and start timing. */
			XAsufw_PerfMonitor.ModuleId = ModuleId;
			XAsufw_PerfMonitor.NestingDepth = XASUFW_VALUE_ONE;

			/** Capture start time last to exclude setup overhead from measurement. */
			XAsufw_PerfMonitor.StartTime = XAsufw_GetTimerValue();
		}
	}
}

/*************************************************************************************************/
/**
 * @brief	This function stops performance measurement for a module.
 *
 * 		If monitoring is paused or ModuleId does not match the active
 * 		measurement, returns immediately. If it matches, NestingDepth
 * 		is decremented. While depth remains above zero (same-module nested
 * 		call returning), the function returns without printing. Only when
 * 		depth reaches zero (outermost caller) is the elapsed time printed
 * 		and state cleared. FuncName comes from __func__ at the call site
 * 		so split-API modules print the correct function name.
 *
 * @param	FuncName	Function name from __func__ at the call site.
 * @param	ModuleId	XASU_MODULE_*_ID of the calling module.
 *
 *************************************************************************************************/
void XAsufw_PerfStop(const char *FuncName, u8 ModuleId)
{
	XAsufw_PerfTime PerfTime;

	/** Only proceed if monitoring is not paused and module ID matches. */
	if ((XAsufw_PerfMonitor.PerfMonState != XASUFW_PERF_MON_PAUSED) &&
	    (XAsufw_PerfMonitor.ModuleId == ModuleId)) {
		/** Decrement nesting depth; print only when outermost caller returns. */
		XAsufw_PerfMonitor.NestingDepth--;
		if (XAsufw_PerfMonitor.NestingDepth == 0U) {
			/** Compute elapsed time from start. */
			XAsufw_MeasurePerfTime(XAsufw_PerfMonitor.StartTime, &PerfTime);

			/** Print execution time using the stop-side function name. */
			XAsufw_Printf(DEBUG_PRINT_ALWAYS,
				       "%s execution time: %llu.%06llu us\n\r",
				       FuncName, (u64)PerfTime.TPerfUs,
				       (u64)PerfTime.TPerfUsFrac);

			/** Clear monitor for next measurement. */
			XAsufw_PerfMonitor.ModuleId = XASU_MAX_MODULES;
		}
	}
}
#endif
/** @} */
