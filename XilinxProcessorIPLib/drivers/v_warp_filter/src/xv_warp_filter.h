// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifndef XV_WARP_FILTER_H
#define XV_WARP_FILTER_H

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
#include "xv_warp_filter_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    UINTPTR Control_BaseAddress;
    u16 max_width; 			/*Maximum width*/
    u16 max_height;			/*Maximum height*/
    u16 axi_mm_data_width;	/*Axi MM port data width*/
    u16 bpc;				/*Max bits per component supported*/
} XV_warp_filter_Config;
#endif

typedef void (*XV_warp_filter_Callback)(void *CallbackRef);
typedef struct {
	XV_warp_filter_Config *config;
	UINTPTR Control_BaseAddress;
    u32 IsReady;
	XV_warp_filter_Callback FrameDoneCallback;
    void *CallbackRef;
	UINTPTR WarpFilterDesc_BaseAddr;
	u32 NumDescriptors;
} XV_warp_filter;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_warp_filter_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_warp_filter_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_warp_filter_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_warp_filter_ReadReg(BaseAddress, RegOffset) \
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
s32 XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, u16 DeviceId);
XV_warp_filter_Config* XV_warp_filter_LookupConfig(u16 DeviceId);
s32 XV_warp_filter_CfgInitialize(XV_warp_filter *InstancePtr, XV_warp_filter_Config *ConfigPtr);
#else
int XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, const char* InstanceName);
int XV_warp_filter_Release(XV_warp_filter *InstancePtr);
#endif

void XV_warp_filter_Start(XV_warp_filter *InstancePtr);
u32 XV_warp_filter_IsDone(XV_warp_filter *InstancePtr);
u32 XV_warp_filter_IsIdle(XV_warp_filter *InstancePtr);
u32 XV_warp_filter_IsReady(XV_warp_filter *InstancePtr);
void XV_warp_filter_EnableAutoRestart(XV_warp_filter *InstancePtr);
void XV_warp_filter_DisableAutoRestart(XV_warp_filter *InstancePtr);
void XV_warp_filter_SetFlushbit(XV_warp_filter *InstancePtr);
u32 XV_warp_filter_Get_FlushDone(XV_warp_filter *InstancePtr);

void XV_warp_filter_Set_desc_addr(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_desc_addr(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_read(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_read(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_reads(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_reads(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_write(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_write(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_read1(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_read1(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_read1s(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_read1s(XV_warp_filter *InstancePtr);
void XV_warp_filter_Set_maxi_write1(XV_warp_filter *InstancePtr, u64 Data);
u64 XV_warp_filter_Get_maxi_write1(XV_warp_filter *InstancePtr);

void XV_warp_filter_InterruptGlobalEnable(XV_warp_filter *InstancePtr);
void XV_warp_filter_InterruptGlobalDisable(XV_warp_filter *InstancePtr);
void XV_warp_filter_InterruptEnable(XV_warp_filter *InstancePtr, u32 Mask);
void XV_warp_filter_InterruptDisable(XV_warp_filter *InstancePtr, u32 Mask);
void XV_warp_filter_InterruptClear(XV_warp_filter *InstancePtr, u32 Mask);
u32 XV_warp_filter_InterruptGetEnabled(XV_warp_filter *InstancePtr);
u32 XV_warp_filter_InterruptGetStatus(XV_warp_filter *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
