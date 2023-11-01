// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_hscaler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_hscaler_CfgInitialize(XV_hscaler *InstancePtr,
                             XV_hscaler_Config *ConfigPtr,
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

void XV_hscaler_Start(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_hscaler_IsDone(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_hscaler_IsIdle(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_hscaler_IsReady(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_hscaler_EnableAutoRestart(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_hscaler_DisableAutoRestart(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_AP_CTRL, 0);
}

void XV_hscaler_Set_HwReg_Height(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_Height(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_hscaler_Set_HwReg_WidthIn(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_WidthIn(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA);
    return Data;
}

void XV_hscaler_Set_HwReg_WidthOut(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_WidthOut(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA);
    return Data;
}

void XV_hscaler_Set_HwReg_ColorMode(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_ColorMode(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA);
    return Data;
}

void XV_hscaler_Set_HwReg_ColorModeOut(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_ColorModeOut(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA);
    return Data;
}

void XV_hscaler_Set_HwReg_PixelRate(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA, Data);
}

u32 XV_hscaler_Get_HwReg_PixelRate(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA);
    return Data;
}

UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE);
}

UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_HighAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH);
}

u32 XV_hscaler_Get_HwReg_hfltCoeff_TotalBytes(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1);
}

u32 XV_hscaler_Get_HwReg_hfltCoeff_BitWidth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_WIDTH_HWREG_HFLTCOEFF;
}

u32 XV_hscaler_Get_HwReg_hfltCoeff_Depth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_DEPTH_HWREG_HFLTCOEFF;
}

u32 XV_hscaler_Write_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_hscaler_Read_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_hscaler_Write_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_hscaler_Read_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + offset + i);
    }
    return length;
}

UINTPTR XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE);
}

UINTPTR XV_hscaler_Get_HwReg_phasesH_V_HighAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH);
}

u32 XV_hscaler_Get_HwReg_phasesH_V_TotalBytes(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1);
}

u32 XV_hscaler_Get_HwReg_phasesH_V_BitWidth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_WIDTH_HWREG_PHASESH_V;
}

u32 XV_hscaler_Get_HwReg_phasesH_V_Depth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_DEPTH_HWREG_PHASESH_V;
}

u32 XV_hscaler_Write_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XV_hscaler_Read_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + (offset + i)*4);
    }
    return length;
}

u32 XV_hscaler_Write_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XV_hscaler_Read_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + offset + i);
    }
    return length;
}

void XV_hscaler_InterruptGlobalEnable(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_GIE, 1);
}

void XV_hscaler_InterruptGlobalDisable(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_GIE, 0);
}

void XV_hscaler_InterruptEnable(XV_hscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER, Register | Mask);
}

void XV_hscaler_InterruptDisable(XV_hscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_hscaler_InterruptClear(XV_hscaler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_ISR, Mask);
}

u32 XV_hscaler_InterruptGetEnabled(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
}

u32 XV_hscaler_InterruptGetStatus(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_ISR);
}
