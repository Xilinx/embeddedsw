/******************************************************************************
*
* Copyright (C) 2014 - 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file vdma_api.c
 *
 * This file has high level API to configure and start the VDMA.The file assumes that:
 * The design has VDMA with both MM2S and S2MM path enable.
 * The API's has tested with hardware that has tow VDMA and MM2S to S2MM are back
 * to back connected for each VDMA.
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.0   adk  11/26/15 First release
 ****************************************************************************/

#define DEBUG_MODE		0

/******************** Include files **********************************/
#include "xaxivdma.h"
#include "xparameters.h"
#include "xil_exception.h"

/******************** Data structure Declarations *****************************/

typedef struct vdma_handle
{
	/* The device ID of the VDMA */
	unsigned int device_id;
	/* The state variable to keep track if the initialization is done*/
	unsigned int init_done;
	/** The XAxiVdma driver instance data. */
	XAxiVdma* InstancePtr;
	/* The XAxiVdma_DmaSetup structure contains all the necessary information to
	 * start a frame write or read. */
	XAxiVdma_DmaSetup ReadCfg;
	XAxiVdma_DmaSetup WriteCfg;
	/* Horizontal size of frame */
	unsigned int hsize;
	/* Vertical size of frame */
	unsigned int vsize;
	/* Buffer address from where read and write will be done by VDMA */
	unsigned int buffer_address;
	/* Flag to tell VDMA to interrupt on frame completion*/
	unsigned int enable_frm_cnt_intr;
	/* The counter to tell VDMA on how many frames the interrupt should happen*/
	unsigned int number_of_frame_count;
}vdma_handle;

/******************** Constant Definitions **********************************/

/*
 * Device related constants. These need to defined as per the HW system.
 */
vdma_handle vdma_context[XPAR_XAXIVDMA_NUM_INSTANCES];
static unsigned int context_init=0;

/******************* Function Prototypes ************************************/

static int ReadSetup(vdma_handle *vdma_context);
static int WriteSetup(vdma_handle *vdma_context);
static int StartTransfer(XAxiVdma *InstancePtr);

/*****************************************************************************/
/**
*
* run_triple_frame_buffer API
*
* This API is the interface between application and other API. When application will call
* this API with right argument, This API will call rest of the API to configure the read
* and write path of VDMA,based on ID. After that it will start both the read and write path
* of VDMA
*
* @param	InstancePtr is the handle to XAxiVdma data structure.
* @param	DeviceId is the device ID of current VDMA
* @param	hsize is the horizontal size of the frame. It will be in Pixels.
* 		The actual size of frame will be calculated by multiplying this
* 		with tdata width.
* @param 	vsize is the Vertical size of the frame.
* @param	buf_base_addr is the buffer address where frames will be written
*		and read by VDMA.
* @param 	number_frame_count specifies after how many frames the interrupt
*		should come.
* @param 	enable_frm_cnt_intr is for enabling frame count interrupt
*		when set to 1.
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
******************************************************************************/
int run_triple_frame_buffer(XAxiVdma* InstancePtr, int DeviceId, int hsize,
		int vsize, int buf_base_addr, int number_frame_count,
		int enable_frm_cnt_intr)
{
	int Status,i;
	XAxiVdma_Config *Config;
	XAxiVdma_FrameCounter FrameCfgPtr;

	/* This is one time initialization of state machine context.
	 * In first call it will be done for all VDMA instances in the system.
	 */
	if(context_init==0) {
		for(i=0; i < XPAR_XAXIVDMA_NUM_INSTANCES; i++) {
			vdma_context[i].InstancePtr = NULL;
			vdma_context[i].device_id = -1;
			vdma_context[i].hsize = 0;
			vdma_context[i].vsize = 0;
			vdma_context[i].init_done = 0;
			vdma_context[i].buffer_address = 0;
			vdma_context[i].enable_frm_cnt_intr = 0;
			vdma_context[i].number_of_frame_count = 0;

		}
		context_init = 1;
	}

	/* The below initialization will happen for each VDMA. The API argument
	 * will be stored in internal data structure
	 */

	/* The information of the XAxiVdma_Config comes from hardware build.
	 * The user IP should pass this information to the AXI DMA core.
	 */
	Config = XAxiVdma_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No video DMA found for ID %d\r\n",DeviceId );
		return XST_FAILURE;
	}

	if(vdma_context[DeviceId].init_done ==0) {
		vdma_context[DeviceId].InstancePtr = InstancePtr;

		/* Initialize DMA engine */
		Status = XAxiVdma_CfgInitialize(vdma_context[DeviceId].InstancePtr,
						Config, Config->BaseAddress);
		if (Status != XST_SUCCESS) {
			xil_printf("Configuration Initialization failed %d\r\n",
					Status);
			return XST_FAILURE;
		}

		vdma_context[DeviceId].init_done = 1;
	}

	vdma_context[DeviceId].device_id = DeviceId;
	vdma_context[DeviceId].vsize = vsize;

	vdma_context[DeviceId].buffer_address = buf_base_addr;
	vdma_context[DeviceId].enable_frm_cnt_intr = enable_frm_cnt_intr;
	vdma_context[DeviceId].number_of_frame_count = number_frame_count;
	vdma_context[DeviceId].hsize = hsize * (Config->Mm2SStreamWidth>>3);

	/* Setup the write channel */
	Status = WriteSetup(&vdma_context[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel setup failed %d\r\n", Status);
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}

	/* Setup the read channel */
	Status = ReadSetup(&vdma_context[DeviceId]);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel setup failed %d\r\n", Status);
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}

	/* The frame counter interrupt is enabled, setting VDMA for same */
	if(vdma_context[DeviceId].enable_frm_cnt_intr) {
		FrameCfgPtr.ReadDelayTimerCount = 1;
		FrameCfgPtr.ReadFrameCount = number_frame_count;
		FrameCfgPtr.WriteDelayTimerCount = 1;
		FrameCfgPtr.WriteFrameCount = number_frame_count;

		XAxiVdma_SetFrameCounter(vdma_context[DeviceId].InstancePtr,&FrameCfgPtr);
		/* Enable DMA read and write channel interrupts. The configuration for interrupt
		 * controller will be done by application	 */
		XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
				XAXIVDMA_IXR_ERROR_MASK |
				XAXIVDMA_IXR_FRMCNT_MASK,XAXIVDMA_WRITE);
		XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
				XAXIVDMA_IXR_ERROR_MASK |
				XAXIVDMA_IXR_FRMCNT_MASK,XAXIVDMA_READ);
	} else	{
		/* Enable DMA read and write channel interrupts. The configuration for interrupt
		* controller will be done by application	 */
		XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
					XAXIVDMA_IXR_ERROR_MASK,XAXIVDMA_WRITE);
		XAxiVdma_IntrEnable(vdma_context[DeviceId].InstancePtr,
					XAXIVDMA_IXR_ERROR_MASK ,XAXIVDMA_READ);
	}

	/* Start the DMA engine to transfer */
	Status = StartTransfer(vdma_context[DeviceId].InstancePtr);
	if (Status != XST_SUCCESS) {
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}
#if DEBUG_MODE
	xil_printf("Code is in Debug mode, Make sure that buffer addresses are at valid memory \r\n");
	xil_printf("In triple mode, there has to be six consecutive buffers for Debug mode \r\n");
	{
		u32 pixels,j,Addr = vdma_context[DeviceId].buffer_address;
		u8 *dst,*src;
		u32 total_pixel = vdma_context[DeviceId].stride * vdma_context[DeviceId].vsize;
		src = (unsigned char *)Addr;
		dst = (unsigned char *)Addr + (total_pixel * vdma_context->InstancePtr->MaxNumFrames);

		for(j=0;j<vdma_context->InstancePtr->MaxNumFrames;j++) {
			for(pixels=0;pixels<total_pixel;pixels++) {
				if(src[pixels] != dst[pixels]) {
					xil_printf("VDMA transfer failed: SRC=0x%x, DST=0x%x\r\n",
							src[pixels],dst[pixels]);
					exit(-1);
				}
			}
			src = src + total_pixel;
			dst = dst + total_pixel;
		}
	}
	xil_printf("VDMA transfer is happening and checked for 3 frames \r\n");
#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	vdma_context is the context pointer to the VDMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int ReadSetup(vdma_handle *vdma_context)
{
	int Index;
	u32 Addr;
	int Status;

	vdma_context->ReadCfg.VertSizeInput = vdma_context->vsize;
	vdma_context->ReadCfg.HoriSizeInput = vdma_context->hsize;

	vdma_context->ReadCfg.Stride = vdma_context->hsize;
	vdma_context->ReadCfg.FrameDelay = 0;  /* This example does not test frame delay */

	vdma_context->ReadCfg.EnableCircularBuf = 1;
	vdma_context->ReadCfg.EnableSync = 1;  /* Gen-Lock */

	vdma_context->ReadCfg.PointNum = 0;
	vdma_context->ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

	vdma_context->ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
	/* Configure the VDMA is per fixed configuration, This configuration is being used by majority
	 * of customer. Expert users can play around with this if they have different configurations */

	Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_READ, &vdma_context->ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Read channel config failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = vdma_context->buffer_address;

	for(Index = 0; Index < vdma_context->InstancePtr->MaxNumFrames; Index++) {
		vdma_context->ReadCfg.FrameStoreStartAddr[Index] = Addr;

		/* Initializing the buffer in case of Debug mode */

#if DEBUG_MODE
		{
			u32 i;
			u8 *src;
			u32 total_pixel = vdma_context->stride * vdma_context->vsize;
			src = (unsigned char *)Addr;
			xil_printf("Read Buffer %d address: 0x%x \r\n",Index,Addr);
			for(i=0;i<total_pixel;i++)
			{
				src[i] = i & 0xFF;
			}
		}
#endif
		Addr +=  vdma_context->hsize * vdma_context->vsize;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr, XAXIVDMA_READ,
			vdma_context->ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Read channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	dma_context is the context pointer to the VDMA engine..
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int WriteSetup(vdma_handle *vdma_context)
{
	int Index;
	u32 Addr;
	int Status;

	vdma_context->WriteCfg.VertSizeInput = vdma_context->vsize;
	vdma_context->WriteCfg.HoriSizeInput = vdma_context->hsize;

	vdma_context->WriteCfg.Stride = vdma_context->hsize;
	vdma_context->WriteCfg.FrameDelay = 0;  /* This example does not test frame delay */

	vdma_context->WriteCfg.EnableCircularBuf = 1;
	vdma_context->WriteCfg.EnableSync = 1;  /*  Gen-Lock */

	vdma_context->WriteCfg.PointNum = 0;
	vdma_context->WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

	vdma_context->WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */
	/* Configure the VDMA is per fixed configuration, This configuration
	 * is being used by majority of customers. Expert users can play around
	 * with this if they have different configurations
	 */

	Status = XAxiVdma_DmaConfig(vdma_context->InstancePtr, XAXIVDMA_WRITE, &vdma_context->WriteCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Write channel config failed %d\r\n", Status);

		return Status;
	}

	/* Initialize buffer addresses
	 *
	 * Use physical addresses
	 */
	Addr = vdma_context->buffer_address;
	/* If Debug mode is enabled write frame is shifted 3 Frames
	 * store ahead to compare read and write frames
	 */
#if DEBUG_MODE
	Addr = Addr + vdma_context->InstancePtr->MaxNumFrames * \
			(vdma_context->stride * vdma_context->vsize);
#endif

	for(Index = 0; Index < vdma_context->InstancePtr->MaxNumFrames; Index++) {
		vdma_context->WriteCfg.FrameStoreStartAddr[Index] = Addr;
#if DEBUG_MODE
		xil_printf("Write Buffer %d address: 0x%x \r\n",Index,Addr);
#endif

		Addr += (vdma_context->hsize * vdma_context->vsize);
	}

	/* Set the buffer addresses for transfer in the DMA engine */
	Status = XAxiVdma_DmaSetBufferAddr(vdma_context->InstancePtr,
			XAXIVDMA_WRITE,
			vdma_context->WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Write channel set buffer address failed %d\r\n",
				Status);
		return XST_FAILURE;
	}

	/* Clear data buffer
	 */
#if DEBUG_MODE
	memset((void *)vdma_context->buffer_address, 0,
			vdma_context->ReadCfg.Stride * vdma_context->ReadCfg.VertSizeInput * vdma_context->InstancePtr->MaxNumFrames);
#endif
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function starts the DMA transfers. Since the DMA engine is operating
* in circular buffer mode, video frames will be transferred continuously.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return
*		- XST_SUCCESS if both read and write start successfully
*		- XST_FAILURE if one or both directions cannot be started
*
* @note		None.
*
******************************************************************************/
static int StartTransfer(XAxiVdma *InstancePtr)
{
	int Status;
	/* Start the write channel of VDMA */
	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_WRITE);
	if (Status != XST_SUCCESS) {
		xil_printf("Start Write transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}
	/* Start the Read channel of VDMA */
	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
	if (Status != XST_SUCCESS) {
		xil_printf("Start read transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
