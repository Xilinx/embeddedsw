// ==============================================================
// Copyright (c) 1986 - 2023 Xilinx Inc. All rights reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_FRMBUFRD_H
#define XV_FRMBUFRD_H

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
#include "xv_frmbufrd_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

/**
* This typedef contains configuration information for the frame buffer read core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
#ifndef SDT
  u16 DeviceId;             /**< Unique ID of device */
#else
  char *Name;		    /**< Unique Name of device */
#endif
  UINTPTR BaseAddress;      /**< The base address of the core instance. */
  u16 PixPerClk;            /**< Samples Per Clock */
  u16 MaxWidth;             /**< Maximum columns supported by core instance */
  u16 MaxHeight;            /**< Maximum rows supported by core instance */
  u16 MaxDataWidth;         /**< Maximum Data width of each channel */
  u16 AXIMMDataWidth;       /**< AXI-MM data width */
  u16 AXIMMAddrWidth;       /**< AXI-MM address width */
  u16 RGBX8En;              /**< RGBX8      support */
  u16 YUVX8En;              /**< YUVX8      support */
  u16 YUYV8En;              /**< YUYV8      support */
  u16 RGBA8En;              /**< RGBA8      support */
  u16 YUVA8En;              /**< YUVA8      support */
  u16 RGBX10En;             /**< RGBX10     support */
  u16 YUVX10En;             /**< YUVX10     support */
  u16 Y_UV8En;              /**< Y_UV8      support */
  u16 Y_UV8_420En;          /**< Y_UV8_420  support */
  u16 RGB8En;               /**< RGB8       support */
  u16 YUV8En;               /**< YUV8       support */
  u16 Y_UV10En;             /**< Y_UV10     support */
  u16 Y_UV10_420En;         /**< Y_UV10_420 support */
  u16 Y8En;                 /**< Y8         support */
  u16 Y10En;                /**< Y10        support */
  u16 BGRA8En;              /**< BGRA8      support */
  u16 BGRX8En;              /**< BGRX8      support */
  u16 UYVY8En;              /**< UYVY8      support */
  u16 BGR8En;               /**< BGR8       support */
  u16 RGBX12En;             /**< RGBX12     support */
  u16 RGB16En;              /**< RGB16      support */
  u16 YUVX12En;             /**< YUVX12     support */
  u16 Y_UV12En;             /**< Y_UV12     support */
  u16 Y_UV12_420En;         /**< Y_UV12_420 support */
  u16 Y12En;                /**< Y12      support */
  u16 YUV16En;              /**< YUV16      support */
  u16 Y_UV16En;             /**< Y_UV16     support */
  u16 Y_UV16_420En;         /**< Y_UV16_420 support */
  u16 Y16En;                /**< Y16      support */
  u16 Y_U_V8En;             /**< Y_U_V8 support */
  u16 Y_U_V10En;            /**< Y_U_V10 support */
  u16 Y_U_V8_420En;         /**< Y_U_V8_420   support */
  u16 Y_U_V12En;            /**< Y_U_V12 support */
  u16 Interlaced;           /**< Interlaced support */
  u16 IsTileFormat;           /**< Interlaced support */
#ifdef SDT
  u16 IntrId; 		    /**< Interrupt ID */
  UINTPTR IntrParent; 	    /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_frmbufrd_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
  XV_frmbufrd_Config Config;    /**< Hardware Configuration */
  u32 IsReady;                  /**< Device is initialized and ready */
} XV_frmbufrd;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_frmbufrd_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_frmbufrd_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_frmbufrd_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_frmbufrd_ReadReg(BaseAddress, RegOffset) \
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
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, u16 DeviceId);
XV_frmbufrd_Config* XV_frmbufrd_LookupConfig(u16 DeviceId);
#else
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, UINTPTR BaseAddress);
XV_frmbufrd_Config* XV_frmbufrd_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_frmbufrd_CfgInitialize(XV_frmbufrd *InstancePtr,
                               XV_frmbufrd_Config *ConfigPtr,
                               UINTPTR EffectiveAddr);
#else
int XV_frmbufrd_Initialize(XV_frmbufrd *InstancePtr, const char* InstanceName);
int XV_frmbufrd_Release(XV_frmbufrd *InstancePtr);
#endif

void XV_frmbufrd_Start(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_IsDone(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_IsIdle(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_IsReady(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_EnableAutoRestart(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_DisableAutoRestart(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_SetFlushbit(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_Get_FlushDone(XV_frmbufrd *InstancePtr);

void XV_frmbufrd_Set_HwReg_width(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_width(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_height(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_height(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_stride(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_stride(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_video_format(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_video_format(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr, u64 Data);
u64 XV_frmbufrd_Get_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr, u64 Data);
u64 XV_frmbufrd_Get_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr, u64 Data);
u64 XV_frmbufrd_Get_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_field_id(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_field_id(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_Set_HwReg_fidOutMode(XV_frmbufrd *InstancePtr, u32 Data);
u32 XV_frmbufrd_Get_HwReg_fidOutMode(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_Get_HwReg_fid_error(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_Get_HwReg_field_out(XV_frmbufrd *InstancePtr);

void XV_frmbufrd_InterruptGlobalEnable(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_InterruptGlobalDisable(XV_frmbufrd *InstancePtr);
void XV_frmbufrd_InterruptEnable(XV_frmbufrd *InstancePtr, u32 Mask);
void XV_frmbufrd_InterruptDisable(XV_frmbufrd *InstancePtr, u32 Mask);
void XV_frmbufrd_InterruptClear(XV_frmbufrd *InstancePtr, u32 Mask);
u32 XV_frmbufrd_InterruptGetEnabled(XV_frmbufrd *InstancePtr);
u32 XV_frmbufrd_InterruptGetStatus(XV_frmbufrd *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
