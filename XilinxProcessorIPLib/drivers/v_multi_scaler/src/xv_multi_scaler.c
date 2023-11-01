/*************************************************************************
 * Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_multi_scaler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_multi_scaler_CfgInitialize(XV_multi_scaler *InstancePtr,
		XV_multi_scaler_Config *ConfigPtr)
{
	u16 i;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->MaxCols = ConfigPtr->MaxCols;
	InstancePtr->MaxRows = ConfigPtr->MaxRows;
	InstancePtr->SamplesPerClock = ConfigPtr->SamplesPerClock;
	InstancePtr->MaxDataWidth = ConfigPtr->MaxDataWidth;
	InstancePtr->PhaseShift = ConfigPtr->PhaseShift;
	InstancePtr->ScaleMode = ConfigPtr->ScaleMode;
	InstancePtr->NumTaps = ConfigPtr->NumTaps;
	InstancePtr->MaxOuts = ConfigPtr->MaxOuts;

	InstancePtr->Config.Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
	InstancePtr->Config.MaxCols = ConfigPtr->MaxCols;
	InstancePtr->Config.MaxRows = ConfigPtr->MaxRows;
	InstancePtr->Config.SamplesPerClock = ConfigPtr->SamplesPerClock;
	InstancePtr->Config.MaxDataWidth = ConfigPtr->MaxDataWidth;
	InstancePtr->Config.PhaseShift = ConfigPtr->PhaseShift;
	InstancePtr->Config.ScaleMode = ConfigPtr->ScaleMode;
	InstancePtr->Config.NumTaps = ConfigPtr->NumTaps;
	InstancePtr->Config.MaxOuts = ConfigPtr->MaxOuts;
#ifdef SDT
	InstancePtr->Config.IntrId = ConfigPtr->IntrId;
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif
	return XST_SUCCESS;
}
#endif

void XV_multi_scaler_Start(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL) &
		XV_MULTI_SCALER_AP_AUTO_RESTART_BIT_MASK;
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL, Data |
		XV_MULTI_SCALER_AP_START_BIT_MASK);
}

u32 XV_multi_scaler_IsDone(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	return Data & XV_MULTI_SCALER_AP_DONE_BIT_MASK;
}

u32 XV_multi_scaler_IsIdle(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	return Data & XV_MULTI_SCALER_AP_IDLE_BIT_MASK;
}

u32 XV_multi_scaler_IsReady(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	/* check ap_start to see if the pcore is ready for next input */
	return !(Data & XV_MULTI_SCALER_AP_START_BIT_MASK);
}

void XV_multi_scaler_EnableAutoRestart(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL,
		XV_MULTI_SCALER_AP_AUTO_RESTART_BIT_MASK);
}

void XV_multi_scaler_DisableAutoRestart(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_multi_scaler_Set_HwReg_num_outs(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_NUM_OUTS_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_num_outs(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_NUM_OUTS_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_0_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_0_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_1_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_1_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_2_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_2_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_3_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_3_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_4_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_4_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_5_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_5_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_6_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_6_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_LineRate_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_LineRate_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_InStride_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_InStride_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_OutStride_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_7_DATA, Data);
}

u32 XV_multi_scaler_Get_HwReg_OutStride_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_7_DATA);
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA + 4) << 32;
	return Data;
}

void XV_multi_scaler_Set_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA + 4,
		(u32) (Data >> 32));
}

u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA + 4) << 32;
	return Data;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_0;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_0;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE +
		(offset + i)*4)	= *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_0;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_0;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_1;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_1;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		offset + i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_1;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_1;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_2;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_2;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + offset
		+ i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_2;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_2;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		offset + i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		offset + i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_3;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_3;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + offset
		+ i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + offset
		+ i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_3;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_3;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_4;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_4;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_4;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_4;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_5;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_5;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_5;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_5;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_6;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_6;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_6;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_6;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_7;
}

u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_7;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + offset +
		i);
	}
	return length;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1);
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_7;
}

u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_7;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE +
		(offset + i)*4);
	}
	return length;
}

u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + offset +
		i);
	}
	return length;
}

void XV_multi_scaler_InterruptGlobalEnable(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_GIE, 1);
}

void XV_multi_scaler_InterruptGlobalDisable(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_GIE, 0);
}

void XV_multi_scaler_InterruptEnable(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER, Register | Mask);
}

void XV_multi_scaler_InterruptDisable(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_multi_scaler_InterruptClear(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_ISR, Mask);
}

u32 XV_multi_scaler_InterruptGetEnabled(XV_multi_scaler *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
}

u32 XV_multi_scaler_InterruptGetStatus(XV_multi_scaler *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_ISR);
}

