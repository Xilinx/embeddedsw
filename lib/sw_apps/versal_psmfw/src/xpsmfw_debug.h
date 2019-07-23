/******************************************************************************
*
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
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

#define XPsmFw_Printf(DebugType,...)\
	if(((DebugType) & XPfwDbgCurrentTypes) != (u8)XST_SUCCESS){xil_printf(__VA_ARGS__);}

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_DEBUG_H_ */
