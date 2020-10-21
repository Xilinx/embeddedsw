/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.00  ma   02/18/2020 Initial release
* 1.01  ma   02/21/2020 Added support for retrieve buffer information
*            event logging command through IPI
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 		td   10/19/2020 MISRA C Fixes
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
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XPlmi_LogInfo DebugLog = {
	.LogBuffer.StartAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.Len = XPLMI_DEBUG_LOG_BUFFER_LEN,
	.LogBuffer.CurrentAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.IsBufferFull = (u8)FALSE,
	.LogLevel = (u8)XPlmiDbgCurrentTypes,
};

/* Trace log buffer */
static XPlmi_CircularBuffer TraceLog = {
	.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
	.CurrentAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.IsBufferFull = (u8)FALSE,
};


/*****************************************************************************/
/**
 * @brief	This function retrieves remaining buffer data to the destination. location
 *
 * @param 	SourceAddr from where the buffer data is read
 * @param 	DestAddr to which the buffer data is copied
 * @param 	Len of data to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RetrieveRemBytes(u64 SourceAddr, u64 DestAddr, u32 Len)
{
	u32 RemLen;
	u32 Index;
	u32 Offset;
	u8 RemData;

	RemLen = Len & (XPLMI_WORD_LEN - 1U);
	Offset = Len & ~(XPLMI_WORD_LEN - 1U);
	for (Index = 0U; Index < RemLen; ++Index) {
		RemData = (u8)XPlmi_InByte64(SourceAddr + Offset + Index);
		XPlmi_OutByte64(DestAddr + Offset + Index, RemData);
	}
}

/*****************************************************************************/
/**
 * @brief	This function retrieves buffer data to the destination location.
 *
 * @param 	Buffer Circular buffer structure to which the data should be written
 * @param 	DestAddr to which the buffer data is to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static int XPlmi_RetrieveBufferData(XPlmi_CircularBuffer * Buffer, u64 DestAddr)
{
	int Status = XST_FAILURE;
	u32 Len;

	if (Buffer->IsBufferFull == (u8)TRUE) {
		Len = (u32)((Buffer->StartAddr + Buffer->Len) - Buffer->CurrentAddr);
		Status = XPlmi_DmaXfr(Buffer->CurrentAddr, DestAddr, (Len / XPLMI_WORD_LEN),
			XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->CurrentAddr, DestAddr, Len);
		Status = XPlmi_DmaXfr(Buffer->StartAddr, DestAddr + Len,
				((Buffer->Len - Len) / XPLMI_WORD_LEN), XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, (DestAddr + Len),
				(Buffer->Len - Len));
	} else {
		Status = XPlmi_DmaXfr(Buffer->StartAddr, DestAddr,
			(Buffer->Len / XPLMI_WORD_LEN), XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, DestAddr, Buffer->Len);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides Event Logging command execution.
 *		Command payload parameters are
 *		Sub command
 *		1 - Configure print log level
 *			@Arg1 - Log Level
 *		2 - Configure Debug Log buffer memory
 *			@Arg1 - High Address
 *			@Arg2 - Low Address
 *			@Arg3 - Length
 *		3 - Retrieve Debug Log buffer
 *			@Arg1 - High Address
 *			@Arg2 - Low Address
 *		4 - Retrieve Debug Log buffer information
 *		5 - Configure Trace Log buffer memory
 *			@Arg1 - High Address
 *			@Arg2 - Low Address
 *			@Arg3 - Length
 *		6 - Retrieve Trace Log buffer
 *			@Arg1 - High Address
 *			@Arg2 - Low Address
 *		7 - Retrieve Trace Log buffer information
 *
 * @param	Pointer to the command structure

 * @return	Returns the Status of Event Logging API
 *
 *****************************************************************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 LoggingCmd = Cmd->Payload[0U];
	u32 Arg1 = Cmd->Payload[1U];
	u32 Arg2 = Cmd->Payload[2U];
	u32 Arg3 = Cmd->Payload[3U];
	u64 Addr;

	switch (LoggingCmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
			if (Arg1 <= XPlmiDbgCurrentTypes) {
				DebugLog.LogLevel = (u8)Arg1;
				Status = XST_SUCCESS;
			} else {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_LEVEL, Status);
			}
			break;
		case XPLMI_LOGGING_CMD_CONFIG_LOG_MEM:
			if (Arg3 != 0U) {
				Addr = ((u64)Arg1 << 32U) | Arg2;
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
					(Addr < XPLMI_DEBUG_LOG_BUFFER_ADDR)) ||
					((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
					(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_ADDR, Status);
				} else {
					DebugLog.LogBuffer.StartAddr = Addr;
					DebugLog.LogBuffer.CurrentAddr = Addr;
					DebugLog.LogBuffer.Len = Arg3;
					DebugLog.LogBuffer.IsBufferFull = (u8)FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_LEN, Status);
			}
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA:
			Status = XPlmi_RetrieveBufferData(&DebugLog.LogBuffer,
				((u64)Arg1 << 32U) | Arg2);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO:
			Cmd->Response[1U] = (u32)(DebugLog.LogBuffer.StartAddr >> 32U);
			Cmd->Response[2U] = (u32)(DebugLog.LogBuffer.StartAddr & 0xFFFFFFFFU);
			Cmd->Response[3U] = (u32)(DebugLog.LogBuffer.CurrentAddr -
					DebugLog.LogBuffer.StartAddr);
			Cmd->Response[4U] = DebugLog.LogBuffer.Len;
			Cmd->Response[5U] = DebugLog.LogBuffer.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM:
			if (Arg3 != 0U) {
				Addr = ((u64)Arg1 << 32U) | Arg2;
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
					(Addr < XPLMI_TRACE_LOG_BUFFER_ADDR)) ||
					((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
					(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_ADDR, Status);
				} else {
					TraceLog.StartAddr = Addr;
					TraceLog.CurrentAddr = Addr;
					TraceLog.Len = Arg3;
					TraceLog.IsBufferFull = (u8)FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_LEN, Status);
			}
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
			Status = XPlmi_RetrieveBufferData(&TraceLog, ((u64)Arg1 << 32U) | Arg2);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
			Cmd->Response[1U] = (u32)(TraceLog.StartAddr >> 32U);
			Cmd->Response[2U] = (u32)(TraceLog.StartAddr & 0xFFFFFFFFU);
			Cmd->Response[3U] = (u32)(TraceLog.CurrentAddr -
					TraceLog.StartAddr);
			Cmd->Response[4U] = TraceLog.Len;
			Cmd->Response[5U] = TraceLog.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		default:
			XPlmi_Printf(DEBUG_GENERAL,
				"Received invalid event logging command\n\r");
			Status = XST_INVALID_PARAM;
			break;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the trace events to the Trace Log buffer.
 *
 * @param	TraceData to be stored to buffer
 * @param	Len is number of words in TraceData
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len)
{
	u32 Index;
	XPlmi_PerfTime PerfTime = {0U};

	/* Get time stamp of PLM */
	XPlmi_MeasurePerfTime(((u64)(XPLMI_PIT1_CYCLE_VALUE) << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, &PerfTime);

	TraceData[0U] = TraceData[0U] | (Len << XPLMI_TRACE_LOG_LEN_SHIFT);
	TraceData[1U] = (u32)PerfTime.TPerfMs;
	TraceData[2U] = (u32)PerfTime.TPerfMsFrac;

	for (Index = 0U; Index < Len; Index++) {
		if (TraceLog.CurrentAddr >=
				(TraceLog.StartAddr + TraceLog.Len)) {
			TraceLog.CurrentAddr = TraceLog.StartAddr;
			TraceLog.IsBufferFull = (u8)TRUE;
		}

		XPlmi_Out64(TraceLog.CurrentAddr, TraceData[Index]);
		TraceLog.CurrentAddr += XPLMI_WORD_LEN;
	}
}
