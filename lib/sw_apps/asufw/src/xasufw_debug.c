/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_debug.h"
#include "xasu_sharedmem.h"
#include "xasufw_memory.h"
#include "xasu_def.h"
#if (XPAR_XUARTPSV_NUM_INSTANCES > 0U)
#include "xuartpsv_hw.h"
#endif
#include "xil_io.h"
#include "bspconfig.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_LOG_OFFSET_MASK		(0x7FFFFFFFU)	/**< Mask for Offset bits [30:0] within
								OffsetAndBufStatus */
#define XASUFW_LOG_BUFFER_FULL_MASK	(0x80000000U)	/**< Mask for the buffer full flag
							(bit 31) in OffsetAndBufStatus */

/************************************** Type Definitions *****************************************/
/** Circular buffer structure. */
typedef struct {
	u32 StartAddr;	/**< Start address of log buffer */
	u32 Len;	/**< Size of log in bytes */
	u32 OffsetAndBufStatus;	/**< Bits [30:0] hold the offset of current log from
					Start Address. Bit [31] is the status flag for buffer full,
					if set, log buffer is full and offset gets reset
					to 0 */
} XAsufw_CircularBuffer;

/** Structure holds logging information of buffer. */
typedef struct {
	XAsufw_CircularBuffer LogBuffer;	/**< Instance of circular buffer */
} XAsufw_LogInfo;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
static XAsufw_LogInfo *const DebugLog = (XAsufw_LogInfo *)(UINTPTR)XASU_RTCA_DBG_LOG_BUF_INFO_ADDR;
			/**< Pointer to the structure which holds debug log buffer information. */

static u8 LogBufValidation = XASU_STATUS_FAIL; /**< Variable to indicate if log buffer information is
						valid or not. */

/*************************************************************************************************/
/**
 * @brief	This function prints a character to UART if enabled and logs it to the debug log
 * 		buffer on successful validation of the log buffer information.
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
	if (LogBufValidation == XASU_STATUS_PASS) {
		/** Store the bytes to log buffer. */
		CurrentAddr = DebugLog->LogBuffer.StartAddr +
				(DebugLog->LogBuffer.OffsetAndBufStatus & XASUFW_LOG_OFFSET_MASK);

		/** If the log buffer is full, reset the offset to 0 and set IsBufferFull flag. */
		if (CurrentAddr >= (DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Len)) {
			DebugLog->LogBuffer.OffsetAndBufStatus = XASUFW_LOG_BUFFER_FULL_MASK;
			CurrentAddr = DebugLog->LogBuffer.StartAddr;
		}
		Xil_Out8((UINTPTR)CurrentAddr, (u8)Data);
		++DebugLog->LogBuffer.OffsetAndBufStatus;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function validates the debug log buffer information stored.
 *
 *
 *************************************************************************************************/
void XAsufw_ValidateDebugLogBufferInfo(void)
{
	if ((DebugLog->LogBuffer.StartAddr >= XASUFW_RAM_START_ADDR) &&
		(DebugLog->LogBuffer.StartAddr <= XASUFW_RAM_END_ADDR) &&
		((DebugLog->LogBuffer.StartAddr != XASUFW_DEBUG_LOG_BUFFER_ADDR) ||
		(DebugLog->LogBuffer.Len != XASUFW_DEBUG_LOG_BUFFER_SIZE))) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "\r\nNot logging data to debug log buffer as buffer log info validation failed\r\n");
	} else {
		LogBufValidation = XASU_STATUS_PASS;
	}
}
/** @} */
