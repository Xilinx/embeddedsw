// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_deinterlacer.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_deinterlacer_CfgInitialize(XV_deinterlacer *InstancePtr,
                                  XV_deinterlacer_Config *ConfigPtr,
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

void XV_deinterlacer_Start(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_deinterlacer_IsDone(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_deinterlacer_IsIdle(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_deinterlacer_IsReady(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_deinterlacer_EnableAutoRestart(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_deinterlacer_DisableAutoRestart(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_deinterlacer_Set_width(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WIDTH_DATA, Data);
}

u32 XV_deinterlacer_Get_width(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

void XV_deinterlacer_Set_height(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_HEIGHT_DATA, Data);
}

u32 XV_deinterlacer_Get_height(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}
void XV_deinterlacer_Set_read_fb(XV_deinterlacer *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA, (u32)(Data));
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_deinterlacer_Get_read_fb(XV_deinterlacer *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA);
    Data |= (u64)(XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA + 4)) << 32;
    return Data;
}

void XV_deinterlacer_Set_write_fb(XV_deinterlacer *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA, (u32)Data);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_deinterlacer_Get_write_fb(XV_deinterlacer *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA);
    Data |= (u64)(XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA + 4)) << 32;
    return Data;
}

void XV_deinterlacer_Set_colorFormat(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

u32 XV_deinterlacer_Get_colorFormat(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_COLORFORMAT_DATA);
    return Data;
}

void XV_deinterlacer_Set_algo(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ALGO_DATA, Data);
}

u32 XV_deinterlacer_Get_algo(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ALGO_DATA);
    return Data;
}

void XV_deinterlacer_Set_invert_field_id(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_INVERT_FIELD_ID_DATA, Data);
}

u32 XV_deinterlacer_Get_invert_field_id(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_INVERT_FIELD_ID_DATA);
    return Data;
}
void XV_deinterlacer_InterruptGlobalEnable(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_GIE, 1);
}

void XV_deinterlacer_InterruptGlobalDisable(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_GIE, 0);
}

void XV_deinterlacer_InterruptEnable(XV_deinterlacer *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER, Register | Mask);
}

void XV_deinterlacer_InterruptDisable(XV_deinterlacer *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_deinterlacer_InterruptClear(XV_deinterlacer *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ISR, Mask);
}

u32 XV_deinterlacer_InterruptGetEnabled(XV_deinterlacer *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
}

u32 XV_deinterlacer_InterruptGetStatus(XV_deinterlacer *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ISR);
}
