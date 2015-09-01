/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
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
* @file xtrafgen_g.c
* @addtogroup trafgen_v3_2
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, this file will be automatically generated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a srt  01/24/13 First release
* 1.01a adk  03/09/13 Updated driver to Support Static and Streaming mode.
* 2.00a adk  16/09/13 Fixed CR:737291
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtrafgen.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

XTrafGen_Config XTrafGen_ConfigTable[] =
{
	{
		XPAR_XTRAFGEN_0_DEVICE_ID,
		XPAR_XTRAFGEN_0_BASEADDR,
		XPAR_XTRAFGEN_0_ATG_MODE,
		XPAR_XTRAFGEN_0_ATG_MODE_L2,
		XPAR_XTRAFGEN_0_AXIS_MODE,
		XPAR_XTRAFGEN_0_ADDRESS_WIDTH
	}
};
/** @} */
