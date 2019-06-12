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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
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
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef DEBUG_UART_PS
XUartPsv UartPsvIns;          /* The instance of the UART Driver */
#endif
u32 UartInitialized=FALSE;
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
	/* If UART is already initialized, just return success */
	if (UartInitialized == TRUE)
	{
		goto END;
	}

#ifdef DEBUG_UART_PS
	XUartPsv_Config *Config;
	int Status;

	Config = XUartPsv_LookupConfig(0);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP)
	{
		Config->InputClockHz = 25*1000*1000; //25MHz, SPP
	}

	Status = XUartPsv_CfgInitialize(&UartPsvIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUartPsv_SetBaudRate(&UartPsvIns, 115200); // SPP

	UartInitialized=TRUE;
#endif

#ifdef DEBUG_UART_MDM
	UartInitialized=TRUE;
#endif

END:
	return XST_SUCCESS;
}
