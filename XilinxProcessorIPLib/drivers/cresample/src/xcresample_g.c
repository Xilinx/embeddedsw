
/*******************************************************************
 *
 * Copyright 1986-2014 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************/

#include "xparameters.h"
#include "xcresample.h"

/*
* The configuration table for devices
*/

XCresample_Config XCresample_ConfigTable[] =
{
	{
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_DEVICE_ID,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_BASEADDR,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_S_AXIS_VIDEO_FORMAT,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_M_AXIS_VIDEO_FORMAT,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_S_AXI_CLK_FREQ_HZ,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_HAS_INTC_IF,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_HAS_DEBUG,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_MAX_COLS,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_ACTIVE_ROWS,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_ACTIVE_COLS,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_CHROMA_PARITY,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_FIELD_PARITY,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_INTERLACED,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_NUM_H_TAPS,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_NUM_V_TAPS,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_CONVERT_TYPE,
		XPAR_FMC_SENSOR_INPUT_V_CRESAMPLE_1_COEF_WIDTH
	},
	{
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_DEVICE_ID,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_BASEADDR,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_S_AXIS_VIDEO_FORMAT,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_M_AXIS_VIDEO_FORMAT,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_S_AXI_CLK_FREQ_HZ,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_HAS_INTC_IF,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_HAS_DEBUG,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_MAX_COLS,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_ACTIVE_ROWS,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_ACTIVE_COLS,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_CHROMA_PARITY,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_FIELD_PARITY,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_INTERLACED,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_NUM_H_TAPS,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_NUM_V_TAPS,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_CONVERT_TYPE,
		XPAR_HDMI_OUTPUT_V_CRESAMPLE_1_COEF_WIDTH
	}
};


