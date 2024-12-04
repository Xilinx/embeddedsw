/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_debug.h
*
* This file contains the debug verbose information for PSMFW print
* functionality
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_DEBUG_H
#define XPSMFW_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************Include Files**************************/
#include "xil_printf.h"
#include "xpsmfw_config.h"
#include "xil_types.h"

/**
 * @defgroup debug_psmfw Debug configurations for PSMFW
 * @{
 */

/**
 * @name Debug levels for PSMFW
 * @ingroup debug_psmfw
 * @{
 */
/**
 * Debug levels for PSMFW
 */
#if defined (DEBUG_UART_PS)
#define DEBUG_PRINT_ALWAYS (0x1U) /* Unconditional messages */
#define DEBUG_ERROR (0x2U) /* Error messages */
#define DEBUG_DETAILED (0x4U) /* More debug information */
#else
#define DEBUG_PRINT_ALWAYS (0x0U) /* Unconditional messages */
#define DEBUG_ERROR (0x0U) /* Error messages */
#define DEBUG_DETAILED (0x0U) /* More debug information */
#endif
/** @} */

/**
 * @name Current debug type
 * @ingroup debug_psmfw
 * @{
 */
/**
 * Current debug type
 */
#if defined(XPSMFW_DEBUG_DETAILED)
#define XPfwDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_ERROR) |\
		(DEBUG_PRINT_ALWAYS))
#elif defined(XPSMFW_DEBUG_ERROR)
#define XPfwDbgCurrentTypes ((DEBUG_ERROR) | (DEBUG_PRINT_ALWAYS))
#elif defined(XPSMFW_PRINT_ALWAYS)
#define XPfwDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPfwDbgCurrentTypes (0U)
#endif
/** @} */

#define XPsmFw_Printf(DebugType,...)\
	if(((DebugType) & XPfwDbgCurrentTypes) != (u8)XST_SUCCESS){xil_printf(__VA_ARGS__);} /**< Print output on console according to debug type */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DEBUG_H_ */
