/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xprd_sinit.c
* @addtogroup prd_v1_1
* @{
*
* This file contains the implementation of the XPrd driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date          Changes
* ----- ----- -----------   ---------------------------------------------
* 1.0   ms    07/14/16      First release
* 1.1   ms    01/16/17      Updated the parameter naming from
*                           XPAR_PR_DECOUPLER_NUM_INSTANCES to
*                           XPAR_XPRD_NUM_INSTANCES to avoid compilation
*                           failure for XPAR_PR_DECOUPLER_NUM_INSTANCES as
*                           the tools are generating XPAR_XPRD_NUM_INSTANCES
*                           in the generated xprd_g.c for fixing MISRA-C
*                           files. This is a fix for CR-966099 based on the
*                           update in the tools.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XPrd_Config XPrd_ConfigTable[XPAR_XPRD_NUM_INSTANCES];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XPrd_ConfigTable[] contains the configuration information
* for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XPrd_Config *XPrd_LookupConfig(u16 DeviceId)
{
	XPrd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XPRD_NUM_INSTANCES;
		Index++) {
		if (XPrd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XPrd_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
