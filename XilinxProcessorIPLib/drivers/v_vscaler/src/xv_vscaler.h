// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_VSCALER_H
#define XV_VSCALER_H

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
#include "xv_vscaler_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
/**
* This typedef contains configuration information for the vertical scaler
* core. Each core instance should have a configuration structure
* associated.
*/
typedef struct {
    u16 DeviceId;          /**< Unique ID  of device */
    UINTPTR BaseAddress;   /**< The base address of the core instance. */
    u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
    u16 NumVidComponents;  /**< Number of Video Components */
    u16 MaxWidth;          /**< Maximum columns supported by core instance */
    u16 MaxHeight;         /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;      /**< Maximum Data width of each channel */
    u16 PhaseShift;        /**< Max num of phases (2^PhaseShift) */
    u16 ScalerType;        /**< Scaling Algorithm Selected */
    u16 NumTaps;           /**< Number of taps */
    u16 Is420Enabled;      /**< Color format YUV420 supported by instance */
} XV_vscaler_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_vscaler_Config Config; /**< Hardware Configuration */
    u32 IsReady;              /**< Device is initialized and ready */
} XV_vscaler;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_vscaler_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_vscaler_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_vscaler_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_vscaler_ReadReg(BaseAddress, RegOffset) \
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
int XV_vscaler_Initialize(XV_vscaler *InstancePtr, u16 DeviceId);
XV_vscaler_Config* XV_vscaler_LookupConfig(u16 DeviceId);
int XV_vscaler_CfgInitialize(XV_vscaler *InstancePtr,
                             XV_vscaler_Config *ConfigPtr,
							 UINTPTR EffectiveAddr);
#else
int XV_vscaler_Initialize(XV_vscaler *InstancePtr, const char* InstanceName);
int XV_vscaler_Release(XV_vscaler *InstancePtr);
#endif

void XV_vscaler_Start(XV_vscaler *InstancePtr);
u32 XV_vscaler_IsDone(XV_vscaler *InstancePtr);
u32 XV_vscaler_IsIdle(XV_vscaler *InstancePtr);
u32 XV_vscaler_IsReady(XV_vscaler *InstancePtr);
void XV_vscaler_EnableAutoRestart(XV_vscaler *InstancePtr);
void XV_vscaler_DisableAutoRestart(XV_vscaler *InstancePtr);

void XV_vscaler_Set_HwReg_HeightIn(XV_vscaler *InstancePtr, u32 Data);
u32 XV_vscaler_Get_HwReg_HeightIn(XV_vscaler *InstancePtr);
void XV_vscaler_Set_HwReg_Width(XV_vscaler *InstancePtr, u32 Data);
u32 XV_vscaler_Get_HwReg_Width(XV_vscaler *InstancePtr);
void XV_vscaler_Set_HwReg_HeightOut(XV_vscaler *InstancePtr, u32 Data);
u32 XV_vscaler_Get_HwReg_HeightOut(XV_vscaler *InstancePtr);
void XV_vscaler_Set_HwReg_LineRate(XV_vscaler *InstancePtr, u32 Data);
u32 XV_vscaler_Get_HwReg_LineRate(XV_vscaler *InstancePtr);
void XV_vscaler_Set_HwReg_ColorMode(XV_vscaler *InstancePtr, u32 Data);
u32 XV_vscaler_Get_HwReg_ColorMode(XV_vscaler *InstancePtr);
UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(XV_vscaler *InstancePtr);
UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_HighAddress(XV_vscaler *InstancePtr);
u32 XV_vscaler_Get_HwReg_vfltCoeff_TotalBytes(XV_vscaler *InstancePtr);
u32 XV_vscaler_Get_HwReg_vfltCoeff_BitWidth(XV_vscaler *InstancePtr);
u32 XV_vscaler_Get_HwReg_vfltCoeff_Depth(XV_vscaler *InstancePtr);
u32 XV_vscaler_Write_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length);
u32 XV_vscaler_Read_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length);
u32 XV_vscaler_Write_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length);
u32 XV_vscaler_Read_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length);

void XV_vscaler_InterruptGlobalEnable(XV_vscaler *InstancePtr);
void XV_vscaler_InterruptGlobalDisable(XV_vscaler *InstancePtr);
void XV_vscaler_InterruptEnable(XV_vscaler *InstancePtr, u32 Mask);
void XV_vscaler_InterruptDisable(XV_vscaler *InstancePtr, u32 Mask);
void XV_vscaler_InterruptClear(XV_vscaler *InstancePtr, u32 Mask);
u32 XV_vscaler_InterruptGetEnabled(XV_vscaler *InstancePtr);
u32 XV_vscaler_InterruptGetStatus(XV_vscaler *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
