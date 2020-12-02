// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_AXI4S_REMAP_H
#define XV_AXI4S_REMAP_H

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
#include "xv_axi4s_remap_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

/**
* This typedef contains configuration information for the mixer core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
  u16 DeviceId;            /**< Unique ID  of device */
  UINTPTR BaseAddress;     /**< The base address of the core instance. */
  u16 NumVidComponents;    /**< Number of Video Components */
  u16 MaxWidth;            /**< Maximum columns supported by core instance */
  u16 MaxHeight;           /**< Maximum rows supported by core instance */
  u16 PixPerClkIn;         /**< Input Samples Per Clock */
  u16 PixPerClkOut;        /**< Output Samples Per Clock */
  u16 IsPixPerClockConvEn; /**< Samples Per Clock Coversion Feature Enable*/
  u16 MaxDataWidthIn;      /**< Input Maximum Data width of each channel */
  u16 MaxDataWidthOut;     /**< Output Maximum Data width of each channel */
  u16 IsHdmi420InEn;       /**< HDMI 420 to AXIS 420 converter block En */
  u16 IsHdmi420OutEn;      /**< AXIS 420 to HDMI 420 converter block En */
  u16 IsInPixelDropEn;     /**< Input Pixel Drop block En */
  u16 IsOutPixelRepeatEn;  /**< Output Pixel Repeat block En */
} XV_axi4s_remap_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
  XV_axi4s_remap_Config Config; /**< Hardware Configuration */
  u32 IsReady;                  /**< Device is initialized and ready */
} XV_axi4s_remap;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_axi4s_remap_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_axi4s_remap_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_axi4s_remap_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_axi4s_remap_ReadReg(BaseAddress, RegOffset) \
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
int XV_axi4s_remap_Initialize(XV_axi4s_remap *InstancePtr, u16 DeviceId);
XV_axi4s_remap_Config* XV_axi4s_remap_LookupConfig(u16 DeviceId);
int XV_axi4s_remap_CfgInitialize(XV_axi4s_remap *InstancePtr,
		                 XV_axi4s_remap_Config *ConfigPtr,
		                 UINTPTR EffectiveAddr);
#else
int XV_axi4s_remap_Initialize(XV_axi4s_remap *InstancePtr, const char* InstanceName);
int XV_axi4s_remap_Release(XV_axi4s_remap *InstancePtr);
#endif

void XV_axi4s_remap_Start(XV_axi4s_remap *InstancePtr);
u32 XV_axi4s_remap_IsDone(XV_axi4s_remap *InstancePtr);
u32 XV_axi4s_remap_IsIdle(XV_axi4s_remap *InstancePtr);
u32 XV_axi4s_remap_IsReady(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_EnableAutoRestart(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_DisableAutoRestart(XV_axi4s_remap *InstancePtr);

void XV_axi4s_remap_Set_height(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_height(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_width(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_width(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_ColorFormat(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_ColorFormat(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_inPixClk(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_inPixClk(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_outPixClk(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_outPixClk(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_inHDMI420(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_inHDMI420(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_outHDMI420(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_outHDMI420(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_inPixDrop(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_inPixDrop(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_Set_outPixRepeat(XV_axi4s_remap *InstancePtr, u32 Data);
u32 XV_axi4s_remap_Get_outPixRepeat(XV_axi4s_remap *InstancePtr);

void XV_axi4s_remap_InterruptGlobalEnable(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_InterruptGlobalDisable(XV_axi4s_remap *InstancePtr);
void XV_axi4s_remap_InterruptEnable(XV_axi4s_remap *InstancePtr, u32 Mask);
void XV_axi4s_remap_InterruptDisable(XV_axi4s_remap *InstancePtr, u32 Mask);
void XV_axi4s_remap_InterruptClear(XV_axi4s_remap *InstancePtr, u32 Mask);
u32 XV_axi4s_remap_InterruptGetEnabled(XV_axi4s_remap *InstancePtr);
u32 XV_axi4s_remap_InterruptGetStatus(XV_axi4s_remap *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
