/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
******************************************************************************/
/***************************** Include Files *********************************/
#include "xmpegtsmux.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XMpegtsmux_CfgInitialize(XMpegtsmux *InstancePtr, XMpegtsmux_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XMpegtsmux_Start(XMpegtsmux *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL,
			XMPEGTSMUX_CTRL_EN_MASK);
}

u32 XMpegtsmux_IsDone(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL);
    return (Data >> XMPEGTSMUX_CTRL_IS_DONE_SHIFT) & XMPEGTSMUX_CTRL_IS_DONE_MASK;
}

u32 XMpegtsmux_IsIdle(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL);
    return (Data >> XMPEGTSMUX_CTRL_IS_IDLE_SHIFT) & XMPEGTSMUX_CTRL_IS_IDLE_MASK;
}

u32 XMpegtsmux_IsReady(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL);
    /* check ap_start to see if the pcore is ready for next input */
    return !(Data & XMPEGTSMUX_CTRL_EN_MASK);
}

void XMpegtsmux_EnableAutoRestart(XMpegtsmux *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL,
			XMPEGTSMUX_CTRL_AUTO_RESTART_MASK);
}

void XMpegtsmux_Stop(XMpegtsmux *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_AP_CTRL, XMPEGTSMUX_CTRL_DIS_MASK);
}

u64 XMpegtsmux_Get_status(XMpegtsmux *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STATUS_DATA);
    Data += (u64)XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STATUS_DATA + 4) << 32;
    return Data;
}

void XMpegtsmux_Set_mux_context(XMpegtsmux *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_MUX_CONTEXT_DATA, (u32)(Data));
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_MUX_CONTEXT_DATA + 4, (u32)(Data >> 32));
}

u64 XMpegtsmux_Get_mux_context(XMpegtsmux *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_MUX_CONTEXT_DATA);
    Data += (u64)XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_MUX_CONTEXT_DATA + 4) << 32;
    return Data;
}

void XMpegtsmux_Set_stream_context(XMpegtsmux *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_CONTEXT_DATA, (u32)(Data));
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_CONTEXT_DATA + 4, (u32)(Data >> 32));
}

u64 XMpegtsmux_Get_stream_context(XMpegtsmux *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_CONTEXT_DATA);
    Data += (u64)XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_CONTEXT_DATA + 4) << 32;
    return Data;
}

void XMpegtsmux_Set_data_in(XMpegtsmux *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_IN_DATA, Data);
}

u32 XMpegtsmux_Get_data_in(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_IN_DATA);
    return Data;
}

void XMpegtsmux_Set_data_out_byte_inf(XMpegtsmux *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_OUT_BYTE_INF_DATA, Data);
}

u32 XMpegtsmux_Get_data_out_byte_inf(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_DATA_OUT_BYTE_INF_DATA);
    return Data;
}

void XMpegtsmux_Set_num_desc(XMpegtsmux *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_DESC_DATA, Data);
}

u32 XMpegtsmux_Get_num_desc(XMpegtsmux *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_DESC_DATA);
    return Data;
}

void XMpegtsmux_Set_stream_id_table(XMpegtsmux *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_ID_TABLE_DATA, (u32)(Data));
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_ID_TABLE_DATA + 4, (u32)(Data >> 32));
}

u64 XMpegtsmux_Get_stream_id_table(XMpegtsmux *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_ID_TABLE_DATA);
    Data += (u64)XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_STREAM_ID_TABLE_DATA + 4) << 32;
    return Data;
}

void XMpegtsmux_Set_num_streams_table(XMpegtsmux *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA, (u32)(Data));
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA + 4, (u32)(Data >> 32));
}

u64 XMpegtsmux_Get_num_streams_table(XMpegtsmux *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA);
    Data += (u64)XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_HWSTRUCTIN_NUM_STREAMS_TABLE_DATA + 4) << 32;
    return Data;
}

void XMpegtsmux_InterruptGlobalEnable(XMpegtsmux *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_GIE, XMPEGTSMUX_CTRL_EN_MASK);
}

void XMpegtsmux_InterruptGlobalDisable(XMpegtsmux *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_GIE, XMPEGTSMUX_CTRL_DIS_MASK);
}

void XMpegtsmux_InterruptEnable(XMpegtsmux *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_IER);
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_IER, Register | Mask);
}

void XMpegtsmux_InterruptDisable(XMpegtsmux *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_IER);
    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_IER, Register & (~Mask));
}

void XMpegtsmux_InterruptClear(XMpegtsmux *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMpegtsmux_WriteReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_ISR, Mask);
}

u32 XMpegtsmux_InterruptGetEnabled(XMpegtsmux *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_IER);
}

u32 XMpegtsmux_InterruptGetStatus(XMpegtsmux *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMpegtsmux_ReadReg(InstancePtr->Ctrl_BaseAddress, XMPEGTSMUX_CTRL_ADDR_ISR);
}
