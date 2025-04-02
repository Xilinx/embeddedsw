/******************************************************************************
* Copyright (C) 2020 - 2021  Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#ifdef __cplusplus
extern "C" {
#endif

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

#define FRAME_LENGTH         		0x3B53800;//0x1FAA000;
#ifndef  SDT
#define NUMBER_OF_READ_FRAMES    XPAR_AXIVDMA_0_NUM_FSTORES
#define NUMBER_OF_WRITE_FRAMES   XPAR_AXIVDMA_0_NUM_FSTORES
#else
#define NUMBER_OF_READ_FRAMES    XPAR_XAXIVDMA_0_NUM_FSTORES
#define NUMBER_OF_WRITE_FRAMES   XPAR_XAXIVDMA_0_NUM_FSTORES
#endif
#define BPC (XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_BITS_PER_COLOR * 3 * 4/8)
#ifndef  SDT
#define DDR_MEMORY                  (XPAR_MIG7SERIES_0_BASEADDR +  0x1000000)
#else
#define DDR_MEMORY                  (XPAR_MIG_0_BASEADDRESS +  0x1000000)
#endif

int WriteSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
			u32 dp_vres, u8 pixel);
int ReadSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
			u32 dp_vres, u8 pixel);
void Dprx_StartVDMA(XAxiVdma *InstancePtr, u16 Direction, u32 hres, u32 vres,
			u8 pxl, struct dma_chan_parms *dma_struct);
#ifdef __cplusplus
}
#endif
