/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dp_init.c
* @brief Implements DisplayPort initialization, sink control, and training setup.
*
******************************************************************************/

#include <stdio.h>
#include <xil_exception.h>
#include <xil_cache.h>
#include <xil_printf.h>
#include <xil_assert.h>
#include <sleep.h>

#include "mmi_dpdc_platform.h"
#include "xmmidp.h"
#include "mmi_dp_init.h"
#include "mmi_dp_intr.h"

/* AUX timer limits (counts at core clock rate) */
#define XMMIDP_AUX_250US_LIMIT		0xF5
#define XMMIDP_AUX_2000US_LIMIT		0x7A8
#define XMMIDP_AUX_100000US_LIMIT	0x17ED0

/* Power Management configuration defaults */
#define XMMIDP_PM_CONFIG1_DEFAULT	0x00400008
#define XMMIDP_PM_CONFIG2_DEFAULT	0x00050000

/* General interrupt enable: HPD event + AUX reply + AUX cmd invalid */
#define XMMIDP_GEN_INT_EN_DEFAULT	(XMMIDP_GEN_INT_HPD_EVENT_EN_MASK | \
					 XMMIDP_GEN_INT_AUX_REPLY_EVENT_EN_MASK | \
					 XMMIDP_GEN_INT_AUX_CMD_INVALID_EN_MASK)

/* HPD interrupt enable: IRQ + plug + unplug + unplug error */
#define XMMIDP_HPD_INT_EN_DEFAULT	(XMMIDP_HPD_IRQ_EN_MASK | \
					 XMMIDP_HPD_PLUG_EN_MASK | \
					 XMMIDP_HPD_UNPLUG_EN_MASK | \
					 XMMIDP_HPD_UNPLUG_ERR_EN_MASK)

/* Sink DPCD SET_POWER values */
#define XMMIDP_SINK_POWER_DOWN		0x0
#define XMMIDP_SINK_POWER_UP		0x1

/* Sink power cycle delays (seconds) */
#define XMMIDP_SINK_POWERDOWN_DELAY	10
#define XMMIDP_SINK_POWERUP_DELAY	5

/* Default video stream parameters */
#define XMMIDP_DEFAULT_BPC		8
#define XMMIDP_VID_STREAM_ENABLE	0x1

/*****************************************************************************/
/**
 * Map pixels-per-clock (from xparameters.h) to DP PixModeSel register value.
 *
 * @param	PPC is 1, 2, or 4 pixels per clock.
 *
 * @return	XMMIDP_SINGLE/DUAL/QUAD_PIX_MODE.
 *
 *****************************************************************************/
static u8 XDpDc_PpcToPixModeSel(u8 Ppc)
{
	if ((Ppc != 1U) && (Ppc != 2U) && (Ppc != 4U)) {
		xil_printf("[VID] WARNING: PPC unset or invalid (%u), expected 1, 2, or 4\r\n",
			   Ppc);
	}
	Xil_AssertNonvoid((Ppc == 1U) || (Ppc == 2U) || (Ppc == 4U));

	switch (Ppc) {
	case 4:
		return XMMIDP_QUAD_PIX_MODE;
	case 2:
		return XMMIDP_DUAL_PIX_MODE;
	default:
		return XMMIDP_SINGLE_PIX_MODE;
	}
}

/*****************************************************************************/
/**
 * Map the AVPG pixel format and bpc to the DC video format.
 *
 * @param	Avpg is the AVPG configuration of the stream.
 *
 * @return	DC video format for the stream.
 *
 *****************************************************************************/
static XDc_VideoFormat XDpDc_AvpgToVideoFormat(const AvpgRunConfig *Avpg)
{
	if (Avpg->pix_fmt == 0U) {
		switch (Avpg->bpc) {
		case 10:
			return RGB_10BPC;
		case 12:
			return RGB_12BPC;
		case 8:
			return RGB_8BPC;
		default:
			xil_printf("[VID] WARNING: Unsupported AVPG bpc %u "
				   "(pix_fmt=RGB), using 8\r\n", Avpg->bpc);
			return RGB_8BPC;
		}
	}

	if (Avpg->pix_fmt != 1U)
		xil_printf("[VID] WARNING: Unsupported AVPG pix_fmt %u, "
			   "treating as YCbCr422\r\n", Avpg->pix_fmt);

	switch (Avpg->bpc) {
	case 10:
		return YCbCr422_10BPC;
	case 12:
		return YCbCr422_12BPC;
	case 8:
		return YCbCr422_8BPC;
	default:
		xil_printf("[VID] WARNING: Unsupported AVPG bpc %u "
			   "(pix_fmt=YCbCr422), using 8\r\n", Avpg->bpc);
		return YCbCr422_8BPC;
	}
}

/*****************************************************************************/
/**
 * Select the video format that DP MSA should describe on the wire for a
 * given DP stream.
 *
 * SST-only (DesignMstEn=0): stream 0 only — bypass keeps Stream1Format
 * (set from avpg[0] by BypassSetStream0Format), else OutStreamFormat.
 * MST (DesignMstEn=1): bypass uses avpg[StreamIdx], else OutStreamFormat.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 * @param	StreamId is the DP stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	XDc_VideoFormat for DP stream setup.
 *
 *****************************************************************************/
static XDc_VideoFormat XDpDc_GetDpVideoFormat(const RunConfig *RunCfgPtr,
					      u8 StreamId)
{
	if (!RunCfgPtr->DesignMstEn) {
		if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_BYPASS)
			return RunCfgPtr->Stream1Format;

		return RunCfgPtr->OutStreamFormat;
	}

	if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_BYPASS) {
		u8 StreamIdx = StreamId - XMMIDP_STREAM_ID1;

		return XDpDc_AvpgToVideoFormat(&RunCfgPtr->avpg[StreamIdx]);
	}

	return RunCfgPtr->OutStreamFormat;
}

/*****************************************************************************/
/**
 * Map XDc_VideoFormat to MSA bits-per-color.
 *
 * @param	Fmt is the DC/AVPG video format.
 *
 * @return	BPC value for XMmiDp_SetMsaBpc().
 *
 *****************************************************************************/
static u8 XDpDc_VideoFormatToBpc(XDc_VideoFormat Fmt)
{
	switch (Fmt) {
	case RGB_6BPC:
	case YCbCr444_6BPC:
		return 6;
	case RGB_10BPC:
	case YCbCr444_10BPC:
	case YCbCr422_10BPC:
		return 10;
	case RGB_12BPC:
	case YCbCr444_12BPC:
	case YCbCr422_12BPC:
		return 12;
	default:
		return 8;
	}
}

/*****************************************************************************/
/**
 * Map XDc_VideoFormat to DP video input mapping.
 *
 * @param	Fmt is the DC/AVPG video format.
 *
 * @return	XMmiDp_VidMap value for XMmiDp_SetVideoMapping().
 *
 *****************************************************************************/
static XMmiDp_VidMap XDpDc_VideoFormatToVidMap(XDc_VideoFormat Fmt)
{
	switch (Fmt) {
	case RGB_6BPC:
		return XMMIDP_RGB_6BPC;
	case RGB_10BPC:
		return XMMIDP_RGB_10BPC;
	case RGB_12BPC:
		return XMMIDP_RGB_12BPC;
	case YCbCr444_8BPC:
		return XMMIDP_YCbCr444_8BPC;
	case YCbCr444_10BPC:
		return XMMIDP_YCbCr444_10BPC;
	case YCbCr444_12BPC:
		return XMMIDP_YCbCr444_12BPC;
	case YCbCr422_8BPC:
		return XMMIDP_YCbCr422_8BPC;
	case YCbCr422_10BPC:
		return XMMIDP_YCbCr422_10BPC;
	case YCbCr422_12BPC:
		return XMMIDP_YCbCr422_12BPC;
	default:
		return XMMIDP_RGB_8BPC;
	}
}

/*****************************************************************************/
/**
 * Map XDc_VideoFormat to XVidC color space for MSA misc/timing computation.
 *
 * @param	Fmt is the DC/AVPG video format.
 *
 * @return	XVidC_ColorFormat for XMmiDp_SetVidControllerUseStdVidMode().
 *
 *****************************************************************************/
static XVidC_ColorFormat XDpDc_VideoFormatToColorFormat(XDc_VideoFormat Fmt)
{
	switch (Fmt) {
	case YCbCr422_8BPC:
	case YCbCr422_10BPC:
	case YCbCr422_12BPC:
		return XVIDC_CSF_YCRCB_422;
	case YCbCr444_6BPC:
	case YCbCr444_8BPC:
	case YCbCr444_10BPC:
	case YCbCr444_12BPC:
		return XVIDC_CSF_YCRCB_444;
	default:
		return XVIDC_CSF_RGB;
	}
}

/* Default audio stream parameters */
#define XMMIDP_AUD_CH_PER_DATA_INPUT	2U
#define XMMIDP_AUD_DATA_INPUT_LSB	0x1U
#define XMMIDP_AUD_DATA_INPUT_ALL	0xF
#define XMMIDP_AUD_HBR_MODE_DISABLE	0x0
#define XMMIDP_AUD_TIMESTAMP_VER	0x12
#define XMMIDP_SDP_STREAM_ENABLE	0x1
#define XMMIDP_SDP_FIXED_PRIORITY_EN	0x1

/******************************************************************************/
/**
 * Power-cycle the sink: SET_POWER down, then up.
 *
 * On some panels the down/up transition re-asserts HPD and can re-enter the
 * hot-plug handler. Wake delay is unchanged from SinkPowerUp alone
 * (XMMIDP_SINK_POWERUP_DELAY after power-up); only the power-down phase is
 * added relative to a bare SinkPowerUp path.
 *
 * @param	DpPsuPtr is a pointer to the XMmiDp instance.
 *
 ******************************************************************************/
void XMmiDp_SinkPowerCycle(XMmiDp *DpPsuPtr)
{
	XMmiDp_SinkPowerDown(DpPsuPtr);
	xil_printf("  Putting monitor in power saving mode...\r\n");
	sleep(XMMIDP_SINK_POWERDOWN_DELAY);

	XMmiDp_SinkPowerUp(DpPsuPtr);
	xil_printf("  Waking up monitor...\r\n");
	sleep(XMMIDP_SINK_POWERUP_DELAY);

}

/******************************************************************************/
/**
 * This function puts the sink into power saving mode by writing 0x0 to the
 * DPCD SET_POWER register.
 *
 * @param	DpPsuPtr is a pointer to the XMmiDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XMmiDp_SinkPowerDown(XMmiDp *DpPsuPtr)
{
	u8 Data[8];

	Data[0] = XMMIDP_SINK_POWER_DOWN;

	XMmiDp_AuxWrite(DpPsuPtr,
			XMMIDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

/******************************************************************************/
/**
 * This function wakes the sink from power saving mode by writing 0x1 to the
 * DPCD SET_POWER register.
 *
 * @param	DpPsuPtr is a pointer to the XMmiDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XMmiDp_SinkPowerUp(XMmiDp *DpPsuPtr)
{
	u8 Data[8];

	Data[0] = XMMIDP_SINK_POWER_UP;

	XMmiDp_AuxWrite(DpPsuPtr,
			XMMIDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

static void XDpDc_PrintVideoConfigRegs(XMmiDp *InstancePtr, u8 Stream)
{
	UINTPTR Base = InstancePtr->Config.BaseAddr;
	u32 Off = (Stream - 1) * 0x10000;
	u32 Reg;

	xil_printf("\r\n--------------------------------------------------\r\n");
	xil_printf("  DP Video Config Registers  (Stream %d)\r\n", Stream);
	xil_printf("--------------------------------------------------\r\n");

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VSAMPLE_CTRL + Off);
	xil_printf("  VSAMPLE_CTRL      [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VSAMPLE_CTRL + Off, Reg);
	xil_printf("    PixModeSel      : %d\r\n",
		   (Reg & XMMIDP_VSAMPLE_CTRL_PIXEL_MODE_SELECT_MASK) >>
		   XMMIDP_VSAMPLE_CTRL_PIXEL_MODE_SELECT_SHIFT);
	xil_printf("    VideoMapping    : 0x%X\r\n",
		   (Reg & XMMIDP_VSAMPLE_CTRL_VIDEO_MAPPING_MASK) >>
		   XMMIDP_VSAMPLE_CTRL_VIDEO_MAPPING_SHIFT);
	xil_printf("    StreamEn        : %d\r\n",
		   (Reg & XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_MASK) >>
		   XMMIDP_VSAMPLE_CTRL_VIDEO_STREAM_EN_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VINPUT_POLARITY_CTRL + Off);
	xil_printf("  POLARITY_CTRL     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VINPUT_POLARITY_CTRL + Off, Reg);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_CONFIG1 + Off);
	xil_printf("  VIDEO_CONFIG1     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_CONFIG1 + Off, Reg);
	xil_printf("    HActive         : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG1_HACTIVE_MASK) >>
		   XMMIDP_VIDEO_CONFIG1_HACTIVE_SHIFT);
	xil_printf("    HBlank          : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG1_HBLANK_MASK) >>
		   XMMIDP_VIDEO_CONFIG1_HBLANK_SHIFT);
	xil_printf("    I/P             : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG1_I_P_MASK) >>
		   XMMIDP_VIDEO_CONFIG1_I_P_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_CONFIG2 + Off);
	xil_printf("  VIDEO_CONFIG2     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_CONFIG2 + Off, Reg);
	xil_printf("    VBlank          : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG2_VBLANK_MASK) >>
		   XMMIDP_VIDEO_CONFIG2_VBLANK_SHIFT);
	xil_printf("    VActive(cfg2)   : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG2_HACTIVE_MASK) >>
		   XMMIDP_VIDEO_CONFIG2_HACTIVE_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_CONFIG3 + Off);
	xil_printf("  VIDEO_CONFIG3     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_CONFIG3 + Off, Reg);
	xil_printf("    HSyncWidth      : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG3_HSYNC_WIDTH_MASK) >>
		   XMMIDP_VIDEO_CONFIG3_HSYNC_WIDTH_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_CONFIG4 + Off);
	xil_printf("  VIDEO_CONFIG4     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_CONFIG4 + Off, Reg);
	xil_printf("    VSyncWidth      : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG4_VSYNC_WIDTH_MASK) >>
		   XMMIDP_VIDEO_CONFIG4_VSYNC_WIDTH_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_CONFIG5 + Off);
	xil_printf("  VIDEO_CONFIG5     [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_CONFIG5 + Off, Reg);
	xil_printf("    AvgBytesPerTU   : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_MASK) >>
		   XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_SHIFT);
	xil_printf("    AvgBytesFrac    : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_FRAC_MASK) >>
		   XMMIDP_VIDEO_CONFIG5_AVG_BYTES_PER_TU_FRAC_SHIFT);
	xil_printf("    InitThreshold   : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_MASK) >>
		   XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_SHIFT);
	xil_printf("    InitThresholdHi : %d\r\n",
		   (Reg & XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_HI_MASK) >>
		   XMMIDP_VIDEO_CONFIG5_INIT_THRESHOLD_HI_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_MSA1 + Off);
	xil_printf("  VIDEO_MSA1        [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_MSA1 + Off, Reg);
	xil_printf("    VStart          : %d\r\n",
		   (Reg & XMMIDP_VIDEO_MSA1_VSTART_MASK) >>
		   XMMIDP_VIDEO_MSA1_VSTART_SHIFT);
	xil_printf("    HStart          : %d\r\n",
		   (Reg & XMMIDP_VIDEO_MSA1_HSTART_MASK) >>
		   XMMIDP_VIDEO_MSA1_HSTART_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_MSA2 + Off);
	xil_printf("  VIDEO_MSA2        [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_MSA2 + Off, Reg);
	xil_printf("    Misc0           : 0x%02X\r\n",
		   (Reg & XMMIDP_VIDEO_MSA2_MISC0_MASK) >>
		   XMMIDP_VIDEO_MSA2_MISC0_SHIFT);
	xil_printf("    MVid            : %d\r\n",
		   (Reg & XMMIDP_VIDEO_MSA2_MVID_MASK) >>
		   XMMIDP_VIDEO_MSA2_MVID_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_MSA3 + Off);
	xil_printf("  VIDEO_MSA3        [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_MSA3 + Off, Reg);
	xil_printf("    Misc1           : 0x%02X\r\n",
		   (Reg & XMMIDP_VIDEO_MSA3_MISC1_MASK) >>
		   XMMIDP_VIDEO_MSA3_MISC1_SHIFT);
	xil_printf("    NVid            : %d\r\n",
		   (Reg & XMMIDP_VIDEO_MSA3_NVID_MASK) >>
		   XMMIDP_VIDEO_MSA3_NVID_SHIFT);

	Reg = XMmiDp_ReadReg(Base, XMMIDP_VIDEO_HBLANK_INTERVAL + Off);
	xil_printf("  HBLANK_INTERVAL   [0x%03X] = 0x%08X\r\n",
		   XMMIDP_VIDEO_HBLANK_INTERVAL + Off, Reg);
	xil_printf("    HBlankInterval  : %d\r\n",
		   (Reg & XMMIDP_VIDEO_HBLANK_INTERVAL_MASK) >>
		   XMMIDP_VIDEO_HBLANK_INTERVAL_SHIFT);

	xil_printf("--------------------------------------------------\r\n\r\n");
}

/******************************************************************************/
/**
 * This function prints the video stream, MSA, link, and TU configuration
 * from InstancePtr after XMmiDp_SetVidControllerUseStdVidMode has populated
 * all the fields.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	None.
 *
*******************************************************************************/
static void XDpDc_PrintVideoStreamConfig(XMmiDp *InstancePtr, u8 Stream)
{
	XMmiDp_VideoConfig *Vc = &InstancePtr->VideoConfig[Stream - 1];
	XMmiDp_MainStreamAttributes *Msa = &InstancePtr->MsaConfig[Stream - 1];
	XMmiDp_LinkConfig *Lc = &InstancePtr->LinkConfig;

	xil_printf("\r\n  Video Stream Config (Stream %d):\r\n", Stream);
	xil_printf("    Resolution        : %dx%d\r\n",
		   Vc->HActive, Vc->VActive);
	xil_printf("    Pixel Clock       : %llu Hz\r\n", Msa->PixelClockHz);
	xil_printf("    BitsPerColor      : %d\r\n", Msa->BitsPerColor);
	xil_printf("    HBlank            : %d\r\n", Vc->HBlank);
	xil_printf("    VBlank            : %d\r\n", Vc->VBlank);
	xil_printf("    HStart            : %d\r\n", Msa->HStart);
	xil_printf("    VStart            : %d\r\n", Msa->VStart);
	xil_printf("    HSyncWidth        : %d\r\n", Vc->HSyncWidth);
	xil_printf("    VSyncWidth        : %d\r\n", Vc->VSyncWidth);
	xil_printf("    Link Lanes        : %d (LaneCount reg=%d)\r\n",
		   Lc->NumLanes, Lc->LaneCount);
	xil_printf("    Link Rate         : LinkBW=0x%02X\r\n", Lc->LinkBW);
	xil_printf("    Misc0             : 0x%02X\r\n", Msa->Misc0);
	xil_printf("    Misc1             : 0x%02X\r\n", Msa->Misc1);
	xil_printf("    MVid              : %d (async)\r\n", Msa->MVid);
	xil_printf("    NVid              : 0x%04X\r\n", Msa->NVid);
	xil_printf("    HBlankInterval    : %d\r\n", Msa->HBlankInterval);
	xil_printf("    AvgBytesPerTu     : %d\r\n", Vc->AvgBytesPerTu);
	xil_printf("    AvgBytesPerTuFrac : %d/64\r\n", Vc->AvgBytesPerTuFrac);
	xil_printf("    InitThreshold     : lo=%d  hi=%d\r\n",
		   Vc->InitThreshold, Vc->InitThresholdHi);
}


/******************************************************************************/
/**
 * Read SDT xlnx,mst-mode-en for the DP Tx instance (1=MST design, 0=SST-only).
 *
 * @return	1 if the design is MST-capable, 0 if SST-only.
 *
 ******************************************************************************/
u32 XDpDc_GetDesignMstEn(void)
{
	u32 DesignMstEn;

#if defined(XPAR_MMI_DPTX_MST_MODE_EN)
	DesignMstEn = (u32)XPAR_MMI_DPTX_MST_MODE_EN;
	xil_printf("[MSTM] DesignMstEn=%u (SDT via XPAR_MMI_DPTX_MST_MODE_EN)\r\n",
		   DesignMstEn);
#elif defined(XPAR_XMMIDP_0_MST_MODE_EN)
	DesignMstEn = (u32)XPAR_XMMIDP_0_MST_MODE_EN;
	xil_printf("[MSTM] DesignMstEn=%u (SDT via XPAR_XMMIDP_0_MST_MODE_EN)\r\n",
		   DesignMstEn);
#elif defined(XPAR_XMMIDP_NUM_INSTANCES) && (XPAR_XMMIDP_NUM_INSTANCES > 0)
	extern XMmiDp_Config XMmiDp_ConfigTable[];

	DesignMstEn = (u32)XMmiDp_ConfigTable[0].MstModeEn;
	xil_printf("[MSTM] DesignMstEn=%u (SDT via XMmiDp_ConfigTable[0].MstModeEn)\r\n",
		   DesignMstEn);
#else
	DesignMstEn = 0U;
	xil_printf("[MSTM] DesignMstEn=0 (default, no SDT MST define)\r\n");
#endif

	return DesignMstEn;
}

/******************************************************************************/
/**
 * Set RunCfgPtr->MstMode from SDT mst-mode-en and sink MST_CAP.
 *
 * SST-only design (mst-mode-en=0): MstMode=0 always.
 * MST design (mst-mode-en=1): MstMode=1 if MST_CAP=1, else connect fails.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	XST_SUCCESS if policy is valid, XST_FAILURE otherwise.
 *
*******************************************************************************/
u32 XDpDc_CheckMstSupport(RunConfig *RunCfgPtr)
{
	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;

	RunCfgPtr->MstMode = 0U;

	xil_printf("[MSTM] CheckMstSupport: DesignMstEn=%u byp_streams=%u\r\n",
		   RunCfgPtr->DesignMstEn, RunCfgPtr->byp_streams);

	if (!RunCfgPtr->DesignMstEn) {
		xil_printf("[MSTM] Policy: SST-only design -> MstMode=0 (SST link)\r\n");
		return XST_SUCCESS;
	}

	xil_printf("[MSTM] Policy: MST design, reading sink MST_CAP (DPCD 0x21)...\r\n");
	if (XMmiDp_GetRxMstModeCap(InstancePtr) != XST_SUCCESS) {
		xil_printf("[MSTM] ERROR: Failed to read MST_CAP. "
			   "Connect an MST sink.\r\n");
		return XST_FAILURE;
	}

	xil_printf("[MSTM] Policy: sink MST_CAP=%u\r\n",
		   InstancePtr->RxConfig.MstCap);

	if (InstancePtr->RxConfig.MstCap == 0U) {
		xil_printf("[MSTM] ERROR: MST design requires an MST-capable sink. "
			   "Connect an MST sink.\r\n");
		return XST_FAILURE;
	}

	RunCfgPtr->MstMode = 1U;
	xil_printf("[MSTM] Policy: MST design + MST_CAP=1 -> MstMode=1 "
		   "(MST link, byp_streams=%u)\r\n", RunCfgPtr->byp_streams);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function sets up one video stream: MSA bits per color, pixel mode,
 * video mapping, and standard video mode timing from RunConfig.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 * @param	StreamId is the DP stream ID (XMMIDP_STREAM_ID1..ID4).
 *
 * @return	None.
 *
 * @note	Caller must clear the video controller before the first stream.
 *
*******************************************************************************/
void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr, u8 StreamId)
{
	XMmiDp *InstancePtr;
	XDc_VideoFormat DpFormat;
	u8 Bpc;
	XMmiDp_VidMap VidMap;
	XVidC_ColorFormat ColorFormat;
	u8 StreamIdx;
	u8 Ppc;

	Xil_AssertVoid(RunCfgPtr != NULL);
	Xil_AssertVoid((StreamId >= XMMIDP_STREAM_ID1) &&
		       (StreamId <= XMMIDP_STREAM_ID4));

	StreamIdx = StreamId - XMMIDP_STREAM_ID1;
	InstancePtr = RunCfgPtr->DpPsuPtr;
	Ppc = RunCfgPtr->PPC;

	/* Per stream Bpc, Colorformat */
	DpFormat = XDpDc_GetDpVideoFormat(RunCfgPtr, StreamId);
	Bpc = XDpDc_VideoFormatToBpc(DpFormat);
	VidMap = XDpDc_VideoFormatToVidMap(DpFormat);
	ColorFormat = XDpDc_VideoFormatToColorFormat(DpFormat);

	/* Per Stream PPC */
	if (RunCfgPtr->operatingmode == XDCSUB_OPMODE_BYPASS &&
	    RunCfgPtr->byp_stream_pix_mode[StreamIdx])
		Ppc = RunCfgPtr->byp_stream_pix_mode[StreamIdx];

	XMmiDp_DisableVideoStream(InstancePtr, StreamId);

	xil_printf("[MSTM] SetControllerMstModeEn: stream=%u MstModeEn=%u "
		   "(TU/PBN path)\r\n", StreamId, RunCfgPtr->MstMode);

	/* Setup VideoConfig Structures */
	XMmiDp_SetControllerMstModeEn(InstancePtr, StreamId, RunCfgPtr->MstMode);

	XMmiDp_SetMsaBpc(InstancePtr, StreamId, Bpc);

	/*
	 * The DC drives DP at 1 pixel/clock in every functional mode; PPC
	 * there is the PL input width. Only bypass feeds the PL stream to DP.
	 */
	XMmiDp_SetPixModeSel(InstancePtr, StreamId,
			     (RunCfgPtr->operatingmode == XDCSUB_OPMODE_BYPASS) ?
			     XDpDc_PpcToPixModeSel(Ppc) :
			     XMMIDP_SINGLE_PIX_MODE);
	XMmiDp_SetVideoMapping(InstancePtr, StreamId, VidMap);

	XMmiDp_SetVidControllerUseStdVidMode(InstancePtr,
					     RunCfgPtr->VideoMode,
					     StreamId,
					     ColorFormat);

	XDpDc_PrintVideoStreamConfig(InstancePtr, StreamId);

	/* Write to Video Config registers */
	XMmiDp_ConfigureVideoController(InstancePtr, StreamId);

	/* Enable video stream after all config registers are written */
	XMmiDp_EnableVideoStream(InstancePtr, StreamId);

	XDpDc_PrintVideoConfigRegs(InstancePtr, StreamId);
}

/******************************************************************************/
/**
 * This function configures the audio stream by setting the audio interface,
 * data input, data width, channel count, mute flag, timestamp, clock
 * multiplier, and enabling SDP vertical/horizontal audio streams.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XMmiDp_SetupAudioStream(RunConfig *RunCfgPtr)
{
	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;
	u8 ActiveDataInput;
	u8 EnabledDataInputs;
	u8 NumChannels;

	NumChannels = RunCfgPtr->AudioChannels;

	/*
	 * Enable one I2S data input per stereo pair and expand to contiguous mask.
	 * Example: 1 input -> 0x1, 2 inputs -> 0x3, 3 inputs -> 0x7, 4 inputs -> 0xF.
	 */
	EnabledDataInputs = (NumChannels + (XMMIDP_AUD_CH_PER_DATA_INPUT - 1U)) /
			    XMMIDP_AUD_CH_PER_DATA_INPUT;
	ActiveDataInput = (u8)((XMMIDP_AUD_DATA_INPUT_LSB << EnabledDataInputs) -
			       XMMIDP_AUD_DATA_INPUT_LSB);
	if (ActiveDataInput > XMMIDP_AUD_DATA_INPUT_ALL)
		ActiveDataInput = XMMIDP_AUD_DATA_INPUT_ALL;

	XMmiDp_SetAudStreamInterfaceSel(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUD_INF_I2S);
	XMmiDp_SetAudDataInputEn(InstancePtr, XMMIDP_STREAM_ID1, ActiveDataInput);
	XMmiDp_SetAudDataWidth(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUDIO_INPUT_16_BIT);
	XMmiDp_SetAudHbrModeEn(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUD_HBR_MODE_DISABLE);
	XMmiDp_SetAudNumChannels(InstancePtr, XMMIDP_STREAM_ID1, NumChannels - 1);
	XMmiDp_SetAudMuteFlag(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_CLEAR_AUDIOMUTE);
	XMmiDp_SetAudTimeStampVerNum(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUD_TIMESTAMP_VER);
	XMmiDp_SetAudClkMultFs(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUDIO_CLK_512FS);

	XMmiDp_SetSdpVertAudStreamEn(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_SDP_STREAM_ENABLE);
	XMmiDp_SetSdpVertAudTimeStampEn(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_SDP_STREAM_ENABLE);

	XMmiDp_SetSdpHorAudStreamEn(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_SDP_STREAM_ENABLE);
	XMmiDp_SetSdpHorAudTimeStampEn(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_SDP_STREAM_ENABLE);

	/* Enable fixed-priority arbitration for both vertical and horizontal
	 * SDP slots so audio + audio-timestamp packets get deterministic
	 * delivery timing. */
	XMmiDp_SetSdpVertFixedPriority(InstancePtr, XMMIDP_STREAM_ID1,
				       XMMIDP_SDP_FIXED_PRIORITY_EN);
	XMmiDp_SetSdpHorFixedPriority(InstancePtr, XMMIDP_STREAM_ID1,
				      XMMIDP_SDP_FIXED_PRIORITY_EN);

	XMmiDp_ConfigureAudioController(InstancePtr, XMMIDP_STREAM_ID1);
}

static const char *XDpDc_LinkBwToStr(u8 LinkBW, u32 *RateMbps)
{
	switch (LinkBW) {
	case XMMIDP_DPCD_LINK_BW_SET_162GBPS:
		*RateMbps = 162; return "RBR (1.62 Gbps)";
	case XMMIDP_DPCD_LINK_BW_SET_270GBPS:
		*RateMbps = 270; return "HBR (2.70 Gbps)";
	case XMMIDP_DPCD_LINK_BW_SET_540GBPS:
		*RateMbps = 540; return "HBR2 (5.40 Gbps)";
	case XMMIDP_DPCD_LINK_BW_SET_810GBPS:
		*RateMbps = 810; return "HBR3 (8.10 Gbps)";
	default:
		*RateMbps = 0; return "Unknown";
	}
}

static void XDpDc_PrintSinkCapabilities(XMmiDp *InstancePtr)
{
	u32 RateMbps;
	const char *RateStr;

	RateStr = XDpDc_LinkBwToStr(InstancePtr->RxConfig.MaxLinkBW, &RateMbps);

	xil_printf("  Sink Capabilities (DPCD):\r\n");
	xil_printf("    DPCD Rev          : %d.%d\r\n",
		   InstancePtr->RxConfig.DpcdRev >> 4,
		   InstancePtr->RxConfig.DpcdRev & 0xF);
	xil_printf("    Max Lane Count    : %d\r\n",
		   InstancePtr->RxConfig.MaxNumLanes);
	xil_printf("    Max Link Rate     : 0x%02X  %s\r\n",
		   InstancePtr->RxConfig.MaxLinkBW, RateStr);
	xil_printf("    Enhanced Frame    : %s\r\n",
		   InstancePtr->RxConfig.EnhancedFrameCap ? "Yes" : "No");
	xil_printf("    TPS3 Supported    : %s\r\n",
		   InstancePtr->RxConfig.Tps3Supported ? "Yes" : "No");
	xil_printf("    TPS4 Supported    : %s\r\n",
		   InstancePtr->RxConfig.Tps4Supported ? "Yes" : "No");
	xil_printf("    Max Downspread    : %s\r\n",
		   InstancePtr->RxConfig.MaxDownspread ? "0.5%%" : "None");
	xil_printf("    MST Capable       : %s\r\n",
		   InstancePtr->RxConfig.MstCap ? "Yes" : "No");
	xil_printf("    Ext Receiver Cap  : %s\r\n",
		   InstancePtr->RxConfig.ExtendedReceiverCap ? "Yes" : "No");
}

static void XDpDc_PrintLinkTrainingResult(XMmiDp *InstancePtr,
					   u32 TrainingStatus)
{
	const char *RateStr;
	u32 RateMbps;
	u32 i;

	RateStr = XDpDc_LinkBwToStr(InstancePtr->LinkConfig.LinkBW, &RateMbps);

	if (TrainingStatus != XST_SUCCESS) {
		xil_printf("  ERROR: Link training FAILED\r\n");
		xil_printf("    Final Lanes       : %d\r\n",
			   InstancePtr->LinkConfig.NumLanes);
		xil_printf("    Final Link BW     : 0x%02X\r\n",
			   InstancePtr->LinkConfig.LinkBW);
	} else {
		xil_printf("\r\n  Link Training SUCCESSFUL\r\n");
		xil_printf("  --------------------------------------------------\r\n");
		xil_printf("    Trained Lanes     : %d\r\n",
			   InstancePtr->LinkConfig.NumLanes);
		xil_printf("    Trained Link Rate : 0x%02X  %s\r\n",
			   InstancePtr->LinkConfig.LinkBW, RateStr);
		xil_printf("    Total BW          : %d x %d = %d Mbps\r\n",
			   InstancePtr->LinkConfig.NumLanes, RateMbps,
			   InstancePtr->LinkConfig.NumLanes * RateMbps);
	}

	for (i = 0; i < InstancePtr->LinkConfig.NumLanes; i++) {
		xil_printf("    Lane %d  VS=%d  PE=%d\r\n", i,
			   InstancePtr->LinkConfig.VsLevel[i],
			   InstancePtr->LinkConfig.PeLevel[i]);
	}

	if (TrainingStatus == XST_SUCCESS)
		xil_printf("  --------------------------------------------------\r\n");
}


/*****************************************************************************/
/**
 *
 * Link policy: capped full link training after sink is powered.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	XST_SUCCESS on success, XST_FAILURE otherwise.
 *
 ******************************************************************************/
u32 XDpDc_RunLinkPolicySetup(RunConfig *RunCfgPtr)
{
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;
	u32 Status;

	xil_printf("\r\n==================================================\r\n");
	xil_printf("  Link Training\r\n");
	xil_printf("==================================================\r\n");
	if (RunCfgPtr->MaxLaneCount != 0U) {
		xil_printf("  User max lanes     : %u\r\n",
			   RunCfgPtr->MaxLaneCount);
	}
	if (RunCfgPtr->MaxLinkRate != 0U) {
		xil_printf("  User max link BW   : 0x%02X\r\n",
			   RunCfgPtr->MaxLinkRate);
	}

	Status = XMmiDp_StartFullLinkTrainingCapped(DpPtr,
						    RunCfgPtr->MaxLaneCount,
						    RunCfgPtr->MaxLinkRate);
	XDpDc_PrintSinkCapabilities(DpPtr);
	XDpDc_PrintLinkTrainingResult(DpPtr, Status);

	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] ERROR: Link training failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * Stream policy: configure video and audio streams after link training.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	XST_SUCCESS if stream setup completed.
 *
 * @note	Requires a trained link (RunLinkPolicySetup must succeed first).
 *
 ******************************************************************************/
u32 XDpDc_RunStreamPolicySetup(RunConfig *RunCfgPtr)
{
	XMmiDp *DpPtr = RunCfgPtr->DpPsuPtr;
	u8 StreamId;
	u8 NumStreams;
	u32 Status;

	Status = XDpDc_CheckMstSupport(RunCfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] ERROR: Sink mode policy failed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Multi-stream setup is for MST bypass only. Functional mode and SST
	 * always configure a single DP stream. Clamp to XMMIDP_STREAM_ID4.
	 */
	if (RunCfgPtr->MstMode &&
	    RunCfgPtr->operatingmode == XDCSUB_OPMODE_BYPASS)
		NumStreams = RunCfgPtr->byp_streams;
	else
		NumStreams = 1U;

	if (NumStreams < 1U)
		NumStreams = 1U;
	if (NumStreams > XMMIDP_STREAM_ID4)
		NumStreams = XMMIDP_STREAM_ID4;

	xil_printf("[MSTM] RunStreamPolicySetup: MstMode=%u NumStreams=%u\r\n",
		   RunCfgPtr->MstMode, NumStreams);

	if (RunCfgPtr->MstMode) {
		xil_printf("[MSTM] Enabling MST on sink (DPCD) and TX (CCTL0)...\r\n");
		Status = XMmiDp_SinkMstModeEnable(DpPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("[HPD] ERROR: Failed to enable MST mode\r\n");
			return XST_FAILURE;
		}
	} else {
		xil_printf("[MSTM] SST link: skipping sink/TX MST enable\r\n");
	}

	XMmiDp_ClearVideoController(DpPtr);

	for (StreamId = XMMIDP_STREAM_ID1;
	     StreamId < NumStreams + XMMIDP_STREAM_ID1;
	     StreamId++)
		XMmiDp_SetupVideoStream(RunCfgPtr, StreamId);

	if (RunCfgPtr->AudioEnable)
		XMmiDp_SetupAudioStream(RunCfgPtr);
	else if (RunCfgPtr->SdpEnable == 0U)
		XDpDc_DisableDpAudioSdpStreams(DpPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Run the hot-plug connect sequence: sink power cycle, link policy, then
 * stream policy.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return
 *		- XST_SUCCESS if link and stream policy completed.
 *		- XST_FAILURE if link or stream policy setup failed.
 *
 ******************************************************************************/
u32 XDpDc_RunConnectSequence(RunConfig *RunCfgPtr)
{
	u32 Status;

	XMmiDp_SinkPowerCycle(RunCfgPtr->DpPsuPtr);

	Status = XDpDc_RunLinkPolicySetup(RunCfgPtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* DP video stream registers latch only after the DC leaves reset */
	Status = XDpDc_ReleaseDcReset();
	if (Status != XST_SUCCESS) {
		xil_printf("[HPD] ERROR: Failed to release DC block reset\r\n");
		return XST_FAILURE;
	}

	Status = XDpDc_RunStreamPolicySetup(RunCfgPtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	xil_printf("[HPD] Connect sequence complete\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function initializes the DP core by performing a PHY soft reset,
* powering on the PHY, configuring AUX timer limits, PM configuration,
* disabling SSC, and enabling general/HPD interrupts.
*
* @param        InstancePtr is a pointer to the XMmiDp instance.
*
* @return       XST_SUCCESS.
*
* @note         None.
*
******************************************************************************/
u32 XMmiDp_InitDpCore(XMmiDp *InstancePtr)
{
	XMmiDp_CoreSoftResetAll(InstancePtr);
	XMmiDp_PhySoftReset(InstancePtr);

	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_POWER_DOWN);
	sleep(1);
	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_INTER_P2_POWER);
	sleep(1);
	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_POWER_ON);
	XMmiDp_SetAux_250Us_Cnt_Limit(InstancePtr, XMMIDP_AUX_250US_LIMIT);
	XMmiDp_SetAux_2000Us_Cnt_Limit(InstancePtr, XMMIDP_AUX_2000US_LIMIT);
	XMmiDp_SetAux_100000Us_Cnt_Limit(InstancePtr, XMMIDP_AUX_100000US_LIMIT);
	XMmiDp_SetPmConfig1(InstancePtr, XMMIDP_PM_CONFIG1_DEFAULT);
	XMmiDp_SetPmConfig2(InstancePtr, XMMIDP_PM_CONFIG2_DEFAULT);
	XMmiDp_PhySSCDisable(InstancePtr);

	XMmiDp_GeneralInterruptEnable(InstancePtr, XMMIDP_GEN_INT_EN_DEFAULT);
	XMmiDp_HpdInterruptEnable(InstancePtr, XMMIDP_HPD_INT_EN_DEFAULT);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initializes the DisplayPort subsystem. It configures the
* DP core and prepares for asynchronous HPD-driven link training and
* video setup.
*
* @param        RunCfgPtr is a pointer to the application configuration structure.
*
* @return       XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note         None.
*
******************************************************************************/
u32 XDpDc_InitDpPsuSubsystem(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;
	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;
	InstancePtr->Config.BaseAddr = DP_BASEADDR;

	XMmiDp_CfgInitialize(InstancePtr, InstancePtr->Config.BaseAddr);
	XMmiDp_Initialize(InstancePtr);
	XMmiDp_InitDpCore(InstancePtr);

	/*
	 * Disable DP audio SDP/timestamp streams only when neither audio nor
	 * custom SDP mode is enabled. In SDP test mode, avoid touching this
	 * path because it can suppress custom SDP emission.
	 */
	if ((RunCfgPtr->AudioEnable == XDC_AUD_DISABLE) &&
	    (RunCfgPtr->SdpEnable == 0U)) {
		XDpDc_DisableDpAudioSdpStreams(InstancePtr);
	}

	InstancePtr->LinkConfig.LinkTrained = 0;

	return Status;
}
