/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_debug.c
 *
 * This file contains APIs for print.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xil_types.h"
#include "xil_io.h"
#include "xplm_error.h"
#include "xplm_hw.h"
#include "xstatus.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#define XPLM_DEBUG_LOG_BUFFER_HIGH_ADDR	(XPLM_DEBUG_LOG_BUFFER_ADDR + XPLM_DEBUG_LOG_BUFFER_LEN - 1U)
/**< Log buffer high address */

#define XPLM_DEBUG_LOG_BUFFER_ADDR	(XPLM_RAM_BASEADDR + 0xAE00U)
/**< Start address of the log buffer in PMC RAM */

#define XPLM_DEBUG_LOG_BUFFER_LEN	(0x400U)
/**< Log buffer length in PMC RAM */

/** @cond spartanup_plm_internal */
#define XPLM_QEMU_STDOUT_BASEADDDRESS 	(0x04042004U)
/** @endcond */

/**************************** Type Definitions *******************************/
typedef u32 XReserved_Var_t; /**< Placeholder for the reserved fields with type u32. */

/**
 * Circular buffer Structure.
 */
typedef struct {
	u32 StartAddr;	/**< Start address of log buffer */
	XReserved_Var_t Reserved;	/**< Reserved Field */
	u32 Len;	/**< Length of log in bytes */
	u32 Offset: 31;	/**< Variable that holds the offset of current log from Start Address */
	u32 IsBufferFull: 1;	/**< If set, Log buffer is full and Offset gets reset to 0 */
} XPlm_CircularBuffer;

/**
 * Log buffer Structure.
 */
typedef struct {
	XPlm_CircularBuffer LogBuffer;	/**< Instance of @ref XPlm_CircularBuffer. */
} XPlm_LogInfo;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/** Pointer to the log buffer structure in PMC RAM. */
XPlm_LogInfo *DebugLog = (XPlm_LogInfo *)XPLM_RTCFG_DBG_LOG_BUF_ADDR;

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	Initializes the log buffer start address and maximum buffer length.
 *
 *****************************************************************************/
void XPlm_InitDebugLogBuffer(void)
{
	DebugLog->LogBuffer.StartAddr = XPLM_DEBUG_LOG_BUFFER_ADDR;
	DebugLog->LogBuffer.Len = XPLM_DEBUG_LOG_BUFFER_LEN;
	DebugLog->LogBuffer.Offset = 0x0U;
	DebugLog->LogBuffer.IsBufferFull = FALSE;
}

/*****************************************************************************/
/**
 * This function prints and logs the terminal prints to debug log buffer
 *
 * @param	c is the character to be printed and logged
 *
 *****************************************************************************/
void outbyte(char c)
{
	u32 CurrentAddr;

#ifdef XPLM_PRINT_QEMU
	Xil_Out32(XPLM_QEMU_STDOUT_BASEADDDRESS, c);
#endif

	/** - Store the received byte onto log buffer. */
	CurrentAddr = DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Offset;
	if (CurrentAddr >= (DebugLog->LogBuffer.StartAddr + DebugLog->LogBuffer.Len)) {
		DebugLog->LogBuffer.Offset = 0x0U;
		DebugLog->LogBuffer.IsBufferFull = (u32)TRUE;
		CurrentAddr = DebugLog->LogBuffer.StartAddr;
	}

	Xil_Out8(CurrentAddr, c);
	++DebugLog->LogBuffer.Offset;
}

/*****************************************************************************/
/**
 * This function prints debug messages with timestamp
 *
 * @param	DebugType is the PLM Debug level for the message
 * @param	Ctrl1 is the format specified string to print
 *
 *****************************************************************************/
void XPlm_Print(u32 DebugType, const char8 *Ctrl1, ...)
{
	va_list Args;

	va_start(Args, Ctrl1);

	if ((DebugType & XPLM_DEBUG_PRINT_STAGE_INFO_MASK) != 0U) {
		xil_printf("[0x%x] ", (u32)(Xil_In32(PMC_GLOBAL_PMC_FW_STATUS) & XPLM_FW_STATUS_STAGE_MASK));
	}
	xil_vprintf(Ctrl1, Args);

	va_end(Args);
}

/** @} end of spartanup_plm_apis group*/
