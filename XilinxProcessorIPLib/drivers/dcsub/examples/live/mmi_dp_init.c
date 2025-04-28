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

void SinkPowerCycle(XMmiDp *DpPsuPtr)
{
	SinkPowerDown(DpPsuPtr);
	usleep(10000);

	SinkPowerUp(DpPsuPtr);
	usleep(5000);

	SinkPowerUp(DpPsuPtr);
	usleep(4000);
}

void SinkPowerDown(XMmiDp *DpPsuPtr)
{
	u8 Data[8];

	Data[0] = 0x2;

	XMmiDp_AuxWrite(DpPsuPtr,
			XMMIDP_DPCD_SET_POWER_DP_PWR_VOLTAGE, 1, Data);
}

void SinkPowerUp(XMmiDp *DpPsuPtr)
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
	u32 VideoBw;
	u32 LinkBw;
	u32 TransferUnitSize;

	XMmiDp_VideoConfig *VideoConfig;
	XMmiDp_VSampleCtrl *VSampleCtrl;
	XMmiDp_MainStreamAttributes *MsaConfig;
	XMmiDp_LinkConfig *LinkConfig;
	XVidC_VideoTimingMode *Vtm;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XMMIDP_STREAM_ID1) ||
		       (Stream == XMMIDP_STREAM_ID2) ||
		       (Stream == XMMIDP_STREAM_ID3) ||
		       (Stream == XMMIDP_STREAM_ID4));

	MsaConfig = &InstancePtr->MsaConfig[Stream - 1];

	Xil_AssertVoid((MsaConfig->BitsPerColor == 6) ||
		       (MsaConfig->BitsPerColor == 8) ||
		       (MsaConfig->BitsPerColor == 10) ||
		       (MsaConfig->BitsPerColor == 12) ||
		       (MsaConfig->BitsPerColor == 16));

	VideoConfig = &InstancePtr->VideoConfig[Stream - 1];
	VSampleCtrl = &InstancePtr->VSampleCtrl[Stream - 1];
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

	MsaConfig->NVid = 0x8000;
	MsaConfig->MVid = 0;
	MsaConfig->Misc0 = 0x28;
	MsaConfig->Misc1 = 0;
	MsaConfig->HBlankInterval = 0x0000035f;

	BitsPerPixel = MsaConfig->BitsPerColor * 3;
	VideoConfig->AvgBytesPerTuFrac = 0x3D;
	VideoConfig->InitThreshold = 0x20;
	TransferUnitSize = 64;

	VideoBw = ((MsaConfig->PixelClockHz / 1000) * BitsPerPixel) / 8;
	LinkBw = (LinkConfig->LaneCount * LinkConfig->LinkRate * 27);
	VideoConfig->AvgBytesPerTu = (VideoBw * TransferUnitSize) / LinkBw;
	VideoConfig->AvgBytesPerTu = 0x8;
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
					     XVIDC_VM_640x480_60_P, XMMIDP_STREAM_ID1);

	XMmiDp_ConfigureVideoController(InstancePtr, XMMIDP_STREAM_ID1);
}

u32 XMmiDp_HpdPoll(XMmiDp *InstancePtr)
{
	u32 GenIntStatus;
	u32 HpdIntStatus;
	u32 HpdState;
	u32 HpdStatus;
	u32 HpdUnplugErr;
	u32 HpdHotUnplug;
	u32 HpdHotPlug;
	u32 HpdIrq;
	u32 HpdEvent;
	u32 RegVal;
	u32 Status;

	Status = XST_FAILURE;
	GenIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0);

	HpdEvent = (GenIntStatus & XMMIDP_GEN_INT0_HPD_EVENT_MASK) >> XMMIDP_GEN_INT0_HPD_EVENT_SHIFT;
	HpdIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0);
	HpdState = (HpdIntStatus & XMMIDP_HPD_STATUS0_STATE_MASK) >> XMMIDP_HPD_STATUS0_STATE_SHIFT;
	HpdStatus = (HpdIntStatus & XMMIDP_HPD_STATUS0_STATUS_MASK) >> XMMIDP_HPD_STATUS0_STATUS_SHIFT;
	HpdUnplugErr = (HpdIntStatus & XMMIDP_HPD_STATUS0_UNPLUG_ERR_MASK) >>
		       XMMIDP_HPD_STATUS0_UNPLUG_ERR_SHIFT;
	HpdHotUnplug = (HpdIntStatus & XMMIDP_HPD_STATUS0_HOT_UNPLUG_MASK) >>
		       XMMIDP_HPD_STATUS0_HOT_UNPLUG_SHIFT;
	HpdHotPlug = (HpdIntStatus & XMMIDP_HPD_STATUS0_HOT_PLUG_MASK) >> XMMIDP_HPD_STATUS0_HOT_PLUG_SHIFT;
	HpdIrq = (HpdIntStatus & XMMIDP_HPD_STATUS0_HPD_IRQ_MASK) >> XMMIDP_HPD_STATUS0_HPD_IRQ_SHIFT;

	if (HpdEvent) {
		if (HpdStatus && HpdIrq) {
			HpdIntStatus = 0;

			xil_printf("HPD IRQ Detected\n");

			Status = StartFullLinkTraining(InstancePtr);
			if (Status != XST_SUCCESS) {
				xil_printf("DpPsu LinkTraining failed\n");
				return Status;
			}

			HpdIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0);
			HpdIntStatus |= XMMIDP_HPD_STATUS0_STATUS_MASK;
			XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_HPD_STATUS0, HpdIntStatus);

		} else if (!HpdStatus && HpdHotUnplug) {
			GenIntStatus = 0;

			xil_printf("HPD Unplug Detected\n");
			XMmiDp_SetPhyXmitDisable(InstancePtr);

			GenIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0);
			GenIntStatus |= XMMIDP_GEN_INT0_HPD_EVENT_MASK;
			XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0, GenIntStatus);

		} else if (HpdStatus && HpdHotPlug) {

			GenIntStatus = 0;

			xil_printf("HPD HotPlug Detected\n");
			Status = StartFullLinkTraining(InstancePtr);
			if (Status != XST_SUCCESS) {
				xil_printf("DpPsu LinkTraining failed\n");
				return Status;
			}

			GenIntStatus = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0);
			GenIntStatus |= XMMIDP_GEN_INT0_HPD_EVENT_MASK;
			XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_GEN_INT0, GenIntStatus);
		}
	} else {
		xil_printf("Waiting for HPD event\n");
	}

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
u32 StartFullLinkTraining(XMmiDp *InstancePtr)
{
	u32 Status;

	Status = XMmiDp_GetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to get Rx Cap\n");
		return XST_FAILURE;
	}

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
		xil_printf("Link needs training\n");
	} else {
		xil_printf("Error checking link status\n");
		return XST_FAILURE;
	}

	InstancePtr->LinkConfig.LaneCount = PHY_LANES_2;
	InstancePtr->LinkConfig.NumLanes = 2;
	InstancePtr->LinkConfig.LinkRate = PHY_RATE_HBR_270GBPS;
	InstancePtr->LinkConfig.LinkBW = XMMIDP_DPCD_LINK_BW_SET_270GBPS;

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
	XMmiDp_SetPhyTrainingPattern(InstancePtr, PHY_NO_TRAIN);
	XMmiDp_SetDpcdTrainingPattern(InstancePtr, PHY_NO_TRAIN);

	xil_printf("Link Trained for %d Lanes", InstancePtr->LinkConfig.NumLanes);
	xil_printf("Link Trained for %d BW", InstancePtr->LinkConfig.LinkBW);

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
	XMmiDp_SetPhyPowerdown(InstancePtr, PHY_POWER_ON);
	XMmiDp_SetAux_250Us_Cnt_Limit(InstancePtr, 0xF5);
	XMmiDp_SetAux_2000Us_Cnt_Limit(InstancePtr, 0x7A8);
	XMmiDp_SetAux_100000Us_Cnt_Limit(InstancePtr, 0x17ED0);
	XMmiDp_SetPmConfig1(InstancePtr, 0x00400008);
	XMmiDp_SetPmConfig2(InstancePtr, 0x00050000);
	XMmiDp_PhySSCDisable(InstancePtr);
	XMmiDp_GeneralInterruptEnable(InstancePtr, 0x0000000B);
	XMmiDp_HpdInterruptEnable(InstancePtr, 0x0000000F);

	if (XMmiDp_IsConnected(InstancePtr)) {
		xil_printf("Power Cycle\n");
		SinkPowerCycle(InstancePtr);
	}

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

	XMmiDp_CfgInitialize(InstancePtr, DP_BASEADDR);
	XMmiDp_Initialize(InstancePtr);
	XMmiDp_InitDpCore(InstancePtr);

	XMmiDp_SetupVideoStream(RunCfgPtr);

	while (1) {
		Status = XMmiDp_HpdPoll(InstancePtr);
		if (Status == XST_SUCCESS) {
			xil_printf("DpPsu LinkTraining Successful\n");
			break;
		} else {
			xil_printf("DpPsu LinkTraining failed\n");
		}

	}

	return Status;
}
