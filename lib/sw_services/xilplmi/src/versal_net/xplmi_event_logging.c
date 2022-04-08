/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_event_logging.c
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
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
*       ana  10/19/2020 Added doxygen comments
* 1.03  nsk  12/14/2020 Modify the peripheral definitions to canonical
*                       definitions.
* 1.04  bm   03/04/2021 Add Address range check for Log Buffers
*       ma   03/24/2021 Store DebugLog structure to RTCA
*       ma   03/24/2021 Fix issue in log level configuration
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_event_logging.h"
#include "xplmi_update.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_DEBUG_LOG_BUFFER_HIGH_ADDR	((u64)XPLMI_DEBUG_LOG_BUFFER_ADDR + \
						XPLMI_DEBUG_LOG_BUFFER_LEN - 1U)
#define XPLMI_TRACE_LOG_BUFFER_HIGH_ADDR	((u64)XPLMI_TRACE_LOG_BUFFER_ADDR + \
						XPLMI_TRACE_LOG_BUFFER_LEN - 1U)

#define XPLMI_TRACE_LOG_BUFFER	(0U)
#define XPLMI_DEBUG_LOG_BUFFER	(1U)

#define XPLMI_TRACE_LOG_VERSION (1U)
#define XPLMI_TRACE_LOG_LCVERSION (1U)
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */
XPlmi_LogInfo *DebugLog = (XPlmi_LogInfo *)(UINTPTR)XPLMI_RTCFG_DBG_LOG_BUF_ADDR;

/* Trace log buffer */
static XPlmi_CircularBuffer TraceLog = {
	.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
	.Offset = 0x0U,
	.IsBufferFull = (u32)FALSE,
};

EXPORT_GENERIC_DS(TraceLog, XPLMI_TRACELOG_DS_ID, XPLMI_TRACE_LOG_VERSION,
	XPLMI_TRACE_LOG_LCVERSION, sizeof(TraceLog), (u32)(UINTPTR)&TraceLog);


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
		RemData = XPlmi_InByte64((SourceAddr + Offset) + Index);
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
static int XPlmi_RetrieveBufferData(const XPlmi_CircularBuffer * Buffer, u64 DestAddr)
{
	int Status = XST_FAILURE;
	u64 CurrentAddr;
	u32 Len;

	if (Buffer->IsBufferFull == (u32)TRUE) {
		Len = (u32)(Buffer->Len - Buffer->Offset);
		CurrentAddr = (Buffer->StartAddr + (u64)Buffer->Offset);
		Status = XPlmi_DmaXfr(CurrentAddr, DestAddr, (Len / XPLMI_WORD_LEN),
			XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(CurrentAddr, DestAddr, Len);
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
 * @brief	This function configures Log Buffer with the given memory address
 * and size. It also validates the given address range
 *
 * @param 	LogBuffer is the circular buffer structure to be configured
 * @param 	StartAddr is the starting address of the given buffer
 * @param 	NumBytes is number of bytes the circular buffer can hold
 * @param 	BufType is the type of log buffer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_ConfigureLogMem(XPlmi_CircularBuffer *LogBuffer, u64 StartAddr,
		u32 NumBytes, u8 BufType)
{
	int Status = XST_FAILURE;
	u64 EndAddr = 0U;

	if (NumBytes != 0U) {
		EndAddr = StartAddr + NumBytes - 1U;
		Status = XPlmi_VerifyAddrRange(StartAddr, EndAddr);
		if ((Status != XST_SUCCESS) &&
			(BufType == XPLMI_TRACE_LOG_BUFFER) &&
			((StartAddr < (u64)XPLMI_TRACE_LOG_BUFFER_ADDR) ||
			(EndAddr > (u64)XPLMI_TRACE_LOG_BUFFER_HIGH_ADDR))) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_ADDR,
					Status);
		}
		else if ((Status != XST_SUCCESS) &&
			(BufType == XPLMI_DEBUG_LOG_BUFFER) &&
			((StartAddr < (u64)XPLMI_DEBUG_LOG_BUFFER_ADDR) ||
			(EndAddr > (u64)XPLMI_DEBUG_LOG_BUFFER_HIGH_ADDR))) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_ADDR,
					Status);
		}
		else {
			LogBuffer->StartAddr = StartAddr;
			LogBuffer->Offset = 0x0U;
			LogBuffer->Len = NumBytes;
			LogBuffer->IsBufferFull = (u32)FALSE;
			Status = XST_SUCCESS;
		}
	} else {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_BUF_LEN,
					Status);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function retrieves buffer info into cmd response.
 *
 * @param 	Cmd is the pointer to command structure
 * @param 	LogBuffer is the Circular buffer structure to be retrieved
 *
 *****************************************************************************/
static void XPlmi_RetrieveBufferInfo(XPlmi_Cmd *Cmd,
		const XPlmi_CircularBuffer *LogBuffer)
{
	Cmd->Response[1U] = (u32)(LogBuffer->StartAddr >> 32U);
	Cmd->Response[2U] = (u32)(LogBuffer->StartAddr & 0xFFFFFFFFU);
	Cmd->Response[3U] = (u32)(LogBuffer->Offset);
	Cmd->Response[4U] = LogBuffer->Len;
	Cmd->Response[5U] = LogBuffer->IsBufferFull;
}

/**
 * @}
 * @endcond
 */

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
	u64 Arg1 = (u64)Cmd->Payload[1U];
	u64 Arg2 = (u64)Cmd->Payload[2U];
	u32 Arg3 = Cmd->Payload[3U];
	u64 StartAddr;

	switch (LoggingCmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
			Arg1 = (u64)((1U << (u32)Arg1) - 1U);
			if (Arg1 <= XPlmiDbgCurrentTypes) {
				DebugLog->LogLevel = (u8)Arg1;
				Status = XST_SUCCESS;
			} else {
				Status = XPlmi_UpdateStatus(XPLMI_ERR_INVALID_LOG_LEVEL,
						Status);
			}
			break;
		case XPLMI_LOGGING_CMD_CONFIG_LOG_MEM:
			StartAddr = (Arg1 << 32U) | Arg2;
			Status = XPlmi_ConfigureLogMem(&DebugLog->LogBuffer,
					StartAddr, Arg3, XPLMI_DEBUG_LOG_BUFFER);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA:
			Status = XPlmi_RetrieveBufferData(&DebugLog->LogBuffer,
				(Arg1 << 32U) | Arg2);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO:
			XPlmi_RetrieveBufferInfo(Cmd, &DebugLog->LogBuffer);
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM:
			StartAddr = (Arg1 << 32U) | Arg2;
			Status = XPlmi_ConfigureLogMem(&TraceLog, StartAddr,
					Arg3, XPLMI_TRACE_LOG_BUFFER);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
			Status = XPlmi_RetrieveBufferData(&TraceLog,
					(Arg1 << 32U) | Arg2);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
			XPlmi_RetrieveBufferInfo(Cmd, &TraceLog);
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

/**
 * @{
 * @cond xplmi_internal
 */
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
	XPlmi_MeasurePerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, &PerfTime);

	TraceData[0U] = TraceData[0U] | (Len << XPLMI_TRACE_LOG_LEN_SHIFT);
	TraceData[1U] = (u32)PerfTime.TPerfMs;
	TraceData[2U] = (u32)PerfTime.TPerfMsFrac;

	for (Index = 0U; Index < Len; Index++) {
		if ((TraceLog.StartAddr + TraceLog.Offset) >=
				(TraceLog.StartAddr + TraceLog.Len)) {
			TraceLog.Offset = 0x0U;
			TraceLog.IsBufferFull = (u32)TRUE;
		}

		XPlmi_Out64((TraceLog.StartAddr + TraceLog.Offset), TraceData[Index]);
		TraceLog.Offset += XPLMI_WORD_LEN;
	}
}

/*****************************************************************************/
/**
 * @brief	This function initializes the the DebugLog structure.
 *
 *****************************************************************************/
void XPlmi_InitDebugLogBuffer(void)
{
	DebugLog->LogBuffer.StartAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR;
	DebugLog->LogBuffer.Len = XPLMI_DEBUG_LOG_BUFFER_LEN;
	DebugLog->LogBuffer.Offset = 0x0U;
	DebugLog->LogBuffer.IsBufferFull = (u32)FALSE;
	DebugLog->LogLevel = (u8)XPlmiDbgCurrentTypes;
	DebugLog->PrintToBuf = (u8)TRUE;
}

/**
 * @}
 * @endcond
 */

/** @} */
