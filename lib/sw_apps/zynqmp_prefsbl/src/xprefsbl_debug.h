/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xprefsbl_debug.h
*
* This file contains the debug verbose information for Pre-FSBL print functionality
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who             Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana		   07/02/20      First release
*
* </pre>
*
******************************************************************************/

#ifndef XPREFSBL_DEBUG_H
#define XPREFSBL_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPREFSBL_SUCCESS		(0x0U)
#define PREFSBL_PRINT_VAL		(0U)
#define PREFSBL_DEBUG_VAL		(0U)
#define PREFSBL_DEBUG_DETAILED_VAL	(0U)

/**
 * Pre-FSBL Debug options
 */

#if PREFSBL_PRINT_VAL
#define PREFSBL_PRINT
#endif

#if PREFSBL_DEBUG_VAL
#define PREFSBL_DEBUG
#endif

#if PREFSBL_DEBUG_DETAILED_VAL
#define PREFSBL_DEBUG_DETAILED
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * Debug levels for Pre-FSBL
 */
#define DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL	      (0x00000002U)    /* general debug  messages */
#define DEBUG_INFO			  (0x00000004U)    /* More debug information */
#define DEBUG_DETAILED	      (0x00000008U)    /* More debug information */

#if defined (PREFSBL_DEBUG_DETAILED)
#define XPreFsblDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
         (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PREFSBL_DEBUG)
#define XPreFsblDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PREFSBL_PRINT)
#define XPreFsblDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPreFsblDbgCurrentTypes (0U)
#endif
#define XPreFsbl_Printf(DebugType,...) \
		if(((DebugType) & XPreFsblDbgCurrentTypes)!=XPREFSBL_SUCCESS) {xil_printf (__VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif /* XPREFSBL_DEBUG_H */