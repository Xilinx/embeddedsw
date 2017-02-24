/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xaxis_switch_sinit.c
* @addtogroup axis_switch_v1_1
* @{
*
* This file contains static initialization method for Xilinx AXI4-Stream
* Source Control Router core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.00  sha 07/15/15 Defined macro XPAR_XAXIS_SWITCH_NUM_INSTANCES if not
*                    defined in xparameters.h
* 1.2   ms  02/20/17 Fixed compilation warning. This is a fix for CR-969126.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxis_switch.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XAXIS_SWITCH_NUM_INSTANCES
#define XPAR_XAXIS_SWITCH_NUM_INSTANCES			0
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XAxis_Switch_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xaxis_switch_g.c file.
*
* @param	DeviceId is the unique core ID of the XAxis_Switch core for
*		the lookup operation.
*
* @return	XAxisScr_LookupConfig returns a reference to a config record
*		in the configuration table (in xaxis_switch_g.c)
*		corresponding to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XAxis_Switch_Config *XAxisScr_LookupConfig(u16 DeviceId)
{
	extern XAxis_Switch_Config
		XAxis_Switch_ConfigTable[XPAR_XAXIS_SWITCH_NUM_INSTANCES];
	XAxis_Switch_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; Index < XPAR_XAXIS_SWITCH_NUM_INSTANCES;
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XAxis_Switch_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxis_Switch_ConfigTable[Index];
			break;
		}
	}

	return (XAxis_Switch_Config *)CfgPtr;
}
/** @} */
