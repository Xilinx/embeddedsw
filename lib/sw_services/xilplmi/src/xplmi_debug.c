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
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  ana  11/26/2019 Updated Uart Device ID
*       kc   01/16/2020 Removed xilpm dependency in PLMI for UART
*       ma   02/18/2020 Added support for logging terminal prints
*       ma   03/02/2020 Implement PLMI own outbyte to support logging as well
*       bsv  04/04/2020 Code clean up
* 1.03  kc   07/28/2020 Moved LpdInitialized from xplmi_debug.c to xplmi.c
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
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
#include "xparameters.h"

/* PLM specific outbyte function */
void outbyte(char c);

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* SPP Input Clk Freq should be 25 MHz */
#define XPLMI_SPP_INPUT_CLK_FREQ	(25000000U)
#define XPLMI_UART_BAUD_RATE		(115200U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

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
	u8 Index = 0U;

	/* Initialize UART */
	/* If UART is already initialized, just return success */
	if ((LpdInitialized & UART_INITIALIZED) == UART_INITIALIZED) {
		Status = XST_SUCCESS;
		goto END;
	}

#if (XPAR_XUARTPSV_NUM_INSTANCES > 0U)
	XUartPsv UartPsvIns;
	XUartPsv_Config *Config;

	for (Index = 0U; Index < XPAR_XUARTPSV_NUM_INSTANCES; Index++) {

		Status = XPlmi_MemSetBytes(&UartPsvIns, sizeof(XUartPsv),
				0U, sizeof(XUartPsv));
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_MEMSET, Status);
			goto END;
		}

		Config = XUartPsv_LookupConfig(Index);
		if (NULL == Config) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_LOOKUP, (int)Index);
			goto END;
		}

		if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP) {
			Config->InputClockHz = XPLMI_SPP_INPUT_CLK_FREQ;
		}

		Status = XUartPsv_CfgInitialize(&UartPsvIns, Config,
				Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_CFG, Status);
			goto END;
		}
		Status = XUartPsv_SetBaudRate(&UartPsvIns, XPLMI_UART_BAUD_RATE);
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLMI_ERR_UART_PSV_SET_BAUD_RATE,
					Status);
			goto END;
		}
	}

#ifndef PLM_PRINT_NO_UART
	LpdInitialized |= UART_INITIALIZED;
#endif
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
		XUartPsv_SendByte(STDOUT_BASEADDRESS, (u8)c);
	}
#endif

	if (DebugLog.LogBuffer.CurrentAddr >=
			(DebugLog.LogBuffer.StartAddr + DebugLog.LogBuffer.Len)) {
		DebugLog.LogBuffer.CurrentAddr = DebugLog.LogBuffer.StartAddr;
		DebugLog.LogBuffer.IsBufferFull = (u8)TRUE;
	}

	XPlmi_OutByte64(DebugLog.LogBuffer.CurrentAddr, c);
	++DebugLog.LogBuffer.CurrentAddr;
}
