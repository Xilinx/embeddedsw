/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file mmi_dc_bypass_test.c
 *
 * This file implements the DC Bypass SST mode test. In bypass mode the DC
 * block is bypassed and video from AVPG in PL is routed directly to DP TX.
 *
 * Architecture is extensible to MST by iterating over additional streams
 * (indexed by stream number, with per-stream AVPG/VTC/PPC from xparameters),
 * but only SST (Stream 0) is enabled in this implementation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   07/15/26  Initial release
 * 1.1   ck   07/29/26  Add alpha/PM bypass GPIO path from reference
 * </pre>
 *
 *****************************************************************************/



#include <xil_assert.h>
#include <xil_printf.h>
#include <xvidc.h>
#include <xparameters.h>
#include "mmi_dc_bypass_test.h"

/** Pixels per clock per bypass stream, from the XSA. */
static const u8 BypassStreamPpc[BYPASS_MAX_STREAMS] = {
	XPAR_XDCSUB_0_DC_STREAM0_PIXEL_MODE,
	XPAR_XDCSUB_0_DC_STREAM1_PIXEL_MODE,
	XPAR_XDCSUB_0_DC_STREAM2_PIXEL_MODE,
	XPAR_XDCSUB_0_DC_STREAM3_PIXEL_MODE,
};

/*****************************************************************************/
/**
 * Return per-stream PPC from xparameters.h (XSA-fixed; ignores menu).
 *
 * @param	StreamIdx is the 0-based bypass stream index.
 *
 * @return	PPC (1, 2, or 4) for that stream.
 *
 *****************************************************************************/
u8 XDpDc_BypassPpcFromXsa(u8 StreamIdx)
{
	Xil_AssertNonvoid(StreamIdx < BYPASS_MAX_STREAMS);

	return BypassStreamPpc[StreamIdx];
}

#if defined(XPAR_XVTC_NUM_INSTANCES)
#include "mmi_dpdc_platform.h"
#include "mmi_dp_init.h"
#include "mmi_dp_intr.h"

#include "xavpg.h"
#include "xvtc.h"

extern XDcSub DcSub;
extern XDcDma DcDma;
extern XDc Dc;
extern XMmiDp DpPsuPtr;

static XVtc BypassVtc0;

/* Fallback video timing for 1920x1080@60 */
static XDc_VideoTiming VidTiming_1920_1080 = {
	.HTotal = 2200, .HSWidth = 44, .HRes = 1920, .HStart = 192,
	.VTotal = 1125, .VSWidth = 5, .VRes = 1080, .VStart = 41
};

/** PL AVPG base address for bypass stream 0. */
static const UINTPTR BypassAvpgBase[] = {
	BYPASS_AVPG_0_BASEADDR,
};

/*****************************************************************************/
/**
 * Map XSA PPC to AVPG driver dual/quad flag (AvpgRunConfig.ppc).
 *
 *****************************************************************************/
static u8 XDpDc_BypassAvpgPpcFlag(u8 PhysPpc)
{
	return (PhysPpc == 4U) ? 1U : 0U;
}

/*****************************************************************************/
/**
 * Program AVPG; for single-PPC (1) designs fix HRES and pattern PPC field.
 *
 *****************************************************************************/
static void XDpDc_BypassProgramAvpg(UINTPTR AvpgBase, AvpgRunConfig *AvpgCfg,
				    u32 Width, u32 Height, u8 PhysPpc)
{
	XAvpgSetConfig(AvpgBase, AvpgCfg, Width, Height);

	if (PhysPpc == 1U) {
		XAvpg_WriteReg(AvpgBase, XAV_PATGEN_HRES, Width);
		XAvpg_WriteReg(AvpgBase, XAV_PATGEN_MODE_PATTERN, AvpgCfg->pattern);
	}
}

/*****************************************************************************/
/**
 * Return non-zero when alpha/PM bypass GPIO blocks are present in the design.
 *
 *****************************************************************************/
static int XDpDc_BypassAlphaGpioPresent(void)
{
	return (BYPASS_ALPHA_EN_BASEADDR != 0U) &&
	       (BYPASS_ALPHA_GPIO_BASEADDR != 0U) &&
	       (BYPASS_LV_AVTPG_PM_BASEADDR != 0U) &&
	       (BYPASS_LIVE_IN_GPIO_BASEADDR != 0U);
}

/*****************************************************************************/
/**
 * Hold bypass data path and program alpha/PM GPIO (reference InitBypassMode).
 *
 *****************************************************************************/
static void XDpDc_BypassInitAlphaGpioPath(void)
{
	if (!XDpDc_BypassAlphaGpioPresent())
		return;

	XDc_WriteReg(BYPASS_ALPHA_EN_BASEADDR, BYPASS_GPIO_DATA_OFFSET,
		     BYPASS_GPIO_HOLD);
	XDc_WriteReg(BYPASS_LIVE_IN_GPIO_BASEADDR, BYPASS_GPIO_DATA_OFFSET,
		     BYPASS_GPIO_HOLD);
	XDc_WriteReg(BYPASS_LV_AVTPG_PM_BASEADDR, BYPASS_GPIO_DATA_OFFSET,
		     BYPASS_LV_AVTPG_PM_VAL);
	XDc_WriteReg(BYPASS_ALPHA_GPIO_BASEADDR, BYPASS_GPIO_TRI_OFFSET,
		     BYPASS_ALPHA_GPIO_TRI_VAL);
}

/*****************************************************************************/
/**
 * Release live input and finalize alpha bypass GPIO (reference EnableAvTpg).
 *
 *****************************************************************************/
static void XDpDc_BypassEnableAlphaGpioPath(void)
{
	if (!XDpDc_BypassAlphaGpioPresent())
		return;

	XDc_WriteReg(BYPASS_LIVE_IN_GPIO_BASEADDR, BYPASS_GPIO_DATA_OFFSET,
		     BYPASS_GPIO_RELEASE);
	XDc_WriteReg(BYPASS_ALPHA_EN_BASEADDR, BYPASS_GPIO_TRI_OFFSET,
		     BYPASS_ALPHA_EN_TRI_SETUP);
	XDc_WriteReg(BYPASS_ALPHA_EN_BASEADDR, BYPASS_GPIO_TRI_OFFSET,
		     BYPASS_ALPHA_EN_TRI_FINAL);
}

/*****************************************************************************/
/**
 *
 * XDpDc_BypassInitVtc - Initialize VTC for bypass mode
 *
 * Configures the Video Timing Controller with the resolution and PPC
 * from RunCfgPtr. Horizontal parameters are divided by PPC.
 *
 * @param    RunCfgPtr - Run-time configuration with VideoMode and PPC.
 * @param    VtcPtr    - Pointer to XVtc instance.
 * @param    VtcBase   - Base address of the VTC.
 *
 * @return   XST_SUCCESS on success.
 *
 *****************************************************************************/
static u32 XDpDc_BypassInitVtc(RunConfig *RunCfgPtr, XVtc *VtcPtr,
				UINTPTR VtcBase)
{
	XVtc_Config CfgPtr;
	XVtc_SourceSelect SourceSelect = {0};
	XVtc_Timing VideoTiming;
	XVtc_Signal Signal;
	XVtc_HoriOffsets Hoff;
	XVtc_Polarity ConvPolarity;
	const XVidC_VideoTimingMode *VidTimingMode;
	const XVidC_VideoTiming *VidTiming;
	u8 Ppc = RunCfgPtr->PPC;

	XVtc_CfgInitialize(VtcPtr, &CfgPtr, VtcBase);

	XVtc_Reset(VtcPtr);
	XVtc_DisableGenerator(VtcPtr);
	XVtc_Disable(VtcPtr);

	/* All timing from generator registers */
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

	XVtc_SetSource(VtcPtr, &SourceSelect);

	VidTimingMode = XVidC_GetVideoModeData(RunCfgPtr->VideoMode);
	if (!VidTimingMode) {
		xil_printf("  ERROR: No video timing for mode %d\r\n",
			   RunCfgPtr->VideoMode);
		return XST_FAILURE;
	}
	VidTiming = &VidTimingMode->Timing;

	VideoTiming.HActiveVideo = VidTiming->HActive / Ppc;
	VideoTiming.HFrontPorch = VidTiming->HFrontPorch / Ppc;
	VideoTiming.HSyncWidth = VidTiming->HSyncWidth / Ppc;
	VideoTiming.HBackPorch = VidTiming->HBackPorch / Ppc;
	VideoTiming.HSyncPolarity = VidTiming->HSyncPolarity;

	VideoTiming.VActiveVideo = VidTiming->VActive;
	VideoTiming.V0FrontPorch = VidTiming->F0PVFrontPorch;
	VideoTiming.V0SyncWidth = VidTiming->F0PVSyncWidth;
	VideoTiming.V0BackPorch = VidTiming->F0PVBackPorch;
	VideoTiming.V1FrontPorch = VidTiming->F1VFrontPorch;
	VideoTiming.V1SyncWidth = VidTiming->F1VSyncWidth;
	VideoTiming.V1BackPorch = VidTiming->F1VBackPorch;
	VideoTiming.VSyncPolarity = VidTiming->VSyncPolarity;

	XVtc_ConvTiming2Signal(VtcPtr, &VideoTiming, &Signal, &Hoff, &ConvPolarity);

	XVtc_Polarity Polarity = {0};
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol  = 1;
	Polarity.VBlankPol       = 1;
	Polarity.VSyncPol        = 1;
	Polarity.HBlankPol       = 1;
	Polarity.HSyncPol        = 1;

	XVtc_SetPolarity(VtcPtr, &Polarity);
	XVtc_SetGenerator(VtcPtr, &Signal);
	XVtc_SetGeneratorHoriOffset(VtcPtr, &Hoff);

	XVtc_Enable(VtcPtr);
	XVtc_EnableGenerator(VtcPtr);
	XVtc_RegUpdateEnable(VtcPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * XDpDc_BypassConfigureLiveGenerator - Configure AVPG + VTC for bypass SST
 *
 * Programs AVPG[0] and VTC for the active bypass stream. PPC is taken from
 * xparameters.h. Architecture supports MST by extending to iterate over
 * byp_streams — but only stream 0 is configured here.
 *
 * @param    RunCfgPtr - Run-time configuration. avpg[0] provides bus format.
 *
 * @return   XST_SUCCESS on success, XST_FAILURE if AVPG base is invalid.
 *
 *****************************************************************************/
u32 XDpDc_BypassConfigureLiveGenerator(RunConfig *RunCfgPtr)
{
	u8 stream_idx = 0;
	u8 PhysPpc;
	UINTPTR AvpgBase;
	u32 Status;

	AvpgBase = BypassAvpgBase[stream_idx];
	if (AvpgBase == 0) {
		xil_printf("  ERROR: No AVPG base for stream %d\r\n", stream_idx);
		return XST_FAILURE;
	}
	if (BYPASS_VTC_0_BASEADDR == 0) {
		xil_printf("  ERROR: No VTC base for stream %d\r\n", stream_idx);
		return XST_FAILURE;
	}
	if (BYPASS_LIVE_IN_GPIO_BASEADDR == 0) {
		xil_printf("  ERROR: No live input GPIO base in xparameters\r\n");
		return XST_FAILURE;
	}

	/* Hold data path while configuring */
	XDc_WriteReg(BYPASS_LIVE_IN_GPIO_BASEADDR,
		     BYPASS_GPIO_DATA_OFFSET, BYPASS_GPIO_HOLD);

	xil_printf("  Bypass stream %u source: AVPG=0x%08X VTC=0x%08X\r\n",
		   stream_idx, (u32)AvpgBase, (u32)BYPASS_VTC_0_BASEADDR);

	/* VTC / AVPG: PPC from xparameters (XSA) */
	PhysPpc = XDpDc_BypassPpcFromXsa(stream_idx);
	RunCfgPtr->PPC = PhysPpc;
	Status = XDpDc_BypassInitVtc(RunCfgPtr, &BypassVtc0, BYPASS_VTC_0_BASEADDR);
	if (Status != XST_SUCCESS)
		return Status;
	XVtc_WriteReg(BYPASS_VTC_0_BASEADDR, 0x0, BYPASS_VTC_CTRL_ENABLE);

	/* AVPG: configure and enable */
	xil_printf("  Bypass AVPG[%d]: %s %d-bpc, %d PPC, pattern %d\r\n",
		   stream_idx,
		   RunCfgPtr->avpg[stream_idx].pix_fmt ? "YCbCr422" : "RGB",
		   RunCfgPtr->avpg[stream_idx].bpc,
		   PhysPpc,
		   RunCfgPtr->avpg[stream_idx].pattern);

	RunCfgPtr->avpg[stream_idx].ppc = XDpDc_BypassAvpgPpcFlag(PhysPpc);
	XDpDc_BypassProgramAvpg(AvpgBase, &RunCfgPtr->avpg[stream_idx],
				RunCfgPtr->Width, RunCfgPtr->Height, PhysPpc);
	XAvpg_WriteReg(AvpgBase, XAV_PATGEN_EN, 0x1);

	/* Release data path â€” AVPG data flows to DP TX */
	if (XDpDc_BypassAlphaGpioPresent())
		XDpDc_BypassEnableAlphaGpioPath();
	else
		XDc_WriteReg(BYPASS_LIVE_IN_GPIO_BASEADDR,
			     BYPASS_GPIO_DATA_OFFSET, BYPASS_GPIO_RELEASE);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Map bypass AVPG stream 0 bus format to RunConfig.Stream1Format.
 *
 * Naming note: bypass PL uses AVPG stream index 0, while DP TX and RunConfig
 * use Stream1Format / XMMIDP_STREAM_ID1 for the single SST output.
 *
 *****************************************************************************/
static void XDpDc_BypassSetStream0Format(RunConfig *RunCfgPtr)
{
	if (RunCfgPtr->avpg[0].pix_fmt == XAVPATGEN_CS_RGB) {
		switch (RunCfgPtr->avpg[0].bpc) {
		case 10: RunCfgPtr->Stream1Format = RGB_10BPC; break;
		case 12: RunCfgPtr->Stream1Format = RGB_12BPC; break;
		default: RunCfgPtr->Stream1Format = RGB_8BPC; break;
		}
	} else {
		switch (RunCfgPtr->avpg[0].bpc) {
		case 10: RunCfgPtr->Stream1Format = YCbCr422_10BPC; break;
		case 12: RunCfgPtr->Stream1Format = YCbCr422_12BPC; break;
		default: RunCfgPtr->Stream1Format = YCbCr422_8BPC; break;
		}
	}
}
/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the DC Subsystem (XDcDma,
* XDc)
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
u32 InitBypassDcSubsystem(RunConfig *RunCfgPtr)
{

	XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
	XDc *DcPtr = DcSubPtr->DcPtr;

	/* Program clock source selection */
	XDc_VidClkSelect(DcPtr);

	/* DC Video Frame Switch rate */
	XDc_SetVidFrameSwitch(DcPtr);

	/* DC Video Interface */
	XDc_SetVidInterfaceMode(DcPtr);

	return XST_SUCCESS;
}

u32 InitBypassPlatform(RunConfig *RunCfgPtr)
{
	u32 Status = XST_SUCCESS;

	xil_printf("  Configuring clock wizard...\r\n");
	Status = XDpDc_InitClkWiz(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Clock Wizard init failed\r\n");
		return Status;
	}

	/* Restore full pixel clock for DP MSA / downstream */
	RunCfgPtr->PixelClkHz *= XDpDc_BypassPpcFromXsa(0U);

	/* DC subsystem init */
	xil_printf("  Configuring DC subsystem (bypass)...\r\n");
	Status = InitBypassDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: DC Subsystem init failed\r\n");
		return Status;
	}

	/* Alpha bypass + live AVTPG PM GPIO (reference InitBypassMode) */
	XDpDc_BypassInitAlphaGpioPath();

	/* VTC + AVPG setup via bypass-specific generator */
	Status = XDpDc_BypassConfigureLiveGenerator(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Bypass live generator failed\r\n");
		return Status;
	}

	xil_printf("  Enabling DisplayPort output (SST, async HPD)...\r\n");

	Status = XDpDc_InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: DP subsystem init failed\r\n");
		return Status;
	}

	XDpDc_SetupInterrupts(RunCfgPtr);

	return XST_SUCCESS;

}

u32 InitBypassRunConfig(RunConfig *RunCfgPtr)
{
	const XVidC_VideoTimingMode *Vtm;
	XDc *DcPtr;
	u8 stream_idx;

	/* DP TX init (SST, single stream) */

	if (RunCfgPtr->operatingmode != XDCSUB_OPMODE_BYPASS) {
		xil_printf("%s: Invalid config\r\n", __func__);
		return XST_INVALID_PARAM;
	}

	/* Dp Tx init: honor menu stream count on MST designs;SST-only uses 1 */
	if(!RunCfgPtr->DesignMstEn)
		RunCfgPtr->byp_streams = 1;

	RunCfgPtr->livevidselect = XDCSUB_LIVVID_SEL_V01;
	RunCfgPtr->PPC = XDpDc_BypassPpcFromXsa(0U);

	for (stream_idx = 0U; stream_idx < BYPASS_MAX_STREAMS; stream_idx++) {
		u8 ppc = XDpDc_BypassPpcFromXsa(stream_idx);

		RunCfgPtr->avpg[stream_idx].ppc = XDpDc_BypassAvpgPpcFlag(ppc);
	}

	XDpDc_BypassSetStream0Format(RunCfgPtr); /* AVPG stream 0 -> Stream1Format */

	xil_printf("\r\n========================================\r\n");

	if(RunCfgPtr->DesignMstEn)
		xil_printf("DisplayPort DC Bypass MST Video Test\r\n");
	else
		xil_printf("DisplayPort DC Bypass SST Video Test\r\n");
	xil_printf("  Stream 0: LIVE (AVPG), %dx%d, %d PPC\r\n",
		   RunCfgPtr->Width, RunCfgPtr->Height,
		   XDpDc_BypassPpcFromXsa(0U));
	xil_printf("  Stream 0 source: AVPG=0x%08X VTC=0x%08X\r\n",
		   (u32)BypassAvpgBase[0], (u32)BYPASS_VTC_0_BASEADDR);

	if(RunCfgPtr->DesignMstEn)
		xil_printf(" Active Streams: %d\r\n", RunCfgPtr->byp_streams);

	xil_printf("========================================\r\n\r\n");

	/* Instance setup */
	RunCfgPtr->DcSubPtr = &DcSub;
	RunCfgPtr->DcSubPtr->DcPtr = &Dc;
	RunCfgPtr->DcSubPtr->DmaPtr = &DcDma;
	RunCfgPtr->DpPsuPtr = &DpPsuPtr;

	DcPtr = RunCfgPtr->DcSubPtr->DcPtr;
	DcPtr->Config.BaseAddr = DC_BASEADDR;

	/* Resolve video timing */
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
		xil_printf("WARNING: No timing for mode %d, "
			   "using 1920x1080\r\n", RunCfgPtr->VideoMode);
		DcPtr->VideoTiming = VidTiming_1920_1080;
	}

	/* DC bypass configuration */
	XDcSub_SetVidInterfaceMode(RunCfgPtr->DcSubPtr, XDC_VID_BYPASS);
	XDcSub_VidClkSelect(RunCfgPtr->DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfgPtr->DcSubPtr, BYPASS_DC_FRAME_SWITCH_VAL);


	/*
	 * Clock scaling for bypass:
	 * The clk wiz output drives the PL video pipeline and
	 * pl_mmi_dc_1x_clk directly, so it must run at pixel_clock / PPC.
	 */
	RunCfgPtr->PixelClkHz /= XDpDc_BypassPpcFromXsa(0U);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * XDpDc_MmiDcBypassTest - Bypass SST test entry point
 *
 * Configures DC in bypass mode, programs clock wizard for the PPC-scaled
 * pixel clock, invokes BypassConfigureLiveGenerator for AVPG/VTC setup,
 * then initializes DP for asynchronous HPD-driven link training and video.
 *
 * @param    RunCfgPtr - Run-time configuration.
 *           operatingmode must be XDCSUB_OPMODE_BYPASS.
 *
 * @return   XST_SUCCESS on success, error code otherwise.
 *
 *****************************************************************************/
u32 XDpDc_MmiDcBypassTest(RunConfig *RunCfgPtr)
{
	u32 Status = XST_SUCCESS;

	/* Initialize the bypass application configuration */
	Status = InitBypassRunConfig(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to get test configuration\r\n");
		return Status;
	}

	Status = InitBypassPlatform(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("FAILED to Initialize Platform\r\n");
		return Status;
	}

	xil_printf("\r\n  Bypass %s mode active.\r\n", RunCfgPtr->DesignMstEn ? "MST" : "SST");

	return XST_SUCCESS;
}

#endif /* defined(XPAR_XVTC_NUM_INSTANCES) */
