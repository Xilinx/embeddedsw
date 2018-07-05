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
* @file xcfa_sinit.c
* @addtogroup cfa_v7_1
* @{
*
* This file contains static initialization methods for Xilinx CFA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 7.0   adk    01/07/14 First release.
*                       Implemented XCfa_LookupConfig function.
* 7.1   ms     01/16/17 Updated the parameter naming from
*                       XPAR_CFA_NUM_INSTANCES to XPAR_XCFA_NUM_INSTANCES
*                       to avoid  compilation failure for
*                       XPAR_CFA_NUM_INSTANCES as the tools are generating
*                       XPAR_XCFA_NUM_INSTANCES in the generated xcfa_g.c
*                       for fixing MISRA-C files. This is a fix for
*                       CR-966099 based on the update in the tools.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Declaration *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XCfa_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcfa_g.c file.
*
* @param	DeviceId is the unique device ID of the core for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the
*		configuration table (in xcfa_g.c) corresponding to
*		<i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCfa_Config *XCfa_LookupConfig(u16 DeviceId)
{
	extern XCfa_Config XCfa_ConfigTable[XPAR_XCFA_NUM_INSTANCES];
	u32 Index;
	XCfa_Config *CfgPtr = NULL;

	/* To get the reference pointer to XCfa_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCFA_NUM_INSTANCES);
								Index++) {

		/* Compare device Id with configTable's device Id */
		if (XCfa_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCfa_ConfigTable[Index];
			break;
		}
	}

	return (XCfa_Config *)CfgPtr;
}
/** @} */
