/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1_frl.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmitxss1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xv_hdmitxss1_frl.h"
#include "xv_hdmitxss1.h"

#ifdef XPAR_XV_HDMI_TX_FRL_ENABLE
/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
*
* This function sets the FFE Levels.
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem.
* @param   FfeLevel    FFE level value to be set.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFfeLevels(XV_HdmiTxSs1 *InstancePtr, u8 FfeLevel)
{
	InstancePtr->HdmiTx1Ptr->Stream.Frl.FfeLevels = FfeLevel;
}

/*****************************************************************************/
/**
*
* This function returns the FFE Level for the selected lane.
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem.
*
* @param Lane        Lane number for which the FFE Level is requested.
* @return u8         FFE Level for the specified lane.
*
* @return FFE Level.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetTxFfe(XV_HdmiTxSs1 *InstancePtr, u8 Lane)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.LaneFfeAdjReq.Byte[Lane];
}

/*****************************************************************************/
/**
*
* This function returns the FRL Rate.
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem.
*
* @return FRL Rate.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlRate(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.FrlRate;
}

/*****************************************************************************/
/**
*
* This function returns the FRL Line Rate.
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem.
*
* @return FRL Line Rate.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlLineRate(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.LineRate;
}

/*****************************************************************************/
/**
*
* This function returns the the number of active FRL lanes.
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem
*
* @return Number of active FRL lanes.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetFrlLanes(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.Frl.Lanes;
}

/*****************************************************************************/
/**
*
* This function is called when the FRL link training requires configuration
* from application.
*
* @param  CallbackRef is a pointer to the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlConfigCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_CFG, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlConfigCallback) {
	  HdmiTxSs1Ptr->FrlConfigCallback(HdmiTxSs1Ptr->FrlConfigRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when the FRL  link training requires configuring of
* FFE.
*
* @param  CallbackRef is a pointer to the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlFfeCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlFfeCallback) {
	  HdmiTxSs1Ptr->FrlFfeCallback(HdmiTxSs1Ptr->FrlFfeRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when the FRL link training passes and sink is ready
* to receive video, audio and control packets.
*
* @param  CallbackRef is a pointer to the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlStartCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LT_PASS, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlStartCallback) {
	  HdmiTxSs1Ptr->FrlStartCallback(HdmiTxSs1Ptr->FrlStartRef);
	}
}

/*****************************************************************************/
/**
*
* This function is called when sink requested for FRL to be stopped.
*
* @param  CallbackRef is a pointer to the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_FrlStopCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->FrlStopCallback) {
	  HdmiTxSs1Ptr->FrlStopCallback(HdmiTxSs1Ptr->FrlStopRef);
	}
}
#endif /* XPAR_XV_HDMI_TX_FRL_ENABLE */

/*****************************************************************************/
/**
*
* This function is called during FRL link training when it is decided to
* fallback to the legacy HDMI TMDS mode.
*
* @param  CallbackRef is a pointer to the callback reference.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_TmdsConfigCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_TMDS_START, 0);
#endif

	/* Check if user callback has been registered */
	if (HdmiTxSs1Ptr->TmdsConfigCallback) {
	  HdmiTxSs1Ptr->TmdsConfigCallback(HdmiTxSs1Ptr->TmdsConfigRef);
	}
}

#ifdef XPAR_XV_HDMI_TX_FRL_ENABLE
/*****************************************************************************/
/**
*
* This function starts the Legacy HDMI TMDS Mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	Status on if TMDS mode is successfully started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_TmdsStart(XV_HdmiTxSs1 *InstancePtr)
{
	int Status = XST_FAILURE;

	Status = XV_HdmiTx1_FrlRate(InstancePtr->HdmiTx1Ptr,
			XHDMIC_MAXFRLRATE_NOT_SUPPORTED);
	XV_HdmiTx1_FrlModeEn(InstancePtr->HdmiTx1Ptr, FALSE);
	XV_HdmiTx1_FrlExecute(InstancePtr->HdmiTx1Ptr);

	InstancePtr->HdmiTx1Ptr->Stream.IsFrl = FALSE;

	return Status;
}

/*****************************************************************************/
/**
*
* This function starts the TMDS mode.
*
* @param    InstancePtr is a pointer to the XV_HdmiTx1 core instance.
*
* @return	Status on if TMDS can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_StartTmdsMode(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_StartTmdsMode(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function starts the Fixed Rate Link Training.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @param    FrlRate specifies the FRL rate to be attempted
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return	Status on if FrlTraining can be started or not.
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_StartFrlTraining(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_MaxFrlRate FrlRate)
{
#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_FRL_START,
			FrlRate);
#endif

	return XV_HdmiTx1_StartFrlTraining(InstancePtr->HdmiTx1Ptr,
			FrlRate);
}

/*****************************************************************************/
/**
*
* This function sets maximum FRL Rate supported by the system.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @param    MaxFrlRate specifies maximum rates supported
* 			- 0 = FRL Not Supported
* 			- 1 = 3 Lanes 3Gbps
* 			- 2 = 4 Lanes 3Gbps
*			- 3 = 4 Lanes 6Gbsp
*			- 4 = 4 Lanes 8Gbps
*			- 5 = 4 Lanes 10Gbps
*			- 6 = 4 Lanes 12Gbps
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlMaxFrlRate(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_MaxFrlRate MaxFrlRate)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XV_HdmiTx1_SetFrlMaxFrlRate(InstancePtr->HdmiTx1Ptr, MaxFrlRate);
}

/*****************************************************************************/
/**
*
* This function starts FRL stream. This should be called after the bridge,
* video, audio are all active.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_FrlStreamStart(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_FrlStreamStart(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
int XV_HdmiTxSs1_FrlStreamStop(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_FrlStreamStop(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_SetFrlWrongLtp(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function stops FRL video stream.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_ClearFrlWrongLtp(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_ClearFrlWrongLtp(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
 * Set the FRL LTP for a specific lane.
 *
 * @param    InstancePtr Pointer to the XV_HdmiTxSs1 core instance.
 * @param    Lane        Lane number to set the LTP for (0-based).
 * @param    Ltp         FRL LTP Type to set for the lane.
 *
 * @note     For debug purposes.
 *******************************************************************************/
void XV_HdmiTxSs1_SetFrlLtp(XV_HdmiTxSs1 *InstancePtr, u8 Lane,
			XV_HdmiTx1_FrlLtpType Ltp)
{
	XV_HdmiTx1_SetFrlLtp(InstancePtr->HdmiTx1Ptr, Lane, Ltp);
}

/*****************************************************************************/
/**
*
* This function sets the CKE Source for External
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlExtVidCke(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_FrlExtVidCkeSource(InstancePtr->HdmiTx1Ptr, TRUE);
}

/*****************************************************************************/
/**
*
* This function sets the CKE Source for Internal Generated
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetFrlIntVidCke(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_FrlExtVidCkeSource(InstancePtr->HdmiTx1Ptr, FALSE);
}
#endif /* XPAR_XV_HDMI_TX_FRL_ENABLE */

/************************* FRL Bandwidth Calculation *************************/

/**************************** Constant Definitions ***************************/
/**
 * FRL Overhead and Timing Constants from HDMI 2.1 Spec Table 6-41
 */
#define XV_HDMITXSS1_FRL_TOLERANCE_PIXEL_CLOCK_PPM	5000	/**< ±0.50% = 5000 ppm */
#define XV_HDMITXSS1_FRL_TOLERANCE_FRL_BIT_PPM		300	/**< ±300 ppm */
#define XV_HDMITXSS1_FRL_TB_BORROWED_MAX		400	/**< Max Tri-Bytes borrowable */
#define XV_HDMITXSS1_FRL_CFRL_CB			510	/**< Characters per Character Block */
#define XV_HDMITXSS1_FRL_CFRL_SB_3LANE			2043	/**< Characters per Super Block (3L) */
#define XV_HDMITXSS1_FRL_CFRL_SB_4LANE			2044	/**< Characters per Super Block (4L) */

/**
 * Overhead percentages scaled by 100000 for fixed-point calculation
 */
#define XV_HDMITXSS1_FRL_OVERHEAD_SB_3LANE		147	/**< 0.147% * 100000 */
#define XV_HDMITXSS1_FRL_OVERHEAD_SB_4LANE		196	/**< 0.196% * 100000 */
#define XV_HDMITXSS1_FRL_OVERHEAD_RS			1566	/**< 1.566% * 100000 */
#define XV_HDMITXSS1_FRL_OVERHEAD_MAP			122	/**< 0.122% * 100000 */
#define XV_HDMITXSS1_FRL_OVERHEAD_MARGIN		300	/**< 0.300% * 100000 */

/**
 * FRL 16b/18b Encoding Constants
 */
#define XV_HDMITXSS1_FRL_ENCODE_DATA_BITS		16	/**< Data bits per symbol */
#define XV_HDMITXSS1_FRL_ENCODE_LINE_BITS		18	/**< Line bits per symbol (16b/18b) */
#define XV_HDMITXSS1_FRL_TRIBYTE_BITS			24	/**< Bits per tribyte (3 bytes) */
#define XV_HDMITXSS1_FRL_TB_PER_3CHAR			2	/**< Tribytes per 3 FRL characters */
#define XV_HDMITXSS1_FRL_CHAR_PER_2TB			3	/**< FRL characters per 2 tribytes */

/**
 * Display Formatting Constants
 */
#define XV_HDMITXSS1_FRL_DISP_ROUND			5	/**< Rounding value for display */
#define XV_HDMITXSS1_FRL_DISP_DIVISOR			10	/**< Divisor for Gbps display */
#define XV_HDMITXSS1_FRL_DISP_DECIMAL			100	/**< Divisor for 2-digit decimal */

/**
 * Unit Conversion Constants
 */
#define XV_HDMITXSS1_FRL_GBPS_TO_BPS			1000000000ULL	/**< 1 Gbps = 10^9 bps */
#define XV_HDMITXSS1_FRL_MBPS_DIVISOR			1000000		/**< Divisor for Mbps */
#define XV_HDMITXSS1_FRL_PPM_SCALE			1000000		/**< PPM scale factor */
#define XV_HDMITXSS1_FRL_OVERHEAD_SCALE			100000		/**< Overhead percentage scale */
#define XV_HDMITXSS1_FRL_AUDIO_MBPS_DIVISOR		10000		/**< Audio Mbps display divisor */

/**
 * Color Format and TMDS Constants
 */
#define XV_HDMITXSS1_FRL_YUV420_DIVISOR			2		/**< YCbCr 4:2:0 bandwidth divisor */
#define XV_HDMITXSS1_TMDS_MAX_BW_MBPS			18000		/**< TMDS max bandwidth: 18 Gbps */

/**
 * Precomputed common overhead: RS + MAP + Margin (lane-independent portion)
 */
#define XV_HDMITXSS1_FRL_OVERHEAD_COMMON	(XV_HDMITXSS1_FRL_OVERHEAD_RS + \
						 XV_HDMITXSS1_FRL_OVERHEAD_MAP + \
						 XV_HDMITXSS1_FRL_OVERHEAD_MARGIN)

/*****************************************************************************/
/**
*
* This function performs FRL bandwidth calculations and determines if the
* requested video and audio streams can be supported by the trained FRL link.
* Audio parameters are automatically fetched from the instance.
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs1 instance.
*		The trained FRL LineRate and Lanes are fetched from
*		InstancePtr->HdmiTx1Ptr->Stream.Frl after FRL training.
*		Audio parameters are fetched from:
*		- InstancePtr->AudioEnabled
*		- InstancePtr->HdmiTx1Ptr->Stream.Audio.SampleFrequency
*		- InstancePtr->AudioChannels
* @param	VmId is the video mode ID used to lookup timing information
*		via XVidC_GetVideoModeData().
* @param	ColorFormat is the color format (RGB, YCbCr444, YCbCr422,
*		YCbCr420).
* @param	Bpc is the bits per color component (8, 10, 12, 16).
*
* @return
*		- TRUE if video+audio bandwidth fits within the trained FRL
*		  rate.
*		- FALSE if bandwidth exceeds capacity or resolution is not
*		  divisible by 4.
*
* @note		This function prints bandwidth calculation results and
*		diagnostic information to the console via xil_printf.
*		For TMDS mode (LineRate=0), returns TRUE if bandwidth is
*		within 18 Gbps limit, FALSE otherwise. For FRL mode, FRL
*		training must be completed before calling this function.
*
******************************************************************************/
u8 XV_HdmiTxSs1_FrlCalcBandwidth(XV_HdmiTxSs1 *InstancePtr,
		XVidC_VideoMode VmId, XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc)
{
	/* Video timing mode pointer from VmId lookup */
	const XVidC_VideoTimingMode *VmPtr;

	/* Common calculation variables */
	u64 PixelClock;
	u32 BitsPerPixel;
	u8 BitsPerComponent;
	u8 HasAudio;

	/* Audio parameters fetched from instance */
	u8 IsAudioEnabled;
	u32 AudioSampleRate;
	u8 AudioChannels;

	/* Validate instance pointer */
	if (InstancePtr == NULL || InstancePtr->HdmiTx1Ptr == NULL) {
		xil_printf("Error: Invalid instance pointer\r\n");
		return FALSE;
	}

	/* Lookup video mode data from VmId */
	VmPtr = XVidC_GetVideoModeData(VmId);
	if (VmPtr == NULL) {
		xil_printf("Error: Invalid VmId (%d) - video mode not found\r\n", VmId);
		return FALSE;
	}

	/* Fetch audio parameters from instance */
	IsAudioEnabled = InstancePtr->AudioEnabled;
	AudioSampleRate = InstancePtr->HdmiTx1Ptr->Stream.Audio.SampleFrequency;
	AudioChannels = InstancePtr->AudioChannels;

	/* Pre-calculate common values used in both TMDS and FRL paths */
	BitsPerComponent = (u8)Bpc;
	HasAudio = (IsAudioEnabled && AudioSampleRate > 0 && AudioChannels > 0);

	/* Calculate pixel clock (common for both modes) */
	PixelClock = (u64)VmPtr->Timing.HTotal * (u64)VmPtr->Timing.F0PVTotal *
		     (u64)VmPtr->FrameRate;

	/* Calculate bits per pixel based on color format (common for both modes) */
	switch (ColorFormat) {
	case XVIDC_CSF_RGB:
	case XVIDC_CSF_YCRCB_444:
		BitsPerPixel = 3 * BitsPerComponent;
		break;
	case XVIDC_CSF_YCRCB_422:
		BitsPerPixel = 2 * BitsPerComponent;
		break;
	case XVIDC_CSF_YCRCB_420:
		BitsPerPixel = (3 * BitsPerComponent) / 2;
		break;
	default:
		BitsPerPixel = 3 * BitsPerComponent;
		break;
	}

	/* Check if TMDS or FRL mode */
	{
		u8 TrainedLineRate = InstancePtr->HdmiTx1Ptr->Stream.Frl.LineRate;
		u8 TrainedLanes = InstancePtr->HdmiTx1Ptr->Stream.Frl.Lanes;

		if (TrainedLineRate == 0 || TrainedLanes == 0) {
			/* TMDS mode - calculate TMDS bandwidth */
			u64 TmdsBwMbps;
			u8 TmdsSupported;

			/*
			 * TMDS uses 8b/10b encoding (25% overhead)
			 * TMDS Bandwidth = PixelClock × BitsPerPixel × (10/8)
			 */
			TmdsBwMbps = (PixelClock * (u64)BitsPerPixel * 10ULL) /
				     (8ULL * XV_HDMITXSS1_FRL_MBPS_DIVISOR);

			/*
			 * Get max TMDS clock from sink EDID (if available)
			 * Default to HDMI 2.0 max of 600 MHz = 18 Gbps
			 */
			u32 MaxTmdsBwMbps = XV_HDMITXSS1_TMDS_MAX_BW_MBPS; /* 18000 */

			TmdsSupported = (TmdsBwMbps <= MaxTmdsBwMbps) ? TRUE : FALSE;
			if (!TmdsSupported) {
				/*
				 * Show FRL bandwidth (16b/18b) since FRL mode is required
				 * FRL Bandwidth = PixelClock × BitsPerPixel × (18/16)
				 */
				u32 FrlBwMbps = (u32)((PixelClock * (u64)BitsPerPixel *
						      XV_HDMITXSS1_FRL_ENCODE_LINE_BITS) /
						     (XV_HDMITXSS1_FRL_ENCODE_DATA_BITS *
						      XV_HDMITXSS1_FRL_MBPS_DIVISOR));
				u32 ReqDispBw = (FrlBwMbps + 4) / XV_HDMITXSS1_FRL_DISP_DIVISOR;
				xil_printf("========================================\r\n");
				xil_printf("Resolution requires FRL mode.\r\n");
				xil_printf("Connected sink does not support FRL.\r\n");
				xil_printf("TMDS Max Bandwidth: %d Gbps\r\n",
					   MaxTmdsBwMbps / 1000);
				xil_printf("Required FRL Bandwidth: %d.%02d Gbps\r\n",
					   ReqDispBw / XV_HDMITXSS1_FRL_DISP_DECIMAL,
					   ReqDispBw % XV_HDMITXSS1_FRL_DISP_DECIMAL);
				xil_printf("========================================\r\n");
			}
			xil_printf("\r\n");
			return TmdsSupported;
		}
	}

	/* FRL mode calculations */
	{
		/* FRL-specific variables */
		u64 VideoTribyte, AudioTribyte, RequiredTribyte;
		u64 AudioBwBps, FrlAudioBwBps;
		u32 DataBw, FrlDataBw;
		u64 TotalFrlMbps;
		u64 RbitMin, RFRLCharMin, UsableBandwidth, MaxTributePerRate;
		u32 OverheadMax, OverheadSB;
		u8 TrainedLineRate, TrainedLanes;
		u8 IsDiv4, IsSupported;
		u8 Multiplier;

		/* Check if resolution is divisible by 4 */
		IsDiv4 = (((VmPtr->Timing.HActive % 4) == 0) &&
			  ((VmPtr->Timing.VActive % 4) == 0)) ? TRUE : FALSE;

		if (!IsDiv4) {
			return FALSE;
		}

		/* Calculate multiplier for tribyte calculation */
		Multiplier = (ColorFormat == XVIDC_CSF_YCRCB_422) ? 2 : 3;

		/* Calculate required video tribytes per second */
		VideoTribyte = (u64)VmPtr->Timing.HTotal * (u64)VmPtr->Timing.F0PVTotal *
			  (u64)VmPtr->FrameRate * (u64)BitsPerComponent * (u64)Multiplier;
		if (ColorFormat == XVIDC_CSF_YCRCB_420) {
			VideoTribyte = VideoTribyte / XV_HDMITXSS1_FRL_YUV420_DIVISOR;
		}
		VideoTribyte = VideoTribyte / XV_HDMITXSS1_FRL_TRIBYTE_BITS;

		/* Calculate audio bandwidth if enabled */
		AudioBwBps = HasAudio ?
			(u64)AudioSampleRate * 32ULL * (u64)AudioChannels : 0;

		/* Calculate raw data bandwidth in Mbps */
		DataBw = (u32)((PixelClock * (u64)BitsPerPixel) / XV_HDMITXSS1_FRL_MBPS_DIVISOR);

		/* Get trained FRL parameters */
		TrainedLineRate = InstancePtr->HdmiTx1Ptr->Stream.Frl.LineRate;
		TrainedLanes = InstancePtr->HdmiTx1Ptr->Stream.Frl.Lanes;

		/* Apply FRL 16b/18b encoding overhead for internal calculations */
		FrlDataBw = (DataBw * XV_HDMITXSS1_FRL_ENCODE_LINE_BITS) /
			    XV_HDMITXSS1_FRL_ENCODE_DATA_BITS;
		FrlAudioBwBps = (AudioBwBps * XV_HDMITXSS1_FRL_ENCODE_LINE_BITS) /
				XV_HDMITXSS1_FRL_ENCODE_DATA_BITS;
		TotalFrlMbps = (u64)FrlDataBw + (FrlAudioBwBps / XV_HDMITXSS1_FRL_MBPS_DIVISOR);

		/* Calculate tribytes for FRL capacity check */
		AudioTribyte = FrlAudioBwBps / XV_HDMITXSS1_FRL_TRIBYTE_BITS;
		VideoTribyte = (VideoTribyte * XV_HDMITXSS1_FRL_ENCODE_LINE_BITS) /
			       XV_HDMITXSS1_FRL_ENCODE_DATA_BITS;
		if(HasAudio)
			RequiredTribyte = VideoTribyte + AudioTribyte;
		else
			RequiredTribyte = VideoTribyte;

		/* Calculate max tribytes for trained FRL rate */
		RbitMin = (u64)TrainedLineRate * XV_HDMITXSS1_FRL_GBPS_TO_BPS;
		RbitMin = (RbitMin * (XV_HDMITXSS1_FRL_PPM_SCALE - XV_HDMITXSS1_FRL_TOLERANCE_FRL_BIT_PPM)) /
			  XV_HDMITXSS1_FRL_PPM_SCALE;
		RFRLCharMin = RbitMin / XV_HDMITXSS1_FRL_ENCODE_LINE_BITS;

		OverheadSB = (TrainedLanes == 3) ? XV_HDMITXSS1_FRL_OVERHEAD_SB_3LANE :
						  XV_HDMITXSS1_FRL_OVERHEAD_SB_4LANE;
		OverheadMax = OverheadSB + XV_HDMITXSS1_FRL_OVERHEAD_COMMON;

		UsableBandwidth = RFRLCharMin * (u64)TrainedLanes;
		UsableBandwidth = (UsableBandwidth * (XV_HDMITXSS1_FRL_OVERHEAD_SCALE - OverheadMax)) /
				  XV_HDMITXSS1_FRL_OVERHEAD_SCALE;
		MaxTributePerRate = (UsableBandwidth * XV_HDMITXSS1_FRL_TB_PER_3CHAR) /
				    XV_HDMITXSS1_FRL_CHAR_PER_2TB;

		/* Check if trained FRL rate can support this resolution */
		IsSupported = (MaxTributePerRate >= RequiredTribyte) ? TRUE : FALSE;

		/* Display diagnostic info when FRL support fails */
		if (!IsSupported) {
			u32 LinkBwGbps = TrainedLineRate * TrainedLanes;
			u32 ReqDispBw;

			/* Display audio bandwidth if enabled */
			if (HasAudio) {
				u32 AudioDispBw = (u32)(FrlAudioBwBps / XV_HDMITXSS1_FRL_AUDIO_MBPS_DIVISOR);
				u32 TotalReqBw = (u32)((TotalFrlMbps + 4) / XV_HDMITXSS1_FRL_DISP_DIVISOR);
				xil_printf("========================================\r\n");
				xil_printf("Requested Audio Bandwidth: %d.%02d Mbps\r\n",
					   AudioDispBw / XV_HDMITXSS1_FRL_DISP_DECIMAL,
					   AudioDispBw % XV_HDMITXSS1_FRL_DISP_DECIMAL);
				xil_printf("Total Requested Bandwidth: %d.%02d Gbps\r\n",
					   TotalReqBw / XV_HDMITXSS1_FRL_DISP_DECIMAL,
					   TotalReqBw % XV_HDMITXSS1_FRL_DISP_DECIMAL);
			}

			xil_printf("========================================\r\n");
			xil_printf("Info: Requested bandwidth exceeds trained link capacity\r\n");
			xil_printf("Current Link Bandwidth: %d Gbps (%d Lanes x %d Gbps)\r\n",
				   LinkBwGbps, TrainedLanes, TrainedLineRate);

			/* Display Link bandwidth */
			ReqDispBw = (FrlDataBw + 4) / XV_HDMITXSS1_FRL_DISP_DIVISOR;
			xil_printf("Requested Link Bandwidth: %d.%02d Gbps\r\n",
				   ReqDispBw / XV_HDMITXSS1_FRL_DISP_DECIMAL,
				   ReqDispBw % XV_HDMITXSS1_FRL_DISP_DECIMAL);

			xil_printf("========================================\r\n");
		}
		xil_printf("\r\n");

		return IsSupported;
	}
}
