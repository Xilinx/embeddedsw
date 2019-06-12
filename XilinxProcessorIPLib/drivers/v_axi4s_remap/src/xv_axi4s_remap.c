/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/***************************** Include Files *********************************/
#include "xv_axi4s_remap.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_axi4s_remap_CfgInitialize(XV_axi4s_remap *InstancePtr,
		                 XV_axi4s_remap_Config *ConfigPtr,
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

void XV_axi4s_remap_Start(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_axi4s_remap_IsDone(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_axi4s_remap_IsIdle(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_axi4s_remap_IsReady(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_axi4s_remap_EnableAutoRestart(XV_axi4s_remap *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_axi4s_remap_DisableAutoRestart(XV_axi4s_remap *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, 0);
}

void XV_axi4s_remap_Set_height(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_HEIGHT_DATA, Data);
}

u32 XV_axi4s_remap_Get_height(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}

void XV_axi4s_remap_Set_width(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_WIDTH_DATA, Data);
}

u32 XV_axi4s_remap_Get_width(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

void XV_axi4s_remap_Set_ColorFormat(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

u32 XV_axi4s_remap_Get_ColorFormat(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_COLORFORMAT_DATA);
    return Data;
}

void XV_axi4s_remap_Set_inPixClk(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXCLK_DATA, Data);
}

u32 XV_axi4s_remap_Get_inPixClk(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXCLK_DATA);
    return Data;
}

void XV_axi4s_remap_Set_outPixClk(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXCLK_DATA, Data);
}

u32 XV_axi4s_remap_Get_outPixClk(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXCLK_DATA);
    return Data;
}

void XV_axi4s_remap_Set_inHDMI420(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INHDMI420_DATA, Data);
}

u32 XV_axi4s_remap_Get_inHDMI420(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INHDMI420_DATA);
    return Data;
}

void XV_axi4s_remap_Set_outHDMI420(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTHDMI420_DATA, Data);
}

u32 XV_axi4s_remap_Get_outHDMI420(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTHDMI420_DATA);
    return Data;
}

void XV_axi4s_remap_Set_inPixDrop(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXDROP_DATA, Data);
}

u32 XV_axi4s_remap_Get_inPixDrop(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXDROP_DATA);
    return Data;
}

void XV_axi4s_remap_Set_outPixRepeat(XV_axi4s_remap *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXREPEAT_DATA, Data);
}

u32 XV_axi4s_remap_Get_outPixRepeat(XV_axi4s_remap *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXREPEAT_DATA);
    return Data;
}

void XV_axi4s_remap_InterruptGlobalEnable(XV_axi4s_remap *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_GIE, 1);
}

void XV_axi4s_remap_InterruptGlobalDisable(XV_axi4s_remap *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_GIE, 0);
}

void XV_axi4s_remap_InterruptEnable(XV_axi4s_remap *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER, Register | Mask);
}

void XV_axi4s_remap_InterruptDisable(XV_axi4s_remap *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_axi4s_remap_InterruptClear(XV_axi4s_remap *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_ISR, Mask);
}

u32 XV_axi4s_remap_InterruptGetEnabled(XV_axi4s_remap *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
}

u32 XV_axi4s_remap_InterruptGetStatus(XV_axi4s_remap *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_ISR);
}
