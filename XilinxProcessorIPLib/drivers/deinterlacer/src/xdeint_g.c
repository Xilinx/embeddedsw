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
* @file XDeint_g.c
* @addtogroup deinterlacer_v3_2
* @{
*
* This file contains a template for configuration table of Xilinx Video
* Deinterlacer For a real hardware system, Xilinx Platform Studio (XPS) will
* automatically generate a real configuration table to match the configuration
* of the Deinterlacer devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a rjh    07/10/11 First release
* 2.00a rjh    18/01/12 Updated for v_deinterlacer 2.00
* 3.2   adk    02/13/14 Adherence to Xilinx coding, Doxygen guidelines.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/

#include "xdeint.h"
#include "xparameters.h"

/**
* The configuration table for Video Deinterlacers devices
*/
XDeint_Config XDeint_ConfigTable[] = {
	{
		XPAR_FMC_SENSOR_INPUT_V_DEINTERLACER_1_DEVICE_ID,
		XPAR_FMC_SENSOR_INPUT_V_DEINTERLACER_1__BASEADDR
	}
};
/** @} */
