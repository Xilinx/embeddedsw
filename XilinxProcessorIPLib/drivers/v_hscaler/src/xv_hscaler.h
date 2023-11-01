// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_HSCALER_H
#define XV_HSCALER_H

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
#include "xv_hscaler_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
* This typedef contains configuration information for the horizontal scaler
* core. Each core instance should have a configuration structure
* associated.
*/
typedef struct {
#ifndef SDT
    u16 DeviceId;         /**< Unique ID  of device */
#else
    char *Name;
#endif
    UINTPTR BaseAddress;  /**< The base address of the core instance. */
    u16 PixPerClk;        /**< Samples Per Clock supported by core instance */
    u16 NumVidComponents; /**< Number of Video Components */
    u16 MaxWidth;         /**< Maximum columns supported by core instance */
    u16 MaxHeight;        /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;     /**< Maximum Data width of each channel */
    u16 PhaseShift;       /**< Max num of phases (2^PhaseShift) */
    u16 ScalerType;       /**< Scaling Algorithm Selected */
    u16 NumTaps;          /**< Number of taps */
    u16 Is422Enabled;     /**< Color format YUV422 supported by instance */
    u16 Is420Enabled;     /**< Color format YUV420 supported by instance */
    u16 IsCscEnabled;     /**< Color space conversion supported by instance */
} XV_hscaler_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_hscaler_Config Config; /**< Hardware Configuration */
    u32 IsReady;              /**< Device is initialized and ready */
} XV_hscaler;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_hscaler_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_hscaler_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_hscaler_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_hscaler_ReadReg(BaseAddress, RegOffset) \
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
int XV_hscaler_Initialize(XV_hscaler *InstancePtr, u16 DeviceId);
XV_hscaler_Config* XV_hscaler_LookupConfig(u16 DeviceId);
#else
int XV_hscaler_Initialize(XV_hscaler *InstancePtr, UINTPTR BaseAddress);
XV_hscaler_Config* XV_hscaler_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_hscaler_CfgInitialize(XV_hscaler *InstancePtr,
                             XV_hscaler_Config *ConfigPtr,
							 UINTPTR EffectiveAddr);
#else
int XV_hscaler_Initialize(XV_hscaler *InstancePtr, const char* InstanceName);
int XV_hscaler_Release(XV_hscaler *InstancePtr);
#endif

void XV_hscaler_Start(XV_hscaler *InstancePtr);
u32 XV_hscaler_IsDone(XV_hscaler *InstancePtr);
u32 XV_hscaler_IsIdle(XV_hscaler *InstancePtr);
u32 XV_hscaler_IsReady(XV_hscaler *InstancePtr);
void XV_hscaler_EnableAutoRestart(XV_hscaler *InstancePtr);
void XV_hscaler_DisableAutoRestart(XV_hscaler *InstancePtr);

void XV_hscaler_Set_HwReg_Height(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_Height(XV_hscaler *InstancePtr);
void XV_hscaler_Set_HwReg_WidthIn(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_WidthIn(XV_hscaler *InstancePtr);
void XV_hscaler_Set_HwReg_WidthOut(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_WidthOut(XV_hscaler *InstancePtr);
void XV_hscaler_Set_HwReg_ColorMode(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_ColorMode(XV_hscaler *InstancePtr);
void XV_hscaler_Set_HwReg_ColorModeOut(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_ColorModeOut(XV_hscaler *InstancePtr);
void XV_hscaler_Set_HwReg_PixelRate(XV_hscaler *InstancePtr, u32 Data);
u32 XV_hscaler_Get_HwReg_PixelRate(XV_hscaler *InstancePtr);
UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(XV_hscaler *InstancePtr);
UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_HighAddress(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_hfltCoeff_TotalBytes(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_hfltCoeff_BitWidth(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_hfltCoeff_Depth(XV_hscaler *InstancePtr);
u32 XV_hscaler_Write_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length);
u32 XV_hscaler_Read_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length);
u32 XV_hscaler_Write_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length);
u32 XV_hscaler_Read_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length);
UINTPTR XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(XV_hscaler *InstancePtr);
UINTPTR XV_hscaler_Get_HwReg_phasesH_V_HighAddress(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_phasesH_V_TotalBytes(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_phasesH_V_BitWidth(XV_hscaler *InstancePtr);
u32 XV_hscaler_Get_HwReg_phasesH_V_Depth(XV_hscaler *InstancePtr);
u32 XV_hscaler_Write_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length);
u32 XV_hscaler_Read_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length);
u32 XV_hscaler_Write_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length);
u32 XV_hscaler_Read_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length);

void XV_hscaler_InterruptGlobalEnable(XV_hscaler *InstancePtr);
void XV_hscaler_InterruptGlobalDisable(XV_hscaler *InstancePtr);
void XV_hscaler_InterruptEnable(XV_hscaler *InstancePtr, u32 Mask);
void XV_hscaler_InterruptDisable(XV_hscaler *InstancePtr, u32 Mask);
void XV_hscaler_InterruptClear(XV_hscaler *InstancePtr, u32 Mask);
u32 XV_hscaler_InterruptGetEnabled(XV_hscaler *InstancePtr);
u32 XV_hscaler_InterruptGetStatus(XV_hscaler *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
