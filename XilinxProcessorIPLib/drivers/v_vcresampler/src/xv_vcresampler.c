// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_vcresampler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_vcresampler_CfgInitialize(XV_vcresampler *InstancePtr,
                                 XV_vcresampler_Config *ConfigPtr,
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

void XV_vcresampler_Start(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_vcresampler_IsDone(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_vcresampler_IsIdle(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_vcresampler_IsReady(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_vcresampler_EnableAutoRestart(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_vcresampler_DisableAutoRestart(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_vcresampler_Set_HwReg_width(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_vcresampler_Get_HwReg_width(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_vcresampler_Set_HwReg_height(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_vcresampler_Get_HwReg_height(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_vcresampler_Set_HwReg_input_video_format(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA, Data);
}

u32 XV_vcresampler_Get_HwReg_input_video_format(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_vcresampler_Set_HwReg_output_video_format(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA, Data);
}

u32 XV_vcresampler_Get_HwReg_output_video_format(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_vcresampler_Set_HwReg_coefs_0_0(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA, Data);
}

u32 XV_vcresampler_Get_HwReg_coefs_0_0(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA);
    return Data;
}

void XV_vcresampler_InterruptGlobalEnable(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_GIE, 1);
}

void XV_vcresampler_InterruptGlobalDisable(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_GIE, 0);
}

void XV_vcresampler_InterruptEnable(XV_vcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER);
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER, Register | Mask);
}

void XV_vcresampler_InterruptDisable(XV_vcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER);
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_vcresampler_InterruptClear(XV_vcresampler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_ISR, Mask);
}

u32 XV_vcresampler_InterruptGetEnabled(XV_vcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER);
}

u32 XV_vcresampler_InterruptGetStatus(XV_vcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_ISR);
}
