/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_debug.h
 *
 * This file contains the code to enable debug levels in Firmware.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_DEBUG_H
#define XPLM_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_config.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/**
 * Debug levels for FW
 */
#define DEBUG_PRINT_ALWAYS	(0x01U)    /* unconditional messages */
#define DEBUG_GENERAL		(0x02U)    /* general debug  messages */
#define DEBUG_INFO		(0x04U)    /* More debug information */
#define DEBUG_DETAILED		(0x08U)    /* More debug information */

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
void XPlm_Print(u32 DebugType, const char8 *Ctrl1, ...);

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef XPLM_PRINT_PERF
#define DEBUG_PRINT_PERF	DEBUG_PRINT_ALWAYS
#else
#define DEBUG_PRINT_PERF	(0U)
#endif

#if defined (XPLM_DEBUG_DETAILED)
#define XPlmDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
			     (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (XPLM_DEBUG_INFO)
#define XPlmDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
			     (DEBUG_PRINT_ALWAYS))
#elif defined (XPLM_DEBUG)
#define XPlmDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (XPLM_PRINT)
#define XPlmDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPlmDbgCurrentTypes (0U)
#endif

#define XPLM_DEBUG_PRINT_STAGE_INFO_MASK		(0x100U)

#ifdef XPLM_DEBUG_PRINTS
#define XPlm_Printf(DebugType, ...) \
	if(((DebugType) & (XPlmDbgCurrentTypes)) != (u32)FALSE) { \
		XPlm_Print((DebugType | XPLM_DEBUG_PRINT_STAGE_INFO_MASK), __VA_ARGS__);\
	}
#define XPlm_Printf_WoS(DebugType, ...) \
	if(((DebugType) & (XPlmDbgCurrentTypes)) != (u32)FALSE) { \
		XPlm_Print(DebugType, __VA_ARGS__);\
	}
#else
#define XPlm_Printf(DebugType, ...) {};
#define XPlm_Printf_WoS(DebugType, ...) {};
#endif

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
void XPlm_InitDebugLogBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* XPLM_DEBUG_H */
