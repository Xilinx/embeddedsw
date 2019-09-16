/******************************************************************************
*
* Copyright (C) 2010 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xaxicdma_g.c
* @addtogroup axicdma_v4_6
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, XPS will automatically generate this file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a jz   08/16/10 First release
* 4.5   rsp  07/04/18 Sync XAxiCdma_Config initializer fields
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xaxicdma.h"
/************************** Constant Definitions *****************************/

XAxiCdma_Config XAxiCdma_ConfigTable[] =
{
	{
		XPAR_AXICDMA_0_DEVICE_ID,
		XPAR_AXICDMA_0_BASEADDR,
		XPAR_AXICDMA_0_INCLUDE_DRE,
		XPAR_AXICDMA_0_USE_DATAMOVER_LITE,
		XPAR_AXICDMA_0_M_AXI_DATA_WIDTH,
		XPAR_AXICDMA_0_M_AXI_MAX_BURST_LEN,
		XPAR_AXICDMA_0_ADDR_WIDTH
	}
};
/** @} */
