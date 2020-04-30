// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_letterbox.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_letterbox_CfgInitialize(XV_letterbox *InstancePtr,
                               XV_letterbox_Config *ConfigPtr,
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

void XV_letterbox_Start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_letterbox_IsDone(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_letterbox_IsIdle(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_letterbox_IsReady(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_letterbox_EnableAutoRestart(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_letterbox_DisableAutoRestart(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL, 0);
}

void XV_letterbox_Set_HwReg_width(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_width(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_height(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_height(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_video_format(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_video_format(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_col_start(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_START_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_col_start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_START_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_col_end(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_END_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_col_end(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_END_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_row_start(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_START_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_row_start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_START_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_row_end(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_END_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_row_end(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_END_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_Y_R_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_Y_R_VALUE_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_Y_R_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_Y_R_VALUE_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_Cb_G_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CB_G_VALUE_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_Cb_G_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CB_G_VALUE_DATA);
    return Data;
}

void XV_letterbox_Set_HwReg_Cr_B_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CR_B_VALUE_DATA, Data);
}

u32 XV_letterbox_Get_HwReg_Cr_B_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CR_B_VALUE_DATA);
    return Data;
}

void XV_letterbox_InterruptGlobalEnable(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_GIE, 1);
}

void XV_letterbox_InterruptGlobalDisable(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_GIE, 0);
}

void XV_letterbox_InterruptEnable(XV_letterbox *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER);
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER, Register | Mask);
}

void XV_letterbox_InterruptDisable(XV_letterbox *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER);
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_letterbox_InterruptClear(XV_letterbox *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_ISR, Mask);
}

u32 XV_letterbox_InterruptGetEnabled(XV_letterbox *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER);
}

u32 XV_letterbox_InterruptGetStatus(XV_letterbox *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_ISR);
}
