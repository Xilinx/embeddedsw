/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* @file xnandpsu_sinit.c
* @addtogroup nandpsu_v1_3
* @{
*
* The implementation of the XNandPsu driver's static initialzation
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/
#include "xstatus.h"
#include "xparameters.h"
#include "xnandpsu.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

extern XNandPsu_Config XNandPsu_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the controller configuration based on the unique controller ID. A
* table contains the configuration info for each controller in the system.
*
* @param	DeviceID is the ID of the controller to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified
*		controller ID was not found.
*
******************************************************************************/
XNandPsu_Config *XNandPsu_LookupConfig(u16 DevID)
{
	XNandPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XNANDPSU_NUM_INSTANCES; Index++) {
		if (XNandPsu_ConfigTable[Index].DeviceId == DevID) {
			CfgPtr = &XNandPsu_ConfigTable[Index];
			break;
		}
	}

	return (XNandPsu_Config *)CfgPtr;
}
/** @} */
