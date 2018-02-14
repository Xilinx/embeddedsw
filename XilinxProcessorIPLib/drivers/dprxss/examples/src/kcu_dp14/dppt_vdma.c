/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
				    XAXIVDMA_WRITE, 
				    &define_function[n].WriteCfg);

	if (Status != XST_SUCCESS) {
		  xil_printf("Write channel config failed %d\r\n", Status);
		  return XST_FAILURE;
	}

	/* 
	 * Initialize buffer addresses
	 * Use physical addresses
	 */
	Addr = define_function[n].WR_ADDR_BASE
				+ define_function[n].BlockStartOffset;
	for(i = 0; i < NUMBER_OF_WRITE_FRAMES; i++) {
			define_function[n].WriteCfg.FrameStoreStartAddr[i] = Addr;
			Addr += FRAME_LENGTH;
	}

	/* Set the buffer addresses for transfer in the DMA engine */
	Status = XAxiVdma_DmaSetBufferAddr(&define_function[n].AxiVdma,
					   XAXIVDMA_WRITE,
			define_function[n].WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel set buffer address "
			   "failed %d\r\n", Status);
		return XST_FAILURE;
	}
	
	return XST_SUCCESS;
}

int ReadSetup(struct dma_chan_parms define_function[1], u32 dp_hres,
	      u32 dp_vres, u8 pixel)
{
	int i=0;
	u32 Addr;
	int Status;
	int n = 0;

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
				    XAXIVDMA_READ,
				    &define_function[n].ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf ("failed here \r\n");
		return XST_FAILURE;
	}

	/* 
	 * Initialize buffer addresses
	 * These addresses are physical addresses
	 */
	Addr = define_function[n].RD_ADDR_BASE +
		define_function[n].BlockStartOffset;
	for(i = 0; i < NUMBER_OF_READ_FRAMES; i++) {
		define_function[n].ReadCfg.FrameStoreStartAddr[i] = Addr;
		Addr += FRAME_LENGTH;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(&define_function[n].AxiVdma,
					   XAXIVDMA_READ,
			define_function[n].ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel set buffer "
			   "address failed %d\n\r", Status);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


void Dprx_StartVDMA(XAxiVdma *InstancePtr, u16 Direction,
			u32 hres, u32 vres, u8 pxl,
			struct dma_chan_parms *dma_struct)
{
	u32 Status;
	/* Soft reset VDMA */
	XAxiVdma_Reset (InstancePtr, Direction);
	while (XAxiVdma_ResetNotDone(InstancePtr, Direction)) {
		/* Wait. */
	}

//	xil_printf ("Starting VDMA .");
	if (Direction == XAXIVDMA_READ) {
		Status = ReadSetup(dma_struct, hres, vres, pxl);
	} else {
		xil_printf ("Starting VDMA .");
		Status = WriteSetup(dma_struct, hres, vres, pxl);
	}

	xil_printf (".");
	if (Status != XST_SUCCESS) {
		if (Direction == XAXIVDMA_READ) {
			xil_printf("Read channel setup "
					"failed %d\r\n", Status);
		} else {
			xil_printf("Write channel setup "
					"failed %d\r\n", Status);
		}
	}
	
	Status = XAxiVdma_DmaStart(InstancePtr, Direction);
	if (Status != XST_SUCCESS) {
		if (Direction == XAXIVDMA_READ) {
			xil_printf("Start Read transfer "
					"failed %d\r\n", Status);
		} else {
			xil_printf("Start Write transfer "
					"failed %d\r\n", Status);
		}
	}

	xil_printf (".");
}

int ReadSetup_trunc(struct dma_chan_parms define_function[1], u32 dp_hres,
			u32 dp_vres, u8 pixel, u32 offset)
{
	int i=0;
	u32 Addr;
	int Status;
	int n = 0;

	define_function[n].ReadCfg.VertSizeInput = 2160;
	define_function[n].ReadCfg.HoriSizeInput = (3840 * BPC) / pixel;

	define_function[n].ReadCfg.Stride = (dp_hres * BPC)/ pixel;
	define_function[n].ReadCfg.FrameDelay = 1;
	define_function[n].ReadCfg.EnableCircularBuf = 1;
	define_function[n].ReadCfg.EnableSync = 1;
	define_function[n].ReadCfg.PointNum = 0;
	define_function[n].ReadCfg.EnableFrameCounter = 0;
	define_function[n].ReadCfg.FixedFrameStoreAddr = 0;
	Status = XAxiVdma_DmaConfig(&define_function[n].AxiVdma,
				    XAXIVDMA_READ,
				    &define_function[n].ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf ("failed here \r\n");
		return XST_FAILURE;
	}

	/* 
	 * Initialize buffer addresses
	 * These addresses are physical addresses
	 */
	Addr = define_function[n].RD_ADDR_BASE  +
		define_function[n].BlockStartOffset + offset;
	for(i = 0; i < NUMBER_OF_READ_FRAMES; i++) {
		define_function[n].ReadCfg.FrameStoreStartAddr[i] = Addr;
		Addr += FRAME_LENGTH;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(&define_function[n].AxiVdma,
					   XAXIVDMA_READ,
			define_function[n].ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel set buffer "
			   "address failed %d\n\r", Status);
		return XST_FAILURE;
	}
	
	return XST_SUCCESS;
}

void Dprx_StartVDMA_trunc(XAxiVdma *InstancePtr, u16 Direction,
			u32 hres, u32 vres, u8 pxl,
			struct dma_chan_parms *dma_struct, u32 offset)
{
	u32 Status;
	/* Soft reset VDMA */
	XAxiVdma_Reset (InstancePtr, Direction);
	while (XAxiVdma_ResetNotDone(InstancePtr, Direction)) {
		/* Wait */
	}

	if (Direction == XAXIVDMA_READ) {
		Status = ReadSetup_trunc(dma_struct, hres, vres, pxl, offset);
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

void vdma_stop(struct dma_chan_parms *dma_struct) {
	int try = 0;

	XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
	XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_READ);
	XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
	XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_READ);
	while ((XAxiVdma_ResetNotDone(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE)) ||
	       (XAxiVdma_ResetNotDone(&dma_struct[0].AxiVdma, XAXIVDMA_READ))) {

		XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
		XAxiVdma_DmaStop(&dma_struct[0].AxiVdma, XAXIVDMA_READ);
		XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_WRITE);
		XAxiVdma_Reset (&dma_struct[0].AxiVdma, XAXIVDMA_READ);
		try++;

		if(try > 20)
			break;
	}
}

void vdma_start(struct dma_chan_parms *dma_struct, u32 dp_msa_hres,
		u32 dp_msa_vres, u8 pixel, int monitor_8K, u8 LineRate,
		u32 recv_frame_clk_int) {
	u8 pixel_int = 0;

	Dprx_StartVDMA(&dma_struct[0].AxiVdma, XAXIVDMA_WRITE,
			dp_msa_hres, dp_msa_vres, pixel,dma_struct);

	/* This is 8K pass-thorugh mode */
	if (monitor_8K != 1 && LineRate == 0x1E) {
		/* 4K120 will be changed to 4K60 */
		if ((recv_frame_clk_int * dp_msa_hres *
		     dp_msa_vres) > 4096*2160*60) {
			Dprx_StartVDMA_trunc(&dma_struct[0].AxiVdma, 
					XAXIVDMA_READ, dp_msa_hres,
					dp_msa_vres, pixel, dma_struct, 0);
		} else {

			pixel_int = 0x4;

			Dprx_StartVDMA(&dma_struct[0].AxiVdma, XAXIVDMA_READ,
						dp_msa_hres, dp_msa_vres,
						pixel_int, dma_struct);
		}
	} else {
		Dprx_StartVDMA(&dma_struct[0].AxiVdma, XAXIVDMA_READ,
				dp_msa_hres, dp_msa_vres, pixel,dma_struct);
	}

}
