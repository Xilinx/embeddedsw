/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_debug.c
*
* This is the file which contains uart initialization code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/13/2018 Initial release
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  ana  11/26/2019 Updated Uart Device ID
*       kc   01/16/2020 Removed xilpm dependency in PLMI for UART
*       ma   02/18/2020 Added support for logging terminal prints
*       ma   03/02/2020 Implement PLMI own outbyte to support logging as well
*       bsv  04/04/2020 Code clean up
* 1.03  kc   07/28/2020 Moved LpdInitialized from xplmi_debug.c to xplmi.c
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bm   02/01/2021 Add XPlmi_Print functions using xil_vprintf
*       ma   03/24/2021 Store DebugLog structure to RTCA
*       ma   03/24/2021 Print logs to memory when PrintToBuf is TRUE
*       td   05/20/2021 Support user configurable uart baudrate
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       bm   08/12/2021 Added support to configure uart during run-time
*       rb   08/11/2021 Fix compilation warnings
*       bsv  09/05/2021 Disable prints in slave boot modes in case of error
* 1.06  am   11/24/2021 Fixed doxygen warning
* 1.07  skd  04/21/2022 Misra-C violation Rule 8.3 fixed
* 1.08  bm   07/06/2022 Refactor versal and versal_net code
* 1.09  sk   11/09/2022 Added Timeout settings info for JTAG_SBI Bootmode
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.10  ng   07/06/2023 Added support for SDT flow
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi.h"
#include "xplmi_debug.h"
#ifdef DEBUG_UART_PS
#include "xuartpsv.h"
#endif
#include "xil_types.h"
#include "xstatus.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_hw.h"
#include <stdarg.h>

#ifdef PLM_DEBUG_DETAILED
#warning "Log Level set to Detailed,for JTAG_SBI Bootmode device program may timeout refer PLM wiki to change xsdb timeout"
#endif


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_SPP_INPUT_CLK_FREQ	(25000000U) /**< SPP Input Clk Freq
						should be 25 MHz */
#define XPLMI_UART_SELECT_CURRENT	(0U) /**< Flag indicates current uart is selected */

#if (XPLMI_UART_NUM_INSTANCES > 0U)
#define XPLMI_UART_SELECT_0		(1U) /**< Flag indicates UART0 is selected */
#define XPLMI_UART_ENABLE 		(0U) /**< Flag indicates UART prints are enabled */
#define XPLMI_UART_DISABLE		(1U) /**< Flag indicates UART prints are disabled */
#endif

#if (XPLMI_UART_NUM_INSTANCES > 1U)
#define XPLMI_UART_SELECT_1		(2U) /**< Flag indicates UART1 is selected */
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the PS UART
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_UART_MEMSET on UART instance failure.
 * 			- XPLMI_ERR_UART_LOOKUP if UART driver lookup fails.
 * 			- XPLMI_ERR_UART_CFG if UART driver configuration fails.
 * 			- XPLMI_ERR_UART_PSV_SET_BAUD_RATE if failed to set the baud rate.
 *
 *****************************************************************************/
int XPlmi_InitUart(void)
{
	int Status = XST_FAILURE;
	u32 *UartBaseAddr = XPlmi_GetUartBaseAddr();

	/**
	 * - Initialize UART, If UART is already initialized, just return success.
	 */
	if (XPlmi_IsUartInitialized() == (u8)TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}

#if (XPLMI_UART_NUM_INSTANCES > 0U)
	u8 Index = 0U;
	XUartPsv UartPsvIns;
	XUartPsv_Config *Config;

#ifndef SDT
	for (Index = 0U; Index < (u8)XPLMI_UART_NUM_INSTANCES; Index++) {
#else
	for (Index = 0U; XUartPsv_ConfigTable[Index].Name != NULL; Index++) {
#endif
		Status = XPlmi_MemSetBytes(&UartPsvIns, sizeof(XUartPsv),
				0U, sizeof(XUartPsv));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_MEMSET, Status);
			goto END;
		}

		/** - Read the UART config. */
	#ifndef SDT
		Config = XUartPsv_LookupConfig(Index);
	#else
		Config = XUartPsv_LookupConfig(XUartPsv_ConfigTable[Index].BaseAddress);
	#endif
		if (NULL == Config) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_LOOKUP, (int)Index);
			goto END;
		}

		/* - Set the input clock frequency to 25MHz for SPP platforms. */
		if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP) {
			Config->InputClockHz = XPLMI_SPP_INPUT_CLK_FREQ;
		}

		Status = XUartPsv_CfgInitialize(&UartPsvIns, Config,
				Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_CFG, Status);
			goto END;
		}

		/** - Set the baud rate. */
		Status = XUartPsv_SetBaudRate(&UartPsvIns, Config->BaudRate);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_PSV_SET_BAUD_RATE,
					Status);
			goto END;
		}
	}
	/** - Set UART_INITIALIZED flag. */
	XPlmi_SetLpdInitialized(UART_INITIALIZED);
#endif

#ifdef DEBUG_UART_MDM
	XPlmi_SetLpdInitialized(UART_INITIALIZED);
#endif

#if !defined(PLM_PRINT_NO_UART) && defined(STDOUT_BASEADDRESS)
	/**
	 * - If Uart is enabled and the prints are enabled, set
	 * UART_PRINT_ENABLED flag.
	 */
	XPlmi_SetLpdInitialized(UART_PRINT_ENABLED);
#endif
#ifdef STDOUT_BASEADDRESS
	*UartBaseAddr = (u32)STDOUT_BASEADDRESS;
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function configures the PS UART base address
 *
 * @param 	UartSelect is the uart number to be selected
 * @param 	UartEnable is the flag used to enable or disable uart
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_NO_UART_PRESENT if UART is not available to configure.
 * 			- XPLMI_ERR_CURRENT_UART_INVALID if selected UART base address is
 * 			invalid.
 * 			- XPLMI_ERR_INVALID_UART_SELECT if invalid UART SELECT argument is
 * 			passed.
 * 			- XPLMI_ERR_INVALID_UART_ENABLE if invalid UART ENABLE argument is
 * 			passed.
 *
 *****************************************************************************/
int XPlmi_ConfigUart(u8 UartSelect, u8 UartEnable)
{
	int Status = XPLMI_ERR_NO_UART_PRESENT;
#if (XPLMI_UART_NUM_INSTANCES > 0U)
	/** - Set the UART base address based on the selected UART. */
	u32 *UartBaseAddr = XPlmi_GetUartBaseAddr();

	if (UartSelect == XPLMI_UART_SELECT_0) {
		*UartBaseAddr = XPLMI_UART_0_BASEADDR;
	}
#if (XPLMI_UART_NUM_INSTANCES > 1U)
	else if (UartSelect == XPLMI_UART_SELECT_1) {
		*UartBaseAddr = XPLMI_UART_1_BASEADDR;
	}
#endif
	else if (UartSelect == XPLMI_UART_SELECT_CURRENT) {
		if (*UartBaseAddr == XPLMI_INVALID_UART_BASE_ADDR) {
			Status = XPLMI_ERR_CURRENT_UART_INVALID;
			goto END;
		}
	}
	else {
		Status = XPLMI_ERR_INVALID_UART_SELECT;
		goto END;
	}

	/** - Enable or Disable the UART based on the input. */
	if (UartEnable == XPLMI_UART_ENABLE) {
		XPlmi_SetLpdInitialized(UART_PRINT_ENABLED);
	}
	else if (UartEnable == XPLMI_UART_DISABLE) {
		XPlmi_UnSetLpdInitialized(UART_PRINT_ENABLED);
	}
	else {
		Status = XPLMI_ERR_INVALID_UART_ENABLE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints and logs the terminal prints to debug log buffer
 *
 * @param	c is the character to be printed and logged
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void outbyte(char c)
{
	u64 CurrentAddr;
	/** - If UART is enabled, send the byte to UART. */
#if (XPLMI_UART_NUM_INSTANCES > 0U)
	u32 *UartBaseAddr = XPlmi_GetUartBaseAddr();
	if (XPlmi_IsUartPrintInitialized() == (u8)TRUE) {
		XUartPsv_SendByte(*UartBaseAddr, (u8)c);
	}
#endif
	/** - Store the byte onto log buffer. */
	if (DebugLog->PrintToBuf == (u8)TRUE) {
		CurrentAddr = DebugLog->LogBuffer.StartAddr +
			DebugLog->LogBuffer.Offset;
		if (CurrentAddr >= (DebugLog->LogBuffer.StartAddr +
				DebugLog->LogBuffer.Len)) {
			DebugLog->LogBuffer.Offset = 0x0U;
			DebugLog->LogBuffer.IsBufferFull = TRUE;
			CurrentAddr = DebugLog->LogBuffer.StartAddr;
		}

		XPlmi_OutByte64(CurrentAddr, (u8)c);
		++DebugLog->LogBuffer.Offset;
	}
}

/*****************************************************************************/
/**
 * @brief   This function prints debug messages with timestamp
 *
 * @param   DebugType is the PLM Debug level for the message
 * @param   Ctrl1 is the format specified string to print
 *
 *****************************************************************************/
void XPlmi_Print(u16 DebugType, const char8 *Ctrl1, ...)
{
	va_list Args;

	va_start(Args, Ctrl1);

	if (((DebugType) & (DebugLog->LogLevel)) != 0U) {
		if ((DebugType & XPLMI_DEBUG_PRINT_TIMESTAMP_MASK) != 0U) {
			XPlmi_PrintPlmTimeStamp();
		}
		xil_vprintf(Ctrl1, Args);
	}
	va_end(Args);
}
