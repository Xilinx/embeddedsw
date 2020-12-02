// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_vscaler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_vscaler_CfgInitialize(XV_vscaler *InstancePtr,
                             XV_vscaler_Config *ConfigPtr,
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

void XV_vscaler_Start(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_vscaler_IsDone(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_vscaler_IsIdle(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_vscaler_IsReady(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_vscaler_EnableAutoRestart(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_vscaler_DisableAutoRestart(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_vscaler_Set_HwReg_HeightIn(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA, Data);
}

u32 XV_vscaler_Get_HwReg_HeightIn(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA);
    return Data;
}

void XV_vscaler_Set_HwReg_Width(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_vscaler_Get_HwReg_Width(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_vscaler_Set_HwReg_HeightOut(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA, Data);
}

u32 XV_vscaler_Get_HwReg_HeightOut(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA);
    return Data;
}

void XV_vscaler_Set_HwReg_LineRate(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA, Data);
}

u32 XV_vscaler_Get_HwReg_LineRate(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA);
    return Data;
}

void XV_vscaler_Set_HwReg_ColorMode(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA, Data);
}

u32 XV_vscaler_Get_HwReg_ColorMode(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA);
    return Data;
}

UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE);
}

UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_HighAddress(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH);
}

u32 XV_vscaler_Get_HwReg_vfltCoeff_TotalBytes(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1);
}

u32 XV_vscaler_Get_HwReg_vfltCoeff_BitWidth(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_VSCALER_CTRL_WIDTH_HWREG_VFLTCOEFF;
}

u32 XV_vscaler_Get_HwReg_vfltCoeff_Depth(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_VSCALER_CTRL_DEPTH_HWREG_VFLTCOEFF;
}

u32 XV_vscaler_Write_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_vscaler_Read_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_vscaler_Write_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_vscaler_Read_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + offset + i);
    }
    return length;
}

void XV_vscaler_InterruptGlobalEnable(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_GIE, 1);
}

void XV_vscaler_InterruptGlobalDisable(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_GIE, 0);
}

void XV_vscaler_InterruptEnable(XV_vscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER);
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER, Register | Mask);
}

void XV_vscaler_InterruptDisable(XV_vscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER);
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_vscaler_InterruptClear(XV_vscaler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_ISR, Mask);
}

u32 XV_vscaler_InterruptGetEnabled(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER);
}

u32 XV_vscaler_InterruptGetStatus(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_ISR);
}
