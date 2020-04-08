/*******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

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
#include "xbasic_types.h"
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

//#define FRAME_LENGTH         		0x3B53800;//0x1FAA000;
#define FRAME_LENGTH        0x8000000; // This is good for 8K4K resolution
#define NUMBER_OF_READ_FRAMES    XPAR_AXIVDMA_0_NUM_FSTORES
#define NUMBER_OF_WRITE_FRAMES   XPAR_AXIVDMA_0_NUM_FSTORES
#define BPC (XPAR_DPRXSS_0_BITS_PER_COLOR * 3 * 4/8)
#define DDR_MEMORY               \
	(XPAR_MEMORY_SUBSYSTEM_DDR4_0_BASEADDR +  0x1000000)


int WriteSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
			u32 dp_vres, u8 pixel);
int ReadSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
			u32 dp_vres, u8 pixel);
void Dprx_StartVDMA(XAxiVdma *InstancePtr, u16 Direction, u32 hres, u32 vres,
			u8 pxl, struct dma_chan_parms *dma_struct);
void Dprx_StartVDMA_trunc(XAxiVdma *InstancePtr, u16 Direction, u32 hres, 
	u32 vres, u8 pxl, struct dma_chan_parms *dma_struct, u32 offset);
