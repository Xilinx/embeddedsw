/******************************************************************************
*
* Copyright (C) 2014-2019 Xilinx, Inc.  All rights reserved.
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
* @file xqspipsu_g.c
* @addtogroup qspipsu_v1_10
* @{
*
* This file contains a configuration table that specifies the configuration of
* QSPIPSU devices in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  08/21/14 First release
*       sk  04/24/15 Modified the code according to MISRAC-2012.
* 1.8	tjs 07/09/18 Fixed gcc warnings. (CR#1006336).
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xqspipsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each QSPIPSU device
 * in the system.
 */
XQspiPsu_Config XQspiPsu_ConfigTable[XPAR_XQSPIPSU_NUM_INSTANCES] = {
	{
		XPAR_XQSPIPSU_0_DEVICE_ID, /* Device ID for instance */
		XPAR_XQSPIPSU_0_BASEADDR,  /* Device base address */
		XPAR_XQSPIPSU_0_QSPI_CLK_FREQ_HZ,
		XPAR_XQSPIPSU_0_QSPI_MODE,
		XPAR_XQSPIPSU_0_QSPI_BUS_WIDTH,
		XPAR_XQSPIPSU_0_IS_CACHE_COHERENT
	},
};
/** @} */
