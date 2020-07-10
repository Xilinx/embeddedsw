/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file XPlmi_event_logging.h
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
typedef struct XPlmi_CircularBuffer {
	u64 StartAddr;
	u64 CurrentAddr;
	u32 Len;
	u8 IsBufferFull;
}XPlmi_CircularBuffer;

typedef struct XPlmi_LogInfo {
	XPlmi_CircularBuffer LogBuffer;
	u8 LogLevel;
}XPlmi_LogInfo;

/************************** Function Prototypes ******************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd);
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len);

/***************** Macros (Inline Functions) Definitions *********************/
/** Event Logging sub command IDs */
#define XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL		(0x1U)
#define XPLMI_LOGGING_CMD_CONFIG_LOG_MEM			(0x2U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA		(0x3U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO	(0x4U)
#define XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM		(0x5U)
#define XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA	(0x6U)
#define XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO	(0x7U)

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
 * @brief	This function writes to trace buffer.
 *
 * @param	Header
 * *
 * @return	None
 * *
 ******************************************************************************/
static inline void XPlmi_TraceLog2(u32 Header)
{
        u32 TraceBuffer[] = {Header, 0U, 0U};
        XPlmi_StoreTraceLog(TraceBuffer, XPLMI_ARRAY_SIZE(TraceBuffer));
}

/*****************************************************************************/
/**
 * @brief	This function writes to trace buffer.
 *
 * @param 	Header
 * @param	Arg1
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
 * @brief	This function writes to trace buffer.
 *
 * @param 	Header
 * @param	Arg1
 * @param	Arg2
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
 * @brief	This function writes to trace buffer.
 *
 * @param 	Header
 * @param	Arg1
 * @param	Arg2
 * @param	Arg3
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
extern XPlmi_LogInfo DebugLog;

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_EVENT_LOGGING_H */
