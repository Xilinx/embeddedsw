// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_csc.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_csc_CfgInitialize(XV_csc *InstancePtr,
                         XV_csc_Config *ConfigPtr,
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

void XV_csc_Start(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_csc_IsDone(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_csc_IsIdle(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_csc_IsReady(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_csc_EnableAutoRestart(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_csc_DisableAutoRestart(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, 0);
}

void XV_csc_Set_HwReg_InVideoFormat(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_INVIDEOFORMAT_DATA, Data);
}

u32 XV_csc_Get_HwReg_InVideoFormat(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_INVIDEOFORMAT_DATA);
    return Data;
}

void XV_csc_Set_HwReg_OutVideoFormat(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_OUTVIDEOFORMAT_DATA, Data);
}

u32 XV_csc_Get_HwReg_OutVideoFormat(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_OUTVIDEOFORMAT_DATA);
    return Data;
}

void XV_csc_Set_HwReg_width(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_csc_Get_HwReg_width(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_csc_Set_HwReg_height(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_csc_Get_HwReg_height(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ColStart(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLSTART_DATA, Data);
}

u32 XV_csc_Get_HwReg_ColStart(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLSTART_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ColEnd(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLEND_DATA, Data);
}

u32 XV_csc_Get_HwReg_ColEnd(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLEND_DATA);
    return Data;
}

void XV_csc_Set_HwReg_RowStart(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWSTART_DATA, Data);
}

u32 XV_csc_Get_HwReg_RowStart(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWSTART_DATA);
    return Data;
}

void XV_csc_Set_HwReg_RowEnd(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWEND_DATA, Data);
}

u32 XV_csc_Get_HwReg_RowEnd(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWEND_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K11(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_DATA, Data);
}

u32 XV_csc_Get_HwReg_K11(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K12(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_DATA, Data);
}

u32 XV_csc_Get_HwReg_K12(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K13(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_DATA, Data);
}

u32 XV_csc_Get_HwReg_K13(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K21(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_DATA, Data);
}

u32 XV_csc_Get_HwReg_K21(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K22(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_DATA, Data);
}

u32 XV_csc_Get_HwReg_K22(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K23(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_DATA, Data);
}

u32 XV_csc_Get_HwReg_K23(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K31(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_DATA, Data);
}

u32 XV_csc_Get_HwReg_K31(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K32(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_DATA, Data);
}

u32 XV_csc_Get_HwReg_K32(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K33(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_DATA, Data);
}

u32 XV_csc_Get_HwReg_K33(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ROffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ROffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_GOffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_GOffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_BOffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_BOffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ClampMin_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ClampMin_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ClipMax_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ClipMax_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K11_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K11_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K12_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K12_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K13_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K13_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K21_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K21_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K22_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K22_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K23_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K23_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K31_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K31_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K32_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K32_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_K33_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_2_DATA, Data);
}

u32 XV_csc_Get_HwReg_K33_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_2_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ROffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_2_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ROffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_2_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_GOffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_2_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_GOffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_2_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_BOffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_2_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_BOffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_2_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ClampMin_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_2_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ClampMin_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_2_V_DATA);
    return Data;
}

void XV_csc_Set_HwReg_ClipMax_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_2_V_DATA, Data);
}

u32 XV_csc_Get_HwReg_ClipMax_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_2_V_DATA);
    return Data;
}

void XV_csc_InterruptGlobalEnable(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_GIE, 1);
}

void XV_csc_InterruptGlobalDisable(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_GIE, 0);
}

void XV_csc_InterruptEnable(XV_csc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER, Register | Mask);
}

void XV_csc_InterruptDisable(XV_csc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_csc_InterruptClear(XV_csc *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_ISR, Mask);
}

u32 XV_csc_InterruptGetEnabled(XV_csc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
}

u32 XV_csc_InterruptGetStatus(XV_csc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_ISR);
}
