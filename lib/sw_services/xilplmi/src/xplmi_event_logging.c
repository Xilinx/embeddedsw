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
	.LogBuffer.RemLen = XPLMI_DEBUG_LOG_BUFFER_LEN,
	.LogBuffer.IsBufferFull = FALSE,
	.LogLevel = XPlmiDbgCurrentTypes,
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
 *	*	0 - Configure print log level
 *	*		@Arg1 - Log Level
 *	*	1 - Configure Debug Log buffer memory
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
 *	*		@Arg3 - Length
 *	*	2 - Retrieve Debug Log buffer
 *	*		@Arg1 - High Address
 *	*		@Arg2 - Low Address
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
		default:
		{
			XPlmi_Printf(DEBUG_GENERAL, "Received invalid event logging command\n\r");
			Status = XST_INVALID_PARAM;
			break;
		}
	}
	return Status;
}
