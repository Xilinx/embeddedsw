// ==============================================================
// Copyright (c) 1986 - 2020 Xilinx, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_scenechange.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_scenechange_CfgInitialize(XV_scenechange *InstancePtr, XV_scenechange_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XV_scenechange_Start(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_scenechange_IsDone(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_scenechange_IsIdle(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_scenechange_IsReady(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_scenechange_EnableAutoRestart(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_scenechange_DisableAutoRestart(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, 0);
}

void XV_scenechange_Set_HwReg_width_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad0_vld(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_CTRL);
    return Data & 0x1;
}

void XV_scenechange_Set_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_1_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_1_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_1_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_1_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_1_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_1_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_1_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_1_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_1_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_1_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD1_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER1_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER1_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_2_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_2_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_2_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_2_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_2_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_2_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_2_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_2_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_2_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_2_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD2_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_3_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_3_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_3_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_3_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_3_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_3_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_3_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_3_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_3_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_3_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD3_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_4_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_4_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_4_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_4_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_4_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_4_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_4_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_4_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_4_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_4_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD4_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER4_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER4_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_5_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_5_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_5_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_5_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_5_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_5_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_5_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_5_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_5_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_5_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD5_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER5_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER5_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_6_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_6_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_6_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_6_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_6_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_6_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_6_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_6_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_6_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_6_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD6_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER6_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER6_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_width_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_7_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_width_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_7_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_height_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_7_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_height_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_7_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stride_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_7_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stride_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_7_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_video_format_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_7_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_video_format_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_7_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_subsample_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_7_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_subsample_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_7_DATA);
    return Data;
}

u32 XV_scenechange_Get_HwReg_sad7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD7_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER7_V_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER7_V_DATA);
    return Data;
}

void XV_scenechange_Set_HwReg_stream_enable(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STREAM_ENABLE_DATA, Data);
}

u32 XV_scenechange_Get_HwReg_stream_enable(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STREAM_ENABLE_DATA);
    return Data;
}

void XV_scenechange_InterruptGlobalEnable(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_GIE, 1);
}

void XV_scenechange_InterruptGlobalDisable(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_GIE, 0);
}

void XV_scenechange_InterruptEnable(XV_scenechange *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER, Register | Mask);
}

void XV_scenechange_InterruptDisable(XV_scenechange *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_scenechange_InterruptClear(XV_scenechange *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_ISR, Mask);
}

u32 XV_scenechange_InterruptGetEnabled(XV_scenechange *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
}

u32 XV_scenechange_InterruptGetStatus(XV_scenechange *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_ISR);
}
