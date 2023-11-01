// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_hcresampler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_hcresampler_CfgInitialize(XV_hcresampler *InstancePtr,
                                 XV_hcresampler_Config *ConfigPtr,
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

void XV_hcresampler_Start(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_hcresampler_IsDone(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_hcresampler_IsIdle(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_hcresampler_IsReady(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_hcresampler_EnableAutoRestart(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_hcresampler_DisableAutoRestart(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_hcresampler_Set_HwReg_width(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_hcresampler_Get_HwReg_width(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_hcresampler_Set_HwReg_height(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_hcresampler_Get_HwReg_height(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_hcresampler_Set_HwReg_input_video_format(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA, Data);
}

u32 XV_hcresampler_Get_HwReg_input_video_format(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_hcresampler_Set_HwReg_output_video_format(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA, Data);
}

u32 XV_hcresampler_Get_HwReg_output_video_format(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_hcresampler_Set_HwReg_coefs_0_0(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA, Data);
}

u32 XV_hcresampler_Get_HwReg_coefs_0_0(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA);
    return Data;
}

void XV_hcresampler_InterruptGlobalEnable(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_GIE, 1);
}

void XV_hcresampler_InterruptGlobalDisable(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_GIE, 0);
}

void XV_hcresampler_InterruptEnable(XV_hcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER, Register | Mask);
}

void XV_hcresampler_InterruptDisable(XV_hcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_hcresampler_InterruptClear(XV_hcresampler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_ISR, Mask);
}

u32 XV_hcresampler_InterruptGetEnabled(XV_hcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
}

u32 XV_hcresampler_InterruptGetStatus(XV_hcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_ISR);
}
