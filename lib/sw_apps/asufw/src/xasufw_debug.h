/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_debug.h
 * @addtogroup Overview
 * @{
 *
 * This file contains the code to enable debug levels in ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 *       ma   07/01/24 Move the print related debug enable macros to xasufw_config.h file
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_DEBUG_H
#define XASUFW_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_printf.h"
#include "xasufw_config.h"

/************************************ Constant Definitions ***************************************/
/**
 * Debug levels for ASUFW
 */
#define DEBUG_PRINT_ALWAYS  (1U)    /**< Unconditional messages only */
#define DEBUG_GENERAL       (2U)    /**< General debug information */
#define DEBUG_INFO          (4U)    /**< More debug information */
#define DEBUG_DETAILED      (8U)    /**< Detailed debug information */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#if defined (ASUFW_DEBUG_DETAILED) /**< Prints detailed debug information */
#define XAsufwDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
			       (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (ASUFW_DEBUG_INFO) /**< Prints more debug information */
#define XAsufwDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
			       (DEBUG_PRINT_ALWAYS))
#elif defined (ASUFW_DEBUG) /**< Prints general debug information */
#define XAsufwDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (ASUFW_PRINT) /**< Prints only minimal information */
#define XAsufwDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XAsufwDbgCurrentTypes (0U)
#endif

/** Define for ASUFW print */
#define XAsufw_Printf(DebugType, ...) \
	if(((DebugType) & (XAsufwDbgCurrentTypes)) != (u8)FALSE) { \
		xil_printf(__VA_ARGS__);\
	}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_DEBUG_H */
/** @} */
