/******************************************************************************
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xbram_g.c
* @addtogroup bram_v4_2
* @{
*
* This file contains a configuration table that specifies the configuration
* of BRAM devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   11/05/10 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbram.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each BRAM device in the
 * system. The order must match the XBram_Config definition.
 */
XBram_Config XBram_ConfigTable[] = {
	{
	 XPAR_BRAM_0_DEVICE_ID,
	 XPAR_BRAM_0_ECC,
	 XPAR_BRAM_0_FAULT_INJECT,
	 XPAR_BRAM_0_CE_FAILING_REGISTERS,
	 XPAR_BRAM_0_CE_FAILING_DATA_REGISTERS,
	 XPAR_BRAM_0_UE_FAILING_REGISTERS,
	 XPAR_BRAM_0_UE_FAILING_DATA_REGISTERS,
	 XPAR_BRAM_0_ECC_STATUS_REGISTERS,
	 XPAR_BRAM_0_CE_COUNTER_WIDTH,
	 XPAR_BRAM_0_ECC_ONOFF_REGISTER,
	 XPAR_BRAM_0_ECC_ONOFF_RESET_VALUE,
	 XPAR_BRAM_0_WRITE_ACCESS,
	 XPAR_BRAM_0_BASEADDR,
	 XPAR_BRAM_0_HIGHADDR,
	 XPAR_BRAM_0_CTRL_BASEADDR,
	 XPAR_BRAM_0_CTRL_HIGHADDR,
	}
};
/** @} */
