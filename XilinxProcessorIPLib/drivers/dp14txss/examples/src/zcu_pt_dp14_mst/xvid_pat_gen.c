/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvid_pat_gen.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  shad   01/29/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/


//#include "xvid_pat_gen.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xdptxss.h"
#include "xclk_wiz.h"

#define XILINX_DISPLAYPORT_VID_BASE_ADDRESS		\
	XPAR_DP_TX_HIER_0_AV_PAT_GEN_0_BASEADDR
#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET	0x1000
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET	0x2000
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET	0x3000

u32 StreamOffset[4] = {0, XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET};

//u32 StreamOffsetAddr[4] = {0, XPAR_TX_SUBSYSTEM_AV_PAT_GEN_1_BASEADDR,
//		XPAR_TX_SUBSYSTEM_AV_PAT_GEN_2_BASEADDR,
//		XPAR_TX_SUBSYSTEM_AV_PAT_GEN_3_BASEADDR};

u8 StreamPattern_vpg[5] = {0x11, 0x13, 0x15, 0x16, 0x10};


#define NEW_PAT_GEN 1


/************************** Constant Definitions *****************************/


#define	PatternGeneratorEnable 0x0 //0 - Pattern Generator Enable
#define	VsyncPolarity 0x4 //0 Vsync Polarity 
#define	HsyncPolarity 0x8 //0 - Hsync Polarity 
#define	DataEnablePolarity 0xC //0 - Data Enable Polarity 
#define	VsyncWidth 0x10 //13:0 -  Vsync Width
#define	VerticalBackPorch 0x14 //13:0 - Vertical Back Porch 
#define	VerticalFrontPorch 0x18 //13:0 - Vertical Front Porch 
#define	VerticalResolution 0x1C //13:0 - Vertical Resolution 
#define	HsyncWidth 0x20 //13:0 - Hsync Width 
#define	HorizontalBackPorch 0x24 //13:0 - Horizontal Back Porch
#define	HorizontalFrontPorch 0x28 //13:0 - Horizontal Front Porch
#define	HorizontalResolution 0x2C //13:0 - Horizontal Resolution 
#define	Framelock_Enable_Delay 0x34 //31 - Frame lock Enable 
                                    //10:0 -  Frame lock Delay

#define	FrameLock_AlignHsync_LineFrac 0x3C //16: Frame Lock Align Hsync
                                           //10:0 - Frame lock Line Frac

#define	HdColorBarCfg 0x40 //2:0 - hd color bar cfg
#define	HSBLANK 0x44 //13:0 - HSBLANK 
#define	HSSYNC 0x48 //13:0 - HSSYNC
#define	HESYNC 0x4C //13:0 - HESYNC 
#define	HEBLNK 0x50 //13:0 - HEBLNK 
#define	VSBLNK 0x54 //13:0 - VSBLNK 
#define	VSSYNC 0x58 //13:0 - VSSYNC 
#define	VESYNC 0x5C //13:0 - VESYNC 
#define	VEBLNK 0x60 //13:0 - VEBLNK 
#define	MISC0 0x300 //7:0 - MISC0 
#define	MISC1 0x304 //7:0 - MISC1 

#define	TestPatternControl 0x308 //2:0 Test Pattern
                                 //4 - En Sw Pattern
                                 //8 - Dual Pixel Mode
                                 //9 - Quad Pixel Mode 

#define	Audio_Control 0x400 //0: Audio Reset
                            //1: Audio Start
                            //2: Audio Drop 

#define	AudioSampleRate_Chcount 0x404 //3:0 - Audio Sample Rate 
                                      //11:8 - Audio Channel Count

//AudioPatternCH
//1:0 - Audio Pattern Ch1
//11:8 - Audio Period 
#define	AudioPatternCH1 0x410
#define	AudioPatternCH2 0x420
#define	AudioPatternCH3 0x430 
#define	AudioPatternCH4 0x440
#define	AudioPatternCH5 0x450 
#define	AudioPatternCH6 0x460 
#define	AudioPatternCH7 0x470 
#define	AudioPatternCH8 0x480

#define AudioCHStatus1 0x4A0
#define AudioCHStatus2 0x4A4

#define	AudioCheckStart 0x4B8 //0 - Audio check start
#define	Timer 0x4C0 //31:0 - Timer 

#define DualPixelMode 0x100
#define QuadPixelMode 0x200

#include "sleep.h"
#define MicrosecToWait 1 /*to avoid hang with A53 system*/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/
/* This typedef specifies video pattern generator parameter information. */
typedef struct {
	XVidC_VideoTiming Timing;

	u32 DePolarity;
	u32 FrameLock0;
	u32 FrameLock1;
	u32 HdColorBarMode;

	u32 TcHsBlnk;
	u32 TcHsSync;
	u32 TcHeSync;
	u32 TcHeBlnk;
	u32 TcVsBlnk;
	u32 TcVsSync;
	u32 TcVeSync;
	u32 TcVeBlnk;

	u32 VidClkSel;
	u32 MVid;
	u32 VidClkD;

	u32 DSMode;
	u32 MvidBy2;
	u32 VidClkDBy2;

	u32 Misc0;
	u32 Misc1;
} Vpg_VidgenConfig;

/************************** Function Prototypes ******************************/

static void VidgenSetConfig(XDp *InstancePtr, Vpg_VidgenConfig *VidgenConfig,
				u8 Stream, u8 VSplitMode, u8 first_time);
static void VidgenWriteConfig(XDp *InstancePtr,
				Vpg_VidgenConfig *VidgenConfig, u8 Stream);
void Vpg_VidgenSetTestPattern(XDp *InstancePtr, u8 Stream);

void Vpg_Audio_start(void);
void Vpg_Audio_stop(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function updates required timing values in Video Pattern Generator
* core.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	VSplitMode specifies vertical split mode to be used for
*		splitting resolution vertically.
*
* @return
*		- XST_SUCCESS if Video pattern configured successfully.
*
* @note		None.
*
******************************************************************************/
int Vpg_StreamSrcConfigure(XDp *InstancePtr, u8 VSplitMode, u8 first_time)
{
	Vpg_VidgenConfig VidgenConfig;
	u8 StreamIndex;

	/* Set up timing parameters */
	memset((void *)&VidgenConfig.Timing, 0, sizeof(XVidC_VideoTiming));

	/* Check MST enable flag */
	if (InstancePtr->TxInstance.MstEnable) {
		for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
			if (XDp_TxMstStreamIsEnabled(InstancePtr,
					(XDP_TX_STREAM_ID1) + StreamIndex)) {
				/* Calculate VPG parameters */
				VidgenSetConfig(InstancePtr, &VidgenConfig,
					(XDP_TX_STREAM_ID1) + StreamIndex,
						VSplitMode, first_time);

				/* Update VPG with parameter */
				VidgenWriteConfig(InstancePtr, &VidgenConfig,
					(XDP_TX_STREAM_ID1) + StreamIndex);

				/* Set pattern to stream 1, 2, 3, 4 */
				Vpg_VidgenSetTestPattern(InstancePtr,
					(XDP_TX_STREAM_ID1) + StreamIndex);

				/* Enable VPG for each stream. Number of VPG
				 * are equal to number of streams
				 */
				XDp_WriteReg(
					XILINX_DISPLAYPORT_VID_BASE_ADDRESS,
						StreamOffset[StreamIndex],
							0x1);
			}
		}
	}
	else {
		/* Calculate VPG parameters */
		VidgenSetConfig(InstancePtr, &VidgenConfig,
					(XDP_TX_STREAM_ID1), 0, first_time);

		/* Update VPG with parameter */
		VidgenWriteConfig(InstancePtr, &VidgenConfig,
					(XDP_TX_STREAM_ID1));

		/* Set pattern to stream 1, 2, 3, 4 */
		Vpg_VidgenSetTestPattern(InstancePtr,
			(XDP_TX_STREAM_ID1));

		/* Enable VPG for only one stream */
		XDp_WriteReg(XILINX_DISPLAYPORT_VID_BASE_ADDRESS,
				PatternGeneratorEnable, 0x1);
	}

	return (XST_SUCCESS);
}



/*****************************************************************************/
/**
*
* This function sets pattern to generate in Video Pattern Generator core.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	Stream is the stream number for which to set the pattern.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Vpg_VidgenSetTestPattern(XDp *InstancePtr, u8 Stream)
{
#if !NEW_PAT_GEN
	if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x4) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], TestPatternControl,
				(QuadPixelMode | StreamPattern_vpg[Stream - 1]));
	}
	else if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x2) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], TestPatternControl,
				(DualPixelMode | StreamPattern_vpg[Stream - 1]));
	}
	else {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], TestPatternControl,
			StreamPattern_vpg[Stream - 1]);
	}
#else
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[Stream - 1], TestPatternControl,
			(QuadPixelMode | StreamPattern_vpg[Stream - 1]));


#endif
}

/*****************************************************************************/
/**
*
* This function sets user pattern
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	Pattern number to set with
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern)
{
	u8 StreamIndex = 1;

	                for (StreamIndex = 1; StreamIndex < 5; StreamIndex++) {
#if !NEW_PAT_GEN
	if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x4) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[StreamIndex-1], TestPatternControl,
				(QuadPixelMode | (Pattern+(StreamIndex-1))));

	}
	else if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x2) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[StreamIndex-1], TestPatternControl,
			(DualPixelMode | (Pattern+(StreamIndex-1))));

	}
	else {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[StreamIndex-1], TestPatternControl,
			(Pattern+(StreamIndex-1)));

	}
#else
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[StreamIndex-1], TestPatternControl,
			(QuadPixelMode | (Pattern+(StreamIndex-1))));

	usleep(MicrosecToWait);

#endif
	                }
}


/*****************************************************************************/
/**
*
* This function starts audio pattern generator
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Vpg_Audio_start(void){
	//unsigned int MicrosecToWait = 1;
	usleep(MicrosecToWait);

	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], Audio_Control,	0x1);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], Audio_Control,	0x2);


	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], AudioPatternCH1,0x2);

	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], AudioPatternCH2,0x2);

	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], AudioCHStatus1, 0x10000244);//channel status

	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], AudioCHStatus2,0x40000000);//channel status

	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], AudioSampleRate_Chcount,0x202);
}

/*****************************************************************************/
/**
*
* This function stops audio pattern generator
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Vpg_Audio_stop(void){
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[0], Audio_Control,0x0);
}

/*****************************************************************************/
/**
*
* This function calculates video pattern generator parameters.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	VidgenConfig is a pointer to Vpg_VidgenConfig that will be
*		used to timing parameters.
* @param	Stream is the stream number that will be used to ret rive
*		MSA parameters.
* @param	VSplitMode specifies vertical split mode to be used for
*		splitting resolution vertically.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void VidgenSetConfig(XDp *InstancePtr, Vpg_VidgenConfig *VidgenConfig,
				u8 Stream, u8 VSplitMode, u8 first_time)
{
	XDp_TxMainStreamAttributes *MsaConfig =
			&InstancePtr->TxInstance.MsaConfig[Stream - 1];
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;
	u32 UserPixelWidth;
	u8 DSBypass;
	XVidC_VideoMode VmId;
//	u32 Status;
    u32 Count = 0;
	VmId = MsaConfig->Vtm.VmId;

	/* Configure MSA values from the Display Monitor Timing (DMT) table.
	 * Will provide a way to optionally acquire these values from the EDID
	 * of the sink.
	 */
	VidgenConfig->MVid = MsaConfig->PixelClockHz / 1000;
	VidgenConfig->VidClkSel =
			(LinkConfig->LinkRate == (XDP_TX_LINK_BW_SET_270GBPS));

	UserPixelWidth = MsaConfig->UserPixelWidth;
	VidgenConfig->Misc0 = MsaConfig->Misc0;
	VidgenConfig->Misc1 = MsaConfig->Misc1;
#if !NEW_PAT_GEN
	VidgenConfig->DePolarity = 0x0;
	VidgenConfig->FrameLock0 = 0;
	VidgenConfig->FrameLock1 = 0;
	VidgenConfig->HdColorBarMode = 0;


	VidgenConfig->Timing.VSyncPolarity =
					MsaConfig->Vtm.Timing.VSyncPolarity;
	VidgenConfig->Timing.HSyncPolarity =
					MsaConfig->Vtm.Timing.HSyncPolarity;
	VidgenConfig->Timing.F0PVSyncWidth =
					MsaConfig->Vtm.Timing.F0PVSyncWidth;
	VidgenConfig->Timing.F0PVBackPorch =
					MsaConfig->Vtm.Timing.F0PVBackPorch;
	VidgenConfig->Timing.F0PVFrontPorch =
					MsaConfig->Vtm.Timing.F0PVFrontPorch;
	VidgenConfig->Timing.VActive = MsaConfig->Vtm.Timing.VActive;

	/* In Vertical frame split mode double the Horizontal parameters to
	* VPG
	*/
	if ((VmId == XVIDC_VM_UHD2_60_P) && VSplitMode) {
		/* Re-compute horizontal values based on user pixel width. */
		VidgenConfig->Timing.HActive =
			MsaConfig->Vtm.Timing.HActive * 2 / UserPixelWidth;
		VidgenConfig->Timing.HBackPorch =
			MsaConfig->Vtm.Timing.HBackPorch * 2 / UserPixelWidth;
		VidgenConfig->Timing.HFrontPorch =
			MsaConfig->Vtm.Timing.HFrontPorch * 2 / UserPixelWidth;
		VidgenConfig->Timing.HSyncWidth =
			MsaConfig->Vtm.Timing.HSyncWidth * 2 / UserPixelWidth;

		VidgenConfig->MVid /= 2;
		DSBypass = 0;
	}
	else {
		/* Re-compute horizontal values based on user pixel width. */
		VidgenConfig->Timing.HActive =
			MsaConfig->Vtm.Timing.HActive / UserPixelWidth;
		VidgenConfig->Timing.HBackPorch =
			MsaConfig->Vtm.Timing.HBackPorch / UserPixelWidth;
		VidgenConfig->Timing.HFrontPorch =
			MsaConfig->Vtm.Timing.HFrontPorch / UserPixelWidth;
		VidgenConfig->Timing.HSyncWidth =
			MsaConfig->Vtm.Timing.HSyncWidth / UserPixelWidth;

		VidgenConfig->MVid /= UserPixelWidth;
		DSBypass = 1;
	}

	if (DSBypass == 1) {
		VidgenConfig->DSMode = 0;
	}
	else {
		VidgenConfig->DSMode = 1;
	}

	/* Configure the pattern generator. */
	VidgenConfig->TcHsBlnk = VidgenConfig->Timing.HActive - 1;
	VidgenConfig->TcHsSync = VidgenConfig->Timing.HActive +
				VidgenConfig->Timing.HFrontPorch - 1 ;
	VidgenConfig->TcHeSync = VidgenConfig->Timing.HActive +
				VidgenConfig->Timing.HFrontPorch +
				VidgenConfig->Timing.HSyncWidth - 1;
	VidgenConfig->TcHeBlnk = VidgenConfig->Timing.HActive +
				VidgenConfig->Timing.HFrontPorch +
				VidgenConfig->Timing.HSyncWidth +
				VidgenConfig->Timing.HBackPorch - 1;
	VidgenConfig->TcVsBlnk = VidgenConfig->Timing.VActive - 1;
	VidgenConfig->TcVsSync = VidgenConfig->Timing.VActive +
				VidgenConfig->Timing.F0PVFrontPorch - 1;
	VidgenConfig->TcVeSync = VidgenConfig->Timing.VActive +
				VidgenConfig->Timing.F0PVFrontPorch +
				VidgenConfig->Timing.F0PVSyncWidth - 1;
	VidgenConfig->Timing.F0PVTotal = VidgenConfig->Timing.VActive +
				VidgenConfig->Timing.F0PVFrontPorch +
				VidgenConfig->Timing.F0PVSyncWidth +
				VidgenConfig->Timing.F0PVBackPorch - 1;
#else

	VidgenConfig->Timing.VActive = MsaConfig->Vtm.Timing.VActive;
	VidgenConfig->Timing.HActive =
		MsaConfig->Vtm.Timing.HActive / 4;

#endif
}

/*****************************************************************************/
/**
*
* This function writes timing parameters to Video Pattern Generator core.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	VidgenConfig is a pointer to Vpg_VidgenConfig that will be
*		used to write parameter values into Video Pattern Generator
*		core registers.
* @param	Stream is the stream number used to identify the steam for
*		which parameters to be written.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void VidgenWriteConfig(XDp *InstancePtr,
				Vpg_VidgenConfig *VidgenConfig, u8 Stream)
{
#if !NEW_PAT_GEN
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VsyncPolarity,
				VidgenConfig->Timing.VSyncPolarity | 1);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HsyncPolarity,
				VidgenConfig->Timing.HSyncPolarity | 1);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VsyncWidth,
				VidgenConfig->Timing.F0PVSyncWidth);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VerticalBackPorch,
				VidgenConfig->Timing.F0PVBackPorch);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VerticalFrontPorch,
				VidgenConfig->Timing.F0PVFrontPorch);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VerticalResolution,
				VidgenConfig->Timing.VActive);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HsyncWidth,
				VidgenConfig->Timing.HSyncWidth);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HorizontalBackPorch,
				VidgenConfig->Timing.HBackPorch);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HorizontalFrontPorch,
				VidgenConfig->Timing.HFrontPorch);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HorizontalResolution,
				VidgenConfig->Timing.HActive);
	usleep(MicrosecToWait);


	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], DataEnablePolarity,
				VidgenConfig->DePolarity);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], Framelock_Enable_Delay,
				VidgenConfig->FrameLock0);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], FrameLock_AlignHsync_LineFrac,
				VidgenConfig->FrameLock1);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HdColorBarCfg,
				VidgenConfig->HdColorBarMode);

	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HSBLANK,
				VidgenConfig->TcHsBlnk);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HSSYNC,
				VidgenConfig->TcHsSync);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HESYNC,
				VidgenConfig->TcHeSync);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HEBLNK,
				VidgenConfig->TcHeBlnk);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VSBLNK,
				VidgenConfig->TcVsBlnk);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VSSYNC,
				VidgenConfig->TcVsSync);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VESYNC,
				VidgenConfig->TcVeSync);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VEBLNK,
				VidgenConfig->Timing.F0PVTotal);
	usleep(MicrosecToWait);
#else
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VerticalResolution,
				VidgenConfig->Timing.VActive);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HorizontalResolution,
				VidgenConfig->Timing.HActive);
	usleep(MicrosecToWait);
#endif

	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], MISC0,
				InstancePtr->TxInstance.MsaConfig[Stream - 1].Misc0);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], MISC1,
				InstancePtr->TxInstance.MsaConfig[Stream - 1].Misc1);
}


