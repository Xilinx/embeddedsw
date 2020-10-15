/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.00  kc   07/13/2018 Initial release
* 1.01  kc   07/16/2019 Added PERF macro to print task times
*       ma   08/01/2019 Added LPD init code
* 1.02  ana  11/26/2019 Updated Uart Device ID
*       kc   01/16/2020 Removed xilpm dependency in PLMI for UART
*       ma   02/18/2020 Added support for logging terminal prints
*       ma   03/02/2020 Implement PLMI own outbyte to support logging as well
*       bsv  04/04/2020 Code clean up
* 1.03  kc   07/28/2020 Moved LpdInitialized from xplmi_debug.c to xplmi.c
*       bm   10/14/2020 Code clean up
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
#include "xplmi_config.h"
#include "xplmi_event_logging.h"
#include "xplmi_proc.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/
/**
 * Debug levels for PLM
 */
#define DEBUG_PRINT_ALWAYS	(0x00000001U)    /* unconditional messages */
#define DEBUG_GENERAL		(0x00000002U)    /* general debug  messages */
#define DEBUG_INFO		(0x00000004U)    /* More debug information */
#define DEBUG_DETAILED		(0x00000008U)    /* More debug information */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef PLM_PRINT_PERF
#define DEBUG_PRINT_PERF	DEBUG_PRINT_ALWAYS
#else
#define DEBUG_PRINT_PERF	(0U)
#endif

#if defined (PLM_DEBUG_DETAILED)
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

/* Check if UART is present in design */
#if defined (STDOUT_BASEADDRESS)
/* Check if MDM uart or PS Uart */
#if (STDOUT_BASEADDRESS == 0xF0310000U)
#define DEBUG_UART_MDM
#elif ((STDOUT_BASEADDRESS == 0xFF000000U) || \
			(STDOUT_BASEADDRESS == 0xFF010000U))
#define DEBUG_UART_PS
#endif
#endif

/************************** Function Prototypes ******************************/
/* Functions defined in xplmi_debug.c */
int XPlmi_InitUart(void);

/************************** Variable Definitions *****************************/
#define XPlmi_Printf(DebugType, ...) \
	if(((DebugType) & (DebugLog.LogLevel)) != (u8)FALSE) { \
		XPlmi_PrintPlmTimeStamp(); \
		xil_printf (__VA_ARGS__); \
	}

/* Prints without TimeStamp */
#define XPlmi_Printf_WoTimeStamp(DebugType, ...) \
	if(((DebugType) & (DebugLog.LogLevel)) != (u8)FALSE) { \
		xil_printf (__VA_ARGS__); \
	}

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_DEBUG_H */
