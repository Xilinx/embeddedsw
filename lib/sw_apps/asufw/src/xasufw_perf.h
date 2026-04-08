/**************************************************************************************************
* Copyright (c) 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_perf.h
 *
 * This file Contains the function prototypes, defines and macros for the performance measurement.
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
#ifndef XASUFW_PERF_H_
#define XASUFW_PERF_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *****************************************************/
#include "xasufw_util.h"
#include "xasu_def.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

#ifdef XASU_PERF_MEASUREMENT_ENABLE
/************************** Constant Definitions *************************************************/
#define XASUFW_PERF_MON_PAUSED		(1U)	/**< Value to pause performance monitoring. */
#define XASUFW_PERF_MON_RESUMED		(0U)	/**< Value to resume performance monitoring. */

/************************************** Type Definitions *****************************************/
/**
 * @brief Global performance monitor object.
 *
 * A single flat object that guards against nested measurement. NestingDepth
 * tracks how many PERF_START calls are active for the current ModuleId.
 * Cross-module calls (different ID) are suppressed because NestingDepth > 0.
 * Same-module calls (e.g. PssSignGenerate -> PvtExp, both RSA_ID) increment
 * NestingDepth so the inner STOP decrements without printing, and only the
 * outermost STOP (depth back to 0) prints the elapsed time.
 */
typedef struct {
	u8 PerfMonState;	/**< XASUFW_PERF_MON_PAUSED or XASUFW_PERF_MON_RESUMED. */
	u8 NestingDepth;	/**< Number of active PERF_START calls for the current ModuleId. 0 = idle. */
	u8 ModuleId;		/**< Module ID of the active measurement (XASU_MODULE_*_ID). */
	u64 StartTime;		/**< Timer value captured at start. */
} XAsufw_PerfObj;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
/** @brief Global performance monitor instance. */
extern XAsufw_PerfObj XAsufw_PerfMonitor;

void XAsufw_PerfStart(u8 ModuleId);
void XAsufw_PerfStop(const char *FuncName, u8 ModuleId);

/**
 * @brief Start performance measurement for a module.
 *
 * @param	ModuleId	XASU_MODULE_*_ID of the calling module.
 */
#define XASUFW_MEASURE_PERF_START(ModuleId) \
	XAsufw_PerfStart(ModuleId)

/**
 * @brief Stop performance measurement for a module. Uses __func__ at the call
 *        site for accurate function name in split-API modules.
 *
 * @param	ModuleId	XASU_MODULE_*_ID of the calling module.
 */
#define XASUFW_MEASURE_PERF_STOP(ModuleId) \
	XAsufw_PerfStop(__func__, (ModuleId))

/**
 * @brief Set performance monitoring state (pause or resume).
 *
 * @param	Val	XASUFW_PERF_MON_PAUSED to pause, XASUFW_PERF_MON_RESUMED to resume.
 */
#define XASUFW_PERF_SET_MONITORING_STATE(Val) \
	(XAsufw_PerfMonitor.PerfMonState = (u8)(Val))

#else
/**
 * @brief No operation when XASU_PERF_MEASUREMENT_ENABLE is not defined.
 */
#define XASUFW_MEASURE_PERF_START(ModuleId)
/**
 * @brief No operation when XASU_PERF_MEASUREMENT_ENABLE is not defined.
 */
#define XASUFW_MEASURE_PERF_STOP(ModuleId)
/**
 * @brief No operation when XASU_PERF_MEASUREMENT_ENABLE is not defined.
 */
#define XASUFW_PERF_SET_MONITORING_STATE(Val)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_PERF_H_ */
/** @} */
