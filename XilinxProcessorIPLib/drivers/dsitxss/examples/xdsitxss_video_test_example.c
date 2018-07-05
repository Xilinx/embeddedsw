/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdsitxss_video_test_example.c
*
* This file contains a design example using the XDsiTxSs driver. It performs a
* video test on the MIPI DSI Tx Subsystem. It tests DSI TX functionality with
* following sequence.
* Presently the application is supporting only RGB888 format. 3 bytes per pixel
* BRAM -- (MM2S)--VDMA (MM2S)--DSI TX-- CSI RX -- (S2MM)VDMA(S2MM)--BRAM
* This function will read input from BRAM0, provide BRAM input
* to VDMA MM2S, VDMA feeds MM2S input to DSI TX controller, the output of DSI
* will be connected to CSI RX controller. The output of CSI given to S2MM of
* VDMA. VDMA writes S2MM data on BRAM1. Finally compare BRAM0 and BRAM1 data
* User has to provide all necessary inputs as per desired resolution
*
* @note		None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/2/16 Initial Release for MIPI DSI TX subsystem
* 1.1 ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings
*                  are available in all examples. This is a fix for
*                  CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsitxss.h"
#include "xaxivdma.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_exception.h"
#include "xintc.h"

/************************** Constant Definitions *****************************/

/* The unique device ID of the MIPI DSI Rx Subsystem instance
 * to be used
 */
#define XDSITXSS_DEVICE_ID	XPAR_DSITXSS_0_DEVICE_ID

/* DMA Related constants */
#define DMA_DEVICE_ID		XPAR_AXI_VDMA_0_DEVICE_ID

#define BRAM_0_ADDRESS		XPAR_AXI_BRAM_CTRL_0_S_AXI_BASEADDR
#define BRAM_1_ADDRESS		XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR

#define NO_BYTES_PER_PIXEL		3
#define HORIZONTAL_RESOLUTION		4
#define HORIZONTAL_RESOLUTION_BYTES	HORIZONTAL_RESOLUTION * NO_BYTES_PER_PIXEL
#define VERTICAL_RESOLUTION		2
#define FRAME_COUNTER			1
#define STRIDE				HORIZONTAL_RESOLUTION_BYTES

/* Specify peripheral Timing parameters */
#define HACT_VALUE	HORIZONTAL_RESOLUTION_BYTES
#define HSA_VALUE	60
#define HFP_VALUE	60
#define HBP_VALUE	60

#define VACT_VALUE	VERTICAL_RESOLUTION
#define VSA_VALUE	60
#define VFP_VALUE	60
#define VBP_VALUE	60
#define BLLP_BURST_VALUE	0x0

#define VDMA_S2MM	1
#define VDMA_MM2S	2

/* Debug Constants */
#define MM2S_HALT_SUCCESS	121
#define MM2S_HALT_FAILURE	122
#define S2MM_HALT_SUCCESS	131
#define S2MM_HALT_FAILURE	132

#define ENABLE_FRAME_COUNT	(1<<4)
#define ENABLE_CIRCULAR_MODE	(1<<1)
#define START_VDMA		(1<<0)

/* BRAM Buffers */
u8 *SrcBuffer = (u8*)BRAM_0_ADDRESS;
u8 *DstBuffer = (u8*)BRAM_1_ADDRESS;

/***************** Macros (Inline functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 DsiTxSs_VideoTestExample(u32 DeviceId);

/************************** Variable Definitions *****************************/

/* The DSI TX Subsystem instance.*/
XDsiTxSs DsiTxSsInst;

#define VDMA_BASEADDR	XPAR_AXI_VDMA_0_BASEADDR

volatile u32 *Mm2sStatusReg = (u32*)(VDMA_BASEADDR + 0x4);
volatile u32 *S2mmStatusReg = (u32*)(VDMA_BASEADDR + 0x34);

volatile u32 *VdmaParkPtrReg = (u32 *)(VDMA_BASEADDR + 0x28);

volatile u32 *VdmaS2MMCrReg = (u32 *)(VDMA_BASEADDR + 0x30);
volatile u32 *VdmaS2MMStatusReg = (u32 *)(VDMA_BASEADDR + 0x34);
volatile u32 *VdmaS2MMVertSizeReg = (u32 *)(VDMA_BASEADDR + 0xA0);
volatile u32 *VdmaS2MMHoriSizeReg = (u32 *)(VDMA_BASEADDR + 0xA4);
volatile u32 *VdmaS2MMFrmDlrStrideReg = (u32 *)(VDMA_BASEADDR + 0xA8);

volatile u32 *VdmaS2MMFrameBuffer0Reg = (u32 *)(VDMA_BASEADDR + 0xAC);

volatile u32 *VdmaMM2SCrReg = (u32 *)(VDMA_BASEADDR + 0x00);
volatile u32 *VdmaMMS2StatusReg = (u32 *)(VDMA_BASEADDR + 0x4);
volatile u32 *VdmaMM2SVertSizeReg = (u32 *)(VDMA_BASEADDR + 0x50);
volatile u32 *VdmaMM2SHoriSizeReg = (u32 *)(VDMA_BASEADDR + 0x54);
volatile u32 *VdmaMM2SFrmDlrStrideReg = (u32 *)(VDMA_BASEADDR + 0x58);

volatile u32 *VdmaMM2SFrameBuffer0Reg = (u32 *)(VDMA_BASEADDR + 0x5C);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function wait until the DMA channel halts
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
*.@param	VdmaBaseAddr VDMA base address
*
* @return
*		MM2S_HALT_SUCCESS, S2MM_HALT_SUCCESS in success
*		MM2S_HALT_FAILURE, S2MM_HALT_FAILURE on failure
*
* @note		None.
*
******************************************************************************/
s32 WaitForCompletion(s32 VdmaChannel, u32 *VdmaBaseAddr)
{
    if (VdmaChannel == VDMA_MM2S) {
	while (!(*Mm2sStatusReg & 0x1)) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"Waiting for MM2S to halt ..."
			"MM2S SR = 0x%x\r\n", *Mm2sStatusReg);
	}
	if ((*Mm2sStatusReg & 0x1)) {
		xdbg_printf(XDBG_DEBUG_GENERAL," MM2S_HALT_SUCCESS \r\n");
		return MM2S_HALT_SUCCESS;
	}else {
		xil_printf(" returning MM2S_HALT_FAILURE \r\n");
		return MM2S_HALT_FAILURE;
		}
    }
    else if (VdmaChannel == VDMA_S2MM) {
	xdbg_printf(XDBG_DEBUG_GENERAL," Poll on s2mm status register\r\n");
	while (!(*S2mmStatusReg & 0x1)) {
		xdbg_printf(XDBG_DEBUG_GENERAL," Waiting for S2MM to halt ..."
				"S2MM SR = 0x%x\r\n", *S2mmStatusReg);
	}
	if((*S2mmStatusReg & 0x1)) {
		xdbg_printf(XDBG_DEBUG_GENERAL," MM2S_HALT_SUCCESS \r\n");
		return S2MM_HALT_SUCCESS;
	}
	else {
		xil_printf(" returning S2MM_HALT_FAILURE \r\n");
		return S2MM_HALT_FAILURE;
	}
    }
	return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function ResetVDMA
*
* @param	None
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void ResetVDMA()
{
	*VdmaS2MMCrReg |= 0x4;
	while (*VdmaS2MMCrReg & 0x4);
	*VdmaMM2SCrReg |= 0x4;
	while (*VdmaMM2SCrReg & 0x4);
}

/*****************************************************************************/
/**
*
* This function to setup Horizontal size and stride
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void ConfigureChannel(u32 VdmaChannel)
{
    u8 ii = 0;
    u8 offset = 0;
    volatile u8 *BaseAddress;
    if (VdmaChannel == VDMA_MM2S) {
	*VdmaMM2SHoriSizeReg = HORIZONTAL_RESOLUTION_BYTES;
	*VdmaMM2SFrmDlrStrideReg = STRIDE;
	BaseAddress = (u32)XPAR_BRAM_0_BASEADDR;
	 while(ii < FRAME_COUNTER) {
		*(VdmaMM2SFrameBuffer0Reg + offset) = (u32)BaseAddress;
		offset = offset + 1;
		BaseAddress = BaseAddress +
		(HORIZONTAL_RESOLUTION_BYTES * VERTICAL_RESOLUTION);
		ii++;
	}
    }
    else if (VdmaChannel == VDMA_S2MM) {
	*VdmaS2MMHoriSizeReg = HORIZONTAL_RESOLUTION_BYTES;
	*VdmaS2MMFrmDlrStrideReg = STRIDE;

	BaseAddress = (u32)XPAR_BRAM_1_BASEADDR;
	while(ii < FRAME_COUNTER) {
		*(VdmaS2MMFrameBuffer0Reg + offset) = (u32)BaseAddress;
		offset = offset + 1;
		BaseAddress = BaseAddress +
		(HORIZONTAL_RESOLUTION_BYTES * VERTICAL_RESOLUTION);
		ii++;
		}
	}
}

/*****************************************************************************/
/**
*
* This function to setup the VDMA Control Register
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
* @param	FrameCount specifies number of frames to transfer
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void SetupVdmaCR(s32 VdmaChannel, s32 FrameCount)
{
	if (VdmaChannel == VDMA_MM2S) {
		*VdmaMM2SCrReg = (FrameCount << 16) | ENABLE_FRAME_COUNT |
			ENABLE_CIRCULAR_MODE | START_VDMA ;
	}
	else if (VdmaChannel==VDMA_S2MM) {
		*VdmaS2MMCrReg = (FrameCount << 16) | ENABLE_FRAME_COUNT |
			ENABLE_CIRCULAR_MODE | START_VDMA ;
	}
}

/*****************************************************************************/
/**
*
* This function to start mm2s or s2mm VdmaChannel - write the vertical size
*
* @param	VdmaChannel specifes VdmaChannel is MM2S or S2MM
* @param	VerticalSize specifes No of lines
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void StartChannel(s32 VdmaChannel, s32 VerticalSize) {
	if (VdmaChannel == VDMA_MM2S) {
		*VdmaMM2SVertSizeReg = VerticalSize;
	}else if (VdmaChannel == VDMA_S2MM) {
		*VdmaS2MMVertSizeReg = VerticalSize;
	}
}

/*****************************************************************************/
/**
*
* This is the main function for XDsiTxSs self test example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the self test example passed.
*		- XST_FAILURE if the self test example was unsuccessful.
*
* @note		None.
*
******************************************************************************/
#ifndef TESTAPP_GEN
s32 main()
{
	u32 Status;

	xil_printf("---------------------------------------\n\r");
	xil_printf("MIPI DSI TX Subsystem Vido test example\n\r");
	xil_printf("----------------------------------------\n\r\n\r");

#if XPAR_MICROBLAZE_USE_ICACHE
	Xil_ICacheInvalidate();
	Xil_ICacheDisable();
#endif

#if XPAR_MICROBLAZE_USE_DCACHE
	Xil_DCacheInvalidate();
	Xil_DCacheDisable();
#endif

	Status = DsiTxSs_VideoTestExample(XDSITXSS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI TX Subsystem Video test example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran MIPI DSI TX Subsystem Video test example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the video test example using the
* XDsiTxSs driver.
*
* @param	DeviceId is the unique device ID of the MIPI DSI RX
*			Subsystem core.
*
* @return
*		- XST_FAILURE if any of MIPI DSI RX Subsystem sub-core self
*		test failed.
*		- XST_SUCCESS, if all of MIPI DSI RX Subsystem sub-core self
*		test passed.
*
* @note		None.
*
******************************************************************************/
u32 DsiTxSs_VideoTestExample(u32 DeviceId)
{
	u32 Status = XST_SUCCESS;
	XDsiTxSs_Config *ConfigPtr;
	s32 ii, jj;
	ULONG PixelIndex;
	ULONG ErrorCount = 0;
	u8 DataValue;
	XDsi_VideoTiming  Timing;

	/* Obtain the device configuration for the MIPI DSI TX Subsystem */
	ConfigPtr = XDsiTxSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the DsiTxSsInst's Config
	 * structure. */
	Status = XDsiTxSs_CfgInitialize(&DsiTxSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI DSI TX SS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Run the self test. */
	Status = XDsiTxSs_SelfTest(&DsiTxSsInst);
	memset((void *)XPAR_BRAM_1_BASEADDR, 0x99,
	(XPAR_BRAM_1_HIGHADDR - XPAR_BRAM_1_BASEADDR));
	memset((void *)XPAR_BRAM_0_BASEADDR, 0x00,
	(XPAR_BRAM_0_HIGHADDR - XPAR_BRAM_0_BASEADDR));

	/*
	 * Fill the buffer with the data value
	 */
	PixelIndex = 0;
	DataValue  = 1;
	for ( ii = 0; ii < VERTICAL_RESOLUTION; ii++ ) {
		for ( jj = 0; jj < HORIZONTAL_RESOLUTION; jj++ ) {
		SrcBuffer[ PixelIndex + 0 ] = (u8) DataValue;
		SrcBuffer[ PixelIndex + 1 ] = (u8) DataValue + 1;
		SrcBuffer[ PixelIndex + 2 ] = (u8) DataValue + 2;
		DataValue += NO_BYTES_PER_PIXEL;
		PixelIndex += NO_BYTES_PER_PIXEL;
		    if(DataValue >= 0xF) {
			DataValue += 1;
		    }
		}
	}

	/* Reset the VMDA */
	ResetVDMA();

	SetupVdmaCR(VDMA_MM2S,FRAME_COUNTER);
	SetupVdmaCR(VDMA_S2MM,FRAME_COUNTER);

	ConfigureChannel(VDMA_MM2S);
	ConfigureChannel(VDMA_S2MM);

	StartChannel(VDMA_S2MM, VERTICAL_RESOLUTION);
	StartChannel(VDMA_MM2S, VERTICAL_RESOLUTION);

	/* Reset the DSI device */
	XDsiTxSs_Reset(&DsiTxSsInst);

	/* Set the Video Peripheral timing parameters */
	Timing.HActive		= HACT_VALUE;
	Timing.HBackPorch	= HBP_VALUE;
	Timing.HFrontPorch	= HFP_VALUE;
	Timing.HSyncWidth	= HSA_VALUE;
	Timing.VActive		= VACT_VALUE;
	Timing.VBackPorch	= VBP_VALUE;
	Timing.VFrontPorch	= VFP_VALUE;
	Timing.VSyncWidth	= VSA_VALUE;
	Timing.BLLPBurst	= BLLP_BURST_VALUE;

	XDsiTxSs_SetCustomVideoInterfaceTiming(&DsiTxSsInst,
			XDSI_VM_NON_BURST_SYNC_EVENT, &Timing);

	/* Enable DSI core */
	XDsiTxSs_Activate(&DsiTxSsInst, XDSITXSS_ENABLE);

	/* Wait for S2MM transfer completion */
	Status = WaitForCompletion(VDMA_S2MM, (u32*)VDMA_BASEADDR);
	if (Status != S2MM_HALT_SUCCESS) {
		 xil_printf("S2MM channel has not halted\r\n");
		 return XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}
	/* Wait for MM2SS transfer completion */
	Status = WaitForCompletion(VDMA_MM2S, (u32*)VDMA_BASEADDR);
	if (Status != MM2S_HALT_SUCCESS) {
		xil_printf("MM2S channel has not halted\r\n");
		return XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}
	/* Compare source and destination buffers */
	PixelIndex = 0;
	ErrorCount = 0;
	for ( ii = 0; ii < VERTICAL_RESOLUTION; ii++ ) {
	    for ( jj = 0; jj < HORIZONTAL_RESOLUTION; jj++) {
		if((SrcBuffer[ PixelIndex + 0 ] == DstBuffer[ PixelIndex + 0 ])
		&& (SrcBuffer[ PixelIndex + 1 ] == DstBuffer[ PixelIndex + 2 ])
		&& (SrcBuffer[ PixelIndex + 2 ] == DstBuffer[ PixelIndex + 1 ]))
		{
			PixelIndex = PixelIndex + 3;
		}
		else {
			ErrorCount++;
			break;
		}
	    }
	    if(ErrorCount > 0) {
		break;
	    }
	}
	if(ErrorCount > 0) {
		xil_printf("Source pattern does not match with "
		"destination pattern ... \r\n");
		return XST_FAILURE;
	}
	xil_printf("Source pattern match with destination pattern ... \r\n");
	return Status;
}
