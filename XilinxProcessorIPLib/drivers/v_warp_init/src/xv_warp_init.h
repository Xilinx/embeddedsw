// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_init.h
 * @addtogroup v_warp_init Overview
 */

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
/**
 * Configuration structure for warp init IP core
 */
typedef struct {
#ifndef SDT
    u16 DeviceId;             /**< Unique ID of device */
#else
    char *Name;               /**< Device name string */
#endif
    UINTPTR Ctrl_BaseAddress; /**< Control interface base address */
    u16 max_width;            /**< Maximum supported width */
    u16 max_height;           /**< Maximum supported height */
    u8  warp_type;            /**< Supported warp type */
    u16 axi_mm_data_width;    /**< AXI MM port data width */
    u16 bpc;                  /**< Maximum bits per component supported */
    u16 max_control_pts;      /**< Maximum supported control points for arbitrary warp */
#ifdef SDT
    u16 IntrId;               /**< Interrupt ID */
    UINTPTR IntrParent;       /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_warp_init_Config;
#endif

/** Callback function pointer type */
typedef void (*XV_warp_init_Callback)(void *CallbackRef);

/**
 * Driver instance structure for warp init IP core
 */
typedef struct {
	XV_warp_init_Config *config;          /**< Pointer to configuration structure */
	UINTPTR Ctrl_BaseAddress;             /**< Control interface base address */
    u32 IsReady;                          /**< Device is initialized and ready */
    XV_warp_init_Callback FrameDoneCallback; /**< Frame done callback function */
    void *CallbackRef;                    /**< Callback reference pointer */
    UINTPTR RemapVectorDesc_BaseAddr;     /**< Remap vector descriptor base address */
    u32 NumDescriptors;                   /**< Number of descriptors configured */
} XV_warp_init;

/** Word type definition */
typedef u32 word_type;

/**
 * Warp type enumeration
 */
typedef enum {
	DISTORTION_LENS = 0,      /**< Lens distortion warp type */
	DISTORTION_ARBITRARY = 1  /**< Arbitrary distortion warp type */
}XV_warp_init_warp_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
/** Write a value to a register at the specified offset */
#define XV_warp_init_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/** Read a value from a register at the specified offset */
#define XV_warp_init_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
/** Write a value to a register at the specified offset (Linux version) */
#define XV_warp_init_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)

/** Read a value from a register at the specified offset (Linux version) */
#define XV_warp_init_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

/** Assert macro for void functions */
#define Xil_AssertVoid(expr)    assert(expr)

/** Assert macro for non-void functions */
#define Xil_AssertNonvoid(expr) assert(expr)

/** Success return status code */
#define XST_SUCCESS             0

/** Device not found error code */
#define XST_DEVICE_NOT_FOUND    2

/** Open device failed error code */
#define XST_OPEN_DEVICE_FAILED  3

/** Component is ready status flag */
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, u16 DeviceId);
XV_warp_init_Config* XV_warp_init_LookupConfig(u16 DeviceId);
#else
int XV_warp_init_Initialize(XV_warp_init *InstancePtr, UINTPTR BaseAddress);
XV_warp_init_Config* XV_warp_init_LookupConfig(UINTPTR BaseAddress);
#endif
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
