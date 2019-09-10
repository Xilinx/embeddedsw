/******************************************************************************
*
* Copyright (C) 2002 - 2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xuartlite_g.c
* @addtogroup uartlite_v3_3
* @{
*
* This file contains a configuration table that specifies the configuration of
* UART Lite devices in the system. Each device in the system should have an
* entry in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xuartlite.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * The configuration table for UART Lite devices
 */
XUartLite_Config XUartLite_ConfigTable[XPAR_XUARTLITE_NUM_INSTANCES] =
{
	{
		XPAR_UARTLITE_0_DEVICE_ID,	/* Unique ID of device */
		XPAR_UARTLITE_0_BASEADDR,	/* Device base address */
		XPAR_UARTLITE_0_BAUDRATE,	/* Fixed baud rate */
		XPAR_UARTLITE_0_USE_PARITY,	/* Fixed parity */
		XPAR_UARTLITE_0_ODD_PARITY,	/* Fixed parity type */
		XPAR_UARTLITE_0_DATA_BITS	/* Fixed data bits */
	},
};


/** @} */
