// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_mix.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_mix_CfgInitialize(XV_mix *InstancePtr,
		                 XV_mix_Config *ConfigPtr,
						 UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

    /* Setup the instance */
    InstancePtr->Config = *ConfigPtr;
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set the flag to indicate the driver is ready */
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XV_mix_Start(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_mix_IsDone(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_mix_IsIdle(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_mix_IsReady(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_mix_EnableAutoRestart(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_mix_DisableAutoRestart(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, 0);
}

void XV_mix_SetFlushbit(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress,
		XV_MIX_CTRL_ADDR_AP_CTRL);
    Data |= XV_MIX_CTRL_BITS_FLUSH_BIT;
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress,
		XV_MIX_CTRL_ADDR_AP_CTRL, Data);
}

u32 XV_mix_Get_FlushDone(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL)
			       & XV_MIX_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}

void XV_mix_Set_HwReg_width(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_mix_Get_HwReg_width(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_mix_Set_HwReg_height(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_mix_Get_HwReg_height(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_mix_Set_HwReg_video_format(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

u32 XV_mix_Get_HwReg_video_format(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_mix_Set_HwReg_background_Y_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_Y_R_DATA, Data);
}

u32 XV_mix_Get_HwReg_background_Y_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_Y_R_DATA);
    return Data;
}

void XV_mix_Set_HwReg_background_U_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_U_G_DATA, Data);
}

u32 XV_mix_Get_HwReg_background_U_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_U_G_DATA);
    return Data;
}

void XV_mix_Set_HwReg_background_V_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_V_B_DATA, Data);
}

u32 XV_mix_Get_HwReg_background_V_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_V_B_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerEnable(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERENABLE_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerEnable(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERENABLE_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer1_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer1_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer1_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer1_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer1_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer1_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_2_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_2_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer2_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer2_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer2_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer2_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer2_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer2_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_3_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_3_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer3_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer3_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer3_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer3_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer3_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer3_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_4_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_4_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer4_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer4_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer4_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer4_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer4_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer4_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_5_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_5_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer5_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer5_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer5_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer5_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer5_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer5_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_6_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_6_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer6_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer6_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer6_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer6_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer6_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer6_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_7_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_7_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer7_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer7_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer7_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer7_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer7_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer7_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layerAlpha_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerAlpha_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_8_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_8_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer8_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer8_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer8_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer8_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer8_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer8_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA + 4) << 32;
    return Data;
}

////////
u32 XV_mix_Get_HwReg_layerAlpha_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_9_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_9_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer9_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer9_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer9_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer9_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer9_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer9_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA + 4) << 32;
    return Data;
}

////
u32 XV_mix_Get_HwReg_layerAlpha_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_10_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_10_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer10_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer10_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer10_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer10_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer10_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer10_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_11_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_11_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer11_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer11_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer11_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer11_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer11_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer11_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_12_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_12_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer12_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer12_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer12_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer12_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer12_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer12_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_13_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_13_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer13_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer13_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer13_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer13_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer13_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer13_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_14_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_14_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer14_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer14_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer14_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer14_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer14_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer14_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_15_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_15_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer15_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer15_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer15_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer15_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer15_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer15_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
u32 XV_mix_Get_HwReg_layerAlpha_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartX_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartX_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStartY_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStartY_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerWidth_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerWidth_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerStride_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerStride_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerHeight_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerHeight_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerScaleFactor_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerScaleFactor_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layerVideoFormat_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_16_DATA, Data);
}

u32 XV_mix_Get_HwReg_layerVideoFormat_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_16_DATA);
    return Data;
}

void XV_mix_Set_HwReg_layer16_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer16_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer16_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer16_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA + 4) << 32;
    return Data;
}

void XV_mix_Set_HwReg_layer16_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_mix_Get_HwReg_layer16_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA + 4) << 32;
    return Data;
}

///
void XV_mix_Set_HwReg_reserve(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_RESERVE_DATA, Data);
}

u32 XV_mix_Get_HwReg_reserve(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_RESERVE_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoStartX(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTX_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoStartX(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTX_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoStartY(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTY_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoStartY(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTY_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoWidth(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOWIDTH_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoWidth(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOWIDTH_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoHeight(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOHEIGHT_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoHeight(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOHEIGHT_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoScaleFactor(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSCALEFACTOR_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoScaleFactor(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSCALEFACTOR_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoAlpha(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOALPHA_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoAlpha(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOALPHA_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_R_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_R_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_G_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_G_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_B_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_B_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_R_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_R_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_G_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_G_DATA);
    return Data;
}

void XV_mix_Set_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_B_DATA, Data);
}

u32 XV_mix_Get_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_B_DATA);
    return Data;
}

u32 XV_mix_Get_HwReg_logoR_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE);
}

u32 XV_mix_Get_HwReg_logoR_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH);
}

u32 XV_mix_Get_HwReg_logoR_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1);
}

u32 XV_mix_Get_HwReg_logoR_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOR_V;
}

u32 XV_mix_Get_HwReg_logoR_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOR_V;
}

u32 XV_mix_Write_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_mix_Write_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + offset + i);
    }
    return length;
}

u32 XV_mix_Get_HwReg_logoG_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE);
}

u32 XV_mix_Get_HwReg_logoG_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH);
}

u32 XV_mix_Get_HwReg_logoG_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1);
}

u32 XV_mix_Get_HwReg_logoG_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOG_V;
}

u32 XV_mix_Get_HwReg_logoG_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOG_V;
}

u32 XV_mix_Write_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_mix_Write_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + offset + i);
    }
    return length;
}

u32 XV_mix_Get_HwReg_logoB_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE);
}

u32 XV_mix_Get_HwReg_logoB_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH);
}

u32 XV_mix_Get_HwReg_logoB_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1);
}

u32 XV_mix_Get_HwReg_logoB_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOB_V;
}

u32 XV_mix_Get_HwReg_logoB_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOB_V;
}

u32 XV_mix_Write_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_mix_Write_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + offset + i);
    }
    return length;
}

u32 XV_mix_Get_HwReg_logoA_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE);
}

u32 XV_mix_Get_HwReg_logoA_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH);
}

u32 XV_mix_Get_HwReg_logoA_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1);
}

u32 XV_mix_Get_HwReg_logoA_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOA_V;
}

u32 XV_mix_Get_HwReg_logoA_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOA_V;
}

u32 XV_mix_Write_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_mix_Write_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_mix_Read_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + offset + i);
    }
    return length;
}

void XV_mix_InterruptGlobalEnable(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_GIE, 1);
}

void XV_mix_InterruptGlobalDisable(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_GIE, 0);
}

void XV_mix_InterruptEnable(XV_mix *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER, Register | Mask);
}

void XV_mix_InterruptDisable(XV_mix *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_mix_InterruptClear(XV_mix *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_ISR, Mask);
}

u32 XV_mix_InterruptGetEnabled(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
}

u32 XV_mix_InterruptGetStatus(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_ISR);
}
