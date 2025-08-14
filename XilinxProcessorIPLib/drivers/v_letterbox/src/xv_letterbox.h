// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_letterbox.h
 * @addtogroup xv_letterbox Overview
 */
#ifndef XV_LETTERBOX_H
#define XV_LETTERBOX_H

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
#include "xv_letterbox_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
* This typedef contains configuration information for the letterbox core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
    u16 DeviceId;          /**< Unique ID  of device */
#else
    char *Name;
#endif
    UINTPTR BaseAddress;   /**< The base address of the core instance. */
    u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
    u16 NumVidComponents;  /**< Number of Video Components */
    u16 MaxWidth;          /**< Maximum columns supported by core instance */
    u16 MaxHeight;         /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;      /**< Maximum Data width of each channel */
} XV_letterbox_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_letterbox_Config Config;  /**< Hardware Configuration */
    u32 IsReady;                 /**< Device is initialized and ready */
} XV_letterbox;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_letterbox_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_letterbox_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_letterbox_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_letterbox_ReadReg(BaseAddress, RegOffset) \
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
int XV_letterbox_Initialize(XV_letterbox *InstancePtr, u16 DeviceId);
XV_letterbox_Config* XV_letterbox_LookupConfig(u16 DeviceId);
#else
int XV_letterbox_Initialize(XV_letterbox *InstancePtr, UINTPTR BaseAddress);
XV_letterbox_Config* XV_letterbox_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_letterbox_CfgInitialize(XV_letterbox *InstancePtr,
                               XV_letterbox_Config *ConfigPtr,
							   UINTPTR EffectiveAddr);
#else
int XV_letterbox_Initialize(XV_letterbox *InstancePtr, const char* InstanceName);
int XV_letterbox_Release(XV_letterbox *InstancePtr);
#endif

void XV_letterbox_Start(XV_letterbox *InstancePtr);
u32 XV_letterbox_IsDone(XV_letterbox *InstancePtr);
u32 XV_letterbox_IsIdle(XV_letterbox *InstancePtr);
u32 XV_letterbox_IsReady(XV_letterbox *InstancePtr);
void XV_letterbox_EnableAutoRestart(XV_letterbox *InstancePtr);
void XV_letterbox_DisableAutoRestart(XV_letterbox *InstancePtr);

void XV_letterbox_Set_HwReg_width(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_width(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_height(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_height(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_video_format(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_video_format(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_col_start(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_col_start(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_col_end(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_col_end(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_row_start(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_row_start(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_row_end(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_row_end(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_Y_R_value(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_Y_R_value(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_Cb_G_value(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_Cb_G_value(XV_letterbox *InstancePtr);
void XV_letterbox_Set_HwReg_Cr_B_value(XV_letterbox *InstancePtr, u32 Data);
u32 XV_letterbox_Get_HwReg_Cr_B_value(XV_letterbox *InstancePtr);

void XV_letterbox_InterruptGlobalEnable(XV_letterbox *InstancePtr);
void XV_letterbox_InterruptGlobalDisable(XV_letterbox *InstancePtr);
void XV_letterbox_InterruptEnable(XV_letterbox *InstancePtr, u32 Mask);
void XV_letterbox_InterruptDisable(XV_letterbox *InstancePtr, u32 Mask);
void XV_letterbox_InterruptClear(XV_letterbox *InstancePtr, u32 Mask);
u32 XV_letterbox_InterruptGetEnabled(XV_letterbox *InstancePtr);
u32 XV_letterbox_InterruptGetStatus(XV_letterbox *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
