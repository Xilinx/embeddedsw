/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bsv  08/02/2021 Code clean up to reduce size
*       bm   08/12/2021 Added support to configure uart during run-time
*       bsv  08/13/2021 Code clean up to reduce size by optimizing
*                       XPlmi_RetrieveBufferData
*       bsv  08/15/2021 Removed unwanted goto statements
*       rb   08/11/2021 Fix compilation warnings
*       ma   08/17/2021 Added check for buffer length to be word aligned
*                       Return error codes as minor error codes from this file
*       ma   09/13/2021 Set PLM prints log level to 0 in
*                       XPlmi_InitDebugLogBuffer function
* 1.06  bsv  06/03/2022 Add CommandInfo to a separate section in elf
*       bm   07/06/2022 Refactor versal and versal_net code
* 1.07  ng   03/12/2023 Fixed Coverity warnings
* 1.07  ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       dd   09/12/2023 MISRA-C violation Rule 10.8 fixed
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_event_logging.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_util.h"
#include "xil_util.h"
#include "xplmi_modules.h"
#include "xplmi_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**@cond xplmi_internal
 * @{
 */

#define XPLMI_DEBUG_LOG_BUFFER_HIGH_ADDR	(XPLMI_DEBUG_LOG_BUFFER_ADDR + \
						XPLMI_DEBUG_LOG_BUFFER_LEN - 1U)
#define XPLMI_TRACE_LOG_BUFFER_HIGH_ADDR	(XPLMI_TRACE_LOG_BUFFER_ADDR + \
						XPLMI_TRACE_LOG_BUFFER_LEN - 1U)

#define XPLMI_TRACE_LOG_BUFFER	(0U)
#define XPLMI_DEBUG_LOG_BUFFER	(1U)

/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */
XPlmi_LogInfo *DebugLog = (XPlmi_LogInfo *)(UINTPTR)XPLMI_RTCFG_DBG_LOG_BUF_ADDR;


/*****************************************************************************/
/**
 * @brief	This function retrieves buffer data to the destination location.
 *
 * @param 	Buffer Circular buffer structure to which the data should be written
 * @param 	DestAddr to which the buffer data is to be copied
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static int XPlmi_RetrieveBufferData(const XPlmi_CircularBuffer * Buffer,
	u64 DestAddr)
{
	int Status = XST_FAILURE;
	u64 CurrentAddr;
	u32 Len;
	u8 LogLevel = DebugLog->LogLevel;

	DebugLog->LogLevel = 0U;
	if (Buffer->IsBufferFull == (u32)TRUE) {
		Len = (u32)(Buffer->Len - Buffer->Offset);
		CurrentAddr = (Buffer->StartAddr + (u64)Buffer->Offset);
	}
	else {
		Len = Buffer->Len;
		CurrentAddr = Buffer->StartAddr;
	}
	Status = XPlmi_MemCpy64(DestAddr, CurrentAddr, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Buffer->IsBufferFull == (u32)TRUE) {
		DestAddr += Len;
		Len = Buffer->Offset;
		Status = XPlmi_MemCpy64(DestAddr, Buffer->StartAddr, Len);
	}

END:
	DebugLog->LogLevel = LogLevel;
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures Log Buffer with the given memory address
 * 			and size. It also validates the given address range
 *
 * @param 	LogBuffer is the circular buffer structure to be configured
 * @param 	StartAddr is the starting address of the given buffer
 * @param 	NumBytes is number of bytes the circular buffer can hold
 * @param 	BufType is the type of log buffer
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_INVALID_LOG_BUF_LEN if invalid log buffer
 * 			length is passed.
 * 			- XPLMI_ERR_INVALID_LOG_BUF_ADDR if invalid log buffer
 * 			address is passed.
 *
 *****************************************************************************/
static int XPlmi_ConfigureLogMem(XPlmi_CircularBuffer *LogBuffer, u64 StartAddr,
		u32 NumBytes, u8 BufType)
{
	int Status = XST_FAILURE;
	u64 EndAddr = 0U;
	u32 StartLimit;
	u32 EndLimit;

	if ((NumBytes == 0U) ||
		((NumBytes & XPLMI_WORD_LEN_MASK) != 0U)) {
		Status = (int)XPLMI_ERR_INVALID_LOG_BUF_LEN;
		goto END1;
	}

	EndAddr = StartAddr + NumBytes - 1U;
	Status = XPlmi_VerifyAddrRange(StartAddr, EndAddr);
	if (Status != XST_SUCCESS) {
		if (BufType == XPLMI_TRACE_LOG_BUFFER) {
			StartLimit = XPLMI_TRACE_LOG_BUFFER_ADDR;
			EndLimit = XPLMI_TRACE_LOG_BUFFER_HIGH_ADDR;
		}
		else if (BufType == XPLMI_DEBUG_LOG_BUFFER) {
			StartLimit = XPLMI_DEBUG_LOG_BUFFER_ADDR;
			EndLimit = XPLMI_DEBUG_LOG_BUFFER_HIGH_ADDR;
		}
		else {
			goto END;
		}
		if ((StartAddr < StartLimit) || (EndAddr > EndLimit)) {
			Status = (int)XPLMI_ERR_INVALID_LOG_BUF_ADDR;
			goto END1;
		}
	}
END:
	LogBuffer->StartAddr = StartAddr;
	LogBuffer->Offset = 0x0U;
	LogBuffer->Len = NumBytes;
	LogBuffer->IsBufferFull = (u32)FALSE;
	Status = XST_SUCCESS;

END1:
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
 *			Arg1 - Log Level
 *		2 - Configure Debug Log buffer memory
 *			Arg1 - High Address
 *			Arg2 - Low Address
 *			Arg3 - Length
 *		3 - Retrieve Debug Log buffer
 *			Arg1 - High Address
 *			Arg2 - Low Address
 *		4 - Retrieve Debug Log buffer information
 *		5 - Configure Trace Log buffer memory
 *			Arg1 - High Address
 *			Arg2 - Low Address
 *			Arg3 - Length
 *		6 - Retrieve Trace Log buffer
 *			Arg1 - High Address
 *			Arg2 - Low Address
 *		7 - Retrieve Trace Log buffer information
 *		8 - Configure Uart
 *			Arg1 - Uart Select
 *			Arg2 - Uart Enable
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_INVALID_LOG_LEVEL on invalid log level.
 * 			- XST_INVALID_PARAM on invalid logging command.
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
	XPlmi_CircularBuffer *TraceLog = XPlmi_GetTraceLogInst();
	XPLMI_EXPORT_CMD(XPLMI_EVENT_LOGGING_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);

	switch (LoggingCmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
			Arg1 = ((u64)1U << (u32)Arg1) - 1U;
			if (Arg1 <= XPlmiDbgCurrentTypes) {
				DebugLog->LogLevel = (u8)((Arg1 << XPLMI_LOG_LEVEL_SHIFT) | Arg1);
				Status = XST_SUCCESS;
			} else {
				Status = (int)XPLMI_ERR_INVALID_LOG_LEVEL;
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
			Status = XPlmi_ConfigureLogMem(TraceLog, StartAddr,
					Arg3, XPLMI_TRACE_LOG_BUFFER);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
			Status = XPlmi_RetrieveBufferData(TraceLog,
					(Arg1 << 32U) | Arg2);
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
			XPlmi_RetrieveBufferInfo(Cmd, TraceLog);
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_UART:
			Status = XPlmi_ConfigUart((u8)Arg1, (u8)Arg2);
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
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len)
{
	u32 Index;
	XPlmi_PerfTime PerfTime;
	XPlmi_CircularBuffer *TraceLog = XPlmi_GetTraceLogInst();

	/* Get time stamp of PLM */
	XPlmi_MeasurePerfTime((XPLMI_PIT1_CYCLE_VALUE << 32U) |
		XPLMI_PIT2_CYCLE_VALUE, &PerfTime);

	TraceData[0U] = TraceData[0U] | (Len << XPLMI_TRACE_LOG_LEN_SHIFT);
	TraceData[1U] = (u32)PerfTime.TPerfMs;
	TraceData[2U] = (u32)PerfTime.TPerfMsFrac;

	for (Index = 0U; Index < Len; Index++) {
		if (TraceLog->Offset >= TraceLog->Len) {
			TraceLog->Offset = 0x0U;
			TraceLog->IsBufferFull = (u32)TRUE;
		}

		XPlmi_Out64((TraceLog->StartAddr + TraceLog->Offset), TraceData[Index]);
		TraceLog->Offset += XPLMI_WORD_LEN;
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
	DebugLog->LogLevel = 0U;
	DebugLog->PrintToBuf = (u8)TRUE;
}

/**
 * @}
 * @endcond
 */

/** @} */
