/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
 *
 * @file xpfw_debug.h
 *
 * This file contains the debug verbose information for PMUFW print
 * functionality
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver	Who		Date		Changes
 * ---- ---- -------- ------------------------------
 *
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/


#ifndef XPFW_DEBUG_H
#define XPFW_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************Include Files**************************/
#include "xil_printf.h"
#include "xpfw_config.h"
#include "xil_types.h"

/**
 * Debug levels for PMUFW
 */
#define DEBUG_PRINT_ALWAYS (0x00000001U) /* Unconditional messages */
#define DEBUG_ERROR (0x00000002U) /* Error messages */
#define DEBUG_DETAILED (0x00000004U) /* More debug information */

#if defined(XPFW_DEBUG_DETAILED)
#define XPfwDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_ERROR) |\
		(DEBUG_PRINT_ALWAYS))
#elif defined(XPFW_DEBUG_ERROR)
#define XPfwDbgCurrentTypes ((DEBUG_ERROR) | (DEBUG_PRINT_ALWAYS))
#elif defined(XPFW_PRINT)
#define XPfwDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPfwDbgCurrentTypes (0U)
#endif

#define XPfw_Printf(DebugType,...)\
	if(((DebugType) & XPfwDbgCurrentTypes) != (u8)XST_SUCCESS){xil_printf(__VA_ARGS__);}

#ifdef __cplusplus
}
#endif

#endif /* XPFW_DEBUG_H_ */
