// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_demosaic.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_demosaic_CfgInitialize(XV_demosaic *InstancePtr,
                               XV_demosaic_Config *ConfigPtr,
                               UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    InstancePtr->Config = *ConfigPtr;
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set the flag to indicate the driver is ready */
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XV_demosaic_Start(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_demosaic_IsDone(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_demosaic_IsIdle(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_demosaic_IsReady(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_demosaic_EnableAutoRestart(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_demosaic_DisableAutoRestart(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, 0);
}

void XV_demosaic_Set_HwReg_width(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_demosaic_Get_HwReg_width(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_demosaic_Set_HwReg_height(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_demosaic_Get_HwReg_height(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_demosaic_Set_HwReg_bayer_phase(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_BAYER_PHASE_DATA, Data);
}

u32 XV_demosaic_Get_HwReg_bayer_phase(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_BAYER_PHASE_DATA);
    return Data;
}

void XV_demosaic_InterruptGlobalEnable(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_GIE, 1);
}

void XV_demosaic_InterruptGlobalDisable(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_GIE, 0);
}

void XV_demosaic_InterruptEnable(XV_demosaic *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER, Register | Mask);
}

void XV_demosaic_InterruptDisable(XV_demosaic *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_demosaic_InterruptClear(XV_demosaic *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_ISR, Mask);
}

u32 XV_demosaic_InterruptGetEnabled(XV_demosaic *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
}

u32 XV_demosaic_InterruptGetStatus(XV_demosaic *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_ISR);
}
