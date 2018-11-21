/******************************************************************************
* Copyright (C) 2018 Xilinx, Inc. All rights reserved.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file XPlmi_debug.h
*
* This file contains the code to enable debug levels in PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a kc   07/13/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_DEBUG_H
#define XPLMI_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xparameters.h"
#include "xplmi_config.h"

/************************** Constant Definitions *****************************/
/**
 * Debug levels for PLM
 */
#define DEBUG_PRINT_ALWAYS    (0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL	      (0x00000002U)    /* general debug  messages */
#define DEBUG_INFO	      (0x00000004U)    /* More debug information */
#define DEBUG_DETAILED	      (0x00000008U)    /* More debug information */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if !defined (STDOUT_BASEADDRESS)
#define XPlmiDbgCurrentTypes (0U)
#elif defined (PLM_DEBUG_DETAILED)
#define XPlmiDbgCurrentTypes ((DEBUG_DETAILED) | (DEBUG_INFO) | \
			       (DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PLM_DEBUG_INFO)
#define XPlmiDbgCurrentTypes ((DEBUG_INFO) | (DEBUG_GENERAL) | \
			       (DEBUG_PRINT_ALWAYS))
#elif defined (PLM_DEBUG)
#define XPlmiDbgCurrentTypes ((DEBUG_GENERAL) | (DEBUG_PRINT_ALWAYS))
#elif defined (PLM_PRINT)
#define XPlmiDbgCurrentTypes (DEBUG_PRINT_ALWAYS)
#else
#define XPlmiDbgCurrentTypes (0U)
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

/************************** Function Prototypes ******************************/
/* Functions defined in xplmi_uart.c */
int XPlmi_InitUart(void );

/************************** Variable Definitions *****************************/
extern u32 UartInitialized;


#define XPlmi_Printf(DebugType, ...) \
	if((((DebugType) & XPlmiDbgCurrentTypes) != 0x0U) && \
	   (UartInitialized == TRUE)) \
	{xil_printf (__VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_DEBUG_H */
