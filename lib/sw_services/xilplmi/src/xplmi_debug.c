/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
* 1.01  ma   03/02/2020 Implement PLMI own outbyte to support logging as well
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_debug.h"
#ifdef DEBUG_UART_PS
#include "xuartpsv.h"
#endif
#include "xil_types.h"
#include "xstatus.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"

/* PLM specific outbyte function */
void outbyte(char c);

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* SPP Input Clk Freq should be 25 MHz */
#define XPLMI_SPP_INPUT_CLK_FREQ	(25000000U)
#define XPLMI_UART_BAUD_RATE		(115200U)

#if ((XPAR_XUARTPSV_NUM_INSTANCES == 2U) && \
			(STDOUT_BASEADDRESS == 0xFF010000U))
#define XPLMI_UART_INDEX	(1U)
#else
#define XPLMI_UART_INDEX	(0U)
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef DEBUG_UART_PS
XUartPsv UartPsvIns;	/* The instance of the UART Driver */
#endif
u8 LpdInitialized = FALSE;

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function initializes the PS UART
 *
 * @param	None
 *
 * @return	Returns XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_InitUart(void)
{
	int Status = XST_FAILURE;

	/* Initialize UART */
	/* If UART is already initialized, just return success */
	if ((LpdInitialized & UART_INITIALIZED) == UART_INITIALIZED) {
		Status = XST_SUCCESS;
		goto END;
	}

#ifdef DEBUG_UART_PS
	XUartPsv_Config *Config;

	Config = XUartPsv_LookupConfig(XPLMI_UART_INDEX);
	if (NULL == Config) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_UART_LOOKUP, 0x0U);
		goto END;
	}

	if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP) {
		Config->InputClockHz = XPLMI_SPP_INPUT_CLK_FREQ;
	}

	Status = XUartPsv_CfgInitialize(&UartPsvIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_UART_CFG, Status);
		goto END;
	}
	XUartPsv_SetBaudRate(&UartPsvIns, XPLMI_UART_BAUD_RATE);
	LpdInitialized |= UART_INITIALIZED;
#endif

#ifdef DEBUG_UART_MDM
	LpdInitialized |= UART_INITIALIZED;
#endif

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function prints and logs the terminal prints to debug log buffer
 *
 * @param	c is the character to be printed and logged
 *
 * @return	None
 *
 *****************************************************************************/
void outbyte(char c)
{
#ifdef STDOUT_BASEADDRESS
	if(((LpdInitialized) & UART_INITIALIZED) == UART_INITIALIZED) {
		XUartPsv_SendByte(STDOUT_BASEADDRESS, c);
	}
#endif

	if (DebugLog.LogBuffer.CurrentAddr >=
			(DebugLog.LogBuffer.StartAddr + DebugLog.LogBuffer.Len)) {
		DebugLog.LogBuffer.CurrentAddr = DebugLog.LogBuffer.StartAddr;
		DebugLog.LogBuffer.IsBufferFull = TRUE;
	}

	XPlmi_OutByte64(DebugLog.LogBuffer.CurrentAddr, c);
	++DebugLog.LogBuffer.CurrentAddr;
}
