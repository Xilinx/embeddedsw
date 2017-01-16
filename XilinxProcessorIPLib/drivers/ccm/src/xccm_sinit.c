/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
* @file xccm_sinit.c
* @addtogroup ccm_v6_0
* @{
*
* This file contains static initialization methods for Xilinx CCM core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- --------------------------------------------------
* 6.0   adk     03/06/14 First release.
*                        Implemented XCcm_LookupConfig function.
* 6.1   ms     01/16/17  Updated the parameter naming from
*                        XPAR_CCM_NUM_INSTANCES to XPAR_XCCM_NUM_INSTANCES
*                        to avoid  compilation failure for
*                        XPAR_CCM_NUM_INSTANCES as the tools are generating
*                        XPAR_XCCM_NUM_INSTANCES in the generated xccm_g.c
*                        for fixing MISRA-C files. This is a fix for
*                        CR-966099 based on the update in the tools.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"
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
* XCcm_LookupConfig returns a reference to an XCcm_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xccm_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xccm_g.c) corresponding to <i>DeviceId</i>, or NULL
*		if no match is found.
*
* @note		None.
******************************************************************************/
XCcm_Config *XCcm_LookupConfig(u16 DeviceId)
{
	extern XCcm_Config XCcm_ConfigTable[XPAR_XCCM_NUM_INSTANCES];
	XCcm_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCCM_NUM_INSTANCES);
								Index++) {
		if (XCcm_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCcm_ConfigTable[Index];
			break;
		}
	}

	return (XCcm_Config *)CfgPtr;
}
/** @} */
