/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dc_nonlive_test.c
* @brief Implements non-live display pipeline setup and execution helpers.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <xil_exception.h>
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_types.h>
#include <sleep.h>

#include "mmi_dc_nonlive_test.h"
#include "mmi_dc_setup_frames.h"
#include "mmi_dp_init.h"
#include "mmi_dpdc_platform.h"
#include "mmi_dc_cursor.h"
#include "mmi_dc_partial_plane.h"
#include "mmi_dc_sdp.h"

/************************** Constant Definitions *****************************/

#define CLK_LOCK			1
#define PL_AUD_CLK_MULT			512
#define NL_LATENCY			0x0138
#define V_LINE_LATENCY			0x2
#define XDCDMA_IEN_VSYNC_INT_MASK	0x0000000C
#define XDCDMA_INTR_ID			179
#define XDCDMA_INTR_PARENT		0xe2000000

RunConfig RunCfg;
XDcSub DcSub;
XDcDma DcDma;
XDc Dc;

XMmiDp DpPsuPtr;
FrameInfo Video1FbPtr;
FrameInfo Video2FbPtr;
FrameInfo Video3FbPtr;
FrameInfo Video4FbPtr;
FrameInfo Video5FbPtr;
FrameInfo Video6FbPtr;
FrameInfo CursorFbPtr;
FrameInfo SdpFbPtr;

XDcDma_Descriptor *Desc1 = (XDcDma_Descriptor *)0x33500;
XDcDma_Descriptor *Desc2 = (XDcDma_Descriptor *)0x33600;
XDcDma_Descriptor *Desc3 = (XDcDma_Descriptor *)0x33700;
XDcDma_Descriptor *Desc4 = (XDcDma_Descriptor *)0x33800;
XDcDma_Descriptor *Desc5 = (XDcDma_Descriptor *)0x33900;
XDcDma_Descriptor *Desc6 = (XDcDma_Descriptor *)0x33A00;
XDcDma_Descriptor *Desc7 = (XDcDma_Descriptor *)0x33B00;

XDcDma_Descriptor *AudDesc0 = (XDcDma_Descriptor *)0x08000000;
XDcDma_Descriptor *AudDesc1 = (XDcDma_Descriptor *)0x08000100;
XDcDma_Descriptor *AudDesc2 = (XDcDma_Descriptor *)0x08000200;
XDcDma_Descriptor *AudDesc3 = (XDcDma_Descriptor *)0x08000300;
XDcDma_Descriptor *AudDesc4 = (XDcDma_Descriptor *)0x08000400;
XDcDma_Descriptor *AudDesc5 = (XDcDma_Descriptor *)0x08000500;
XDcDma_Descriptor *AudDesc6 = (XDcDma_Descriptor *)0x08000600;
XDcDma_Descriptor *AudDesc7 = (XDcDma_Descriptor *)0x08000700;

/* SDTV CSC Coefficients */
u32 CSCCoeff_RGB[] = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000 };
u32 CSCOffset_RGB[] = { 0x0000, 0x0000, 0x0000 };

/* YUV-to-RGB input conversion (SDTV BT.601) */
u32 In_CSCCoeff_YUV[] = { 0x1000, 0x0000, 0x166F, 0x1000, 0x7A7F, 0x7493, 0x1000, 0x1C5A, 0x0000 };
u32 In_CSCOffset_YUV[] = { 0x0000, 0x1800, 0x1800 };

/* RGB-to-YUV output conversion (SDTV BT.601) */
u32 Out_CSCCoeff_YUV[] = { 0x04C8, 0x0964, 0x01D3, 0x7D4C, 0x7AB4, 0x0800, 0x0800, 0x7945, 0x7EB5 };
u32 Out_CSCOffset_YUV[] = { 0x0000, 0x0800, 0x0800 };

XDc_VideoTiming VidTiming_640_480 = { .HTotal = 800, .HSWidth = 96, .HRes = 640, .HStart = 144, .VTotal = 525, .VSWidth = 2, .VRes = 480, .VStart = 35};

/******************************************************************************/
/**
 * This function configures the DC subsystem pointer with video timing,
 * stream sources, input/output formats, CSC coefficients, global alpha,
 * audio settings, cursor blend, and partial plane blend parameters.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	XST_SUCCESS if audio/video settings are valid, otherwise XST_FAILURE.
 *
 * @note	Cursor blend and partial plane blend are configured here

 *		before hardware init (XDcSub_ConfigureDcVideo).
 *
*******************************************************************************/
u32 XDpDc_InitDcSubPtr(RunConfig *RunCfgPtr)
{
	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	const XVidC_VideoTimingMode *Vtm;

	DcPtr->Config.BaseAddr = DC_BASEADDR;

	Vtm = XVidC_GetVideoModeData(RunCfgPtr->VideoMode);
	if (Vtm) {
		DcPtr->VideoTiming.HTotal  = Vtm->Timing.HTotal;
		DcPtr->VideoTiming.HSWidth = Vtm->Timing.HSyncWidth;
		DcPtr->VideoTiming.HRes    = Vtm->Timing.HActive;
		DcPtr->VideoTiming.HStart  = Vtm->Timing.HSyncWidth +
					     Vtm->Timing.HBackPorch;
		DcPtr->VideoTiming.VTotal  = Vtm->Timing.F0PVTotal;
		DcPtr->VideoTiming.VSWidth = Vtm->Timing.F0PVSyncWidth;
		DcPtr->VideoTiming.VRes    = Vtm->Timing.VActive;
		DcPtr->VideoTiming.VStart  = Vtm->Timing.F0PVSyncWidth +
					     Vtm->Timing.F0PVBackPorch;
	} else {
		xil_printf("WARNING: No timing data for VideoMode %d, "
			   "using 640x480 fallback\r\n", RunCfgPtr->VideoMode);
		DcPtr->VideoTiming = VidTiming_640_480;
	}

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_NONLIVE, XDC_VIDSTREAM2_NONLIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputNonLiveVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
					  RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	/* 0=blend Stream1&2, 1=Stream2 only */
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);

	/* Set Global Alpha based on partial plane blend mode
	 * Stream1 partial plane (Stream1=blend, Stream2=source): Alpha = 0x0 (transparent)
	 * Stream2 partial plane (Stream2=blend, Stream1=source): Alpha = 0xFF (opaque)
	 * No partial plane: Alpha = 0xA5 (semi-transparent for normal blending)
	 */
	u8 global_alpha;
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE) {
		/* Stream1 as blend plane on Stream2 source */
		global_alpha = 0x0;
	} else if (RunCfgPtr->Stream2PbEnable == PB_ENABLE) {
		/* Stream2 as blend plane on Stream1 source */
		global_alpha = 0xFF;
	} else {
		/* Normal blending mode */
		global_alpha = 0xA5;
	}
	XDcSub_SetGlobalAlpha(RunCfgPtr->DcSubPtr, ALPHA_ENABLE, global_alpha);
	XDcSub_SetStreamPixelScaling(RunCfgPtr->DcSubPtr, NULL, NULL);

	/* Select CSC coefficients based on input/output color space */
	const XDc_VideoAttribute *Vid1 = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream1Format);
	const XDc_VideoAttribute *Vid2 = XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);

	u32 *In1Coeff, *In1Offset, *In2Coeff, *In2Offset;
	u32 *OutCoeff, *OutOffset;

	if (Vid1->IsRGB == TRUE) {
		In1Coeff = CSCCoeff_RGB;
		In1Offset = CSCOffset_RGB;
	} else {
		In1Coeff = In_CSCCoeff_YUV;
		In1Offset = In_CSCOffset_YUV;
	}

	if (Vid2->IsRGB == TRUE) {
		In2Coeff = CSCCoeff_RGB;
		In2Offset = CSCOffset_RGB;
	} else {
		In2Coeff = In_CSCCoeff_YUV;
		In2Offset = In_CSCOffset_YUV;
	}

	/* Output CSC: RGB output uses identity, YUV output uses RGB-to-YUV */
	XDc_VideoAttribute *OutVid = XDc_GetOutputVideoAttribute(RunCfgPtr->OutStreamFormat, XDC_VID_FUNCTIONAL);
	if (OutVid != NULL && OutVid->IsRGB == TRUE) {
		OutCoeff = CSCCoeff_RGB;
		OutOffset = CSCOffset_RGB;
	} else {
		OutCoeff = Out_CSCCoeff_YUV;
		OutOffset = Out_CSCOffset_YUV;
	}

	xil_printf("[CSC] Stream1: %s, Stream2: %s, Output: %s\r\n",
		   Vid1->IsRGB ? "RGB (identity)" : "YUV->RGB",
		   Vid2->IsRGB ? "RGB (identity)" : "YUV->RGB",
		   (OutVid != NULL && OutVid->IsRGB) ? "RGB (identity)" : "RGB->YUV");

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, In1Coeff, In1Offset, In2Coeff, In2Offset);
	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, OutCoeff, OutOffset);
	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_EnableStream1Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_EnableStream2Buffers(RunCfgPtr->DcSubPtr, 1, 15);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x30);
	XDcSub_SetNonLiveLatency(RunCfgPtr->DcSubPtr, NL_LATENCY, V_LINE_LATENCY);

	if(RunCfgPtr->AudioEnable)
	{
		u8 AudioChannels = RunCfgPtr->AudioChannels;
		u32 Status;

		XDcSub_SetAudInterfaceMode(RunCfgPtr->DcSubPtr,XDC_AUD_FUNCTIONAL);
		XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONLIVE);
		XDcSub_EnableAudio(RunCfgPtr->DcSubPtr);
		XDcSub_AudClkSelect(RunCfgPtr->DcSubPtr,0);

		XDcSub_AudioChannelSelect(RunCfgPtr->DcSubPtr, 7, 0x1FF);
		XDcSub_EnableAudioBuffer(RunCfgPtr->DcSubPtr, 1, 15);

		/* Set the audio channels count value */
		Status = XDcSub_SetAudioChCtrl(RunCfgPtr->DcSubPtr, AudioChannels);
		if (Status != XST_SUCCESS) {
			xil_printf("ERROR: Failed to program audio channel count (status=0x%X)\r\n",
				   Status);
			return Status;
		}
	}

	/* Initialize SDP if enabled (BEFORE hardware init) */
	if (RunCfgPtr->SdpEnable) {
		XDpDc_ConfigureSdp(RunCfgPtr);
	}

	/* Initialize cursor blend if enabled (BEFORE hardware init) */
	if (RunCfgPtr->CursorEnable == CB_ENABLE) {
		XDpDc_ConfigureCursorBlend(RunCfgPtr);
	}

	/* Initialize partial plane blend if enabled (BEFORE hardware init) */
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE || RunCfgPtr->Stream2PbEnable == PB_ENABLE) {
		XDpDc_ConfigurePartialPlaneBlend(RunCfgPtr);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the application configuration by assigning
* DC subsystem, DMA, DP, and frame buffer pointers, then initializing
* frames and configuring the DC subsystem.
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, else XST_FAILURE.
*

* @note         User-configurable parameters (resolution, format, cursor, audio)
*               are set from the menu in main() before this function is called.
*
*****************************************************************************/
u32 XDpDc_InitializeRunConfig(RunConfig *RunCfgPtr)
{
	u32 Status = XST_SUCCESS;

	/* All user configurable params */
	RunCfgPtr->DcSubPtr = &DcSub;
	RunCfgPtr->DcSubPtr->DcPtr = &Dc;
	RunCfgPtr->DcSubPtr->DmaPtr = &DcDma;
	RunCfgPtr->V1_FbInfo = &Video1FbPtr;
	RunCfgPtr->V2_FbInfo = &Video2FbPtr;
	RunCfgPtr->V3_FbInfo = &Video3FbPtr;
	RunCfgPtr->V4_FbInfo = &Video4FbPtr;
	RunCfgPtr->V5_FbInfo = &Video5FbPtr;
	RunCfgPtr->V6_FbInfo = &Video6FbPtr;
	RunCfgPtr->Cursor_FbInfo = &CursorFbPtr;
	RunCfgPtr->Sdp_FbInfo = &SdpFbPtr;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;
	RunCfgPtr->Desc1 = Desc1;
	RunCfgPtr->Desc2 = Desc2;
	RunCfgPtr->Desc3 = Desc3;
	RunCfgPtr->Desc4 = Desc4;
	RunCfgPtr->Desc5 = Desc5;
	RunCfgPtr->Desc6 = Desc6;
	RunCfgPtr->Desc7 = Desc7;

	RunCfgPtr->DcBaseAddr = DC_BASEADDR;
	RunCfgPtr->DcDmaBaseAddr = DCDMA_BASEADDR;

	XDpDc_InitFrames(RunCfgPtr);
	Status = XDpDc_InitDcSubPtr(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: Invalid DC/Audio configuration\r\n");
		return Status;
	}

	return Status;

}

/*****************************************************************************/
/**
*
* The purpose of this function is to configure the test params and the platform
* driver & Xdc driver to set DC Mode
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, else XST_FAILURE.
*
* @note         None.
*
*****************************************************************************/
u32 XDpDc_MmiDcNonliveTest(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;

	/* Initialize the application configuration */
	xil_printf("\r\n========================================\r\n");
	xil_printf("DisplayPort DC Non-Live Video Test\r\n");
	xil_printf("========================================\r\n\r\n");
	xil_printf("[1/2] Initializing configuration...\r\n");
	Status = XDpDc_InitializeRunConfig(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to get test configuration\r\n");
		return Status;
	}
	xil_printf("  Configuration initialized successfully\r\n\r\n");

	/* Platform Initialization */
	xil_printf("[2/2] Initializing platform...\r\n");
	Status = XDpDc_InitPlatform(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize platform\r\n");
		return Status;
	}
	xil_printf("  Platform initialized successfully\r\n\r\n");

	xil_printf("========================================\r\n");
	xil_printf("Initialization Complete\r\n");
	xil_printf("========================================\r\n\r\n");

	return Status;

}
