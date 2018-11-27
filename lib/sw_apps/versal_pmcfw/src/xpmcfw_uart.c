/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_uart.c
*
* This is the file which contains uart initialization code for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   05/04/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"

#ifdef DEBUG_UART_PS
#include "xuartpsv.h"
#endif
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

#ifdef DEBUG_UART_PS
/*****************************************************************************/
/**
 * This function initializes the PS UART
 *
 * @param none
 *
 * @return	returns XPMCFW_SUCCESS on success
 *
 *****************************************************************************/
XStatus XPmcFw_InitPsUart(void )
{
	XUartPsv_Config *Config;
	XStatus Status;

	Config = XUartPsv_LookupConfig(0);
	if (NULL == Config) {
		return XPMCFW_FAILURE;
	}

	Config->InputClockHz = 25*1000*1000; //25MHz, SPP

	Status = XUartPsv_CfgInitialize(&UartPsvIns, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XPMCFW_FAILURE;
	}

	XUartPsv_SetBaudRate(&UartPsvIns, 115200); // SPP

	UartInitialized=TRUE;
	return XPMCFW_SUCCESS;
}
#endif

#ifdef DEBUG_UART_MDM
/*****************************************************************************/
/**
 * This function initializes the MDM UART
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XPmcFw_InitMdmUart(void )
{
	UartInitialized=TRUE;
}
#endif
