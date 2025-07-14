/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_debug.c
 *
 * This file contains the code to log debug details in ASUFW to reserved ASUFW data RAM
 * and also routing the prints to UART is disabled when ASU subsystem doesn't have access to UART.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ss   07/14/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasu_sharedmem.h"
#include "xuartpsv_hw.h"
#include "bspconfig.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_DEBUG_LOG_BUFFER_SIZE	(0x1000U)	/**< Size of debug log buffer in bytes */

#define XASUFW_DEBUG_LOG_BUFFER_ADDR	(0xEBE5DC00U)	/**< Address of debug log buffer in
								data RAM */

/************************************** Type Definitions *****************************************/
/** Circular buffer structure. */
typedef struct {
	u32 StartAddr;	/**< Start address of log buffer */
	u32 Len;	/**< Size of log in bytes */
	u32 Offset:31;	/**< Variable that holds the offset of current log
				from Start Address */
	u32 IsBufferFull:1;	/**< If set, Log buffer is full and Offset
					gets reset to 0 */
} XAsufw_CircularBuffer;

/** Structure holds logging information of buffer. */
typedef struct {
	XAsufw_CircularBuffer LogBuffer;	/**< Instance of circular buffer */
} XAsufw_LogInfo;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
XAsufw_LogInfo *DebugLog = (XAsufw_LogInfo *)(UINTPTR)XASU_RTCA_DBG_LOG_BUF_INFO_ADDR;

/*************************************************************************************************/
/**
 * @brief	This function prints a character to UART if enabled and logs it to the debug log
 * 		buffer.
 *
 * @param	Data	The character to be printed and logged
 *
 *************************************************************************************************/
void outbyte(char Data)
{
	u32 CurrentAddr;
	/** If UART is enabled, send the byte to UART. */
#if (XPAR_XUARTPSV_NUM_INSTANCES > 0U)
	XUartPsv_SendByte(STDOUT_BASEADDRESS, (u8)Data);
#endif
	/** Store the bytes to log buffer. */
	CurrentAddr = DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Offset;

	/** If the log buffer is full, reset the offset to 0 and set IsBufferFull flag. */
	if (CurrentAddr >= (DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Len)) {
		DebugLog->LogBuffer.Offset = 0x0U;
		DebugLog->LogBuffer.IsBufferFull = TRUE;
		CurrentAddr = DebugLog->LogBuffer.StartAddr;
	}
	Xil_Out8((UINTPTR)CurrentAddr, (u8)Data);
	++DebugLog->LogBuffer.Offset;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes the the DebugLog structure.
 *
 *************************************************************************************************/
void XAsufw_InitDebugLogBuffer(void)
{
	DebugLog->LogBuffer.StartAddr = XASUFW_DEBUG_LOG_BUFFER_ADDR;
	DebugLog->LogBuffer.Len = XASUFW_DEBUG_LOG_BUFFER_SIZE;
	DebugLog->LogBuffer.Offset = 0x00U;
	DebugLog->LogBuffer.IsBufferFull = (u32)FALSE;
}
/** @} */
