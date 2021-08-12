/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_event_logging.h
*
* This file contains the code for event logging.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   02/18/2020 Initial release
* 1.01  ma   02/21/2020 Added support for retrieve buffer information
* 						event logging command through IPI
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/04/2020 Code clean up
* 1.02  kc   06/18/2020 Made static functions inline
*       bm   10/14/2020 Code clean up
* 1.03  ma   03/24/2021 Store DebugLog structure to RTCA
* 1.04  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bm   08/12/2021 Added support to configure uart during run-time
*
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_EVENT_LOGGING_H
#define XPLMI_EVENT_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#include "xplmi_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/* Circular buffer Structure */
typedef struct {
	u64 StartAddr;	/**< Start address of log buffer */
	u32 Len;	/**< Length of log in bytes */
	u32 Offset:31;	/**< Variable that holds the offset of current log
				from Start Address */
	u32 IsBufferFull:1;	/**< If set, Log buffer is full and Offset
					gets reset to 0 */
} XPlmi_CircularBuffer;

typedef struct {
	XPlmi_CircularBuffer LogBuffer;	/**< Instance of circular buffer */
	u8 LogLevel;	/**< LogLevel indicates levels like DEBUG_INFO */
	u8 PrintToBuf;	/**< If set, log is also written to PMC_RAM */
} XPlmi_LogInfo;

/**@cond xplmi_internal
 * @{
 */

/************************** Function Prototypes ******************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd);
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len);
void XPlmi_InitDebugLogBuffer(void);

/***************** Macros (Inline Functions) Definitions *********************/
/** Event Logging sub command IDs */
#define XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL		(0x1U)
#define XPLMI_LOGGING_CMD_CONFIG_LOG_MEM			(0x2U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA		(0x3U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO	(0x4U)
#define XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM		(0x5U)
#define XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA	(0x6U)
#define XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO	(0x7U)
#define XPLMI_LOGGING_CMD_CONFIG_UART			(0x8U)
#define XPLMI_LOG_LEVEL_SHIFT		(0x4U)

/* Trace log buffer length shift */
#define XPLMI_TRACE_LOG_LEN_SHIFT		(16U)

/* Trace event IDs */
#define XPLMI_TRACE_LOG_LOAD_IMAGE		(0x1U)

/*
 * Trace log functions
 * TraceBuffer structure
 * 		0U - Header
 * 		1U - Time stamp in ms
 * 		2U - Time stamp fraction
 * 		3U - Payload
 * 		...
 */
/*****************************************************************************/
/**
 * @brief	This function writes to trace buffer
 *
 * @param	Header of the Trace log
 *
 * @return	None
 *
 ******************************************************************************/
static inline void XPlmi_TraceLog2(u32 Header)
{
        u32 TraceBuffer[] = {Header, 0U, 0U};
        XPlmi_StoreTraceLog(TraceBuffer, XPLMI_ARRAY_SIZE(TraceBuffer));
}

/*****************************************************************************/
/**
 * @brief	This function writes to trace buffer
 *
 * @param 	Header of the Trace log
 * @param	Arg1 of the Trace log
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_TraceLog3(u32 Header, u32 Arg1)
{
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1};
	XPlmi_StoreTraceLog(TraceBuffer, XPLMI_ARRAY_SIZE(TraceBuffer));
}

/*****************************************************************************/
/**
 * @brief	This function writes to trace buffer
 *
 * @param 	Header of the Trace log
 * @param	Arg1 of the Trace log
 * @param	Arg2 of the Trace log
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_TraceLog4(u32 Header, u32 Arg1, u32 Arg2)
{
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1, Arg2};
	XPlmi_StoreTraceLog(TraceBuffer, XPLMI_ARRAY_SIZE(TraceBuffer));
}

/*****************************************************************************/
/**
 * @brief	This function writes to trace buffer
 *
 * @param 	Header of the Trace log
 * @param	Arg1 of the Trace log
 * @param	Arg2 of the Trace log
 * @param	Arg3 of the Trace log
 *
 * @return	None
 *
 *****************************************************************************/
static inline void XPlmi_TraceLog5(u32 Header, u32 Arg1, u32 Arg2, u32 Arg3)
{
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1, Arg2, Arg3};
	XPlmi_StoreTraceLog(TraceBuffer, XPLMI_ARRAY_SIZE(TraceBuffer));
}

/************************** Variable Definitions *****************************/
extern XPlmi_LogInfo *DebugLog;

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_EVENT_LOGGING_H */
