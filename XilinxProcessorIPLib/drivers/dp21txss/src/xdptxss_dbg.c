/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_dbg.c
* @addtogroup dptxss Overview
* @{
*
* This file contains functions to report debug information of DisplayPort TX
* Subsystem sub-cores.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* 2.00 sha 08/07/15 Updated register offsets in debug MSA info.
* 2.00 sha 09/28/15 Added HDCP debug function.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdptxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function prints the capabilities of the DisplayPort sink.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportSinkCapInfo(XDpTxSs *InstancePtr)
{
	u8 NumAudioEps;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	u8 *Dpcd = InstancePtr->DpPtr->TxInstance.RxConfig.DpcdRxCapsField;

	u8 DpcdRevMajor = (Dpcd[XDP_DPCD_REV] &
				XDP_DPCD_REV_MJR_MASK) >>
					XDP_DPCD_REV_MJR_SHIFT;
	u8 DpcdRevMinor = Dpcd[XDP_DPCD_REV] & XDP_DPCD_REV_MNR_MASK;
	u8 MaxLinkRate = Dpcd[XDP_DPCD_MAX_LINK_RATE];
	u8 MaxLaneCount = Dpcd[XDP_DPCD_MAX_LANE_COUNT] &
				XDP_DPCD_MAX_LANE_COUNT_MASK;
	u8 Tps3Support = Dpcd[XDP_DPCD_MAX_LANE_COUNT] &
				XDP_DPCD_TPS3_SUPPORT_MASK;
	u8 EnhancedFrameSupport = Dpcd[XDP_DPCD_MAX_LANE_COUNT] &
				XDP_DPCD_ENHANCED_FRAME_SUPPORT_MASK;
	u8 MaxDownspreadSupport = Dpcd[XDP_DPCD_MAX_DOWNSPREAD] &
				XDP_DPCD_MAX_DOWNSPREAD_MASK;
	u8 NoAuxHandshakeRequired = Dpcd[XDP_DPCD_MAX_DOWNSPREAD] &
				XDP_DPCD_NO_AUX_HANDSHAKE_LINK_TRAIN_MASK;
	u8 NumRxPorts = (Dpcd[XDP_DPCD_NORP_PWR_V_CAP] & (0x01)) + 1;
	u8 PwrDpCap5V = Dpcd[XDP_DPCD_NORP_PWR_V_CAP] & (0x01);
	u8 PwrDpCap12V = Dpcd[XDP_DPCD_NORP_PWR_V_CAP] & (0x20);
	u8 PwrDpCap18V = Dpcd[XDP_DPCD_NORP_PWR_V_CAP] & (0x40);
	u8 DownstreamPortPresent = Dpcd[XDP_DPCD_DOWNSP_PRESENT] & (0x80);
	u8 DownPortType = (Dpcd[XDP_DPCD_DOWNSP_PRESENT] &
				XDP_DPCD_DOWNSP_TYPE_MASK) >>
					XDP_DPCD_DOWNSP_TYPE_SHIFT;
	u8 FormatConversionBlockPresent = Dpcd[XDP_DPCD_DOWNSP_PRESENT] &
					XDP_DPCD_DOWNSP_FORMAT_CONV_MASK;
	u8 DetailedCapInfoAvailable = Dpcd[XDP_DPCD_DOWNSP_PRESENT] &
					XDP_DPCD_DOWNSP_DCAP_INFO_AVAIL_MASK;
	u8 MainLinkAnsi8b10bCodingSupport = Dpcd[XDP_DPCD_ML_CH_CODING_CAP] &
						XDP_DPCD_ML_CH_CODING_MASK;
	u8 DownstreamPortCount = Dpcd[XDP_DPCD_DOWNSP_COUNT_MSA_OUI] &
					XDP_DPCD_DOWNSP_COUNT_MASK;
	u8 MsaTimingParIgnored = Dpcd[XDP_DPCD_DOWNSP_COUNT_MSA_OUI] &
					XDP_DPCD_MSA_TIMING_PAR_IGNORED_MASK;
	u8 OuiSupported = Dpcd[XDP_DPCD_DOWNSP_COUNT_MSA_OUI] &
				XDP_DPCD_OUI_SUPPORT_MASK;
	u8 RxPort0LocalEdidPresent = Dpcd[XDP_DPCD_RX_PORT0_CAP_0] &
			XDP_DPCD_RX_PORTX_CAP_0_LOCAL_EDID_PRESENT_MASK;
	u8 RxPort0AssocPrecedePort = Dpcd[XDP_DPCD_RX_PORT0_CAP_0] &
			XDP_DPCD_RX_PORTX_CAP_0_ASSOC_TO_PRECEDING_PORT_MASK;
	u8 RxPort0BufSizeBpl = (Dpcd[XDP_DPCD_RX_PORT0_CAP_1] + 1) * 32;
	u8 RxPort1LocalEdidPresent = Dpcd[XDP_DPCD_RX_PORT1_CAP_0] &
			XDP_DPCD_RX_PORTX_CAP_0_LOCAL_EDID_PRESENT_MASK;
	u8 RxPort1AssocPrecedePort = Dpcd[XDP_DPCD_RX_PORT1_CAP_0] &
			XDP_DPCD_RX_PORTX_CAP_0_ASSOC_TO_PRECEDING_PORT_MASK;
	u8 RxPort1BufSizeBpl = (Dpcd[XDP_DPCD_RX_PORT1_CAP_1] + 1) * 32;
	u8 IicSpeed = Dpcd[XDP_DPCD_I2C_SPEED_CTL_CAP];
	u8 EdpAltScramblerResetCap = Dpcd[XDP_DPCD_EDP_CFG_CAP] &(0x1);
	u8 EdpFramingChangeCap = Dpcd[XDP_DPCD_EDP_CFG_CAP] & (0x2);
	u8 TraingAuxReadInt = Dpcd[XDP_DPCD_TRAIN_AUX_RD_INTERVAL];
	u8 AdapterForceLoadSenseCap = Dpcd[XDP_DPCD_ADAPTER_CAP] & (0x01);
	u8 AdapterAltI2CPatternCap = Dpcd[XDP_DPCD_ADAPTER_CAP] & (0x02);
	u8 MstmCap = XDp_TxMstCapable(InstancePtr->DpPtr);
	XDp_TxAuxRead(InstancePtr->DpPtr,
				XDP_DPCD_NUM_AUDIO_EPS, 1, &NumAudioEps);

	xil_printf("RX capabilities:\n\r"
		"\tDPCD rev major (0x00000): %d\n\r"
		"\tDPCD rev minor (0x00000): %d\n\r"
		"\tMax link rate (0x00001): %s\n\r"
		"\tMax lane count (0x00002): %d\n\r"
		"\tTPS3 supported (0x00002): %s\n\r"
		"\tEnhanced frame support? (0x00002) %s\n\r"
		"\tMax downspread support? (0x00003) %s\n\r"
		"\tNo AUX handshake required? (0x00003)%s\n\r"
		"\t# of receiver ports (0x00004): %d\n\r"
		"\tDP power 5v cap? (0x00004) %s\n\r"
		"\tDP power 12v cap? (0x00004) %s\n\r"
		"\tDP power 18v cap? (0x00004) %s\n\r"
		"\tDownstream ports present? (0x00005) %s\n\r"
		"\tDownstreamport0 type (0x00005): %s\n\r"
		"\tFormat conversion block present (0x00005): %s\n\r"
		"\tDetailed cap info available? (0x00005) %s\n\r"
		"\tMain link ANSI 8b/10b channel coding support? (0x00006) "
			"%s\n\r"
		"\tDownstream port count (0x00007): %d\n\r"
		"\tMSA timing parameters ignored? (0x00007) %s\n\r"
		"\tOUI supported? (0x00007) %s\n\r"
		"\tReceive port0 local edid present (0x00008) %s\n\r"
		"\tReceive port0 associated to preceding port? (0x00008) "
			"%s\n\r"
		"\tReceive port0 buffer size (0x00009): %d bytes per lane\n\r"
		"\tReceive port1 local edid present (0x0000A) %s\n\r"
		"\tReceive port1 associated to preceding port? (0x0000A) "
			"%s\n\r"
		"\tReceive port1 buffer size (0x0000B): %d bytes per lane\n\r"
		"\tI2C speed (0x0000C): %s\n\r"
		"\tEDP alt scrambler reset cap? (0x0000D) %s\n\r"
		"\tEDP framing change cap? (0x0000D) %s\n\r"
		"\tTraining AUX read interval (0x0000E): %s\n\r"
		"\tAdapter force load sense cap? (0x0000F) %s\n\r"
		"\tAdapter alt i2c pattern cap? (0x0000F) %s\n\r"
		"\tMSTM cap? (0x00021) %s\n\r"
		"\t# of audio eps (0x00022) : %d\n\r",
		DpcdRevMajor,
		DpcdRevMinor,
		(MaxLinkRate == XDP_DPCD_LINK_BW_SET_162GBPS) ? "1.62Gbps" :
		(MaxLinkRate == XDP_DPCD_LINK_BW_SET_270GBPS) ? "2.70Gbps" :
		(MaxLinkRate == XDP_DPCD_LINK_BW_SET_540GBPS) ? "5.40Gbps" :
		"Unknown link rate",
		MaxLaneCount,
		Tps3Support ? "Y" : "N",
		EnhancedFrameSupport ? "Y" : "N",
		MaxDownspreadSupport ? "Y" : "N",
		NoAuxHandshakeRequired ? "Y" : "N",
		NumRxPorts,
		PwrDpCap5V ? "Y" : "N",
		PwrDpCap12V ? "Y" : "N",
		PwrDpCap18V ? "Y" : "N",
		DownstreamPortPresent ? "Y" : "N",
		(DownPortType == XDP_DPCD_DOWNSP_TYPE_DP) ?
		"DisplayPort" :
		(DownPortType == XDP_DPCD_DOWNSP_TYPE_AVGA_ADVII) ?
			"Analog VGA or analog video over DVI-I" :
		(DownPortType == XDP_DPCD_DOWNSP_TYPE_DVI_HDMI_DPPP) ?
			"DVI, HDMI, or DP++" :
		(DownPortType == XDP_DPCD_DOWNSP_TYPE_OTHERS) ?
			"Others" : "Unknown downstream port type",
		FormatConversionBlockPresent ? "Y" : "N",
		DetailedCapInfoAvailable ? "Y" : "N",
		MainLinkAnsi8b10bCodingSupport ? "Y" : "N",
		DownstreamPortCount,
		MsaTimingParIgnored ? "Y" : "N",
		OuiSupported ? "Y" : "N",
		RxPort0LocalEdidPresent ? "Y" : "N",
		RxPort0AssocPrecedePort ? "Y" : "N",
		RxPort0BufSizeBpl,
		RxPort1LocalEdidPresent ? "Y" : "N",
		RxPort1AssocPrecedePort ? "Y" : "N",
		RxPort1BufSizeBpl,
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_NONE) ? "No control" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_1KBIPS) ? "1Kbps" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_5KBIPS) ? "5Kbps" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_10KBIPS) ? "10Kbps" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_100KBIPS) ? "100Kbps" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_400KBIPS) ? "400Kbps" :
		(IicSpeed == XDP_DPCD_I2C_SPEED_CTL_1MBIPS) ? "1Mbps" :
			"Unknown I2C speed",
		EdpAltScramblerResetCap ? "Y" : "N",
		EdpFramingChangeCap ? "Y" : "N",
		(TraingAuxReadInt == XDP_DPCD_TRAIN_AUX_RD_INT_100_400US) ?
			"100us for CR, 400us for CE" : (TraingAuxReadInt ==
			XDP_DPCD_TRAIN_AUX_RD_INT_4MS) ? "4us" :
		(TraingAuxReadInt == XDP_DPCD_TRAIN_AUX_RD_INT_8MS) ? "8us" :
		(TraingAuxReadInt == XDP_DPCD_TRAIN_AUX_RD_INT_12MS) ?
		"12us" : (TraingAuxReadInt ==
			XDP_DPCD_TRAIN_AUX_RD_INT_16MS) ? "16us" :
				"Unknown AUX read interval",
		AdapterForceLoadSenseCap ? "Y" : "N",
		AdapterAltI2CPatternCap ? "Y" : "N",
		MstmCap ? "Y" : "N",
		NumAudioEps
	);

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function reports list of sub-cores included in DisplayPort TX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportCoreInfo(XDpTxSs *InstancePtr)
{
	u32 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\n\rDisplayPort TX Subsystem info:\n\r");

	/* Report all the included cores in the subsystem instance */
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	if (InstancePtr->DsPtr) {
		xil_printf("Dual Splitter:Yes\n\r");
	}
#endif

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	if (InstancePtr->Hdcp1xPtr) {
		xil_printf("High-Bandwidth Content protection (HDCP):Yes\n\r");
	}
	if (InstancePtr->TmrCtrPtr) {
		xil_printf("Timer Counter(0):Yes\n\r");
	}
#endif

	if (InstancePtr->DpPtr) {
		xil_printf("DisplayPort Transmitter(TX):Yes\n\r");
	}

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			xil_printf("Video Timing Controller(VTC) %ld: Yes\n\r",
				Index);
		}
	}

	xil_printf("Audio enabled:%s\n\r",
			InstancePtr->Config.SecondaryChEn? "Yes": "No");
	xil_printf("Max supported bits per color:%d\n\r",
			InstancePtr->Config.MaxBpc);
	xil_printf("HDCP enabled:%s\n\r",
			InstancePtr->Config.HdcpEnable? "Yes": "No");
	xil_printf("Max supported lane count:%d\n\r",
			InstancePtr->Config.MaxLaneCount);
	xil_printf("Multi-Stream Transport mode:%s\n\r",
			InstancePtr->Config.MstSupport? "Yes": "No (SST)");
	xil_printf("Max number of supported streams:%d\n\r",
			InstancePtr->Config.NumMstStreams);
	xil_printf("Subsystem is running in: %s\n\r",
		InstancePtr->UsrOpt.MstSupport?"MST":"SST");

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the current Dual Splitter information.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportSplitterInfo(XDpTxSs *InstancePtr)
{
#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	u8 ISamples;
	u8 OSamples;
	u8 Segments;
	u8 Overlap;
	u16 Height;
	u16 Width;
#endif

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

#if (XPAR_XDUALSPLITTER_NUM_INSTANCES > 0)
	if (InstancePtr->DsPtr) {
		xil_printf("Dual Splitter info:\n\r");
		XDualSplitter_GetImageSize(InstancePtr->DsPtr, &Height,
				&Width);
		XDualSplitter_GetImgParam(InstancePtr->DsPtr, &ISamples,
			&OSamples, &Segments, &Overlap);
		xil_printf("Input Samples :%d\n\r"
			   "Output Samples:%d\n\r"
			   "Overlap       :%d\n\r"
			   "Segments      :%d\n\r"
			   "Mode          :%s\n\r"
			   "Width x Height:%d x %d", ISamples, OSamples,
				Overlap, Segments,
				(Segments == 2)?"Split":"Bypass",
				Width,Height);
	}
#else
	xil_printf("Dual Splitter is not supported in this design \n\r");
#endif
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the current VTC timing information.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportVtcInfo(XDpTxSs *InstancePtr)
{
	XVtc_Timing VideoTiming;
	u32 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	for (Index = 0; Index < InstancePtr->Config.NumMstStreams; Index++) {
		if (InstancePtr->VtcPtr[Index]) {
			XVtc_GetGeneratorTiming(InstancePtr->VtcPtr[Index],
				&VideoTiming);

			xil_printf("VTC%ld timing:\n\r"
				"\tHActiveVideo : %d\n\r"
				"\tHFrontPorch  : %d\n\r"
				"\tHSyncWidth   : %d\n\r"
				"\tHBackPorch   : %d\n\r"
				"\tHSyncPolarity: %d\n\r"
				"\tVActiveVideo : %d\n\r"
				"\tV0FrontPorch : %d\n\r"
				"\tV0SyncWidth  : %d\n\r"
				"\tV0BackPorch  : %d\n\r"
				"\tV1FrontPorch : %d\n\r"
				"\tV1SyncWidth  : %d\n\r"
				"\tV1BackPorch  : %d\n\r"
				"\tVSyncPolarity: %d\n\r"
				"\tInterlaced   : %s\n\n\r\r",Index,
				VideoTiming.HActiveVideo,
				VideoTiming.HFrontPorch,
				VideoTiming.HSyncWidth,
				VideoTiming.HBackPorch,
				VideoTiming.HSyncPolarity,
				VideoTiming.VActiveVideo,
				VideoTiming.V0FrontPorch,
				VideoTiming.V0SyncWidth,
				VideoTiming.V0BackPorch,
				VideoTiming.V1FrontPorch,
				VideoTiming.V1SyncWidth,
				VideoTiming.V1BackPorch,
				VideoTiming.VSyncPolarity,
			VideoTiming.Interlaced ? "Yes" : "No (Progressive)");
		}
	}
	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the link status, selected resolution, bits per color and
* link/lane count symbol error.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportLinkInfo(XDpTxSs *InstancePtr)
{
	u8 Data[8];

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Read Link rate over through channel */
	XDp_TxAuxRead(InstancePtr->DpPtr, XDP_DPCD_LINK_BW_SET, 1, &Data);
	xil_printf("\n\rLINK_BW_SET (0x00100) status in DPCD = %x\n\r",
			(Data[0] & 0xFF));

	/* Read Lane count through AUX channel */
	XDp_TxAuxRead(InstancePtr->DpPtr, XDP_DPCD_LANE_COUNT_SET, 1,
			&Data);
	xil_printf("LANE_COUNT_SET (0x00101) status in DPCD = %x\n\r",
			(Data[0] & XDP_DPCD_LANE_COUNT_SET_MASK));

	/* Read Lane status through AUX channel */
	XDp_TxAuxRead(InstancePtr->DpPtr, XDP_DPCD_STATUS_LANE_0_1, 2,
			&Data);
	xil_printf("LANE0_1_STATUS (0x00202) in DPCD = %x\n\r",
			(Data[0] & 0xFF));
	xil_printf("LANE2_3_STATUS (0x00203) in DPCD = %x\n\r",
			(Data[1] & 0xFF));

	/* Read symbol error through AUX channel */
	XDp_TxAuxRead(InstancePtr->DpPtr,
			XDP_DPCD_SYMBOL_ERROR_COUNT_LANE_0, 8, &Data);
	xil_printf("\n\rSYMBOL_ERROR_COUNT_LANE_0 (0x00210 and 0x00211) "
		"Status = %x%x\n\r",(Data[1] & 0xFF00),(Data[0] & 0xFF));
	xil_printf("SYMBOL_ERROR_COUNT_LANE_1 (0x00212 and 0x00213) "
		"Status = %x%x\n\r",(Data[3] & 0xFF00),(Data[2] & 0xFF));
	xil_printf("SYMBOL_ERROR_COUNT_LANE_2 (0x00214 and 0x00215) "
		"Status = %x%x\n\r",(Data[5] & 0xFF00),(Data[4] & 0xFF));
	xil_printf("SYMBOL_ERROR_COUNT_LANE_3 (0x00216 and 0x00217) "
		"Status = %x%x\n\r",(Data[7] & 0xFF00),(Data[6] & 0xFF));

	xil_printf("\n\rSelected Resolution = %s\n\r",
		XVidC_GetVideoModeStr(InstancePtr->UsrOpt.VmId));
	xil_printf("Selected BPC = %d\n\r",InstancePtr->UsrOpt.Bpc);

	xil_printf("\n\r");
}

/*****************************************************************************/
/**
*
* This function prints the current main stream attributes from the DisplayPort
* TX core.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportMsaInfo(XDpTxSs *InstancePtr)
{
	u8 Stream;
	u32 StreamOffset[4] = {0, 0x380,
				   0x3D0,
				   0x420};
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XDp_Config *TxConfig = &InstancePtr->DpPtr->Config;

	for (Stream = 1; Stream <= InstancePtr->Config.NumMstStreams; Stream++) {
		xil_printf("TX MSA registers:Stream %x\n\r"
				"\tClocks, H Total      	(0x180) : %u\n\r"
				"\tClocks, V Total      	(0x184) : %u\n\r"
				"\tPolarity (V / H)     	(0x188) : %u\n\r"
				"\tHSync Width          	(0x18C) : %u\n\r"
				"\tVSync Width          	(0x190) : %u\n\r"
				"\tHorz Resolution      	(0x194) : %u\n\r"
				"\tVert Resolution      	(0x198) : %u\n\r"
				"\tHorz Start           	(0x19C) : %u\n\r"
				"\tVert Start           	(0x1A0) : %u\n\r"
				"\tMisc0                	(0x1A4) : 0x%08x\n\r"
				"\tMisc1                	(0x1A8) : 0x%08x\n\r"
				"\tUser Pixel Width     	(0x1B8) : %u\n\r",
		Stream,
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_HTOTAL+
						StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_VTOTAL+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_POLARITY+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_HSWIDTH+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_VSWIDTH+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_HRES+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_VRES+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_HSTART+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_VSTART+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_MISC0+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MAIN_STREAM_MISC1+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_USER_PIXEL_WIDTH+
				StreamOffset[Stream - 1])
		);

		if(InstancePtr->DpPtr->TxInstance.LinkConfig.TrainingMode == XDP_TX_TRAINING_MODE_DP14){
			xil_printf(
					"\tM Vid                	(0x1AC) : %u\n\r"
					"\tN Vid                	(0x1B4) : %u\n\r",
					XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_M_VID+
								StreamOffset[Stream - 1]),
					XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_N_VID+
								StreamOffset[Stream - 1])
			);
		}else{
			xil_printf(
					"\tVFreq L                	(0x6D0) : 0x%08x\n\r"
					"\tVFreq H                	(0x6D4) : 0x%08x\n\r",
			XDp_ReadReg(TxConfig->BaseAddr, 0x6D0),
			XDp_ReadReg(TxConfig->BaseAddr, 0X6D4)
			);
		}

		xil_printf(
		"\tTransfer Unit Size   	(0x1B0) : %u\n\r"
		"\tUser Data Count      	(0x1BC) : %u\n\r"
		"\tMinimum bytes per TU 	(0x1C4) : %u\n\r"
		"\tFractional bytes per TU	(0x1C8) : %u\n\r"
		"\tInit wait              	(0x1CC) : %u\n\r",
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_TU_SIZE+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr,
				XDP_TX_USER_DATA_COUNT_PER_LANE+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_MIN_BYTES_PER_TU+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_FRAC_BYTES_PER_TU+
				StreamOffset[Stream - 1]),
		XDp_ReadReg(TxConfig->BaseAddr, XDP_TX_INIT_WAIT+
				StreamOffset[Stream - 1])
	);

		xil_printf("\n\r");
	}
}

/*****************************************************************************/
/**
*
* This function prints the debug display info of the HDCP interface.
*
* @param	InstancePtr is a pointer to the XDpTxSs core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpTxSs_ReportHdcpInfo(XDpTxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
	if (InstancePtr->Hdcp1xPtr)
		XHdcp1x_Info(InstancePtr->Hdcp1xPtr);
	else
#endif
		xil_printf("HDCP is not supported in this design.\n\r");
}
/** @} */
