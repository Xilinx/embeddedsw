/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_sdp.c
*
* This file contains SDP packet generation and DMA configuration
*
******************************************************************************/

#include "mmi_dc_sdp.h"
#include "mmi_dc_setup_frames.h"
#include "xil_printf.h"

#define XDPDC_SDP_CHANNELS		0x8U
#define XDPDC_SDP_CHANNEL_ALLOC		0x13U
#define XDPDC_SDP_SAMPLE_SIZE_16BIT	0x1U
#define XDPDC_SDP_AUDIO_FREQ_48K	0x3U

/*****************************************************************************/
/**
*
* This function initializes the SDP frame buffer metadata.
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     SDP data is generated in memory at XDPDC_SDP_BUFFER_ADDR
*
******************************************************************************/
void XDpDc_InitSdpFrameBuffer(RunConfig *RunCfgPtr)
{
	FrameInfo *Sdp_FbInfo;

	Sdp_FbInfo = RunCfgPtr->Sdp_FbInfo;
	Sdp_FbInfo->Address = XDPDC_SDP_BUFFER_ADDR;
	Sdp_FbInfo->Size = XDPDC_SDP_TOTAL_BYTES;
	Sdp_FbInfo->LineSize = 0U;
	Sdp_FbInfo->Stride = 0U;
}

/*****************************************************************************/
/**
*
* This function generates SDP packets into the SDP memory buffer.
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     RunCfgPtr is unused and kept for API symmetry
*
******************************************************************************/
void XDpDc_FillSdpBuffer(RunConfig *RunCfgPtr)
{
	u32 BufAddr = XDPDC_SDP_BUFFER_ADDR;
	u32 Index;
	u8 Channel = XDPDC_SDP_CHANNELS;
	u8 CA = XDPDC_SDP_CHANNEL_ALLOC;
	u8 SampleSize = XDPDC_SDP_SAMPLE_SIZE_16BIT;
	u8 AudFreqSel = XDPDC_SDP_AUDIO_FREQ_48K;

	(void)RunCfgPtr;

	for (Index = 0U; Index < XDPDC_SDP_PACKET_COUNT; Index++) {
		XDc_WriteReg(BufAddr, 0x0, 0x4B1B8400);
		XDc_WriteReg(BufAddr, 0x4, (0x00000010U |
					    ((u32)CA << 24) |
					    ((u32)AudFreqSel << 10) |
					    ((u32)SampleSize << 8) |
					    ((u32)Channel - 1U)));
		XDc_WriteReg(BufAddr, 0x8, 0x0);
		XDc_WriteReg(BufAddr, 0xc, 0x0);
		XDc_WriteReg(BufAddr, 0x10, 0x0);
		XDc_WriteReg(BufAddr, 0x14, 0x0);
		XDc_WriteReg(BufAddr, 0x18, 0x0);
		XDc_WriteReg(BufAddr, 0x1c, 0x0);
		XDc_WriteReg(BufAddr, 0x20, 0x0);

		BufAddr += XDPDC_SDP_BUFFER_OFFSET;
	}

	xil_printf("  - SDP packet buffer generated (%d packets)\r\n",
		   XDPDC_SDP_PACKET_COUNT);
}

/*****************************************************************************/
/**
*
* This function configures SDP controls in DC subsystem.
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Must be called before XDcSub_ConfigureDcVideo()
*
******************************************************************************/
void XDpDc_ConfigureSdp(RunConfig *RunCfgPtr)
{
	XDcSub_EnableSdp(RunCfgPtr->DcSubPtr);
	XDcSub_SetSdp(RunCfgPtr->DcSubPtr, SDP_DMA);
	XDcSub_SetSdpCursorBuffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_SetSdpAckSel(RunCfgPtr->DcSubPtr, XDC_SDP_DP_ACK);
	XDcSub_SetCursorSdpRdyInterval(RunCfgPtr->DcSubPtr, 4);

	xil_printf("  - SDP configured: source=DMA, ack=DP_ACK, interval=4\r\n");
}

/*****************************************************************************/
/**
*
* This function creates the SDP DMA descriptor (Desc7) and links it to
* the SDP packet buffer.
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Must be called after XDcDma_CfgInitialize()
*
******************************************************************************/
void XDpDc_SetupSdpDescriptor(RunConfig *RunCfgPtr)
{
	XDpDc_CreateDescriptors(RunCfgPtr, RunCfgPtr->Desc7, RunCfgPtr->Sdp_FbInfo, NULL);
	xil_printf("[DESC] SDP: Desc7 @ 0x%08x -> SDP buf 0x%08x\r\n",
		   (u32)(UINTPTR)RunCfgPtr->Desc7, (u32)RunCfgPtr->Sdp_FbInfo->Address);
}

/*****************************************************************************/
/**
*
* This function configures SDP DMA channel trigger state.
*
* @param    RunCfgPtr - Pointer to RunConfig structure
*
* @return   None
*
* @note     Desc7 must be initialized before calling this API
*
******************************************************************************/
void XDpDc_ConfigureSdpDMA(RunConfig *RunCfgPtr)
{
	XDcDma *DmaPtr = RunCfgPtr->DcSubPtr->DmaPtr;

	DmaPtr->SDP.Channel.Current = RunCfgPtr->Desc7;
	DmaPtr->SDP.SDP_TriggerStatus = XDCDMA_TRIGGER_EN;

	xil_printf("  - SDP DMA configured\r\n");
}
