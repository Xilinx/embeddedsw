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
* @file xplmi_event_logging.c
*
* This is the file which contains event logging related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   03/02/2020 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_event_logging.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XPlmi_LogInfo DebugLog = {
	.LogBuffer.StartAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.Len = XPLMI_DEBUG_LOG_BUFFER_LEN,
	.LogBuffer.CurrentAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.IsBufferFull = FALSE,
	.LogLevel = XPlmiDbgCurrentTypes,
};

/* Trace log buffer */
XPlmi_CircularBuffer TraceLog = {
	.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
	.CurrentAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.IsBufferFull = FALSE,
};

/*****************************************************************************/
/**
 * This function retrieves remaining buffer data to the destination location
 *
 * @param SourceAddr Source address from where the buffer data to be read
 * @param DestAddr	 Destination address to which the buffer data to be copied
 * @param Len		 Length of data to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RetrieveRemBytes(u64 SourceAddr, u64 DestAddr, u64 Len)
{
	u32 RemLen;
	u32 i;

	RemLen = Len%4U;
	if (RemLen != 0U) {
		for (i = 0U; i < RemLen; ++i) {
			XPlmi_OutByte64((DestAddr + (Len/4U)*4U + i),
					XPlmi_InByte64((SourceAddr + (Len/4U)*4U + i)));
		}
	}
}

/*****************************************************************************/
/**
 * This function retrieves buffer data to the destination location
 *
 * @param Buffer	Circular buffer structure to which the data to be written
 * @param DestAddr	Destination address to which the buffer data to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RetrieveBufferData(XPlmi_CircularBuffer * Buffer, u64 DestAddr)
{
	u32 Len;

	if (Buffer->IsBufferFull== TRUE) {
		Len = ((Buffer->StartAddr + Buffer->Len) - Buffer->CurrentAddr);
		XPlmi_DmaXfr(Buffer->CurrentAddr, DestAddr, (Len/4U), XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->CurrentAddr, DestAddr, Len);
		XPlmi_DmaXfr(Buffer->StartAddr, (DestAddr+Len),
				((Buffer->Len - Len)/4U), XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, (DestAddr+Len),
				(Buffer->Len - Len));
	} else {
		XPlmi_DmaXfr(Buffer->StartAddr, DestAddr, (Buffer->Len/4U),
				XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, DestAddr, Buffer->Len);
	}
}

/*****************************************************************************/
/**
 * @brief This function provides Event Logging command execution
 *  Command payload parameters are
 *	* Sub command
 *	*	1 - Configure print log level
 *	*		@Arg1 - Log Level
 *	*	2 - Configure Debug Log buffer memory
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
 *	*		@Arg3 - Length
 *	*	3 - Retrieve Debug Log buffer
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
 *	*	4 - Retrieve Debug Log buffer information
 *	*	5 - Configure Trace Log buffer memory
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
 *	*		@Arg3 - Length
 *	*	6 - Retrieve Trace Log buffer
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
 *	*	7 - Retrieve Trace Log buffer information
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of Event Logging API
 *****************************************************************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd)
{
	u32 LoggingCmd = Cmd->Payload[0U];
	u32 Arg1 = Cmd->Payload[1U];
	u32 Arg2 = Cmd->Payload[2U];
	u32 Arg3 = Cmd->Payload[3U];
	u64 Addr;
	int Status = XST_FAILURE;

	switch (LoggingCmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
		{
			if (Arg1 <= XPlmiDbgCurrentTypes) {
				DebugLog.LogLevel = Arg1;
				Status = XST_SUCCESS;
			} else {
				Status = XPLMI_ERR_INVALID_LOG_LEVEL;
			}
			break;
		}
		case XPLMI_LOGGING_CMD_CONFIG_LOG_MEM:
		{
			if (Arg3 != 0U) {
				Addr = (((u64)Arg1 << 32U) | Arg2);
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
						(Addr < XPLMI_DEBUG_LOG_BUFFER_ADDR)) ||
						((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
						(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPLMI_ERR_INVALID_LOG_BUF_ADDR;
				} else {
					DebugLog.LogBuffer.StartAddr = Addr;
					DebugLog.LogBuffer.CurrentAddr = Addr;
					DebugLog.LogBuffer.Len = Arg3;
					DebugLog.LogBuffer.IsBufferFull = FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPLMI_ERR_INVALID_LOG_BUF_LEN;
			}
			break;
		}
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA:
		{
			XPlmi_RetrieveBufferData(&DebugLog.LogBuffer, (((u64)Arg1 << 32U) | Arg2));
			Status = XST_SUCCESS;
			break;
		}
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO:
		{
			Cmd->Response[1U] = DebugLog.LogBuffer.StartAddr >> 32U;
			Cmd->Response[2U] = DebugLog.LogBuffer.StartAddr & 0xFFFFFFFFU;
			Cmd->Response[3U] = (DebugLog.LogBuffer.CurrentAddr -
					DebugLog.LogBuffer.StartAddr);
			Cmd->Response[4U] = DebugLog.LogBuffer.Len;
			Cmd->Response[5U] = DebugLog.LogBuffer.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		}
		case XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM:
		{
			if (Arg3 != 0U) {
				Addr = (((u64)Arg1 << 32U) | Arg2);
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
					(Addr < XPLMI_TRACE_LOG_BUFFER_ADDR)) ||
					((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
					(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPLMI_ERR_INVALID_LOG_BUF_ADDR;
				} else {
					TraceLog.StartAddr = Addr;
					TraceLog.CurrentAddr = Addr;
					TraceLog.Len = Arg3;
					TraceLog.IsBufferFull = FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPLMI_ERR_INVALID_LOG_BUF_LEN;
			}
			break;
		}
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
		{
			XPlmi_RetrieveBufferData(&TraceLog, (((u64)Arg1 << 32U) | Arg2));
			Status = XST_SUCCESS;
			break;
		}
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
		{
			Cmd->Response[1U] = TraceLog.StartAddr >> 32U;
			Cmd->Response[2U] = TraceLog.StartAddr & 0xFFFFFFFFU;
			Cmd->Response[3U] = (u32)(TraceLog.CurrentAddr -
					TraceLog.StartAddr);
			Cmd->Response[4U] = TraceLog.Len;
			Cmd->Response[5U] = TraceLog.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		}
		default:
		{
			XPlmi_Printf(DEBUG_GENERAL, "Received invalid event logging command\n\r");
			Status = XST_INVALID_PARAM;
			break;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function stores the trace events to the Trace Log buffer
 *
 * @param TraceData	Trace data to be stored to buffer
 * @param Len	Number of words in TraceData
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len)
{
	u32 Index;
	XPlmi_PerfTime tPerfTime = {0U};

	/* Get time stamp of PLM */
	XPlmi_MeasurePerfTime((((u64)(XPLMI_PIT1_RESET_VALUE) << 32U) |
			XPLMI_PIT2_RESET_VALUE), &tPerfTime);

	TraceData[0U] = TraceData[0U] | (Len << XPLMI_TRACE_LOG_LEN_SHIFT);
	TraceData[1U] = (u32)tPerfTime.tPerfMs;
	TraceData[2U] = (u32)tPerfTime.tPerfMsFrac;

	for (Index = 0U; Index < Len; Index++) {
		if (TraceLog.CurrentAddr >=
				(TraceLog.StartAddr + TraceLog.Len)) {
			TraceLog.CurrentAddr = TraceLog.StartAddr;
			TraceLog.IsBufferFull = TRUE;
		}

		XPlmi_Out64(TraceLog.CurrentAddr, TraceData[Index]);
		TraceLog.CurrentAddr += 4U;
	}
}
