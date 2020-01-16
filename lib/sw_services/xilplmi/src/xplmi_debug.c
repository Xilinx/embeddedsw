/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

#if ((XPAR_XUARTPSV_NUM_INSTANCES == 2U) && (STDOUT_BASEADDRESS == 0xFF010000))
#define XPLMI_UART_INDEX 1U
#else
#define XPLMI_UART_INDEX 0U
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef DEBUG_UART_PS
XUartPsv UartPsvIns;          /* The instance of the UART Driver */
#endif
u32 LpdInitialized = FALSE;
/*****************************************************************************/


/*****************************************************************************/
/**
 * This function initializes the PS UART
 *
 * @param none
 *
 * @return	returns XPLM_SUCCESS on success
 *
 *****************************************************************************/
int XPlmi_InitUart(void )
{
	int Status;

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
		Config->InputClockHz = 25*1000*1000; //25MHz, SPP
	}

	Status = XUartPsv_CfgInitialize(&UartPsvIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_UART_CFG, Status);
		goto END;
	}

	XUartPsv_SetBaudRate(&UartPsvIns, 115200); // SPP

	LpdInitialized |= UART_INITIALIZED;
#endif

#ifdef DEBUG_UART_MDM
	LpdInitialized |= UART_INITIALIZED;
	Status = XST_SUCCESS;
#endif

END:
	return Status;
}
