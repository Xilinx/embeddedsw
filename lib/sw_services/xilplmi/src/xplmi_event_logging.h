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
	u32 RemLen;
	u32 IsBufferFull;
}XPlmi_CircularBuffer;

typedef struct XPlmi_LogInfo {
	XPlmi_CircularBuffer LogBuffer;
	u32 LogLevel;
}XPlmi_LogInfo;
/***************** Macros (Inline Functions) Definitions *********************/
/* Log Buffer default address and length */
#define XPLMI_DEBUG_LOG_BUFFER_ADDR	0xF2019000U
#define XPLMI_DEBUG_LOG_BUFFER_LEN	0x4000U /* 16KB */

/** Event Logging sub command IDs */
#define XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL		(0x1U)
#define XPLMI_LOGGING_CMD_CONFIG_LOG_MEM			(0x2U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA		(0x3U)
#define XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO	(0x4U)
/************************** Function Prototypes ******************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd);
/************************** Variable Definitions *****************************/
extern XPlmi_LogInfo DebugLog;
#ifdef __cplusplus
}
#endif

#endif /* XPLMI_EVENT_LOGGING_H */
