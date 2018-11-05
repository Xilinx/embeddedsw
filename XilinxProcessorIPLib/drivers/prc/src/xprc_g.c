/******************************************************************************
* Copyright (C) 2010-2016 Xilinx, Inc. All rights reserved.
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
#include "xprc.h"

/* The configuration table for devices */
XPrc_Config XPrc_ConfigTable[] = {

	{
		XPAR_PRC_DEVICE_ID,
		XPAR_PRC_BASEADDR,
		XPAR_PRC_NUM_OF_VSMS,
		XPAR_PRC_CLEARING_BITSTREAM,
		XPAR_PRC_CP_ARBITRATION_PROTOCOL,
		XPAR_PRC_HAS_AXI_LITE_IF,
		XPAR_PRC_RESET_ACTIVE_LEVEL,
		XPAR_PRC_CP_FIFO_DEPTH,
		XPAR_PRC_CP_FIFO_TYPE,
		XPAR_PRC_CP_FAMILY,
		XPAR_PRC_CDC_STAGES,
		{XPAR_PRC_NUM_RMS_VS_SHIFT,
			XPAR_PRC_NUM_RMS_VS_COUNT},
		{XPAR_PRC_NUM_RMS_ALLOC_VS_SHIFT,
			XPAR_PRC_NUM_RMS_ALLOC_VS_COUNT},
		{XPAR_PRC_STRT_IN_SHTDOWN_VS_SHIFT,
			XPAR_PRC_STRT_IN_SHTDOWN_VS_COUNT},
		{XPAR_PRC_NUM_TRGRS_ALLOC_VS_SHIFT,
			XPAR_PRC_NUM_TRGRS_ALLOC_VS_COUNT},
		{XPAR_PRC_SHUTDOWN_ON_ERR_VS_SHIFT,
			XPAR_PRC_SHUTDOWN_ON_ERR_VS_COUNT},
		{XPAR_PRC_HAS_POR_RM_VS_SHIFT,
			XPAR_PRC_HAS_POR_RM_VS_COUNT},
		{XPAR_PRC_POR_RM_VS_SHIFT,
			XPAR_PRC_POR_RM_VS_COUNT},
		{XPAR_PRC_HAS_AXIS_STATUS_VS_SHIFT,
			XPAR_PRC_HAS_AXIS_STATUS_VS_COUNT},
		{XPAR_PRC_HAS_AXIS_CONTROL_VS_SHIFT,
			XPAR_PRC_HAS_AXIS_CONTROL_VS_COUNT},
		{XPAR_PRC_SKIP_RM_STARTUP_AFTER_RESET_VS_SHIFT,
			XPAR_PRC_SKIP_RM_STARTUP_AFTER_RESET_VS_COUNT},
		{XPAR_PRC_NUM_HW_TRIGGERS_VS_SHIFT,
			XPAR_PRC_NUM_HW_TRIGGERS_VS_COUNT},
		XPAR_PRC_VSM_SELECT_MSB,
		XPAR_PRC_VSM_SELECT_LSB,
		XPAR_PRC_TABLE_SELECT_MSB,
		XPAR_PRC_TABLE_SELECT_LSB,
		XPAR_PRC_REG_SELECT_MSB,
		XPAR_PRC_REG_SELECT_LSB,
	}

};
