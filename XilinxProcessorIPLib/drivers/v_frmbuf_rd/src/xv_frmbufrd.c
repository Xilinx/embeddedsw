// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_frmbufrd.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_frmbufrd_CfgInitialize(XV_frmbufrd *InstancePtr,
                               XV_frmbufrd_Config *ConfigPtr,
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

void XV_frmbufrd_Start(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_frmbufrd_IsDone(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_frmbufrd_IsIdle(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_frmbufrd_IsReady(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_frmbufrd_EnableAutoRestart(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_frmbufrd_DisableAutoRestart(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, 0);
}

void XV_frmbufrd_SetFlushbit(XV_frmbufrd* InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress,
			       XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    Data |= XV_FRMBUFRD_CTRL_BITS_FLUSH_BIT;
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress,
			 XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, Data);
}

u32 XV_frmbufrd_Get_FlushDone(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL)
			       & XV_FRMBUFRD_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}

void XV_frmbufrd_Set_HwReg_width(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_width(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

void XV_frmbufrd_Set_HwReg_height(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_height(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

void XV_frmbufrd_Set_HwReg_stride(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_STRIDE_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_stride(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_STRIDE_DATA);
    return Data;
}

void XV_frmbufrd_Set_HwReg_video_format(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_video_format(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

void XV_frmbufrd_Set_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_frmbufrd_Get_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4) << 32;
    return Data;
}

void XV_frmbufrd_Set_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_frmbufrd_Get_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4) << 32;
    return Data;
}

void XV_frmbufrd_Set_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4, (u32)(Data >> 32));
}

u64 XV_frmbufrd_Get_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4) << 32;
    return Data;
}

void XV_frmbufrd_Set_HwReg_field_id(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(InstancePtr->Config.Interlaced);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_field_id(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(InstancePtr->Config.Interlaced);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA);
    return Data;
}

void XV_frmbufrd_Set_HwReg_fidOutMode(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIDOUTMODE_DATA, Data);
}

u32 XV_frmbufrd_Get_HwReg_fidOutMode(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIDOUTMODE_DATA);
    return Data;
}

u32 XV_frmbufrd_Get_HwReg_fid_error(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FID_ERROR_DATA);
    return Data;
}

u32 XV_frmbufrd_Get_HwReg_field_out(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_OUT_DATA);
    return Data;
}

void XV_frmbufrd_InterruptGlobalEnable(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_GIE, 1);
}

void XV_frmbufrd_InterruptGlobalDisable(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_GIE, 0);
}

void XV_frmbufrd_InterruptEnable(XV_frmbufrd *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER, Register | Mask);
}

void XV_frmbufrd_InterruptDisable(XV_frmbufrd *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER, Register & (~Mask));
}

void XV_frmbufrd_InterruptClear(XV_frmbufrd *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_ISR, Mask);
}

u32 XV_frmbufrd_InterruptGetEnabled(XV_frmbufrd *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
}

u32 XV_frmbufrd_InterruptGetStatus(XV_frmbufrd *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_ISR);
}
