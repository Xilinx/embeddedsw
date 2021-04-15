/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file dppt_vdma.c
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

#include "dppt_vdma.h"


int WriteSetup(struct dma_chan_parms define_function[1],
				u32 dp_hres, u32 dp_vres, u8 pixel)
{
	int i;
	u32 Addr;
	int Status;
	int n = 0;
//                for (n = 0; n < NUMBER_OF_DMAS; n++)
//                {
	define_function[n].WriteCfg.VertSizeInput = dp_vres;
	define_function[n].WriteCfg.HoriSizeInput = (dp_hres * BPC) / pixel;
	define_function[n].WriteCfg.Stride = (dp_hres * BPC) / pixel;
	define_function[n].WriteCfg.FrameDelay = 0;
	define_function[n].WriteCfg.EnableCircularBuf = 1;
	define_function[n].WriteCfg.EnableSync = 1;
	define_function[n].WriteCfg.PointNum = 0;
	define_function[n].WriteCfg.EnableFrameCounter = 0;
	define_function[n].WriteCfg.FixedFrameStoreAddr = 0;
	Status = XAxiVdma_DmaConfig(&define_function[n].AxiVdma,
								XAXIVDMA_WRITE, &define_function[n].WriteCfg);
	if (Status != XST_SUCCESS)
	{
		  xil_printf("Write channel config failed %d\r\n", Status);
		  return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * Use physical addresses
	 */
	Addr = define_function[n].WR_ADDR_BASE
				+ define_function[n].BlockStartOffset;
	for(i = 0; i < NUMBER_OF_WRITE_FRAMES; i++) {
			define_function[n].WriteCfg.FrameStoreStartAddr[i] = Addr;
			//(dp_vres * dp_hres * BPC) / pixel; //FRAME_LENGTH ;
			Addr += FRAME_LENGTH;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 */
	Status = XAxiVdma_DmaSetBufferAddr(&define_function[n].AxiVdma,
			   XAXIVDMA_WRITE,define_function[n].WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS)
	{
			xil_printf("Write channel set buffer address failed %d\r\n",
						Status);
		 return XST_FAILURE;
	}
//}
	return XST_SUCCESS;
}

int ReadSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
				u32 dp_vres, u8 pixel) {
	int i=0;
	u32 Addr;
	int Status;
	int n = 0;

//        for (n = 0; n < NUMBER_OF_DMAS; n++)
//        {
	define_function[n].ReadCfg.VertSizeInput = dp_vres;
	define_function[n].ReadCfg.HoriSizeInput = (dp_hres * BPC) / pixel;
	define_function[n].ReadCfg.Stride = (dp_hres * BPC)/ pixel;
	define_function[n].ReadCfg.FrameDelay = 1;
	define_function[n].ReadCfg.EnableCircularBuf = 1;
	define_function[n].ReadCfg.EnableSync = 1;
	define_function[n].ReadCfg.PointNum = 0;
	define_function[n].ReadCfg.EnableFrameCounter = 0;
	define_function[n].ReadCfg.FixedFrameStoreAddr = 0;
	Status = XAxiVdma_DmaConfig(&define_function[n].AxiVdma,
								XAXIVDMA_READ, &define_function[n].ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf ("failed here \r\n");
		return XST_FAILURE;
	}
	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = define_function[n].RD_ADDR_BASE
				+ define_function[n].BlockStartOffset;
	for(i = 0; i < NUMBER_OF_READ_FRAMES; i++) {
			define_function[n].ReadCfg.FrameStoreStartAddr[i] = Addr;

			Addr += FRAME_LENGTH;//(dp_vres * dp_hres * BPC) / pixel;
	}
	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(&define_function[n].AxiVdma,
				XAXIVDMA_READ,define_function[n].ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
			xil_printf("Read channel set buffer address failed %d\n\r", Status);
			return XST_FAILURE;
	}
//     }
	return XST_SUCCESS;
}


void Dprx_StartVDMA(XAxiVdma *InstancePtr, u16 Direction, u32 hres, u32 vres,
			u8 pxl, struct dma_chan_parms *dma_struct){
	u32 Status;
    // Soft reset VDMA
	XAxiVdma_Reset (InstancePtr, Direction);
	while (XAxiVdma_ResetNotDone(InstancePtr, Direction)) {
	}
//	xil_printf ("Starting VDMA .");
	if (Direction == XAXIVDMA_READ) {
		Status = ReadSetup(dma_struct, hres, vres, pxl);
	} else {
		xil_printf ("Starting VDMA .");
		Status = WriteSetup(dma_struct, hres, vres, pxl);
	}
    xil_printf (".");
    if (Status != XST_SUCCESS)
    {
	if (Direction == XAXIVDMA_READ) {
            xil_printf("Read channel setup failed %d\r\n", Status);
	} else {
		xil_printf("Write channel setup failed %d\r\n", Status);
	}
    }
	Status = XAxiVdma_DmaStart(InstancePtr, Direction);
	if (Status != XST_SUCCESS) {
	if (Direction == XAXIVDMA_READ) {
	    xil_printf("Start Read transfer failed %d\r\n", Status);
	} else {
		xil_printf("Start Write transfer failed %d\r\n", Status);
	}
    }
	xil_printf (".");
}
