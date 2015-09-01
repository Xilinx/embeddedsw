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
/****************************************************************************/
/**
*
* @file xtft_sinit.c
* @addtogroup tft_v5_0
* @{
*
* This file defines the implementation of Tft device static initialization
* functionality.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xparameters.h"
#include "xtft.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes ****************************/

/************************** Variable Definitions ****************************/
extern XTft_Config XTft_ConfigTable[];

/************************** Function Definitions ****************************/
/****************************************************************************/
/**
*
* This function obtains the Configuration pointer of the device whose ID
* is being passed as the parameter to the function.
*
* @param	DeviceId is the unique number of the device.
*
* @return	Configuration pointer of the Device whose ID is given.
*
* @note		None.
*
*****************************************************************************/
XTft_Config *XTft_LookupConfig(u16 DeviceId)
{
	XTft_Config *CfgPtr = NULL;
	u32 Index;

	/*
	 * Based on the number of instances of tft defined, compare
	 * the given Device ID with the ID of each instance and get
	 * the configptr of the matched instance.
	 */
	for (Index=0; Index < XPAR_XTFT_NUM_INSTANCES; Index++) {
		if (XTft_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTft_ConfigTable[Index];
		}
	}

	return CfgPtr;
}

/** @} */
