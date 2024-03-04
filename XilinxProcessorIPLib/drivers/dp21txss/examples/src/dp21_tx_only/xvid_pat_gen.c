/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

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
* Ver  Who      Date      Changes
* ---- ---      --------  --------------------------------------------------.
* 1.00  ND      18/10/22  Common DP 2.1 tx only application for zcu102 and
* 						  vcu118
* 1.01	ND		26/02/24  Added support for 13.5 and 20G
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xparameters.h"
#include "xdptxss.h"

#define XILINX_DISPLAYPORT_VID_BASE_ADDRESS		\
	XPAR_TX_SUBSYSTEM_AV_PAT_GEN_0_BASEADDR
#define XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET	0x10000
#define XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET	0x20000
#define XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET	0x30000

u32 StreamOffset[4] = {0, XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET};

u8 StreamPattern_vpg[5] = {0x11, 0x13, 0x15, 0x16, 0x10};

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
#define OctaPixelMode 0x400

#if !defined (XPS_BOARD_ZCU102)
#define PIXEL_MODE OctaPixelMode
#else
#define PIXEL_MODE QuadPixelMode
#endif

#include "sleep.h"
#define MicrosecToWait 1 // to avoid hang in A53

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
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[Stream - 1], TestPatternControl,
			(PIXEL_MODE | StreamPattern_vpg[Stream - 1]));
	usleep(MicrosecToWait);
	/* Enable VPG for only one stream */
	XDp_WriteReg(XILINX_DISPLAYPORT_VID_BASE_ADDRESS,
			PatternGeneratorEnable, 0x1);
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
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
		StreamOffset[0], TestPatternControl,
			(PIXEL_MODE | Pattern));
	usleep(MicrosecToWait);
	/* Enable VPG for only one stream */
	XDp_WriteReg(XILINX_DISPLAYPORT_VID_BASE_ADDRESS,
			PatternGeneratorEnable, 0x1);
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
	VidgenConfig->Timing.VActive = MsaConfig->Vtm.Timing.VActive;
	VidgenConfig->Timing.HActive =
#if (PIXEL_MODE == OctaPixelMode)
		MsaConfig->Vtm.Timing.HActive / 8;
#else
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
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], VerticalResolution,
				VidgenConfig->Timing.VActive);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], HorizontalResolution,
				VidgenConfig->Timing.HActive);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], MISC0,
				InstancePtr->TxInstance.MsaConfig[0].Misc0);
	usleep(MicrosecToWait);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], MISC1,
				InstancePtr->TxInstance.MsaConfig[0].Misc1);
}
