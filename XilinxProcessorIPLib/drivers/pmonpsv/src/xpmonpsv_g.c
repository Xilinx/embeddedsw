/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xpmonpsv_g.c
* @addtogroup pmonpsv_v1_0
* @{
*
* This file contains a configuration table that specifies the configuration
* of Coresight PMon devices in the system.
*
*
* @note None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd   01/20/19 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xpmonpsv.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each System Monitor/ADC
 * device in the system.
 */
XPmonpsv_Config XPmonpsv_ConfigTable[XPAR_XPMONPSV_NUM_INSTANCES] =
{
	{
		XPAR_PSU_CORESIGHT_FPD_ATM_DEVICE_ID,
		XPAR_PSU_CORESIGHT_FPD_ATM_S_AXI_BASEADDR
	},
	{
		XPAR_PSU_CORESIGHT_LPD_ATM_DEVICE_ID,
		XPAR_PSU_CORESIGHT_LPD_ATM_S_AXI_BASEADDR
	}
};
/** @} */
