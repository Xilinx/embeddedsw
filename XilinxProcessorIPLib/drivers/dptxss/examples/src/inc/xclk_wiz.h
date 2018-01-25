/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file dppt.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  YB    07/01/15 Initial release.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "xil_types.h"
#include "../src/xvid_pat_gen.h"

/********************** Constant Definition **********************/
#define CLK_WIZ_BASE      				\
	XPAR_VID_CLK_RST_HIER_CLK_WIZ_0_BASEADDR
#if (XPAR_XHDCP_NUM_INSTANCES > 0 \
		&& XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_GT_DATAWIDTH == 2)
#define CLK_WIZ_BASE_TX                                 XPAR_CLK_WIZ_1_BASEADDR
#define CLK_WIZ_BASE_RX                                 XPAR_CLK_WIZ_0_BASEADDR
#endif
#define CLK_LOCK                        1

#define VCO_FREQ                        600                                                                                                         /*FIXED Value */
#define CLK_WIZ_VCO_FACTOR              (VCO_FREQ * 10000)

/********************** Variables Definition **********************/

/********************** Function Definition **********************/
int wait_for_lock();
void ComputeMandD(XDp *InstancePtr, u32 VidFreq);
void ComputeMandD_txlnk(u32 VidFreq, u16 Link_rate);
void ComputeMandD_rxlnk(u32 VidFreq, u16 Link_rate);
