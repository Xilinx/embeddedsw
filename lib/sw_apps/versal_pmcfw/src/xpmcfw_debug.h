/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*****************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_debug.h
*
* This file contains the debug verbose information for PMCFW print functionality
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a kc	02/23/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPMCFW_DEBUG_H
#define XPMCFW_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xpmcfw_config.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern u32 UartInitialized;

/**
 * Debug levels for PMCFW
 */
#define DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL	      (0x00000002U)    /* general debug  messages */
#define DEBUG_INFO	      (0x00000004U)    /* More debug information */
#define DEBUG_DETAILED	      (0x00000008U)    /* More debug information */

#if !defined (STDOUT_BASEADDRESS)
#define XPmcFwDbgCurrentTypes (0U)
#elif defined (PMCFW_DEBUG_DETAILED)
#define XPmcFwDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
			       (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PMCFW_DEBUG_INFO)
#define XPmcFwDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
			       (DEBUG_PRINT_ALWAYS))
#elif defined (PMCFW_DEBUG)
#define XPmcFwDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PMCFW_PRINT)
#define XPmcFwDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPmcFwDbgCurrentTypes (0U)
#endif

/*Check if UART is present in design */
#if defined (STDOUT_BASEADDRESS)
/*Check if MDM uart or PS Uart */
#if (STDOUT_BASEADDRESS == 0xF0310000)
#define DEBUG_UART_MDM
#elif ((STDOUT_BASEADDRESS == 0xFF000000) || (STDOUT_BASEADDRESS == 0xFF010000))
#define DEBUG_UART_PS
#endif
#endif

#define XPmcFw_Printf(DebugType, ...) \
	if((((DebugType) & XPmcFwDbgCurrentTypes) != 0x0U) && \
	   (UartInitialized == TRUE)) \
	{xil_printf (__VA_ARGS__); }

#ifdef PMCFW_DEBUG_REG_WRITE
#define XPMCFW_DBG_WRITE(Val)	Xil_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE0, Val);
#else
#define XPMCFW_DBG_WRITE(Val)
#endif

/* Functions defined in xpmcfw_uart.c */
XStatus XPmcFw_InitPsUart(void );
XStatus XPmcFw_InitMdmUart(void );
#ifdef __cplusplus
}
#endif

#endif /* XPMCFW_DEBUG_H */
