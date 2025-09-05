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
	u32 VideoBw;
	u32 LinkBw;
	u32 TransferUnitSize;

	XMmiDp_VideoConfig *VideoConfig;
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
	MsaConfig->HBlankInterval = 0x00000690;

	BitsPerPixel = MsaConfig->BitsPerColor * 3;
	VideoConfig->AvgBytesPerTuFrac = 0x3D;
	VideoConfig->InitThreshold = 0x20;
	TransferUnitSize = 64;

	VideoBw = ((MsaConfig->PixelClockHz / 1000) * BitsPerPixel) / 8;
	LinkBw = (LinkConfig->LaneCount * LinkConfig->LinkRate * 27);
	VideoConfig->AvgBytesPerTu = (VideoBw * TransferUnitSize) / LinkBw;
	VideoConfig->AvgBytesPerTu = 0x8;
}

void XMmiDp_DpcdWriteByte(XMmiDp *InstancePtr, u32 DpcdAddr, u8 AuxData)
{
	XMmiDp_AuxWrite(InstancePtr, DpcdAddr, 1, &AuxData);
}

void XMmiDp_ProgramVcpTableSlot1(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	xil_printf("Config DPCD VCP table slot 1\n");
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 0x1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_SET, 1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT, 1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT, 0xC);

	Status = XMmiDp_WaitPayloadTableUpdateStatus(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Payload ID Table Status not updated\n");
	}

}

void XMmiDp_ProgramVcpTableSlot2(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	xil_printf("Config DPCD VCP table slot 2\n");
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 0x1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_SET, 2);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT, 0xD);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT, 0xC);

	Status = XMmiDp_WaitPayloadTableUpdateStatus(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Payload ID Table Status not updated\n");
	}

}

void XMmiDp_ProgramVcpTableSlot3(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	xil_printf("Config DPCD VCP table slot 3\n");
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 0x1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_SET, 3);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT, 0x19);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT, 0xC);

	Status = XMmiDp_WaitPayloadTableUpdateStatus(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Payload ID Table Status not updated\n");
	}

}

void XMmiDp_ProgramVcpTableSlot4(XMmiDp *InstancePtr)
{
	u32 Status = 0;

	xil_printf("Config DPCD VCP table slot 4\n");
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 0x1);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_SET, 4);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_START_TIME_SLOT, 0x25);
	XMmiDp_DpcdWriteByte(InstancePtr, XMMIDP_DPCD_PAYLOAD_ALLOCATE_TIME_SLOT_COUNT, 0xC);

	Status = XMmiDp_WaitPayloadTableUpdateStatus(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Payload ID Table Status not updated\n");
	}

}

void XMmiDp_SetupVideoStream(RunConfig *RunCfgPtr)
{
	u32 StreamId;

	XMmiDp *InstancePtr = RunCfgPtr->DpPsuPtr;

	XMmiDp_ClearVideoController(InstancePtr);

	/* Configure Stream properties */
	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		xil_printf("Configure Video Stream %x\n", StreamId);

		XMmiDp_SetMsaBpc(InstancePtr, StreamId, 8);
		XMmiDp_SetPixModeSel(InstancePtr, StreamId, 0x0);
		XMmiDp_SetVideoMapping(InstancePtr, StreamId, XMMIDP_RGB_8BPC);
		XMmiDp_SetVidStreamEnable(InstancePtr, StreamId, 0x1);
		XMmiDp_SetVidControllerUseStdVidMode(InstancePtr,
						     XVIDC_VM_640x480_60_P, StreamId);

	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoConfig5(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetHBlankInterval(InstancePtr, StreamId);
	}

	/* Virtual Channel Payload table slot */
	XMmiDp_SetMstVcpTable0(InstancePtr, 0x11111110);
	XMmiDp_SetMstVcpTable1(InstancePtr, 0x22211111);
	XMmiDp_SetMstVcpTable2(InstancePtr, 0x22222222);
	XMmiDp_SetMstVcpTable3(InstancePtr, 0x33333332);
	XMmiDp_SetMstVcpTable4(InstancePtr, 0x44433333);
	XMmiDp_SetMstVcpTable5(InstancePtr, 0x44444444);
	XMmiDp_SetMstVcpTable6(InstancePtr, 0x00000004);
	XMmiDp_SetMstVcpTable7(InstancePtr, 0x00000000);

	/* Act Init Seq */
	XMmiDp_ProgramVcpTableSlot1(InstancePtr);
	XMmiDp_InitiateActSeq(InstancePtr);
	XMmiDp_ProgramVcpTableSlot2(InstancePtr);
	XMmiDp_InitiateActSeq(InstancePtr);
	XMmiDp_ProgramVcpTableSlot3(InstancePtr);
	XMmiDp_InitiateActSeq(InstancePtr);
	XMmiDp_ProgramVcpTableSlot4(InstancePtr);
	XMmiDp_InitiateActSeq(InstancePtr);

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoMsa1(InstancePtr, StreamId);
		XMmiDp_SetVInputPolarityCtrl(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoMsa2(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoConfig1(InstancePtr, StreamId);
		XMmiDp_SetVideoMsa3(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoConfig2(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoConfig3(InstancePtr, StreamId);
	}

	for (StreamId = XMMIDP_STREAM_ID1; StreamId <= XMMIDP_STREAM_ID4; StreamId++) {
		XMmiDp_SetVideoConfig4(InstancePtr, StreamId);
	}

	XMmiDp_SetVSampleCtrl(InstancePtr, XMMIDP_STREAM_ID1);
	XMmiDp_SetVSampleCtrl(InstancePtr, XMMIDP_STREAM_ID2);
	XMmiDp_SetVSampleCtrl(InstancePtr, XMMIDP_STREAM_ID3);
	XMmiDp_SetVSampleCtrl(InstancePtr, XMMIDP_STREAM_ID4);

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
	u8 AuxData = 0x0;

	Status = XMmiDp_GetRxCapabilities(InstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to get Rx Cap\n");
		return XST_FAILURE;
	}

	InstancePtr->RxConfig.MaxLaneCount = XMMIDP_PHY_LANES_1;
	InstancePtr->RxConfig.MaxNumLanes = 0x1;
	InstancePtr->RxConfig.MaxLinkRate = XMMIDP_PHY_RATE_HBR_270GBPS;
	InstancePtr->RxConfig.MaxLinkBW = XMMIDP_DPCD_LINK_BW_SET_270GBPS;

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

	InstancePtr->LinkConfig.LaneCount = InstancePtr->RxConfig.MaxLaneCount;
	InstancePtr->LinkConfig.NumLanes = InstancePtr->RxConfig.MaxNumLanes;
	InstancePtr->LinkConfig.LinkRate = InstancePtr->RxConfig.MaxLinkRate;
	InstancePtr->LinkConfig.LinkBW = InstancePtr->RxConfig.MaxLinkBW;

	memset(InstancePtr->LinkConfig.VsLevel, 0, 4);
	memset(InstancePtr->LinkConfig.PeLevel, 0, 4);

	/* By default, Fast Link Train is enabled */
	XMmiDp_FastLinkTrainDisable(InstancePtr);
	XMmiDp_MstModeEnable(InstancePtr);

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
	XMmiDp_AuxWrite(InstancePtr,
			XMMIDP_DPCD_TRAINING_PATTERN_SET, 1, &AuxData);

	xil_printf("Link Trained for %d Lanes\n", InstancePtr->LinkConfig.NumLanes);
	xil_printf("Link Trained for %d BW\n", InstancePtr->LinkConfig.LinkBW);

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
	XMmiDp_SetAux_100000Us_Cnt_Limit(InstancePtr, 0x7ED0);
	XMmiDp_SetPmConfig1(InstancePtr, 0x00400008);
	XMmiDp_SetPmConfig2(InstancePtr, 0x00050000);
	XMmiDp_PhySSCEnable(InstancePtr);
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
	XMmiDp_StartFullLinkTraining(InstancePtr);
	XMmiDp_SetupVideoStream(RunCfgPtr);

	return Status;
}
