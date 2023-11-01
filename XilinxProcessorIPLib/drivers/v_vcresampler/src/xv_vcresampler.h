// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_VCRESAMPLER_H
#define XV_VCRESAMPLER_H

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
#include "xv_vcresampler_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
* This typedef contains configuration information for the vertical
* chroma resampler core. Each core instance should have a configuration
* structure associated.
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
    u16 ResamplingType;    /**< Resampling Method selected */
    u16 NumTaps;           /**< Number of filter taps */
} XV_vcresampler_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_vcresampler_Config Config; /**< Hardware Configuration */
    u32 IsReady;                  /**< Device is initialized and ready */
} XV_vcresampler;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_vcresampler_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_vcresampler_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_vcresampler_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_vcresampler_ReadReg(BaseAddress, RegOffset) \
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
int XV_vcresampler_Initialize(XV_vcresampler *InstancePtr, u16 DeviceId);
XV_vcresampler_Config* XV_vcresampler_LookupConfig(u16 DeviceId);
#else
int XV_vcresampler_Initialize(XV_vcresampler *InstancePtr, UINTPTR BaseAddress);
XV_vcresampler_Config* XV_vcresampler_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_vcresampler_CfgInitialize(XV_vcresampler *InstancePtr,
                                 XV_vcresampler_Config *ConfigPtr,
								 UINTPTR EffectiveAddr);
#else
int XV_vcresampler_Initialize(XV_vcresampler *InstancePtr, const char* InstanceName);
int XV_vcresampler_Release(XV_vcresampler *InstancePtr);
#endif

void XV_vcresampler_Start(XV_vcresampler *InstancePtr);
u32 XV_vcresampler_IsDone(XV_vcresampler *InstancePtr);
u32 XV_vcresampler_IsIdle(XV_vcresampler *InstancePtr);
u32 XV_vcresampler_IsReady(XV_vcresampler *InstancePtr);
void XV_vcresampler_EnableAutoRestart(XV_vcresampler *InstancePtr);
void XV_vcresampler_DisableAutoRestart(XV_vcresampler *InstancePtr);

void XV_vcresampler_Set_HwReg_width(XV_vcresampler *InstancePtr, u32 Data);
u32 XV_vcresampler_Get_HwReg_width(XV_vcresampler *InstancePtr);
void XV_vcresampler_Set_HwReg_height(XV_vcresampler *InstancePtr, u32 Data);
u32 XV_vcresampler_Get_HwReg_height(XV_vcresampler *InstancePtr);
void XV_vcresampler_Set_HwReg_input_video_format(XV_vcresampler *InstancePtr, u32 Data);
u32 XV_vcresampler_Get_HwReg_input_video_format(XV_vcresampler *InstancePtr);
void XV_vcresampler_Set_HwReg_output_video_format(XV_vcresampler *InstancePtr, u32 Data);
u32 XV_vcresampler_Get_HwReg_output_video_format(XV_vcresampler *InstancePtr);
void XV_vcresampler_Set_HwReg_coefs_0_0(XV_vcresampler *InstancePtr, u32 Data);
u32 XV_vcresampler_Get_HwReg_coefs_0_0(XV_vcresampler *InstancePtr);

void XV_vcresampler_InterruptGlobalEnable(XV_vcresampler *InstancePtr);
void XV_vcresampler_InterruptGlobalDisable(XV_vcresampler *InstancePtr);
void XV_vcresampler_InterruptEnable(XV_vcresampler *InstancePtr, u32 Mask);
void XV_vcresampler_InterruptDisable(XV_vcresampler *InstancePtr, u32 Mask);
void XV_vcresampler_InterruptClear(XV_vcresampler *InstancePtr, u32 Mask);
u32 XV_vcresampler_InterruptGetEnabled(XV_vcresampler *InstancePtr);
u32 XV_vcresampler_InterruptGetStatus(XV_vcresampler *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
