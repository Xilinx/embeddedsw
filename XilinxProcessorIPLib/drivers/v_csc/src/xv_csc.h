// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
* @file xv_csc.h
* @addtogroup v_csc Overview
*/
#ifndef XV_CSC_H
#define XV_CSC_H

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
#include "xv_csc_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
* This typedef contains configuration information for the CSC core.
* Each core instance should have a configuration structure
* associated.
*/
typedef struct {
#ifndef SDT
    u16 DeviceId;            /**< Unique ID  of device */
#else
    char *Name;
#endif
    UINTPTR BaseAddress;     /**< The base address of the core instance. */
    u16 PixPerClk;           /**< Samples Per Clock supported by core instance */
    u16 MaxWidth;            /**< Maximum columns supported by core instance */
    u16 MaxHeight;           /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;        /**< Maximum Data width of each channel */
    u16 Is422Enabled;        /**< 4:2:2 color format supported by core instance */
    u16 Is420Enabled;        /**< 4:2:0 color format supported by core instance */
    u16 IsDemoWindowEnabled; /**< CSC demo window supported by core instance */
} XV_csc_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_csc_Config Config;  /**< Hardware Configuration */
    u32 IsReady;           /**< Device is initialized and ready */
} XV_csc;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_csc_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_csc_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_csc_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_csc_ReadReg(BaseAddress, RegOffset) \
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
#ifndef SDT
int XV_csc_Initialize(XV_csc *InstancePtr, u16 DeviceId);
XV_csc_Config* XV_csc_LookupConfig(u16 DeviceId);
#else
int XV_csc_Initialize(XV_csc *InstancePtr, UINTPTR BaseAddress);
XV_csc_Config* XV_csc_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_csc_CfgInitialize(XV_csc *InstancePtr,
                         XV_csc_Config *ConfigPtr,
						 UINTPTR EffectiveAddr);
#else
int XV_csc_Initialize(XV_csc *InstancePtr, const char* InstanceName);
int XV_csc_Release(XV_csc *InstancePtr);
#endif

void XV_csc_Start(XV_csc *InstancePtr);
u32 XV_csc_IsDone(XV_csc *InstancePtr);
u32 XV_csc_IsIdle(XV_csc *InstancePtr);
u32 XV_csc_IsReady(XV_csc *InstancePtr);
void XV_csc_EnableAutoRestart(XV_csc *InstancePtr);
void XV_csc_DisableAutoRestart(XV_csc *InstancePtr);

void XV_csc_Set_HwReg_InVideoFormat(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_InVideoFormat(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_OutVideoFormat(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_OutVideoFormat(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_width(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_width(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_height(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_height(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ColStart(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ColStart(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ColEnd(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ColEnd(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_RowStart(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_RowStart(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_RowEnd(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_RowEnd(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K11(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K11(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K12(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K12(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K13(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K13(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K21(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K21(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K22(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K22(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K23(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K23(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K31(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K31(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K32(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K32(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K33(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K33(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ROffset_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ROffset_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_GOffset_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_GOffset_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_BOffset_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_BOffset_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ClampMin_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ClampMin_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ClipMax_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ClipMax_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K11_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K11_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K12_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K12_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K13_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K13_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K21_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K21_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K22_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K22_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K23_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K23_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K31_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K31_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K32_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K32_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_K33_2(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_K33_2(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ROffset_2_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ROffset_2_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_GOffset_2_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_GOffset_2_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_BOffset_2_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_BOffset_2_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ClampMin_2_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ClampMin_2_V(XV_csc *InstancePtr);
void XV_csc_Set_HwReg_ClipMax_2_V(XV_csc *InstancePtr, u32 Data);
u32 XV_csc_Get_HwReg_ClipMax_2_V(XV_csc *InstancePtr);

void XV_csc_InterruptGlobalEnable(XV_csc *InstancePtr);
void XV_csc_InterruptGlobalDisable(XV_csc *InstancePtr);
void XV_csc_InterruptEnable(XV_csc *InstancePtr, u32 Mask);
void XV_csc_InterruptDisable(XV_csc *InstancePtr, u32 Mask);
void XV_csc_InterruptClear(XV_csc *InstancePtr, u32 Mask);
u32 XV_csc_InterruptGetEnabled(XV_csc *InstancePtr);
u32 XV_csc_InterruptGetStatus(XV_csc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
