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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xdeint_sinit.c
 *
 * This file contains static initialization methods for Xilinx Video
 * Deinterlacer (DEINT) device driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a rjh 07/10/11 First release
 * 2.00a rjh 18/01/12 Updated for v_deinterlacer 2.00
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xdeint.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * XDeint_LookupConfig returns a reference to an XDeint_Config structure
 * based on the unique device id, <i>DeviceId</i>. The return value will refer
 * to an entry in the device configuration table defined in the xdeint_g.c
 * file.
 *
 * @param  DeviceId is the unique device ID of the device for the lookup
 *	   operation.
 *
 * @return XDeint_LookupConfig returns a reference to a config record in the
 *	   configuration table (in xDEINT_g.c) corresponding to <i>DeviceId</i>,
 *	   or NULL if no match is found.
 *
 ******************************************************************************/
XDeint_Config *XDeint_LookupConfig(u16 DeviceId)
{
	extern XDeint_Config XDeint_ConfigTable[];
	XDeint_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XDEINT_NUM_INSTANCES; i++) {
		if (XDeint_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XDeint_ConfigTable[i];
			break;
		}
	}

	return (CfgPtr);
}
