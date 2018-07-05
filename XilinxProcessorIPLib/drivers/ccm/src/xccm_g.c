/*******************************************************************
*
* Copyright 1986-2014 Xilinx, Inc. All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*

*******************************************************************************/


#include "xparameters.h"
#include "xccm.h"

/*
* The configuration table for devices
*/

XCcm_Config XCcm_ConfigTable[] =
{
	{
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_DEVICE_ID,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_BASEADDR,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_S_AXIS_VIDEO_FORMAT,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_M_AXIS_VIDEO_FORMAT,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_MAX_COLS,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_ACTIVE_COLS,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_ACTIVE_ROWS,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_HAS_DEBUG,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_HAS_INTC_IF,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_CLIP,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_CLAMP,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K11,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K12,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K13,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K21,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K22,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K23,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K31,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K32,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_K33,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_ROFFSET,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_GOFFSET,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_BOFFSET,
		XPAR_FMC_SENSOR_INPUT_V_CCM_1_S_AXI_CLK_FREQ_HZ
	}
};


