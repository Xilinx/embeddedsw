/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xrtcpsu_sinit.c
* @addtogroup rtcpsu_v1_8
* @{
*
* This file contains the implementation of the XRtcPsu driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    04/21/15 First release.
* 1.7   sne    03/01/19 Added Versal support.
* 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
*                       modified the code such as
*                       No brackets to loop body,Declared the poiner param
*                       as Pointer to const,No brackets to then/else,
*                       Literal value requires a U suffix,Casting operation to a pointer
*                       Array has no bounds specified,Logical conjunctions need brackets.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrtcpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern XRtcPsu_Config XRtcPsu_ConfigTable[XPAR_XRTCPSU_NUM_INSTANCES];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XRtcPsu_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XRtcPsu_Config *XRtcPsu_LookupConfig(u16 DeviceId)
{
	XRtcPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRTCPSU_NUM_INSTANCES; Index++) {
		if (XRtcPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XRtcPsu_ConfigTable[Index];
			break;
		}
	}

	return (XRtcPsu_Config *)CfgPtr;
}
/** @} */
