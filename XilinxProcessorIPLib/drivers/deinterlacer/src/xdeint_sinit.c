/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xdeint_sinit.c
* @addtogroup deinterlacer_v3_2
* @{
*
* This file contains static initialization methods for Xilinx Video
* Deinterlacer (DEINT) core driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----- -------- ------------------------------------------------------
* 1.00a rjh   07/10/11 First release.
* 2.00a rjh   18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk   02/13/14 Added Doxygen support, adherence to Xilinx
*                      coding standards.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
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
* This function returns a reference to an XDeint_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xdeint_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	XDeint_LookupConfig returns a reference to a config record
*		in the configuration table (in xDEINT_g.c) corresponding to
*		<i>DeviceId</i> or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XDeint_Config *XDeint_LookupConfig(u16 DeviceId)
{
	extern XDeint_Config XDeint_ConfigTable[XPAR_XDEINT_NUM_INSTANCES];
	XDeint_Config *CfgPtr = NULL;
	u32 Index;

	/* To get the reference pointer to XDeint_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XDEINT_NUM_INSTANCES);
						Index++) {

		/* Compare device Id with configTable's device Id */
		if (XDeint_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDeint_ConfigTable[Index];
			break;
		}
	}

	return (XDeint_Config *)(CfgPtr);
}
/** @} */
