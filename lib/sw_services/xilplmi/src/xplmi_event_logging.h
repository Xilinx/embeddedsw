/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
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
* 1.00a ma   13/01/2020 Initial release
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
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/* Circular buffer Structure */
typedef struct XPlmi_CircularBuffer {
	u64 StartAddr;
	u64 CurrentAddr;
	u32 Len;
	u32 IsBufferFull;
}XPlmi_CircularBuffer;

typedef struct XPlmi_LogInfo {
	XPlmi_CircularBuffer LogBuffer;
	u32 LogLevel;
}XPlmi_LogInfo;
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
 * Trace log macros
 * TraceBuffer structure
 * 		0U - Header
 * 		1U - Time stamp in ms
 * 		2U - Time stamp fraction
 * 		3U - Payload
 * 		...
 */
#define XPLMI_TRACE_LOG2(Header)	\
{	\
	u32 TraceBuffer[] = {Header, 0U, 0U};	\
	XPlmi_StoreTraceLog(&TraceBuffer[0U], 3U);	\
}

#define XPLMI_TRACE_LOG3(Header, Arg1)	\
{	\
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1};	\
	XPlmi_StoreTraceLog(&TraceBuffer[0U], 4U);	\
}

#define XPLMI_TRACE_LOG4(Header, Arg1, Arg2)	\
{	\
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1, Arg2};	\
	XPlmi_StoreTraceLog(&TraceBuffer[0U], 5U);	\
}

#define XPLMI_TRACE_LOG5(Header, Arg1, Arg2, Arg3)	\
{	\
	u32 TraceBuffer[] = {Header, 0U, 0U, Arg1, Arg2, Arg3};	\
	XPlmi_StoreTraceLog(&TraceBuffer[0U], 6U);	\
}
/************************** Function Prototypes ******************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd);
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len);
/************************** Variable Definitions *****************************/
extern XPlmi_LogInfo DebugLog;
#ifdef __cplusplus
}
#endif

#endif /* XPLMI_EVENT_LOGGING_H */
