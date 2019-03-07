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
* @file dppt_vdma.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include <stdio.h>
//#include "xbasic_types.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xaxivdma.h"

struct dma_chan_parms {
       u32 Dma_Base_Addr;
       u32 Perf_Dma_Base_Addr;
       u32 AXIVDMA_DEVICE_ID;
       u32 Tx_Intr;
       u32 Rx_Intr;
       u32 RD_ADDR_BASE;
       u32 WR_ADDR_BASE;
       u32 TX_BD_ADDR_BASE;
       u32 RX_BD_ADDR_BASE;
       u32 BlockStartOffset;
       u32 BlockHoriz;
       u32 BlockVert;
       XAxiVdma AxiVdma;
       XAxiVdma_DmaSetup ReadCfg;
       XAxiVdma_DmaSetup WriteCfg;
       XAxiVdma_Config *Config;
       XAxiVdma_FrameCounter FrameCfg;
};

typedef enum
{
       ONLY_READ=1,
       ONLY_WRITE,
       STOP_WRITE,
       STOP_READ,
       BOTH
}vdma_run_mode;

#define FRAME_LENGTH         		0x3B53800;//0x1FAA000;
#define NUMBER_OF_READ_FRAMES    XPAR_AXIVDMA_0_NUM_FSTORES
#define NUMBER_OF_WRITE_FRAMES   XPAR_AXIVDMA_0_NUM_FSTORES
#define BPC (XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_BITS_PER_COLOR * 3 * 4/8)
#define DDR_MEMORY               \
				(XPAR_MEMORY_SUBSYSTEM_DDR4_0_BASEADDR +  0x1000000)


int WriteSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
					u32 dp_vres, u8 pixel);
int ReadSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
					u32 dp_vres, u8 pixel);
void Dprx_StartVDMA(XAxiVdma *InstancePtr, u16 Direction, u32 hres, u32 vres,
					u8 pxl, struct dma_chan_parms *dma_struct);
