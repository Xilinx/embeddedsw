/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_debug.h
*
* This file contains the debug verbose information for FSBL print functionality
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a kc	11/05/13 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_DEBUG_H
#define XFSBL_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xfsbl_config.h"
#include "xil_types.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * Debug levels for FSBL
 */
#define DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL	      (0x00000002U)    /* general debug  messages */
#define DEBUG_INFO	      (0x00000004U)    /* More debug information */
#define DEBUG_DETAILED	      (0x00000008U)    /* More debug information */

#if defined (FSBL_DEBUG_DETAILED)
#define XFsblDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
         (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (FSBL_DEBUG_INFO)
#define XFsblDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
         (DEBUG_PRINT_ALWAYS))
#elif defined (FSBL_DEBUG)
#define XFsblDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (FSBL_PRINT)
#define XFsblDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XFsblDbgCurrentTypes (0U)
#endif
#define XFsbl_Printf(DebugType,...) \
		if(((DebugType) & XFsblDbgCurrentTypes)!=XFSBL_SUCCESS) {xil_printf (__VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif /* XFSBL_DEBUG_H */
