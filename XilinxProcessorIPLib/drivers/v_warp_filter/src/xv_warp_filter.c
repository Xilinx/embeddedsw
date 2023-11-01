// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
/***************************** Include Files *********************************/
#include "xv_warp_filter.h"

/************************** Function Implementation *************************/
#ifndef __linux__
s32 XV_warp_filter_CfgInitialize(XV_warp_filter *InstancePtr, XV_warp_filter_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->config = ConfigPtr;
    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    InstancePtr->WarpFilterDesc_BaseAddr = 0;
    InstancePtr->NumDescriptors = 0;

    return XST_SUCCESS;
}
#endif

void XV_warp_filter_Start(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL) & 0x80;
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_warp_filter_IsDone(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_warp_filter_IsIdle(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_warp_filter_IsReady(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_warp_filter_EnableAutoRestart(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, 0x80);
}

void XV_warp_filter_DisableAutoRestart(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, 0);
}

void XV_warp_filter_SetFlushbit(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress,
		XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    Data |= AP_CTRL_BITS_FLUSH_BIT;
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress,
		XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, Data);
}

u32 XV_warp_filter_Get_FlushDone(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL)
			       & AP_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}


void XV_warp_filter_Set_desc_addr(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_desc_addr(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_read(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_read(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_reads(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_reads(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_write(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_write(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_read1(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_read1(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_read1s(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_read1s(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_Set_maxi_write1(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA + 4, (u32)(Data >> 32));
}

u64 XV_warp_filter_Get_maxi_write1(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA + 4) << 32;
    return Data;
}

void XV_warp_filter_InterruptGlobalEnable(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_GIE, 1);
}

void XV_warp_filter_InterruptGlobalDisable(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_GIE, 0);
}

void XV_warp_filter_InterruptEnable(XV_warp_filter *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER, Register | Mask);
}

void XV_warp_filter_InterruptDisable(XV_warp_filter *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER, Register & (~Mask));
}

void XV_warp_filter_InterruptClear(XV_warp_filter *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_ISR, Mask);
}

u32 XV_warp_filter_InterruptGetEnabled(XV_warp_filter *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
}

u32 XV_warp_filter_InterruptGetStatus(XV_warp_filter *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_ISR);
}
