/******************************************************************************
 * @file dp_display_bridge.c
 * @brief ISP (VISP SS) -> DisplayPort live output bridge implementation.
 *
 * Datapath:
 *   ISP -> frmbuf_wr -> DDR -> frmbuf_rd -> VTC -> DC (live input) -> DP TX
 *
 * The DP core + link-training flow is reused from the proven mmi_dpdc_example
 * sources (mmi_dp_init.c / mmi_dp_intr.c). This file provides the live source
 * (frmbuf_rd), VTC timing, live-input GPIO mux, DC live setup and clock-wizard
 * bring-up, using the REAL hardware base addresses of this design.
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 ******************************************************************************/

#include <string.h>

#include <xil_printf.h>
#include <xil_types.h>
#include <xstatus.h>
#include <sleep.h>

#include <xdcsub.h>
#include <xdc.h>
#include <xdcdma.h>
#include <xmmidp.h>
#include <xvtc.h>
#include <xclk_wiz.h>
#include <xvidc.h>

#include "xv_frmbufrd_l2.h"
#include "xv_frmbufrd.h"        /* low-level XV_frmbufrd_InterruptEnable etc. */
#include "xinterrupt_wrap.h"    /* XSetupInterruptSystem */

#include "mmi_dpdc_example.h"
#include "mmi_dpdc_platform.h"   /* DC_BASEADDR, DP_BASEADDR */
#include "mmi_dc_nonlive_test.h" /* CLK_WIZ_BASEADDR */
#include "mmi_dp_init.h"
#include "mmi_dp_intr.h"
#include "dp_display_bridge.h"

/************************** Constant Definitions *****************************/

/*
 * REAL hardware base addresses for this design's DC live-input pipeline
 * (verified against xparameters.h). NOTE: these differ from the (incorrect)
 * addresses hard-coded in the mmi_dpdc_example live test.
 */
#define DP_LIVE_VTC_BASEADDR		0xB0010000U /* DP_SS_DC_INPUT_PIPELINE_V_TC_0 */
#define DP_LIVE_FRMBUF_RD_BASEADDR	0xB0020000U /* DP_SS_FBR_SS_V_FRMBUF_RD_0    */
#define DP_LIVE_IN_GPIO_BASEADDR	0xB00C0000U /* DP_SS_DC_GPIO_SS_LIVE_INPUT   */

/* Live-input GPIO gate values (mirrors mmi_dpdc_example sequence). */
#define DP_LIVE_GPIO_ARM		0x00010000U
#define DP_LIVE_GPIO_ENABLE		0x00010001U

/* VTC generator control seed value used by the DP example live path. */
#define DP_LIVE_VTC_CTRL_SEED		0x03F7EF06U

/* DC live path latency defaults (from mmi_dpdc_example live setup). */
#define DP_DC_NL_LATENCY		0x0138U
#define DP_DC_V_LINE_LATENCY		0x2U
/*
 * Global alpha for the single live video layer.
 *
 * The DC blends as: out = alpha*Stream2 + (1 - alpha)*Stream1, where alpha is
 * the opacity of the (unused, black) Stream2 graphics layer. So:
 *   0xFF -> 100% Stream2 (black)      -> full blackout
 *   0xA5 -> 65% black + 35% Stream1   -> dim/washed (the mmi_dpdc_example value,
 *                                        meant for blending TWO live streams)
 *   0x00 -> 0% Stream2, 100% Stream1  -> full-brightness video  <-- correct
 * We have only Stream1, so keep Stream2 fully transparent (0x00).
 */
#define DP_DC_GLOBAL_ALPHA		0x00U
#define DP_DC_VID_FRAME_SWITCH		0x3FU

/*
 * DP output refresh rate (the MONITOR refresh, independent of the sensor/ISP
 * capture rate). MUST be a mode the monitor lists in its EDID -- many 1080p
 * monitors accept only 60 Hz, so 30 Hz may show "no signal". To change it,
 * edit BOTH lines below (keep them consistent).
 */
#define DP_REFRESH_FR			XVIDC_FR_30HZ
#define DP_REFRESH_HZ			30U

/************************** Variable Definitions *****************************/

/* Global driver instances required by the reused DP/DC sources. */
RunConfig RunCfg;
XDcSub    DcSub;
XDcDma    DcDma;
XDc       Dc;
XMmiDp    DpPsuPtr;

/* CSC coefficient tables (identity RGB passthrough) referenced as externs by
 * the reused DC live setup. */
u32 CSCCoeff_RGB[]     = { 0x1000, 0x0000, 0x0000, 0x0000, 0x1000, 0x0000, 0x0000, 0x0000, 0x1000 };
u32 CSCOffset_RGB[]    = { 0x0000, 0x0000, 0x0000 };

/* Bridge-local state. */
static XVtc          DpVtc;
static XV_FrmbufRd_l2 DpFrmbufRd;
static XVidC_ColorFormat DpMemCfmt;
static u32 DpStride;
static int DpBridgeReady;
static int DpFrmbufStarted;

/*
 * Vblank-synchronized buffer handoff (matches the working reference design).
 *
 * The frmbuf_wr done callback must NOT write the frmbuf_rd buffer-address
 * register directly -- that happens mid-scanout and tears the displayed frame
 * (visible as motion "shadows"/ghosting). Instead the producer publishes the
 * new buffer into a pending slot, and the frmbuf_rd's own Frame-Done ISR (which
 * fires at the frmbuf_rd vblank -- the safe latch boundary) consumes it and
 * programs SetBufferAddr + Start there.
 */
static volatile u64 DpFbrPendingLuma;
static volatile u64 DpFbrPendingChroma;
static volatile u32 DpFbrPendingValid;
static int          DpFbrIrqArmed;

/*
 * Format selection derived from the DDR memory format (MemCfmt) the ISP
 * frmbuf_wr produces. Supports RGB888 (straight passthrough, no CSC) and
 * YUV422 semi-planar (Y_UV8, DC does YUV->RGB).
 */
static XVidC_ColorFormat DpStreamCfmt;   /* frmbuf_rd AXI-stream color format */
static XDc_VideoFormat   DpDcLiveFmt;    /* DC live input format             */
static XDc_VideoFormat   DpDcNonLiveFmt; /* placeholder for EnableStreamBufs */
static XDc_VideoFormat   DpOutStreamFmt; /* DC + DP output format            */
static u32 *DpInCscCoeff;                /* DC input CSC coefficients        */
static u32 *DpInCscOffset;               /* DC input CSC offsets             */

/*****************************************************************************/
/**
 * Wait for a Clock Wizard instance to report PLL lock.
 * (Local copy of the platform helper so we don't pull in mmi_dpdc_platform.c.)
 *****************************************************************************/
static u32 DpBridge_WaitForClkLock(XClk_Wiz_Config *CfgPtr)
{
	u32 Count = 0;

	while (!(XClk_Wiz_ReadReg(CfgPtr->BaseAddr, XCLK_WIZ_STATUS_OFFSET) & XCLK_WIZ_LOCK)) {
		if (Count == 10000)
			return XST_FAILURE;
		usleep(100);
		Count++;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Program the video Clock Wizard for the requested pixel clock.
 *****************************************************************************/
static u32 DpBridge_InitClkWiz(u64 PixelClkHz)
{
	XClk_Wiz InstancePtr;
	XClk_Wiz_Config *CfgPtr;
	u32 Status;
	u64 Rate;

	CfgPtr = XClk_Wiz_LookupConfig(CLK_WIZ_BASEADDR);
	if (!CfgPtr) {
		xil_printf("  [DP] ERROR: Failed to lookup Clock Wizard IP\r\n");
		return XST_FAILURE;
	}

	Status = XClk_Wiz_CfgInitialize(&InstancePtr, CfgPtr, CfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: Failed to init Clock Wizard IP\r\n");
		return XST_FAILURE;
	}
	InstancePtr.MinErr = 20000;

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_REG25_OFFSET, 0);

	Status = XClk_Wiz_SetRateHz(&InstancePtr, PixelClkHz);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: Clock Wizard SetRateHz failed (%d)\r\n", Status);
		return XST_FAILURE;
	}

	XClk_Wiz_WriteReg(CfgPtr->BaseAddr, XCLK_WIZ_RECONFIG_OFFSET,
			  (XCLK_WIZ_RECONFIG_LOAD | XCLK_WIZ_RECONFIG_SADDR));

	Status = DpBridge_WaitForClkLock(CfgPtr);
	if (Status != XST_SUCCESS)
		xil_printf("  [DP] WARNING: Video clock not locked\r\n");
	else
		xil_printf("  [DP] ClkWiz: PLL locked\r\n");

	XClk_Wiz_GetRate(&InstancePtr, 0, &Rate);
	xil_printf("  [DP] ClkWiz: %llu Hz (%llu.%03llu MHz)\r\n",
		   Rate, Rate / 1000000, (Rate % 1000000) / 1000);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Configure the DC subsystem descriptor for the live YUV422 -> RGB path.
 *****************************************************************************/
static void DpBridge_ConfigDcLive(void)
{
	XDc *DcPtr = RunCfg.DcSubPtr->DcPtr;
	const XVidC_VideoTimingMode *Vtm;

	DcPtr->Config.BaseAddr = DC_BASEADDR;

	Vtm = XVidC_GetVideoModeData(RunCfg.VideoMode);
	if (Vtm) {
		DcPtr->VideoTiming.HTotal  = Vtm->Timing.HTotal;
		DcPtr->VideoTiming.HSWidth = Vtm->Timing.HSyncWidth;
		DcPtr->VideoTiming.HRes    = Vtm->Timing.HActive;
		DcPtr->VideoTiming.HStart  = Vtm->Timing.HSyncWidth + Vtm->Timing.HBackPorch;
		DcPtr->VideoTiming.VTotal  = Vtm->Timing.F0PVTotal;
		DcPtr->VideoTiming.VSWidth = Vtm->Timing.F0PVSyncWidth;
		DcPtr->VideoTiming.VRes    = Vtm->Timing.VActive;
		DcPtr->VideoTiming.VStart  = Vtm->Timing.F0PVSyncWidth + Vtm->Timing.F0PVBackPorch;
	}

	XDcSub_SetVidInterfaceMode(RunCfg.DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(RunCfg.DcSubPtr, XDC_VIDSTREAM1_LIVE, XDC_VIDSTREAM2_LIVE);
	XDcSub_SetInputAudioSelect(RunCfg.DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputLiveStreamFormat(RunCfg.DcSubPtr, RunCfg.Stream1Format,
					RunCfg.Stream2Format);
	/*
	 * XDc_EnableStream1/2Buffers() (invoked by XDcSub_ConfigureDcVideo)
	 * dereferences the NonLiveVideo attributes even in live mode, so set
	 * them to a valid NON-LIVE format (the live format enums are outside the
	 * non-live lookup range). The live mux bypasses the DMA so this only
	 * prevents a NULL dereference.
	 */
	XDcSub_SetInputNonLiveVideoFormat(RunCfg.DcSubPtr, DpDcNonLiveFmt, DpDcNonLiveFmt);
	XDcSub_SetOutputVideoFormat(RunCfg.DcSubPtr, RunCfg.OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(RunCfg.DcSubPtr, 0, 0);

	XDcSub_SetGlobalAlpha(RunCfg.DcSubPtr, ALPHA_ENABLE, DP_DC_GLOBAL_ALPHA);
	XDcSub_SetStreamPixelScaling(RunCfg.DcSubPtr, NULL, NULL);

	/* Input CSC depends on source: RGB -> identity, YUV422 -> RGB (BT.601).
	 * Output CSC is identity RGB (DP output is RGB888). */
	XDcSub_SetInputStreamCSC(RunCfg.DcSubPtr, DpInCscCoeff, DpInCscOffset,
				 DpInCscCoeff, DpInCscOffset);
	XDcSub_SetOutputStreamCSC(RunCfg.DcSubPtr, CSCCoeff_RGB, CSCOffset_RGB);

	XDcSub_SetAudioVideoClkSrc(RunCfg.DcSubPtr);
	DcPtr->VideoClk = 0;
	DcPtr->AudioClk = 0;
	XDcSub_VidClkSelect(RunCfg.DcSubPtr, 0, 0);
	XDcSub_SetVidFrameSwitch(RunCfg.DcSubPtr, DP_DC_VID_FRAME_SWITCH);
	XDcSub_SetNonLiveLatency(RunCfg.DcSubPtr, DP_DC_NL_LATENCY, DP_DC_V_LINE_LATENCY);
}

/*****************************************************************************/
/**
 * Bring the DC hardware up for the live datapath.
 *****************************************************************************/
static void DpBridge_InitDcHw(void)
{
	XDcSub *DcSubPtr = RunCfg.DcSubPtr;
	XDc *DcPtr = DcSubPtr->DcPtr;

	XDcSub_Initialize(DcSubPtr);
	XDc_VidClkSelect(DcPtr);
	XDc_VideoSoftReset(DcPtr);
	XDc_SetVidFrameSwitch(DcPtr);
	XDc_SetVideoTiming(DcPtr);
	XDcSub_ConfigureDcVideo(DcPtr);
}

/*****************************************************************************/
/**
 * Configure the VTC generator to produce timing for the live stream.
 * (Adapted from mmi_dpdc_example InitVtcSubsystem, real VTC base address.)
 *****************************************************************************/
static void DpBridge_InitVtc(void)
{
	XVtc_Config CfgPtr;
	XVtc_Polarity Polarity;
	XVtc_SourceSelect SourceSelect;
	XVtc_Timing VideoTiming;
	XVtc_Signal Signal;
	XVtc_HoriOffsets Hoff;
	const XVidC_VideoTimingMode *VidTimingMode;
	const XVidC_VideoTiming *VidTiming;

	XVtc_CfgInitialize(&DpVtc, &CfgPtr, DP_LIVE_VTC_BASEADDR);

	XVtc_Reset(&DpVtc);
	XVtc_DisableGenerator(&DpVtc);
	XVtc_Disable(&DpVtc);

	memset((void *)&SourceSelect, 0, sizeof(SourceSelect));
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
	XVtc_SetSource(&DpVtc, &SourceSelect);

	VidTimingMode = XVidC_GetVideoModeData(RunCfg.VideoMode);
	VidTiming = &VidTimingMode->Timing;

	VideoTiming.HActiveVideo  = VidTiming->HActive;
	VideoTiming.HFrontPorch   = VidTiming->HFrontPorch;
	VideoTiming.HSyncWidth    = VidTiming->HSyncWidth;
	VideoTiming.HBackPorch    = VidTiming->HBackPorch;
	VideoTiming.HSyncPolarity = VidTiming->HSyncPolarity;
	VideoTiming.VActiveVideo  = VidTiming->VActive;
	VideoTiming.V0FrontPorch  = VidTiming->F0PVFrontPorch;
	VideoTiming.V0SyncWidth   = VidTiming->F0PVSyncWidth;
	VideoTiming.V0BackPorch   = VidTiming->F0PVBackPorch;
	VideoTiming.V1FrontPorch  = VidTiming->F1VFrontPorch;
	VideoTiming.V1SyncWidth   = VidTiming->F1VSyncWidth;
	VideoTiming.V1BackPorch   = VidTiming->F1VBackPorch;
	VideoTiming.VSyncPolarity = VidTiming->VSyncPolarity;

	/* Horizontal timing is per-pixel-per-clock (2 ppc live datapath). */
	VideoTiming.HActiveVideo = VideoTiming.HActiveVideo / RunCfg.PPC;
	VideoTiming.HFrontPorch  = VideoTiming.HFrontPorch  / RunCfg.PPC;
	VideoTiming.HBackPorch   = VideoTiming.HBackPorch   / RunCfg.PPC;
	VideoTiming.HSyncWidth   = VideoTiming.HSyncWidth   / RunCfg.PPC;

	XVtc_ConvTiming2Signal(&DpVtc, &VideoTiming, &Signal, &Hoff, &Polarity);

	memset((void *)&Polarity, 0, sizeof(Polarity));
	Polarity.ActiveChromaPol = 1;
	Polarity.ActiveVideoPol  = 1;
	Polarity.VBlankPol       = 1;
	Polarity.VSyncPol        = 1;
	Polarity.HBlankPol       = 1;
	Polarity.HSyncPol        = 1;

	XVtc_SetPolarity(&DpVtc, &Polarity);
	XVtc_SetGenerator(&DpVtc, &Signal);
	XVtc_SetGeneratorHoriOffset(&DpVtc, &Hoff);

	XVtc_Enable(&DpVtc);
	XVtc_EnableGenerator(&DpVtc);
	XVtc_RegUpdateEnable(&DpVtc);

	/* Seed the VTC generator control register (matches DP example). */
	XVtc_WriteReg(DP_LIVE_VTC_BASEADDR, 0x0, DP_LIVE_VTC_CTRL_SEED);
}

/*****************************************************************************/
/**
 * frmbuf_rd Frame-Done ISR: runs at the frmbuf_rd vblank (the safe latch
 * boundary). If the producer published a new buffer, program it here so the
 * engine latches it cleanly between frames instead of mid-scanout.
 *****************************************************************************/
static void DpBridge_FbrFrameDoneCallback(void *CallbackRef)
{
	(void)CallbackRef;

	if (!DpFbrPendingValid)
		return;

	/* Read payload BEFORE clearing Valid so a racing publish can't leave us
	 * with a half-updated target (worst case we re-latch the same buffer). */
	u64 Luma   = DpFbrPendingLuma;
	u64 Chroma = DpFbrPendingChroma;

	DpFbrPendingValid = 0;

	(void)XVFrmbufRd_SetBufferAddr(&DpFrmbufRd, (UINTPTR)Luma);

	if ((DpMemCfmt == XVIDC_CSF_MEM_Y_UV8)  || (DpMemCfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	    (DpMemCfmt == XVIDC_CSF_MEM_Y_UV10) || (DpMemCfmt == XVIDC_CSF_MEM_Y_UV10_420))
		(void)XVFrmbufRd_SetChromaBufferAddr(&DpFrmbufRd, (UINTPTR)Chroma);

	/* Explicit ap_start at the frame boundary re-samples the buffer-address
	 * scalars cleanly (auto-restart alone latches at an arbitrary phase). */
	XVFrmbufRd_Start(&DpFrmbufRd);
}

/*****************************************************************************/
/**
 * Wire the frmbuf_rd DONE IRQ to the GIC and enable it at the IP. Idempotent.
 * Uses the low-level L1 InterruptEnable so auto-restart is left untouched
 * (the L2 wrapper would disable it).
 *****************************************************************************/
static int DpBridge_FbrArmIrq(void)
{
	int Status;

	if (DpFbrIrqArmed)
		return XST_SUCCESS;

	Status = XVFrmbufRd_SetCallback(&DpFrmbufRd, XVFRMBUFRD_HANDLER_DONE,
					(void *)DpBridge_FbrFrameDoneCallback,
					(void *)&DpFrmbufRd);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: frmbuf_rd SetCallback(DONE) failed (%d)\r\n", Status);
		return Status;
	}

	Status = XSetupInterruptSystem(&DpFrmbufRd, (void *)XVFrmbufRd_InterruptHandler,
				       DpFrmbufRd.FrmbufRd.Config.IntrId,
				       DpFrmbufRd.FrmbufRd.Config.IntrParent, 0xA0);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: frmbuf_rd XSetupInterruptSystem failed (%d)\r\n", Status);
		return Status;
	}

	XV_frmbufrd_InterruptEnable(&DpFrmbufRd.FrmbufRd, XVFRMBUFRD_IRQ_DONE_MASK);
	XV_frmbufrd_InterruptGlobalEnable(&DpFrmbufRd.FrmbufRd);

	DpFbrIrqArmed = 1;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Initialize the frame-buffer reader that streams DDR frames into the DC.
 *****************************************************************************/
static u32 DpBridge_InitFrmbufRd(u32 Width, u32 Height)
{
	XVidC_VideoStream Stream;
	const XVidC_VideoTimingMode *Vtm;
	int Status;

	Status = XVFrmbufRd_Initialize(&DpFrmbufRd, DP_LIVE_FRMBUF_RD_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: frmbuf_rd init failed (%d)\r\n", Status);
		return XST_FAILURE;
	}

	Vtm = XVidC_GetVideoModeData(RunCfg.VideoMode);

	memset(&Stream, 0, sizeof(Stream));
	Stream.VmId          = RunCfg.VideoMode;
	Stream.ColorFormatId = DpStreamCfmt;
	Stream.ColorDepth    = XVIDC_BPC_8;
	Stream.PixPerClk     = XVIDC_PPC_2;
	Stream.FrameRate     = DP_REFRESH_FR;
	Stream.IsInterlaced  = FALSE;
	if (Vtm)
		Stream.Timing = Vtm->Timing;
	Stream.Timing.HActive = Width;
	Stream.Timing.VActive = Height;

	Status = XVFrmbufRd_SetMemFormat(&DpFrmbufRd, DpStride, DpMemCfmt, &Stream);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: frmbuf_rd SetMemFormat failed (%d)\r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Free-run: re-read the current DDR buffer every frame at the display
	 * rate so the DC live input never starves. The ISP updates the buffer
	 * pointer at the sensor rate while the DP outputs at DP_REFRESH_HZ.
	 */
	XV_frmbufrd_EnableAutoRestart(&DpFrmbufRd.FrmbufRd);

	/*
	 * Arm the Frame-Done IRQ so buffer swaps latch at the frmbuf_rd vblank
	 * (avoids mid-scanout tearing / motion "shadows").
	 */
	DpFbrPendingValid = 0;
	(void)DpBridge_FbrArmIrq();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Public: initialize the full ISP -> DP live output bridge.
 *****************************************************************************/
int DpDisplayBridge_Init(u32 Width, u32 Height, XVidC_ColorFormat MemCfmt,
			 u32 Stride, u32 ChromaOffset)
{
	const XVidC_VideoTimingMode *Vtm;
	u32 Status;

	(void)ChromaOffset; /* used per-frame via DpDisplayBridge_UpdateFrame */

	DpBridgeReady = 0;
	DpFrmbufStarted = 0;
	DpMemCfmt = MemCfmt;
	DpStride = Stride;

	/*
	 * Only RGB888 is supported: straight RGB passthrough (identity input CSC),
	 * DP outputs RGB888. Any other source format is SKIPPED -- the bridge stays
	 * uninitialized (DpBridgeReady = 0) so DpDisplayBridge_UpdateFrame() is a
	 * no-op and the application keeps running without DP output.
	 */
	if (MemCfmt == XVIDC_CSF_MEM_RGB8) {
		DpStreamCfmt   = XVIDC_CSF_RGB;
		DpDcLiveFmt    = RGB_8BPC;
		DpDcNonLiveFmt = RGB888;
		DpOutStreamFmt = RGB_8BPC;
		DpInCscCoeff   = CSCCoeff_RGB;
		DpInCscOffset  = CSCOffset_RGB;
		xil_printf("  [DP] source RGB888 -> DP RGB888 (passthrough)\r\n");
	} else {
		xil_printf("  [DP] source cfmt=%d unsupported -> DP output SKIPPED "
			   "(only RGB888)\r\n", MemCfmt);
		return XST_SUCCESS;
	}

	/* --- Build the run configuration for the live datapath --- */
	memset(&RunCfg, 0, sizeof(RunCfg));
	RunCfg.DcSubPtr = &DcSub;
	RunCfg.DcSubPtr->DcPtr = &Dc;
	RunCfg.DcSubPtr->DmaPtr = &DcDma;
	RunCfg.DpPsuPtr = &DpPsuPtr;

	RunCfg.Width  = Width;
	RunCfg.Height = Height;
	RunCfg.PPC    = 2;

	RunCfg.VideoMode = XVidC_GetVideoModeId(Width, Height, DP_REFRESH_FR, FALSE);
	if (RunCfg.VideoMode == XVIDC_VM_NOT_SUPPORTED)
		RunCfg.VideoMode = XVIDC_VM_1920x1080_60_P;

	Vtm = XVidC_GetVideoModeData(RunCfg.VideoMode);
	if (Vtm)
		RunCfg.PixelClkHz = (u64)Vtm->Timing.HTotal *
				    (u64)Vtm->Timing.F0PVTotal * (u64)DP_REFRESH_HZ;
	else
		RunCfg.PixelClkHz = 148500000ULL;

	/* Live input format follows the source; DC/DP output follows DpOutStreamFmt. */
	RunCfg.Stream1Format  = DpDcLiveFmt;
	RunCfg.Stream2Format  = DpDcLiveFmt;
	RunCfg.OutStreamFormat = DpOutStreamFmt;
	RunCfg.AudioEnable    = XDC_AUD_DISABLE;
	RunCfg.SdpEnable      = 0;
	RunCfg.Stream1PbEnable = PB_DISABLE;
	RunCfg.Stream2PbEnable = PB_DISABLE;
	RunCfg.operatingmode   = XDCSUB_OPMODE_FUNCTIONAL;
	RunCfg.presentationmode = XDCSUB_PPTMODE_LIVE;

	/*
	 * Cap the DP link so the transfer-unit (TU) fill stays in a healthy range.
	 * Training to the sink max (4-lane HBR2 = 17.28 Gbps) for this low-rate
	 * 720p30 RGB888 stream (~1.78 Gbps peak) leaves the TU only ~9% full
	 * (AvgBytesPerTU ~6/64), which starves the live video FIFO and shows up as
	 * intermittent black patches. 2-lane HBR (4.32 Gbps) raises TU fill to ~40%
	 * with margin over the peak. (0 = use sink max / no cap.)
	 */
	RunCfg.MaxLaneCount = 2U;                                /* limit to 2 lanes */
	RunCfg.MaxLinkRate  = XMMIDP_DPCD_LINK_BW_SET_270GBPS;   /* HBR (2.70 Gbps)  */

	xil_printf("\r\n=== DP Live Bridge Init (%ux%u @%uHz, RGB888) ===\r\n",
		   Width, Height, (unsigned)DP_REFRESH_HZ);

	/* --- Bring-up sequence (mirrors the proven live datapath order) --- */
	DpBridge_ConfigDcLive();

	Status = DpBridge_InitClkWiz(RunCfg.PixelClkHz);
	if (Status != XST_SUCCESS)
		return Status;

	DpBridge_InitDcHw();

	/* Arm the live-input mux before the source starts streaming. */
	XDc_WriteReg(DP_LIVE_IN_GPIO_BASEADDR, 0x0, DP_LIVE_GPIO_ARM);

	DpBridge_InitVtc();

	Status = DpBridge_InitFrmbufRd(Width, Height);
	if (Status != XST_SUCCESS)
		return Status;

	/* Enable the live-input path into the DC. */
	XDc_WriteReg(DP_LIVE_IN_GPIO_BASEADDR, 0x0, DP_LIVE_GPIO_ENABLE);

	/* Bring up the DP transmitter; link training runs from the HPD ISR. */
	Status = XDpDc_InitDpPsuSubsystem(&RunCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("  [DP] ERROR: DP subsystem init failed\r\n");
		return Status;
	}
	XDpDc_SetupDpInterrupts(&RunCfg);

	DpBridgeReady = 1;
	xil_printf("=== DP Live Bridge ready ===\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Public: display the latest ISP frame via the DP datapath.
 *****************************************************************************/
void DpDisplayBridge_UpdateFrame(u64 LumaAddr, u64 ChromaAddr)
{
	if (!DpBridgeReady)
		return;

	/*
	 * First frame: the engine is idle (no scanout in progress to tear), so
	 * program the buffer and kick it off directly. This bootstrap also gets
	 * the Frame-Done IRQ flowing so subsequent swaps latch at vblank.
	 */
	if (!DpFrmbufStarted) {
		XVFrmbufRd_SetBufferAddr(&DpFrmbufRd, LumaAddr);
		if ((DpMemCfmt == XVIDC_CSF_MEM_Y_UV8)  || (DpMemCfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
		    (DpMemCfmt == XVIDC_CSF_MEM_Y_UV10) || (DpMemCfmt == XVIDC_CSF_MEM_Y_UV10_420))
			XVFrmbufRd_SetChromaBufferAddr(&DpFrmbufRd, ChromaAddr);
		XVFrmbufRd_Start(&DpFrmbufRd);
		DpFrmbufStarted = 1;
		return;
	}

	/*
	 * Steady state: publish the new buffer for the Frame-Done ISR to latch
	 * at the next frmbuf_rd vblank. Write the payload first, then raise Valid
	 * (the ISR clears Valid before reading, so a race just skips one frame).
	 */
	DpFbrPendingLuma   = LumaAddr;
	DpFbrPendingChroma = ChromaAddr;
	__asm__ volatile("dmb sy" : : : "memory");
	DpFbrPendingValid  = 1;
}

/*****************************************************************************/
int DpDisplayBridge_IsReady(void)
{
	return DpBridgeReady;
}
