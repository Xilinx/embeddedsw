/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <stdio.h>
#include <xil_exception.h>
#include <xil_cache.h>
#include <xil_printf.h>
#include <sleep.h>

#include "xmmidp.h"
#include "mmi_dp_init.h"

u8 Edid[128];
u8 EdidNext[128];

void XMmiDp_SinkPowerCycle(XMmiDp *DpPsuPtr)
{
	XMmiDp_SinkPowerDown(DpPsuPtr);
	xil_printf("Monitor in power saving mode\n");
	usleep(10000000);

	XMmiDp_SinkPowerUp(DpPsuPtr);
	xil_printf("Monitor in active mode\n");
	usleep(5000000);

}

void XMmiDp_SinkPowerDown(XMmiDp *DpPsuPtr)
{
	u8 Data[8];

	Data[0] = 0x0;

	XMmiDp_AuxWrite(DpPsuPtr,
			XMMIDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

void XMmiDp_SinkPowerUp(XMmiDp *DpPsuPtr)
{
	u8 Data[8];

	Data[0] = 0x1;

	XMmiDp_AuxWrite(DpPsuPtr,
			XMMIDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

void XMmiDp_SetVidControllerUseStdVidMode(XMmiDp *InstancePtr,
	XVidC_VideoMode VideoMode, u8 Stream)
{
	u8 BitsPerPixel;
	u32 TransferUnitSize;
	double VideoBw;
	double LinkBw;
	double divResult;
	double frac_part;

	XMmiDp_VideoConfig *VideoConfig;
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_LinkConfig *LinkConfig;
	const XVidC_VideoTimingMode *Vtm;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Stream == XMMIDP_STREAM_ID1);

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

	Xil_AssertVoid((MsaConfig->BitsPerColor == 6) ||
		       (MsaConfig->BitsPerColor == 8) ||
		       (MsaConfig->BitsPerColor == 10) ||
		       (MsaConfig->BitsPerColor == 12) ||
		       (MsaConfig->BitsPerColor == 16));

	VideoConfig = &InstancePtr->VideoConfig[Stream - 1];
	LinkConfig = &InstancePtr->LinkConfig;
	Vtm = XVidC_GetVideoModeData(VideoMode);

	VideoConfig->HActive = Vtm->Timing.HActive;
	VideoConfig->HBlank = Vtm->Timing.HFrontPorch +
			      Vtm->Timing.HBackPorch +
			      Vtm->Timing.HSyncWidth;
	VideoConfig->I_P = XVidC_IsInterlaced(Vtm->VmId);

	VideoConfig->VBlank = Vtm->Timing.F0PVFrontPorch +
			      Vtm->Timing.F0PVBackPorch +
			      Vtm->Timing.F0PVSyncWidth;
	VideoConfig->VActive = Vtm->Timing.VActive;

	VideoConfig->HSyncWidth = Vtm->Timing.HSyncWidth;
	VideoConfig->VSyncWidth = Vtm->Timing.F0PVSyncWidth;

	MsaConfig->HStart = Vtm->Timing.HSyncWidth + Vtm->Timing.HBackPorch;
	MsaConfig->VStart = Vtm->Timing.F0PVSyncWidth +
			    Vtm->Timing.F0PVBackPorch;

	VideoConfig->HSyncInPolarity = 0x1;
	VideoConfig->VSyncInPolarity = 0x1;

	MsaConfig->PixelClockHz =
		XVidC_GetPixelClockHzByVmId(Vtm->VmId);

	/* Values being set for 4K resolution running at 594 MHz */
	MsaConfig->NVid = 0x8000;
	MsaConfig->MVid = 0;
	MsaConfig->Misc0 = 0x28;
	MsaConfig->Misc1 = 0;
	MsaConfig->HBlankInterval = 0x00000690 ;

	BitsPerPixel = MsaConfig->BitsPerColor * 3;
	TransferUnitSize = 64;

	VideoBw = ((MsaConfig->PixelClockHz / 1000000) * BitsPerPixel) / 8;
	LinkBw = LinkConfig->NumLanes * LinkConfig->LinkRate;
	divResult = (VideoBw * TransferUnitSize) / LinkBw;
	VideoConfig->AvgBytesPerTu = divResult;

	frac_part = divResult - VideoConfig->AvgBytesPerTu;
	VideoConfig->AvgBytesPerTuFrac = frac_part * TransferUnitSize;

	if (VideoConfig->AvgBytesPerTu <= 16) {
		VideoConfig->InitThreshold = 32;
	} else {
		VideoConfig->InitThreshold = 16;
	}
}

void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr)
{

	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;

	XMmiDp_ClearVideoController(InstancePtr);

	XMmiDp_SetMsaBpc(InstancePtr, XMMIDP_STREAM_ID1, 8);
	XMmiDp_SetPixModeSel(InstancePtr, XMMIDP_STREAM_ID1, 0x0);
	XMmiDp_SetVideoMapping(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_RGB_8BPC);
	XMmiDp_SetVidStreamEnable(InstancePtr, XMMIDP_STREAM_ID1, 0x1);

	XMmiDp_SetVidControllerUseStdVidMode(InstancePtr,
					     XVIDC_VM_3840x2160_60_P, XMMIDP_STREAM_ID1);

	XMmiDp_ConfigureVideoController(InstancePtr, XMMIDP_STREAM_ID1);
}

void XMmiDp_SetupAudioStream(RunConfig *RunCfgPtr)
{
	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;

	XMmiDp_SetAudStreamInterfaceSel(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUD_INF_I2S);
	XMmiDp_SetAudDataInputEn(InstancePtr, XMMIDP_STREAM_ID1, 0xF);
	XMmiDp_SetAudDataWidth(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUDIO_INPUT_16_BIT);
	XMmiDp_SetAudHbrModeEn(InstancePtr, XMMIDP_STREAM_ID1, 0x0);
	XMmiDp_SetAudNumChannels(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUDIO_8_CHANNEL);
	XMmiDp_SetAudMuteFlag(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_CLEAR_AUDIOMUTE);
	XMmiDp_SetAudTimeStampVerNum(InstancePtr, XMMIDP_STREAM_ID1, 0x12);
	XMmiDp_SetAudClkMultFs(InstancePtr, XMMIDP_STREAM_ID1, XMMIDP_AUDIO_CLK_512FS);

	XMmiDp_SetSdpVertAudStreamEn(InstancePtr, XMMIDP_STREAM_ID1,0x1);
	XMmiDp_SetSdpVertAudTimeStampEn(InstancePtr, XMMIDP_STREAM_ID1, 0x1);

	XMmiDp_SetSdpHorAudStreamEn(InstancePtr, XMMIDP_STREAM_ID1,0x1);
	XMmiDp_SetSdpHorAudTimeStampEn(InstancePtr, XMMIDP_STREAM_ID1, 0x1);

	XMmiDp_ConfigureAudioController(InstancePtr, XMMIDP_STREAM_ID1);

}


u32 XMmiDp_HpdPoll(XMmiDp *InstancePtr)
{
	u32 GenIntStatus;
	u32 HpdIntStatus;
	u32 Status;

	Status = XST_FAILURE;
	while (1) {
		GenIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0);
		HpdIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0);

		xil_printf("GenIntStatus =0x%x \nHpdIntStatus=0x%x\n", GenIntStatus, HpdIntStatus);
		if ((GenIntStatus & XMMIDP_GEN_INT0_HPD_EVENT_MASK) == XMMIDP_GEN_INT0_HPD_EVENT_MASK) {
			break;
		}

	}

	if (HpdIntStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK) {
		xil_printf("HPD HOT PLUG\n");
		Status = XST_SUCCESS;
	}

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0, HpdIntStatus);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0, XMMIDP_GEN_INT0_HPD_EVENT_MASK);

	return Status;

}

/******************************************************************************/
/**
 * This function prepares forlink training process. It Checks for current link status.
 * Obtains Edid and RxCapabilities and initializes the Dpcd reg before initiating
 * link training algorithm.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *              - XST_SUCCESS if the training process succeeded.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_StartFullLinkTraining(XMmiDp *InstancePtr)
{
	u32 Status;

	Status = XMmiDp_GetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to get Rx Cap\n");
		return XST_FAILURE;
	}

	if ((InstancePtr->RxConfig.MaxLaneCount != XMMIDP_PHY_LANES_4) ||
		(InstancePtr->RxConfig.MaxLinkRate != XMMIDP_HBR3_LINK_RATE)) {
		xil_printf("Test is only valid for HBR3 and 4 lanes capable sinks\n");
		return XST_FAILURE;
	}

	InstancePtr->RxConfig.MaxLinkRate = XMMIDP_PHY_RATE_HBR3_810GBPS;

	Status = XMmiDp_CheckLinkStatus(InstancePtr, InstancePtr->RxConfig.MaxLaneCount);
	if ( Status == XST_SUCCESS) {
		xil_printf("Link is already trained for %x Lanes\n", InstancePtr->RxConfig.MaxLaneCount);

		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_PHYIF_CTRL_0);
		if (((Status & XMMIDP_PHYIF_CTRL_0_PHY_RATE_MASK)
		     >> XMMIDP_PHYIF_CTRL_0_PHY_RATE_SHIFT)
		    == (InstancePtr->RxConfig.MaxLinkRate)) {
			xil_printf("Link already trained at %d \n", InstancePtr->RxConfig.MaxLinkRate);
			return XST_SUCCESS;
		} else {
			xil_printf("Link needs to be retrained at %d \n", InstancePtr->RxConfig.MaxLinkRate);
		}

	} else if (Status == XST_FAILURE) {
		xil_printf(" Link needs training\n");
	} else {
		xil_printf("Error checking link status\n");
		return XST_FAILURE;
	}

	InstancePtr->LinkConfig.LaneCount = XMMIDP_PHY_LANES_4;
	InstancePtr->LinkConfig.NumLanes = 4;
	InstancePtr->LinkConfig.LinkRate = XMmiDp_GetLinkRate(InstancePtr, XMMIDP_DPCD_LINK_BW_SET_810GBPS);
	InstancePtr->LinkConfig.LinkBW = XMMIDP_DPCD_LINK_BW_SET_810GBPS;

	memset(InstancePtr->LinkConfig.VsLevel, 0, 4);
	memset(InstancePtr->LinkConfig.PeLevel, 0, 4);

	/* By default, Fast Link Train is enabled */
	XMmiDp_FastLinkTrainDisable(InstancePtr);

	Status = XMmiDp_SetSinkDpcdLinkCfgField(InstancePtr);
	if ( Status != XST_SUCCESS) {
		xil_printf("XMmiDp_SetsinkDpcdLinkCfgField Failed %d\n", Status);
		return Status;
	}

	Status = XMmiDp_StartLinkXmit(InstancePtr);
	if ( Status != XST_SUCCESS) {
		xil_printf("XMmiDp_StartLinkXmit Failed %d\n", Status);
		return Status;
	}

	Status = XMmiDp_RunTraining(InstancePtr);
	if ( Status != XST_SUCCESS) {
		return Status;
	}

	/* End Training */
	XMmiDp_SetPhyTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);
	XMmiDp_SetDpcdTrainingPattern(InstancePtr, XMMIDP_PHY_NO_TRAIN);

	xil_printf("Link Trained for %d Lanes\r\n", InstancePtr->LinkConfig.NumLanes);
	xil_printf("Link Trained for %d BW\r\n", InstancePtr->LinkConfig.LinkBW);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Main function to Initialize the DpPsu BaseAddress and Phy Soft Reset
*
* @param        None
*
* @return       XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note         None
*
******************************************************************************/
u32 XMmiDp_InitDpCore(XMmiDp *InstancePtr)
{
	XMmiDp_PhySoftReset(InstancePtr);
	XMmiDp_SetPhyPowerdown(InstancePtr, XMMIDP_PHY_POWER_ON);
	XMmiDp_SetAux_250Us_Cnt_Limit(InstancePtr, 0xF5);
	XMmiDp_SetAux_2000Us_Cnt_Limit(InstancePtr, 0x7A8);
	XMmiDp_SetAux_100000Us_Cnt_Limit(InstancePtr, 0x17ED0);
	XMmiDp_SetPmConfig1(InstancePtr, 0x00400008);
	XMmiDp_SetPmConfig2(InstancePtr, 0x00050000);
	XMmiDp_PhySSCDisable(InstancePtr);

	XMmiDp_GeneralInterruptEnable(InstancePtr, 0x0000000B);
	XMmiDp_HpdInterruptEnable(InstancePtr, 0x0000000F);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Function to Initialize the DpPsu Subsystem.Initializes the core, trains
* the link and configures video stream to display.
*
* @param        None
*
* @return       XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note         None
*
******************************************************************************/
u32 InitDpPsuSubsystem(RunConfig *RunCfgPtr)
{

	u32 Status = XST_SUCCESS;
	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;
	InstancePtr->Config.BaseAddr = DP_BASEADDR;


	XMmiDp_CfgInitialize(InstancePtr, InstancePtr->Config.BaseAddr);
	XMmiDp_Initialize(InstancePtr);
	XMmiDp_InitDpCore(InstancePtr);

	XMmiDp_HpdPoll(InstancePtr);

	XMmiDp_SinkPowerCycle(InstancePtr);
	Status = XMmiDp_StartFullLinkTraining(InstancePtr);
	if ( Status != XST_SUCCESS) {
		xil_printf("HBR3 Link Training Failed %d\n", Status);
		return Status;
	}
	XMmiDp_SetupVideoStream(RunCfgPtr);

	if(RunCfgPtr->AudioEnable)
		XMmiDp_SetupAudioStream(RunCfgPtr);

	return Status;
}
