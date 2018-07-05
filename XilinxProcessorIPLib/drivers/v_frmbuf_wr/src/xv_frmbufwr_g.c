/*******************************************************************
*
* Copyright (C) 2010-2017 Xilinx, Inc. All rights reserved.
*
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
#include "xv_frmbufwr.h"

/*
* The configuration table for devices
*/

XV_frmbufwr_Config XV_frmbufwr_ConfigTable[XPAR_XV_FRMBUFWR_NUM_INSTANCES] =
{
	{
		XPAR_V_FRMBUF_WR_0_DEVICE_ID,
		XPAR_V_FRMBUF_WR_0_S_AXI_CTRL_BASEADDR,
		XPAR_V_FRMBUF_WR_0_SAMPLES_PER_CLOCK,
		XPAR_V_FRMBUF_WR_0_MAX_COLS,
		XPAR_V_FRMBUF_WR_0_MAX_ROWS,
		XPAR_V_FRMBUF_WR_0_MAX_DATA_WIDTH,
		XPAR_V_FRMBUF_WR_0_AXIMM_DATA_WIDTH,
		XPAR_V_FRMBUF_WR_0_AXIMM_ADDR_WIDTH,
		XPAR_V_FRMBUF_WR_0_HAS_RGBX8,
		XPAR_V_FRMBUF_WR_0_HAS_YUVX8,
		XPAR_V_FRMBUF_WR_0_HAS_YUYV8,
		XPAR_V_FRMBUF_WR_0_HAS_RGBX10,
		XPAR_V_FRMBUF_WR_0_HAS_YUVX10,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV8,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV8_420,
		XPAR_V_FRMBUF_WR_0_HAS_RGB8,
		XPAR_V_FRMBUF_WR_0_HAS_YUV8,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV10,
		XPAR_V_FRMBUF_WR_0_HAS_Y_UV10_420,
		XPAR_V_FRMBUF_WR_0_HAS_Y8,
		XPAR_V_FRMBUF_WR_0_HAS_Y10,
		XPAR_V_FRMBUF_WR_0_HAS_BGRX8,
		XPAR_V_FRMBUF_WR_0_HAS_UYVY8,
		XPAR_V_FRMBUF_WR_0_HAS_BGR8,
		XPAR_V_FRMBUF_WR_0_HAS_INTERLACED
	}
};
