// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================
#ifndef XV_WARP_INIT_H
#define XV_WARP_INIT_H

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
#include "xv_warp_init_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    UINTPTR Ctrl_BaseAddress;
    u16 max_width; 			/*Maximum width*/
    u16 max_height;			/*Maximum height*/
    u8  warp_type;			/*Supported warp type*/
    u16 axi_mm_data_width;	/*Axi MM port data width*/
    u16 bpc;				/*Max bits per component supported*/
    u16 max_control_pts;	/*Max supported control points for arbitary warp*/
} XV_warp_init_Config;
#endif

typedef void (*XV_warp_init_Callback)(void *CallbackRef);
typedef struct {
	XV_warp_init_Config *config;
	UINTPTR Ctrl_BaseAddress;
    u32 IsReady;
    XV_warp_init_Callback FrameDoneCallback;
    void *CallbackRef;
    UINTPTR RemapVectorDesc_BaseAddr;
    u32 NumDescriptors;
} XV_warp_init;

typedef u32 word_type;
typedef enum {
	DISTORTION_LENS = 0,
	DISTORTION_ARBITARY = 1
}XV_warp_init_warp_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_warp_init_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_warp_init_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_warp_init_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_warp_init_ReadReg(BaseAddress, RegOffset) \
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
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, u16 DeviceId);
XV_warp_init_Config* XV_warp_init_LookupConfig(u16 DeviceId);
int XV_warp_init_CfgInitialize(XV_warp_init *InstancePtr, XV_warp_init_Config *ConfigPtr);
#else
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, const char* InstanceName);
int XV_warp_init_Release(XV_warp_init *InstancePtr);
#endif

void XV_warp_init_Start(XV_warp_init *InstancePtr);
u32 XV_warp_init_IsDone(XV_warp_init *InstancePtr);
u32 XV_warp_init_IsIdle(XV_warp_init *InstancePtr);
u32 XV_warp_init_IsReady(XV_warp_init *InstancePtr);
void XV_warp_init_EnableAutoRestart(XV_warp_init *InstancePtr);
void XV_warp_init_DisableAutoRestart(XV_warp_init *InstancePtr);
u32 XV_warp_init_Get_FlushDone(XV_warp_init *InstancePtr);
void XV_warp_init_SetFlushbit(XV_warp_init *InstancePtr);
u32 XV_warp_init_Get_ip_status(XV_warp_init *InstancePtr);
void XV_warp_init_Set_maxi_read_write(XV_warp_init *InstancePtr, u64 Data);
u64 XV_warp_init_Get_maxi_read_write(XV_warp_init *InstancePtr);
void XV_warp_init_Set_desc_addr(XV_warp_init *InstancePtr, u64 Data);
u64 XV_warp_init_Get_desc_addr(XV_warp_init *InstancePtr);

void XV_warp_init_InterruptGlobalEnable(XV_warp_init *InstancePtr);
void XV_warp_init_InterruptGlobalDisable(XV_warp_init *InstancePtr);
void XV_warp_init_InterruptEnable(XV_warp_init *InstancePtr, u32 Mask);
void XV_warp_init_InterruptDisable(XV_warp_init *InstancePtr, u32 Mask);
void XV_warp_init_InterruptClear(XV_warp_init *InstancePtr, u32 Mask);
u32 XV_warp_init_InterruptGetEnabled(XV_warp_init *InstancePtr);
u32 XV_warp_init_InterruptGetStatus(XV_warp_init *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
