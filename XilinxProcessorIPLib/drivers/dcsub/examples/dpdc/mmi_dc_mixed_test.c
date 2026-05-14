/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file mmi_dc_mixed_test.c
 *
 * This file implements the mixed mode display test where Stream1 is sourced
 * from a live video input (AVPG pattern generator) and Stream2 is sourced
 * from a non-live DMA frame buffer. The two streams are alpha-blended and
 * output through the DisplayPort interface.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   amd  05/2026  Initial release
 * </pre>
 *
 *****************************************************************************/

#include <xstatus.h>
#include <xil_printf.h>
#include <xparameters.h>
#if defined (XPAR_XVTC_NUM_INSTANCES)
#include "xdc.h"
#include "xvtc.h"
#include "xavpg.h"
#include "mmi_dc_live_test.h"
#include "mmi_dc_nonlive_test.h"
#include "mmi_dc_partial_plane.h"
#include "mmi_dc_mixed_test.h"
#include "mmi_dp_init.h"
#include "mmi_dpdc_platform.h"

extern XVtc Vtc0;
extern u32 InitVtcSubsystem(RunConfig *RunCfgPtr, XVtc *Vtc, UINTPTR Addr);

/*****************************************************************************/
/**
 *
 * This function determines the Stream1 video format based on the AVPG
 * pixel format and bits-per-component settings in the run configuration.
 *
 * @param    RunCfgPtr - Pointer to the RunConfig structure containing
 *           the AVPG pixel format and BPC configuration.
 *
 * @return   None. The Stream1Format field of RunCfgPtr is updated.
 *
 * @note     Supports RGB and YCbCr422 colour spaces at 8, 10, and 12 BPC.
 *           Defaults to 8 BPC when an unsupported value is specified.
 *
 ******************************************************************************/
static void XDpDc_MixedSetLiveStream1Format(RunConfig *RunCfgPtr)
{
	if (RunCfgPtr->avpg[0].pix_fmt == XAVPATGEN_CS_RGB) {
		switch (RunCfgPtr->avpg[0].bpc) {
		case 10:
			RunCfgPtr->Stream1Format = RGB_10BPC;
			break;
		case 12:
			RunCfgPtr->Stream1Format = RGB_12BPC;
			break;
		case 8:
		default:
			RunCfgPtr->Stream1Format = RGB_8BPC;
			break;
		}
	} else {
		switch (RunCfgPtr->avpg[0].bpc) {
		case 10:
			RunCfgPtr->Stream1Format = YCbCr422_10BPC;
			break;
		case 12:
			RunCfgPtr->Stream1Format = YCbCr422_12BPC;
			break;
		case 8:
		default:
			RunCfgPtr->Stream1Format = YCbCr422_8BPC;
			break;
		}
	}
}

/*****************************************************************************/
/**
 *
 * This function configures the DC subsystem for mixed mode operation. It
 * sets up Stream1 as a live input and Stream2 as a non-live (DMA) input,
 * configures colour space conversion for both streams, applies global
 * alpha blending, and enables the stream buffers.
 *
 * @param    RunCfgPtr - Pointer to the RunConfig structure containing
 *           stream formats, partial blend settings, and the DcSub
 *           instance pointer.
 *
 * @return   None.
 *
 * @note     When partial-plane blending is enabled for either stream,
 *           XDpDc_ConfigurePartialPlaneBlend() is called after the
 *           mixed stream-source override to reapply the blend region.
 *           Frame switch is hardcoded to 0x3F to match legacy behavior.
 *
 ******************************************************************************/
static void XDpDc_ConfigureMixedDcSubPtr(RunConfig *RunCfgPtr)
{
	XDcSub *DcSubPtr = RunCfgPtr->DcSubPtr;
	const XDc_VideoAttribute *Vid1 =
		XDc_GetLiveVideoAttribute(RunCfgPtr->Stream1Format);
	const XDc_VideoAttribute *Vid2 =
		XDc_GetNonLiveVideoAttribute(RunCfgPtr->Stream2Format);
	u32 *In1Coeff, *In1Offset, *In2Coeff, *In2Offset;
	u8 global_alpha;

	/* Mixed mode routes stream1 from live input and stream2 from memory DMA. */
	XDcSub_SetVidInterfaceMode(DcSubPtr, XDC_VID_FUNCTIONAL);
	XDcSub_SetVidStreamSrc(DcSubPtr, XDC_VIDSTREAM1_LIVE,
			XDC_VIDSTREAM2_NONLIVE);
	XDcSub_SetInputAudioSelect(DcSubPtr, XDC_AUDSTREAM_NONE);
	XDcSub_SetInputLiveStreamFormat(DcSubPtr, RunCfgPtr->Stream1Format,
			RunCfgPtr->Stream1Format);
	XDcSub_SetInputNonLiveVideoFormat(DcSubPtr, RunCfgPtr->Stream2Format,
			RunCfgPtr->Stream2Format);
	XDcSub_SetOutputVideoFormat(DcSubPtr, RunCfgPtr->OutStreamFormat);
	XDcSub_SetInputStreamLayerControl(DcSubPtr, 0, 0);
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE) {
		global_alpha = 0x0;
	} else if (RunCfgPtr->Stream2PbEnable == PB_ENABLE) {
		global_alpha = 0xFF;
	} else {
		global_alpha = 0xA5;
	}
	XDcSub_SetGlobalAlpha(DcSubPtr, ALPHA_ENABLE, global_alpha);
	XDcSub_SetStreamPixelScaling(DcSubPtr, NULL, NULL);
	/* Old mixed flow uses frame switch 0x3F; keep same behavior. */
	XDcSub_SetVidFrameSwitch(DcSubPtr, 0x3F);

	if (Vid1 == NULL || Vid1->IsRGB == TRUE) {
		In1Coeff = CSCCoeff_RGB;
		In1Offset = CSCOffset_RGB;
	} else {
		In1Coeff = In_CSCCoeff_YUV;
		In1Offset = In_CSCOffset_YUV;
	}

	if (Vid2 == NULL || Vid2->IsRGB == TRUE) {
		In2Coeff = CSCCoeff_RGB;
		In2Offset = CSCOffset_RGB;
	} else {
		In2Coeff = In_CSCCoeff_YUV;
		In2Offset = In_CSCOffset_YUV;
	}

	XDcSub_SetInputStreamCSC(DcSubPtr, In1Coeff, In1Offset, In2Coeff, In2Offset);
	if (RunCfgPtr->Stream1PbEnable == PB_ENABLE ||
			RunCfgPtr->Stream2PbEnable == PB_ENABLE) {
		/* Reapply partial blend after mixed stream-source override. */
		XDpDc_ConfigurePartialPlaneBlend(RunCfgPtr);
	}
	XDcSub_EnableStream1Buffers(DcSubPtr, 0, 15);
	XDcSub_EnableStream2Buffers(DcSubPtr, 1, 15);
}

/*****************************************************************************/
/**
 *
 * This function programs the AVPG0 live video pattern generator for
 * mixed mode. Only AVPG0 is used because mixed mode routes the V01
 * live source to Stream1.
 *
 * @param    RunCfgPtr - Pointer to the RunConfig structure containing
 *           the AVPG configuration, width, and height.
 *
 * @return   None.
 *
 ******************************************************************************/
static void XDpDc_MixedConfigureLiveGeneratorV01(RunConfig *RunCfgPtr)
{
	/*
	 * Mixed mode uses V01 live source, so only AVPG0 is programmed.
	 */
	XAvpgSetConfig(AVPG_0_BASEADDR, &RunCfgPtr->avpg[0], RunCfgPtr->Width,
			RunCfgPtr->Height);
	XAvpg_WriteReg(AVPG_0_BASEADDR, XAV_PATGEN_EN, 0x1);
}

/*****************************************************************************/
/**
 *
 * This function runs the mixed mode display test. Stream1 is driven by a
 * live AVPG pattern generator and Stream2 is driven by a non-live DMA
 * frame buffer. The two streams are alpha-blended and output through the
 * DisplayPort transmitter.
 *
 * @param    RunCfgPtr - Pointer to the RunConfig structure. The
 *           operatingmode must be XDCSUB_OPMODE_FUNCTIONAL and
 *           presentationmode must be XDCSUB_PPTMODE_MIXED.
 *
 * @return   XST_SUCCESS if the test completes successfully.
 *           XST_INVALID_PARAM if the operating or presentation mode is
 *           incorrect.
 *           XST_FAILURE or other error code on subsystem init failure.
 *
 * @note     Cursor and SDP are disabled for the mixed test to avoid
 *           conflicts on shared resources.
 *
 ******************************************************************************/
u32 XDpDc_MmiDcMixedTest(RunConfig *RunCfgPtr)
{
	u32 Status;

	if (RunCfgPtr->operatingmode != XDCSUB_OPMODE_FUNCTIONAL ||
			RunCfgPtr->presentationmode != XDCSUB_PPTMODE_MIXED) {
		xil_printf("%s: Invalid config\r\n", __func__);
		return XST_INVALID_PARAM;
	}

	RunCfgPtr->avpg[1] = RunCfgPtr->avpg[0];
	RunCfgPtr->livevidselect = XDCSUB_LIVVID_SEL_V01;
	XDpDc_MixedSetLiveStream1Format(RunCfgPtr);

	/* Mixed test disables cursor/SDP on shared resources. */
	RunCfgPtr->CursorEnable = CB_DISABLE;
	RunCfgPtr->SdpEnable = 0U;

	xil_printf("\r\n========================================\r\n");
	xil_printf("DisplayPort DC Mixed Video Test\r\n");
	xil_printf("  Stream1: LIVE (AVPG)\r\n");
	xil_printf("  Stream2: NON-LIVE (DMA)\r\n");
	xil_printf("========================================\r\n\r\n");

	Status = XDpDc_InitializeRunConfig(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to get mixed test configuration\r\n");
		return Status;
	}
	XDpDc_ConfigureMixedDcSubPtr(RunCfgPtr);

	xil_printf("  Configuring clock wizard...\r\n");
	Status = XDpDc_InitClkWiz(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize Clock Wizard\r\n");
		return Status;
	}

	xil_printf("  Configuring DC subsystem...\r\n");
	Status = XDpDc_InitDcSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to initialize DC Subsystem\r\n");
		return Status;
	}

	XDc_WriteReg(LIVE_IN_GPIO_BASEADDR, 0x0, 0x00010000);
	/* Keep VTC PPC in sync with AVPG PPC (2 PPC => divide width by 2). */
	RunCfgPtr->PPC = RunCfgPtr->avpg[0].ppc ? 4 : 2;
	InitVtcSubsystem(RunCfgPtr, &Vtc0, VTC_BASEADDR);
	XVtc_WriteReg(VTC_BASEADDR, 0x0, 0x03f7ef06);
	XDpDc_MixedConfigureLiveGeneratorV01(RunCfgPtr);
	XDc_WriteReg(LIVE_IN_GPIO_BASEADDR, 0x0, 0x00010001);

	xil_printf("  Enabling DisplayPort output...\r\n");
	Status = XDpDc_InitDpPsuSubsystem(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: DisplayPort subsystem initialization failed\r\n");
		return Status;
	}
	XDpDc_SetupInterrupts(RunCfgPtr);

	return XST_SUCCESS;
}
#endif /* defined (XPAR_XVTC_NUM_INSTANCES) */