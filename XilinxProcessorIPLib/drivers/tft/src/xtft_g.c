/******************************************************************************
*
* Copyright (C) 2008 - 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
/***************************************************************************/
/**
*
* @file xtft_g.c
* @addtogroup tft_v6_1
* @{
*
* This file contains a configuration table that specifies the parameters of
* TFT devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* 4.00a  bss   01/25/13  Removed the XPAR_TFT_0_DCR_SPLB_SLAVE_IF and
*			 XPAR_TFT_0_DCR_BASEADDR as AXI TFT controller
*			 doesnot support them
* </pre>
*
****************************************************************************/

/***************************** Include Files *******************************/
#include "xtft.h"
#include "xparameters.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions ****************************/
/**
 * The configuration table for TFT devices in the table. Each
 * device should have an entry in this table.
 */
XTft_Config XTft_ConfigTable[] =
{
	{
		XPAR_TFT_0_DEVICE_ID,
		XPAR_TFT_0_BASEADDR,
		XPAR_TFT_0_DEFAULT_TFT_BASE_ADDR,
		XPAR_TFT_0_ADDR_WIDTH
	}
};

/************************** Function Definitions ****************************/
/** @} */
