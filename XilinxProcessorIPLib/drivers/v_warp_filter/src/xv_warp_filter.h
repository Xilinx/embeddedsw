// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_filter.h
 * @addtogroup v_warp_filter Overview
 */

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
/** 8-bit unsigned integer type for Linux */
typedef uint8_t u8;
/** 16-bit unsigned integer type for Linux */
typedef uint16_t u16;
/** 32-bit unsigned integer type for Linux */
typedef uint32_t u32;
/** 64-bit unsigned integer type for Linux */
typedef uint64_t u64;
#else
/**
 * @brief Warp Filter configuration structure.
 *
 * This structure contains the configuration information for the
 * Warp Filter IP instance.
 */
typedef struct {
#ifndef SDT
    u16 DeviceId;             /**< Unique ID of device */
#else
    char *Name;               /**< Device name string */
#endif
    UINTPTR Control_BaseAddress;  /**< Base address of control registers */
    u16 max_width;            /**< Maximum width supported */
    u16 max_height;           /**< Maximum height supported */
    u16 axi_mm_data_width;    /**< AXI MM port data width in bits */
    u16 perf_level;           /**< Performance level configuration */
    u16 bpc;                  /**< Maximum bits per component supported */
#ifdef SDT
    u16 IntrId;               /**< Interrupt ID */
    UINTPTR IntrParent;       /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XV_warp_filter_Config;
#endif

/** Callback function type for frame done event */
typedef void (*XV_warp_filter_Callback)(void *CallbackRef);

/**
 * @brief Warp Filter driver instance structure.
 *
 * This structure contains all the state and configuration information
 * for a Warp Filter driver instance.
 */
typedef struct {
	XV_warp_filter_Config *config;       /**< Pointer to device configuration */
	UINTPTR Control_BaseAddress;         /**< Base address of control registers */
    u32 IsReady;                         /**< Device initialization status flag */
	XV_warp_filter_Callback FrameDoneCallback;  /**< Frame done callback function pointer */
    void *CallbackRef;                   /**< Callback reference pointer */
	UINTPTR WarpFilterDesc_BaseAddr;     /**< Base address of descriptor chain */
	u32 NumDescriptors;                   /**< Number of descriptors in chain */
} XV_warp_filter;

/** Word type definition for register access */
typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
/** Macro to write a 32-bit value to a register */
#define XV_warp_filter_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
/** Macro to read a 32-bit value from a register */
#define XV_warp_filter_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
/** Macro to write a 32-bit value to a register (Linux) */
#define XV_warp_filter_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
/** Macro to read a 32-bit value from a register (Linux) */
#define XV_warp_filter_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

/** Assertion macro for void functions (Linux) */
#define Xil_AssertVoid(expr)    assert(expr)
/** Assertion macro for non-void functions (Linux) */
#define Xil_AssertNonvoid(expr) assert(expr)

/** Success return status */
#define XST_SUCCESS             0
/** Device not found error status */
#define XST_DEVICE_NOT_FOUND    2
/** Open device failed error status */
#define XST_OPEN_DEVICE_FAILED  3
/** Component is ready flag value */
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
s32 XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, u16 DeviceId);
XV_warp_filter_Config* XV_warp_filter_LookupConfig(u16 DeviceId);
#else
s32 XV_warp_filter_Initialize(XV_warp_filter *InstancePtr, UINTPTR BaseAddress);
XV_warp_filter_Config* XV_warp_filter_LookupConfig(UINTPTR BaseAddress);
#endif
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
