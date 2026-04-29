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
#include <sleep.h>

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
#define XMMIDP_PIX_MODE_SINGLE		0x0
#define XMMIDP_VID_STREAM_ENABLE	0x1

/* Default audio stream parameters */
#define XMMIDP_AUD_CH_PER_DATA_INPUT	2U
#define XMMIDP_AUD_DATA_INPUT_LSB	0x1U
#define XMMIDP_AUD_DATA_INPUT_ALL	0xF
#define XMMIDP_AUD_HBR_MODE_DISABLE	0x0
#define XMMIDP_AUD_TIMESTAMP_VER	0x12
#define XMMIDP_SDP_STREAM_ENABLE	0x1

/******************************************************************************/
/**
 * This function power cycles the sink by powering it down and then back up
 * with appropriate delays to allow the monitor to transition states.
 *
 * @param	DpPsuPtr is a pointer to the XMmiDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
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
 * This function sets up the video stream by clearing the video controller,
 * configuring MSA bits per color, pixel mode, video mapping, and applying
 * the standard 640x480 video mode timing.
 *
 * @param	RunCfgPtr is a pointer to the application configuration structure.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr)
{

	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;

	/* Disable video stream before reconfiguring */
	XMmiDp_DisableVideoStream(InstancePtr, XMMIDP_STREAM_ID1);

	XMmiDp_ClearVideoController(InstancePtr);

	XMmiDp_SetMsaBpc(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_DEFAULT_BPC);
	XMmiDp_SetPixModeSel(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_PIX_MODE_SINGLE);
	XMmiDp_SetVideoMapping(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_RGB_8BPC);

	XMmiDp_SetVidControllerUseStdVidMode(InstancePtr,
					     RunCfgPtr->VideoMode,
					     XMMIDP_STREAM_ID1,
					     XVIDC_CSF_RGB);

	XDpDc_PrintVideoStreamConfig(InstancePtr, XMMIDP_STREAM_ID1);

	XMmiDp_ConfigureVideoController(InstancePtr, XMMIDP_STREAM_ID1);

	/* Enable video stream after all config registers are written */
	XMmiDp_EnableVideoStream(InstancePtr, XMMIDP_STREAM_ID1);

	XDpDc_PrintVideoConfigRegs(InstancePtr, XMMIDP_STREAM_ID1);
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

static void XDpDc_ApplyUserCaps(XMmiDp *InstancePtr,
				 u8 UserMaxLanes, u8 UserMaxLinkBW)
{
	if (UserMaxLanes != 0 &&
	    UserMaxLanes < InstancePtr->RxConfig.MaxNumLanes) {
		xil_printf("  User cap: limiting lanes from %d to %d\r\n",
			   InstancePtr->RxConfig.MaxNumLanes, UserMaxLanes);
		InstancePtr->RxConfig.MaxNumLanes = UserMaxLanes;
		InstancePtr->RxConfig.MaxLaneCount =
			XMmiDp_GetLaneCount(InstancePtr, UserMaxLanes);
	}

	if (UserMaxLinkBW != 0 &&
	    UserMaxLinkBW < InstancePtr->RxConfig.MaxLinkBW) {
		xil_printf("  User cap: limiting link BW from 0x%02X to 0x%02X\r\n",
			   InstancePtr->RxConfig.MaxLinkBW, UserMaxLinkBW);
		InstancePtr->RxConfig.MaxLinkBW = UserMaxLinkBW;
		InstancePtr->RxConfig.MaxLinkRate =
			XMmiDp_GetLinkRate(InstancePtr, UserMaxLinkBW);
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

/******************************************************************************/
/**
 * This function prepares for link training. It reads receiver capabilities,
 * applies user-configured lane/bandwidth caps, checks current link status,
 * configures link parameters, and runs the clock recovery and channel
 * equalization training sequence.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.

 * @param       UserMaxLanes User-configured max lane count (0=auto,
 *              1, 2, or 4).
 * @param       UserMaxLinkBW User-configured max link BW in DPCD format
 *              (0=auto, 0x06=RBR, 0x0A=HBR, 0x14=HBR2, 0x1E=HBR3).
 *
 * @return
 *              - XST_SUCCESS if the training process succeeded.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_StartFullLinkTrainingCapped(XMmiDp *InstancePtr,
				       u8 UserMaxLanes, u8 UserMaxLinkBW)
{
	u32 Status;
	const char *RateStr;
	u32 RateMbps;

	xil_printf("\r\n==================================================\r\n");
	xil_printf("  Link Training\r\n");
	xil_printf("==================================================\r\n");

	Status = XMmiDp_GetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to read monitor capabilities\r\n");
		return XST_FAILURE;
	}

	XDpDc_ApplyUserCaps(InstancePtr, UserMaxLanes, UserMaxLinkBW);

	RateStr = XDpDc_LinkBwToStr(InstancePtr->RxConfig.MaxLinkBW, &RateMbps);

	XDpDc_PrintSinkCapabilities(InstancePtr);

	InstancePtr->LinkConfig.LaneCount = InstancePtr->RxConfig.MaxLaneCount;
	InstancePtr->LinkConfig.NumLanes = InstancePtr->RxConfig.MaxNumLanes;
	InstancePtr->LinkConfig.LinkRate = InstancePtr->RxConfig.MaxLinkRate;
	InstancePtr->LinkConfig.LinkBW = InstancePtr->RxConfig.MaxLinkBW;

	xil_printf("  Training Parameters:\r\n");
	xil_printf("    Lanes             : %d\r\n",
		   InstancePtr->LinkConfig.NumLanes);
	xil_printf("    Link BW           : 0x%02X  %s\r\n",
		   InstancePtr->LinkConfig.LinkBW, RateStr);
	xil_printf("    PHY Rate          : %d\r\n",
		   InstancePtr->LinkConfig.LinkRate);

	memset(InstancePtr->LinkConfig.VsLevel, 0, 4);
	memset(InstancePtr->LinkConfig.PeLevel, 0, 4);

	XMmiDp_FastLinkTrainDisable(InstancePtr);
	xil_printf("    Fast Link Train   : Disabled\r\n");

	Status = XMmiDp_SetSinkDpcdLinkCfgField(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to set sink DPCD link config: %d\r\n",
			   Status);
		return Status;
	}
	xil_printf("  Sink DPCD link config set\r\n");

	if (InstancePtr->RxConfig.EnhancedFrameCap) {
		XDpPSu14_EnableCctlEnhanceFraming(InstancePtr);
		xil_printf("  Enhanced framing enabled on TX\r\n");
	}

	Status = XMmiDp_StartLinkXmit(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("  ERROR: Failed to start link transmission: %d\r\n",
			   Status);
		return Status;
	}
	sleep(1);
	xil_printf("  Link transmission started -- running CR + EQ...\r\n");

	Status = XMmiDp_RunTraining(InstancePtr);
	if (Status != XST_SUCCESS) {
		XDpDc_PrintLinkTrainingResult(InstancePtr, Status);
		return Status;
	}

	XMmiDp_SetPhyTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);
	XMmiDp_SetDpcdTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);

	XDpDc_PrintLinkTrainingResult(InstancePtr, XST_SUCCESS);

	InstancePtr->LinkConfig.LinkTrained = 1;

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
* DP core, polls for HPD, power cycles the sink, runs link training, and
* sets up the video and audio streams.
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
