/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
