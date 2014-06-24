/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
/**
*
* @file xscaler_g.c
*
* This file contains a template for configuration table of Xilinx MVI Video
* Scaler devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	 Changes
* -----	----	-------- -------------------------------------------------------
* 1.00a	xd	05/14/09 First release
* 2.00a	xd	12/14/09 Updated doxygen document tags
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/


#include "xparameters.h"
#include "xscaler.h"

/**
 * The configuration table for Scaler devices
 */
XScaler_Config XScaler_ConfigTable[] = {
	{
		XPAR_SCALER_0_DEVICE_ID,
		XPAR_SCALER_0_BASEADDR,
		XPAR_SCALER_0_VERT_TAP_NUM,
		XPAR_SCALER_0_HORI_TAP_NUM,
		XPAR_SCALER_0_MAX_PHASE_NUM,
		XPAR_SCALER_0_MAX_COEF_SETS,
		XPAR_SCALER_0_CHROMA_FORMAT,
		XPAR_SCALER_0_SEPARATE_YC_COEFS,
		XPAR_SCALER_0_SEPARATE_HV_COEFS
	}
};
