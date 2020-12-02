/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
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


#include "xvid_pat_gen.h"

u32 StreamOffset[4] = {0, XILINX_DISPLAYPORT_VID2_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID3_BASE_ADDRESS_OFFSET,
			  XILINX_DISPLAYPORT_VID4_BASE_ADDRESS_OFFSET};

u8 StreamPattern[5] = {0x11, 0x13, 0x15, 0x16, 0x10};

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

static void VidgenSetConfig(XDp *InstancePtr, Vpg_VidgenConfig *VidgenConfig,
				u8 Stream, u8 VSplitMode, u8 first_time);
static void VidgenWriteConfig(XDp *InstancePtr,
				Vpg_VidgenConfig *VidgenConfig, u8 Stream);
//static void WaitTxVsyncs(XDp *InstancePtr, u32 LoopCount, u8 Stream);
static void VidgenComputeMVid(XDp *InstancePtr,
				Vpg_VidgenConfig *VidgenConfig);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function enables video pattern generator.
*
* @param	InstancePtr is a pointer to the XDp instance.
*
* @return
*		- XST_SUCCESS if Video pattern setup successful.
*
* @note		None.
*
******************************************************************************/
int Vpg_StreamSrcSetup(XDp *InstancePtr)
{
//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS), 0x0, 0x2); //0x2
//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS), 0x0, 0x0); //0x0

	return (XST_SUCCESS);
}

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

	//	if(!first_time){
			/* Set pattern to stream 1 */
			Vpg_VidgenSetTestPattern(InstancePtr, (XDP_TX_STREAM_ID1));
	//	}

		/* Enable VPG for only one stream */
		XDp_WriteReg(XILINX_DISPLAYPORT_VID_BASE_ADDRESS, 0x0, 0x1);
//		xil_printf ("Kapil VPG\r\n");
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function calls delay/sleep function registered by an application to
* synchronizes with video streams.
*
* @param	InstancePtr is a pointer to the XDp instance.
*
* @return
*		- XST_SUCCESS if video stream sync successful.
*
* @note		None.
*
******************************************************************************/
int Vpg_StreamSrcSync(XDp *InstancePtr)
{
//	u8 StreamIndex;
//
//	/* Check MST enable flag */
//	if (InstancePtr->TxInstance.MstEnable) {
//		for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
//			if (XDp_TxMstStreamIsEnabled(InstancePtr,
//					(XDP_TX_STREAM_ID1) + StreamIndex)) {
//				WaitTxVsyncs(InstancePtr, 20,
//					(XDP_TX_STREAM_ID1) + StreamIndex);
//			}
//		}
//	}
//	else {
//		WaitTxVsyncs(InstancePtr, 20, (XDP_TX_STREAM_ID1));
//	}
//
//	XDp_WaitUs(InstancePtr, 200000);

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
	if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x4) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x308,
				(0x200 | StreamPattern[Stream - 1]));
	}
	else if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x2) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x308,
			(0x100 | StreamPattern[Stream - 1]));
	}
	else {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x308,
			StreamPattern[Stream - 1]);
	}
}

void Vpg_VidgenSetUserPattern(XDp *InstancePtr, u8 Pattern)
{
	if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x4) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[0], 0x308,
				(0x200 | Pattern));
	}
	else if (XDp_ReadReg(InstancePtr->Config.BaseAddr,
				(XDP_TX_USER_PIXEL_WIDTH)) == 0x2) {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[0], 0x308,
			(0x100 | Pattern));
	}
	else {
		XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[0], 0x308,
			Pattern);
	}
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
//	u32 MVidCalculated;
//	u32 DValCalculated;
//	u32 MVidOriginal;
//	u32 VidClkDCalculatedBy2;
	XVidC_VideoMode VmId;

	VmId = MsaConfig->Vtm.VmId;

//	xil_printf("Clk in HZ:%d, PW:%d\n\r",MsaConfig->PixelClockHz,
//			MsaConfig->UserPixelWidth);

	ComputeMandD(InstancePtr,
			((MsaConfig->PixelClockHz/1000)/MsaConfig->UserPixelWidth) );

	/* Configure MSA values from the Display Monitor Timing (DMT) table.
	 * Will provide a way to optionally acquire these values from the EDID
	 * of the sink.
	 */
	VidgenConfig->MVid = MsaConfig->PixelClockHz / 1000;
	VidgenConfig->VidClkSel =
			(LinkConfig->LinkRate == (XDP_TX_LINK_BW_SET_270GBPS));

	UserPixelWidth = MsaConfig->UserPixelWidth;
//	VidgenConfig->MVid /= UserPixelWidth;
	VidgenConfig->Misc0 = MsaConfig->Misc0;
	VidgenConfig->Misc1 = MsaConfig->Misc1;

	VidgenConfig->DePolarity = 0x0;
	VidgenConfig->FrameLock0 = 0;
	VidgenConfig->FrameLock1 = 0;
	VidgenConfig->HdColorBarMode = 0;

//	xil_printf("MSAConfig id = %d \r\n",MsaConfig->Vtm.VmId);

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
//		xil_printf("VPG HRES = %d PW = %d DSBypass=%d\n\r",
//			VidgenConfig->Timing.HActive,UserPixelWidth, DSBypass);
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
//		xil_printf("VPG HRES = %d PixWidth = %d DSBypass=%d\n\r",
//			VidgenConfig->Timing.HActive,UserPixelWidth, DSBypass);
	}

	/* Calculate MVid */
	/* Take the MVid to local Variable */
//	MVidOriginal = VidgenConfig->MVid;

	/* Compute M and D values */
	VidgenComputeMVid(InstancePtr, VidgenConfig);
//	MVidCalculated = VidgenConfig->MVid;
//	DValCalculated = VidgenConfig->VidClkD;

	if (DSBypass == 1) {
		VidgenConfig->DSMode = 0;
	}
	else {
		VidgenConfig->DSMode = 1;
	}

//	xil_printf("DSMode = %d\n\r",	VidgenConfig->DSMode);

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
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x04,
				VidgenConfig->Timing.VSyncPolarity | 1);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x08,
				VidgenConfig->Timing.HSyncPolarity | 1);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x10,
				VidgenConfig->Timing.F0PVSyncWidth);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x14,
				VidgenConfig->Timing.F0PVBackPorch);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x18,
				VidgenConfig->Timing.F0PVFrontPorch);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x1C,
				VidgenConfig->Timing.VActive);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x20,
				VidgenConfig->Timing.HSyncWidth);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x24,
				VidgenConfig->Timing.HBackPorch);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x28,
				VidgenConfig->Timing.HFrontPorch);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x2C,
				VidgenConfig->Timing.HActive);


	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x0C,
				VidgenConfig->DePolarity);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x34,
				VidgenConfig->FrameLock0);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x3C,
				VidgenConfig->FrameLock1);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x40,
				VidgenConfig->HdColorBarMode);

	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x44,
				VidgenConfig->TcHsBlnk);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x48,
				VidgenConfig->TcHsSync);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x4C,
				VidgenConfig->TcHeSync);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x50,
				VidgenConfig->TcHeBlnk);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x54,
				VidgenConfig->TcVsBlnk);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x58,
				VidgenConfig->TcVsSync);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x5C,
				VidgenConfig->TcVeSync);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x60,
				VidgenConfig->Timing.F0PVTotal);

//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x100,
//				VidgenConfig->VidClkSel);
//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x104,
//				VidgenConfig->MVid);
//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x108,
//				VidgenConfig->VidClkD);

//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x10C,
//				VidgenConfig->DSMode);
//	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x110,
//				VidgenConfig->VidClkDBy2);

	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x300,
				InstancePtr->TxInstance.MsaConfig[0].Misc0);
	XDp_WriteReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
			StreamOffset[Stream - 1], 0x304,
				InstancePtr->TxInstance.MsaConfig[0].Misc1);
}

/*****************************************************************************/
/**
*
* This function provides delay/sleep for vertical sync.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	LoopCount specifies a loop counter.
* @param	Stream is the stream number used to identify the steam for
*		which parameters to be read.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
//static void WaitTxVsyncs(XDp *InstancePtr, u32 LoopCount, u8 Stream)
//{
//	u32 VBlank = XDp_ReadReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x60);
//	u32 VCount = XDp_ReadReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//			StreamOffset[Stream - 1], 0x200);
//	u32 Start = XDp_ReadReg(InstancePtr->Config.BaseAddr,
//			(XDP_TX_INTERRUPT_STATUS) + 0x14);
//	u32 End;
//	u32 Diff;
//
//	while (LoopCount > 0) {
//		End = XDp_ReadReg(InstancePtr->Config.BaseAddr,
//				(XDP_TX_INTERRUPT_STATUS) + 0x14);
//
//		if (Start > End) {
//			Diff = Start - End;
//		}
//		else {
//			Diff = End - Start;
//		}
//
//		if (VCount >= VBlank) {
//			LoopCount--;
//			Diff = 0;
//		}
//		else if (Diff > 100000000) {
//			LoopCount = 0;
//		}
//
//		VCount = XDp_ReadReg((XILINX_DISPLAYPORT_VID_BASE_ADDRESS) +
//				StreamOffset[Stream - 1], 0x200);
//	}
//}


/*****************************************************************************/
/**
*
* This function calculates the M-VID if synchronous mode is used.
*
* @param	InstancePtr is a pointer to the XDp instance.
* @param	VidgenConfig is a pointer to Vpg_VidgenConfig used to update
*		calculated M-VID value.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void VidgenComputeMVid(XDp *InstancePtr,
				Vpg_VidgenConfig *VidgenConfig)
{
	XDp_TxLinkConfig *LinkConfig = &InstancePtr->TxInstance.LinkConfig;

	u32 RefFreq;
	u32 VidFreq = VidgenConfig->MVid;
	u32 MIndex;
	u32 DIndex;
	u32 Div;
	u32 Freq;
	u32 Diff;
	u32 Fvco;
	u32 Minerr = 10000;
	u32 MVal = 0;
	u32 DVal = 0;
	u32 DivVal = 0;

	RefFreq = (LinkConfig->LinkRate == (XDP_TX_LINK_BW_SET_540GBPS)) ?
			270000/2 : (LinkConfig->LinkRate ==
				(XDP_TX_LINK_BW_SET_270GBPS)) ? 135000/2 : 81000/2;

	if (InstancePtr->Config.PayloadDataWidth == 4) {
		RefFreq /= 2;
	}

	for (MIndex = 20; MIndex <= 64; MIndex++) {
		for (DIndex = 1; DIndex <= 80; DIndex++) {
			Fvco = RefFreq * MIndex / DIndex;

			if (Fvco >= 600000 && Fvco <= 900000) {
				for (Div = 1; Div <= 128; Div++) {
					Freq = Fvco/Div;

					if (Freq >= VidFreq) {
						Diff = Freq - VidFreq;
					}
					else {
						Diff = VidFreq - Freq;
					}

					if (Diff == 0) {
						MVal = MIndex;
						DVal = DIndex;
						DivVal = Div;
						MIndex = 257;
						DIndex = 257;
						Div = 257;
						Minerr = 0;
					}
					else if (Diff < Minerr) {
						Minerr = Diff;
						MVal = MIndex;
						DVal = DIndex;
						DivVal = Div;

						if (Minerr < 100) {
							MIndex = 257;
							DIndex = 257;
							Div = 257;
						}
					}
				}
			}
		}
	}

	VidgenConfig->MVid = MVal;
	VidgenConfig->VidClkD = (DivVal & 0xff) | ((DVal & 0xff) << 8);
}
