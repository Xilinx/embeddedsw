// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_warp_init.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_warp_init_CfgInitialize(XV_warp_init *InstancePtr, XV_warp_init_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->config = ConfigPtr;
    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    InstancePtr->RemapVectorDesc_BaseAddr = 0;
    InstancePtr->NumDescriptors = 0;

    return XST_SUCCESS;
}
#endif

void XV_warp_init_Start(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_warp_init_IsDone(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_warp_init_IsIdle(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_warp_init_IsReady(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_warp_init_EnableAutoRestart(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_warp_init_DisableAutoRestart(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, 0);
}

u32 XV_warp_init_Get_ip_status(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IP_STATUS_REG_DATA);
    return Data;
}

void XV_warp_init_Set_maxi_read_write(XV_warp_init *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA, (u32)(Data));
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_init_Get_maxi_read_write(XV_warp_init *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA);
    Data += (u64)XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA + 4) << 32;
    return Data;
}

void XV_warp_init_Set_desc_addr(XV_warp_init *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA, (u32)(Data));
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_init_Get_desc_addr(XV_warp_init *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA);
    Data += (u64)XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA + 4) << 32;
    return Data;
}

void XV_warp_init_InterruptGlobalEnable(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_GIE, 1);
}

void XV_warp_init_InterruptGlobalDisable(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_GIE, 0);
}

void XV_warp_init_InterruptEnable(XV_warp_init *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER, Register | Mask);
}

void XV_warp_init_InterruptDisable(XV_warp_init *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_warp_init_InterruptClear(XV_warp_init *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_ISR, Mask);
}

u32 XV_warp_init_InterruptGetEnabled(XV_warp_init *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
}

u32 XV_warp_init_InterruptGetStatus(XV_warp_init *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_ISR);
}

void XV_warp_init_SetFlushbit(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    Data |= AP_CTRL_FLUSH_BIT;
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL, Data);
}

u32 XV_warp_init_Get_FlushDone(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL) & AP_CTRL_FLUSH_STATUSBIT;
    return Data;
}
