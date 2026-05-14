/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file mmi_dc_live_test.c
 * @brief Implements live display pipeline setup and execution helpers.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <sleep.h>
#include <xdc.h>
#include <xdcdma.h>
#include <xdcsub.h>
#include <xil_cache.h>
#include <xil_exception.h>
#include <xil_io.h>
#include <xil_printf.h>
#include <xil_types.h>
#include <xpseudo_asm_gcc.h>
#include <xstatus.h>
#include <xvidc.h>

#include "mmi_dc_live_test.h"
#include "mmi_dc_partial_plane.h"
#include "mmi_dc_sdp.h"
#include "mmi_dp_init.h"
#include "mmi_dpdc_example.h"
#include "mmi_dpdc_platform.h"

#include "xclk_wiz.h"
#include "xvtc.h"
#include "xavpg.h"

XVtc Vtc0;
XClk_Wiz ClkWiz0;

void LiveAvpgConfig(RunConfig *RunCfgPtr) {
	UINTPTR AvpgBaseAddr[2] = {AVPG_0_BASEADDR, AVPG_1_BASEADDR};
	u32 idx;

	for (idx = 0; idx < 2; idx++)
		XAvpgSetConfig(AvpgBaseAddr[idx], &RunCfgPtr->avpg[idx],
				RunCfgPtr->Width, RunCfgPtr->Height);
}

void InitLiveDcSubPtr(RunConfig *RunCfgPtr)
{
	XDc *DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	const XVidC_VideoTimingMode *Vtm;

	/* Select CSC coefficients based on input/output color space */
	const XDc_VideoAttribute *Vid1 =
		XDc_GetLiveVideoAttribute(RunCfgPtr->Stream1Format);
	const XDc_VideoAttribute *Vid2 =
		XDc_GetLiveVideoAttribute(RunCfgPtr->Stream2Format);
	u32 *In1Coeff, *In1Offset, *In2Coeff, *In2Offset;
	/* Output CSC: RGB output uses identity, YUV output uses RGB-to-YUV */
	XDc_VideoAttribute *OutVid = XDc_GetOutputVideoAttribute(
			RunCfgPtr->OutStreamFormat, XDC_VID_FUNCTIONAL);
	u32 *OutCoeff, *OutOffset;
	u8 global_alpha;

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
	}

	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfgPtr->DcSubPtr, XDC_VIDSTREAM1_LIVE,
			XDC_VIDSTREAM2_LIVE);
	XDcSub_SetInputAudioSelect(RunCfgPtr->DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputLiveStreamFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->Stream1Format,
			RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(RunCfgPtr->DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfgPtr->DcSubPtr, 0, 0);

	/* Set Global Alpha based on partial plane blend mode
	 * Stream1 partial plane (Stream1=blend, Stream2=source): Alpha = 0x0
	 * (transparent) Stream2 partial plane (Stream2=blend, Stream1=source): Alpha
	 * = 0xFF (opaque) No partial plane: Alpha = 0xA5 (semi-transparent for normal
	 * blending)
	 */
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

	XDcSub_SetInputStreamCSC(RunCfgPtr->DcSubPtr, In1Coeff, In1Offset, In2Coeff,
			In2Offset);

	XDcSub_SetOutputStreamCSC(RunCfgPtr->DcSubPtr, OutCoeff, OutOffset);

	XDcSub_SetAudioVideoClkSrc(RunCfgPtr->DcSubPtr);

	/* TODO Remove this */
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);

	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, 0x3F);
#ifndef NL_LATENCY
#define NL_LATENCY			0x0138
#endif

#ifndef V_LINE_LATENCY
#define V_LINE_LATENCY			0x2
#endif
	XDcSub_SetNonLiveLatency(RunCfgPtr->DcSubPtr, NL_LATENCY, V_LINE_LATENCY);
#undef V_LINE_LATENCY
#undef NL_LATENCY

	/* Initialize partial plane blend if enabled (BEFORE hardware init) */
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE ||
			RunCfgPtr->Stream2PbEnable == PB_ENABLE)
		XDpDc_ConfigurePartialPlaneBlend(RunCfgPtr);

}

void InitLiveRunConfig(RunConfig *RunCfgPtr)
{

	RunCfgPtr->DcSubPtr = &DcSub;
	RunCfgPtr->DcSubPtr->DcPtr = &Dc;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;

	RunCfgPtr->PPC = 2;

	if (RunCfgPtr->avpg[0].pix_fmt == 0) {
		switch (RunCfgPtr->avpg[0].bpc) {
			case 8:
				RunCfgPtr->Stream1Format = RGB_8BPC;
				break;
			case 10:
				RunCfgPtr->Stream1Format = RGB_10BPC;
				break;
			case 12:
				RunCfgPtr->Stream1Format = RGB_12BPC;
				break;
		}
	}

	if (RunCfgPtr->avpg[0].pix_fmt == 1) {
		switch (RunCfgPtr->avpg[0].bpc) {
			case 8:
				RunCfgPtr->Stream1Format = YCbCr422_8BPC;
				break;
			case 10:
				RunCfgPtr->Stream1Format = YCbCr422_10BPC;
				break;
			case 12:
				RunCfgPtr->Stream1Format = YCbCr422_12BPC;
				break;
		}
	}

	if (RunCfgPtr->avpg[1].pix_fmt == 0) {
		switch (RunCfgPtr->avpg[1].bpc) {
			case 8:
				RunCfgPtr->Stream2Format = RGB_8BPC;
				break;
			case 10:
				RunCfgPtr->Stream2Format = RGB_10BPC;
				break;
			case 12:
				RunCfgPtr->Stream2Format = RGB_12BPC;
				break;
		}
	}

	if (RunCfgPtr->avpg[1].pix_fmt == 1) {
		switch (RunCfgPtr->avpg[1].bpc) {
			case 8:
				RunCfgPtr->Stream2Format = YCbCr422_8BPC;
				break;
			case 10:
				RunCfgPtr->Stream2Format = YCbCr422_10BPC;
				break;
			case 12:
				RunCfgPtr->Stream2Format = YCbCr422_12BPC;
				break;
		}
	}
}

u32 InitVtcSubsystem(RunConfig *RunCfgPtr, XVtc *Vtc, UINTPTR Addr)
{
	XVtc_Config CfgPtr;
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;
	XVtc_Signal Signal;
	XVtc_HoriOffsets Hoff;
	const XVidC_VideoTimingMode *VidTimingMode;
	const XVidC_VideoTiming *VidTiming;

	u32 PixPerClk = RunCfgPtr->PPC;

	XVtc_CfgInitialize(Vtc, &CfgPtr, Addr);

	/* Disable Generator */
	XVtc_Reset(Vtc);
	XVtc_DisableGenerator(Vtc);
	XVtc_Disable(Vtc);

	/* Set up source select */
	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

	/* 1 = Generator registers, 0 = Detector registers */
	SourceSelect.VChromaSrc = 1;
	SourceSelect.VActiveSrc = 1;
	SourceSelect.VBackPorchSrc = 1;
	SourceSelect.VSyncSrc = 1;
	SourceSelect.VFrontPorchSrc = 1;
	SourceSelect.VTotalSrc = 1;
	SourceSelect.HActiveSrc = 1;
	SourceSelect.HBackPorchSrc = 1;
	SourceSelect.HSyncSrc = 1;
	SourceSelect.HFrontPorchSrc = 1;
	SourceSelect.HTotalSrc = 1;

	XVtc_SetSource(Vtc, &SourceSelect);
	VidTimingMode = XVidC_GetVideoModeData(RunCfgPtr->VideoMode);
	VidTiming = &VidTimingMode->Timing;

	VideoTiming.HActiveVideo = VidTiming->HActive;
	VideoTiming.HFrontPorch = VidTiming->HFrontPorch;
	VideoTiming.HSyncWidth = VidTiming->HSyncWidth;
	VideoTiming.HBackPorch = VidTiming->HBackPorch;
	VideoTiming.HSyncPolarity = VidTiming->HSyncPolarity;

	VideoTiming.VActiveVideo = VidTiming->VActive;
	VideoTiming.V0FrontPorch = VidTiming->F0PVFrontPorch;
	VideoTiming.V0SyncWidth = VidTiming->F0PVSyncWidth;
	VideoTiming.V0BackPorch = VidTiming->F0PVBackPorch;

	VideoTiming.V1FrontPorch = VidTiming->F1VFrontPorch;
	VideoTiming.V1SyncWidth = VidTiming->F1VSyncWidth;
	VideoTiming.V1BackPorch = VidTiming->F1VBackPorch;

	VideoTiming.VSyncPolarity = VidTiming->VSyncPolarity;


	if (PixPerClk == RunCfgPtr->PPC) {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo / RunCfgPtr->PPC;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch / RunCfgPtr->PPC;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch / RunCfgPtr->PPC;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth / RunCfgPtr->PPC;
	}

	XVtc_ConvTiming2Signal(Vtc, &VideoTiming, &Signal, &Hoff, &Polarity);

	/* Setup VTC Polarity Config */
	memset((void *)&Polarity, 0, sizeof(Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol  = 1;
	Polarity.VBlankPol       = VideoTiming.VSyncPolarity;
	Polarity.VSyncPol        = VideoTiming.VSyncPolarity;
	Polarity.HBlankPol       = VideoTiming.HSyncPolarity;
	Polarity.HSyncPol        = VideoTiming.HSyncPolarity;

	Polarity.VBlankPol       = 1;
	Polarity.VSyncPol        = 1;
	Polarity.HBlankPol       = 1;
	Polarity.HSyncPol        = 1;

	XVtc_SetPolarity(Vtc, &Polarity);
	XVtc_SetGenerator(Vtc, &Signal);
	XVtc_SetGeneratorHoriOffset(Vtc, &Hoff);

	XVtc_Enable(Vtc);
	XVtc_EnableGenerator(Vtc);
	XVtc_RegUpdateEnable(Vtc);

	return XST_SUCCESS;
}

void InitLiveDcSubsystem(RunConfig *RunCfgPtr)
{
	XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
	XDc *DcPtr = DcSubPtr->DcPtr;

	XDcSub_Initialize(DcSubPtr);
	XDc_VidClkSelect(DcPtr);
	XDc_VideoSoftReset(DcPtr);
	XDc_SetVidFrameSwitch(DcPtr);
	XDc_SetVideoTiming(DcPtr);
	XDcSub_ConfigureDcVideo(DcPtr);
}

u32 XDpDc_MmiDcLiveTest(RunConfig *RunCfgPtr)
{
	u32 Status;

	if (RunCfgPtr->operatingmode != XDCSUB_OPMODE_FUNCTIONAL &&
			RunCfgPtr->presentationmode != XDCSUB_PPTMODE_LIVE) {
		xil_printf("%s: Invalid config\r\n", __func__);
		return XST_INVALID_PARAM;
	}

	InitLiveRunConfig(RunCfgPtr);

	InitLiveDcSubPtr(RunCfgPtr);

	XDpDc_InitClkWiz(RunCfgPtr);

	InitLiveDcSubsystem(RunCfgPtr);

	XDc_WriteReg(LIVE_IN_GPIO_BASEADDR, 0x0, 0x00010000);

	InitVtcSubsystem(RunCfgPtr, &Vtc0, VTC_BASEADDR);

	XVtc_WriteReg(VTC_BASEADDR, 0x0, 0x03f7ef06);

	LiveAvpgConfig(RunCfgPtr);
	XAvpg_WriteReg(AVPG_0_BASEADDR, 0x0, 0x1);
	XAvpg_WriteReg(AVPG_1_BASEADDR, 0x0, 0x1);

	XDc_WriteReg(LIVE_IN_GPIO_BASEADDR, 0x0, 0x00010001);

	xil_printf("  Enabling DisplayPort output...\r\n");
	/* Initialize DpSubsystem */
	Status = XDpDc_InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: DisplayPort subsystem initialization failed\r\n");
		return Status;
	}

	XDpDc_SetupInterrupts(RunCfgPtr);

	return XST_SUCCESS;
}
