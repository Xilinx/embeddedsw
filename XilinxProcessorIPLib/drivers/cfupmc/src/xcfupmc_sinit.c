/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xcfupmc_sinit.c
* @addtogroup cfupmc_v1_0
* @{
*
* This file contains static initialization methods for Xilinx CFU core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   kc   22/10/17 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfupmc.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* XCfupmc_LookupConfig returns a reference to an XCfupmc_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcfupmc_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xcfupmc_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XCfupmc_Config *XCfupmc_LookupConfig(u16 DeviceId)
{
	extern XCfupmc_Config XCfupmc_ConfigTable[XPAR_XCFUPMC_NUM_INSTANCES];
	XCfupmc_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCFUPMC_NUM_INSTANCES);
								Index++) {
		if (XCfupmc_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCfupmc_ConfigTable[Index];
			break;
		}
	}

	return (XCfupmc_Config *)CfgPtr;
}
/** @} */
