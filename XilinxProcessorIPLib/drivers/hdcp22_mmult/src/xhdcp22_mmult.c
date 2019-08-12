/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp22_mmult.c
* @addtogroup hdcp22_mmult_v1_1
* @{
* @details
*
* This file contains the main implementation of the driver associated with
* the Xilinx HDCP 2.2 Montgomery Multiplier core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  MH     12/07/15 Initial release.
* 1.01  MH     08/04/16 Added 64 bit address support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdcp22_mmult.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XHdcp22_mmult_CfgInitialize(XHdcp22_mmult *InstancePtr, XHdcp22_mmult_Config *ConfigPtr, UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

    InstancePtr->Config.BaseAddress = EffectiveAddr;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XHdcp22_mmult_Start(XHdcp22_mmult *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL) & 0x80;
    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XHdcp22_mmult_IsDone(XHdcp22_mmult *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XHdcp22_mmult_IsIdle(XHdcp22_mmult *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XHdcp22_mmult_IsReady(XHdcp22_mmult *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XHdcp22_mmult_EnableAutoRestart(XHdcp22_mmult *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL, 0x80);
}

void XHdcp22_mmult_DisableAutoRestart(XHdcp22_mmult *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_AP_CTRL, 0);
}

u32 XHdcp22_mmult_Get_U_BaseAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_BASE);
}

u32 XHdcp22_mmult_Get_U_HighAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_HIGH);
}

u32 XHdcp22_mmult_Get_U_TotalBytes(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XHDCP22_MMULT_CTRL_ADDR_U_HIGH - XHDCP22_MMULT_CTRL_ADDR_U_BASE + 1);
}

u32 XHdcp22_mmult_Get_U_BitWidth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_WIDTH_U;
}

u32 XHdcp22_mmult_Get_U_Depth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_DEPTH_U;
}

u32 XHdcp22_mmult_Write_U_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_U_HIGH - XHDCP22_MMULT_CTRL_ADDR_U_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_U_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_U_HIGH - XHDCP22_MMULT_CTRL_ADDR_U_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_BASE + (offset + i)*4);
    }
    return length;
}

u32 XHdcp22_mmult_Write_U_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_U_HIGH - XHDCP22_MMULT_CTRL_ADDR_U_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_U_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_U_HIGH - XHDCP22_MMULT_CTRL_ADDR_U_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_U_BASE + offset + i);
    }
    return length;
}

u32 XHdcp22_mmult_Get_A_BaseAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_BASE);
}

u32 XHdcp22_mmult_Get_A_HighAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_HIGH);
}

u32 XHdcp22_mmult_Get_A_TotalBytes(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XHDCP22_MMULT_CTRL_ADDR_A_HIGH - XHDCP22_MMULT_CTRL_ADDR_A_BASE + 1);
}

u32 XHdcp22_mmult_Get_A_BitWidth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_WIDTH_A;
}

u32 XHdcp22_mmult_Get_A_Depth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_DEPTH_A;
}

u32 XHdcp22_mmult_Write_A_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_A_HIGH - XHDCP22_MMULT_CTRL_ADDR_A_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_A_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_A_HIGH - XHDCP22_MMULT_CTRL_ADDR_A_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_BASE + (offset + i)*4);
    }
    return length;
}

u32 XHdcp22_mmult_Write_A_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_A_HIGH - XHDCP22_MMULT_CTRL_ADDR_A_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_A_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_A_HIGH - XHDCP22_MMULT_CTRL_ADDR_A_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_A_BASE + offset + i);
    }
    return length;
}

u32 XHdcp22_mmult_Get_B_BaseAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_BASE);
}

u32 XHdcp22_mmult_Get_B_HighAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_HIGH);
}

u32 XHdcp22_mmult_Get_B_TotalBytes(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XHDCP22_MMULT_CTRL_ADDR_B_HIGH - XHDCP22_MMULT_CTRL_ADDR_B_BASE + 1);
}

u32 XHdcp22_mmult_Get_B_BitWidth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_WIDTH_B;
}

u32 XHdcp22_mmult_Get_B_Depth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_DEPTH_B;
}

u32 XHdcp22_mmult_Write_B_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_B_HIGH - XHDCP22_MMULT_CTRL_ADDR_B_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_B_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_B_HIGH - XHDCP22_MMULT_CTRL_ADDR_B_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_BASE + (offset + i)*4);
    }
    return length;
}

u32 XHdcp22_mmult_Write_B_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_B_HIGH - XHDCP22_MMULT_CTRL_ADDR_B_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_B_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_B_HIGH - XHDCP22_MMULT_CTRL_ADDR_B_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_B_BASE + offset + i);
    }
    return length;
}

u32 XHdcp22_mmult_Get_N_BaseAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_BASE);
}

u32 XHdcp22_mmult_Get_N_HighAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_HIGH);
}

u32 XHdcp22_mmult_Get_N_TotalBytes(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XHDCP22_MMULT_CTRL_ADDR_N_HIGH - XHDCP22_MMULT_CTRL_ADDR_N_BASE + 1);
}

u32 XHdcp22_mmult_Get_N_BitWidth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_WIDTH_N;
}

u32 XHdcp22_mmult_Get_N_Depth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_DEPTH_N;
}

u32 XHdcp22_mmult_Write_N_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_N_HIGH - XHDCP22_MMULT_CTRL_ADDR_N_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_N_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_N_HIGH - XHDCP22_MMULT_CTRL_ADDR_N_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_BASE + (offset + i)*4);
    }
    return length;
}

u32 XHdcp22_mmult_Write_N_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_N_HIGH - XHDCP22_MMULT_CTRL_ADDR_N_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_N_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_N_HIGH - XHDCP22_MMULT_CTRL_ADDR_N_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_N_BASE + offset + i);
    }
    return length;
}

u32 XHdcp22_mmult_Get_NPrime_BaseAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE);
}

u32 XHdcp22_mmult_Get_NPrime_HighAddress(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH);
}

u32 XHdcp22_mmult_Get_NPrime_TotalBytes(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH - XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + 1);
}

u32 XHdcp22_mmult_Get_NPrime_BitWidth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_WIDTH_NPRIME;
}

u32 XHdcp22_mmult_Get_NPrime_Depth(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHDCP22_MMULT_CTRL_DEPTH_NPRIME;
}

u32 XHdcp22_mmult_Write_NPrime_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH - XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_NPrime_Words(XHdcp22_mmult *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH - XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + (offset + i)*4);
    }
    return length;
}

u32 XHdcp22_mmult_Write_NPrime_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH - XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + offset + i) = *(data + i);
    }
    return length;
}

u32 XHdcp22_mmult_Read_NPrime_Bytes(XHdcp22_mmult *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XHDCP22_MMULT_CTRL_ADDR_NPRIME_HIGH - XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XHDCP22_MMULT_CTRL_ADDR_NPRIME_BASE + offset + i);
    }
    return length;
}

void XHdcp22_mmult_InterruptGlobalEnable(XHdcp22_mmult *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_GIE, 1);
}

void XHdcp22_mmult_InterruptGlobalDisable(XHdcp22_mmult *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_GIE, 0);
}

void XHdcp22_mmult_InterruptEnable(XHdcp22_mmult *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_IER);
    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_IER, Register | Mask);
}

void XHdcp22_mmult_InterruptDisable(XHdcp22_mmult *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_IER);
    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_IER, Register & (~Mask));
}

void XHdcp22_mmult_InterruptClear(XHdcp22_mmult *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XHdcp22_mmult_WriteReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_ISR, Mask);
}

u32 XHdcp22_mmult_InterruptGetEnabled(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_IER);
}

u32 XHdcp22_mmult_InterruptGetStatus(XHdcp22_mmult *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XHdcp22_mmult_ReadReg(InstancePtr->Config.BaseAddress, XHDCP22_MMULT_CTRL_ADDR_ISR);
}

/** @} */
