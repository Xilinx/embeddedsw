/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XMPEGTSMUX_H
#define XMPEGTSMUX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xmpegtsmux_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u64 Ctrl_BaseAddress;
} XMpegtsmux_Config;
#endif

typedef void (*XMpegTsMux_Callback)(void *CallbackRef);
typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
    XMpegTsMux_Callback Callback;
    void *CallbackRef;
} XMpegtsmux;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XMpegtsmux_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XMpegtsmux_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XMpegtsmux_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XMpegtsmux_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XMpegtsmux_Initialize(XMpegtsmux *InstancePtr, u16 DeviceId);
XMpegtsmux_Config* XMpegtsmux_LookupConfig(u16 DeviceId);
int XMpegtsmux_CfgInitialize(XMpegtsmux *InstancePtr, XMpegtsmux_Config *ConfigPtr);
#else
int XMpegtsmux_Initialize(XMpegtsmux *InstancePtr, const char* InstanceName);
int XMpegtsmux_Release(XMpegtsmux *InstancePtr);
#endif

void XMpegtsmux_Start(XMpegtsmux *InstancePtr);
u32 XMpegtsmux_IsDone(XMpegtsmux *InstancePtr);
u32 XMpegtsmux_IsIdle(XMpegtsmux *InstancePtr);
u32 XMpegtsmux_IsReady(XMpegtsmux *InstancePtr);
void XMpegtsmux_EnableAutoRestart(XMpegtsmux *InstancePtr);
void XMpegtsmux_Stop(XMpegtsmux *InstancePtr);

u64 XMpegtsmux_Get_status(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_mux_context(XMpegtsmux *InstancePtr, u64 Data);
u64 XMpegtsmux_Get_mux_context(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_stream_context(XMpegtsmux *InstancePtr, u64 Data);
u64 XMpegtsmux_Get_stream_context(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_data_in(XMpegtsmux *InstancePtr, u32 Data);
u32 XMpegtsmux_Get_data_in(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_data_out_byte_inf(XMpegtsmux *InstancePtr, u32 Data);
u32 XMpegtsmux_Get_data_out_byte_inf(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_num_desc(XMpegtsmux *InstancePtr, u32 Data);
u32 XMpegtsmux_Get_num_desc(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_stream_id_table(XMpegtsmux *InstancePtr, u64 Data);
u64 XMpegtsmux_Get_stream_id_table(XMpegtsmux *InstancePtr);
void XMpegtsmux_Set_num_streams_table(XMpegtsmux *InstancePtr, u64 Data);
u64 XMpegtsmux_Get_num_streams_table(XMpegtsmux *InstancePtr);

void XMpegtsmux_InterruptGlobalEnable(XMpegtsmux *InstancePtr);
void XMpegtsmux_InterruptGlobalDisable(XMpegtsmux *InstancePtr);
void XMpegtsmux_InterruptEnable(XMpegtsmux *InstancePtr, u32 Mask);
void XMpegtsmux_InterruptDisable(XMpegtsmux *InstancePtr, u32 Mask);
void XMpegtsmux_InterruptClear(XMpegtsmux *InstancePtr, u32 Mask);
u32 XMpegtsmux_InterruptGetEnabled(XMpegtsmux *InstancePtr);
u32 XMpegtsmux_InterruptGetStatus(XMpegtsmux *InstancePtr);
void *XMpegTsMuxIntrHandler(void *InstancePtr);
void XMpegTsMux_SetCallback(XMpegtsmux *InstancePtr, void *CallbackFunc,
	void *CallbackRef);

#ifdef __cplusplus
}
#endif

#endif
