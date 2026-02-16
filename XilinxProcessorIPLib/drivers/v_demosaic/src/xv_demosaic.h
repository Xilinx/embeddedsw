// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_demosaic.h
 * @addtogroup v_demosaic Overview
 */

#ifndef XV_DEMOSAIC_H
#define XV_DEMOSAIC_H

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
#include "xv_demosaic_hw.h"

/************************** Constant Definitions *****************************/
#ifndef __linux__
/** Interrupt mask for frame processing done event */
#define XVDEMOSAIC_IRQ_DONE_MASK            (0x01)

/** Interrupt mask for frame ready event */
#define XVDEMOSAIC_IRQ_READY_MASK           (0x02)
#endif

/**************************** Type Definitions ********************************/
#ifdef __linux__
/** 8-bit unsigned integer type for Linux */
typedef uint8_t u8;

/** 16-bit unsigned integer type for Linux */
typedef uint16_t u16;

/** 32-bit unsigned integer type for Linux */
typedef uint32_t u32;
#else
/** Callback function type for Demosaic interrupt events */
typedef void (*XVDemosaic_Callback)(void *InstancePtr);

/** Interrupt handler types for Demosaic IP */
typedef enum {
  XVDEMOSAIC_HANDLER_DONE = 1,  /**< Handler for processing done event (ap_done) */
  XVDEMOSAIC_HANDLER_READY      /**< Handler for ready event (ap_ready) */
} XVDEMOSAIC_HandlerType;

/**
 * @brief Configuration structure for the Demosaic IP core.
 *
 * This structure contains configuration information for the Demosaic core.
 * Each core instance should have a configuration structure associated with it.
 */
typedef struct {
#ifndef SDT
  u16 DeviceId;             /**< Unique device ID of the Demosaic instance */
#else
  char *Name;               /**< Device name string for SDT builds */
#endif
  UINTPTR BaseAddress;      /**< Base address of the core instance registers */
  u16 PixPerClk;            /**< Number of pixels processed per clock cycle */
  u16 MaxWidth;             /**< Maximum frame width in pixels supported by hardware */
  u16 MaxHeight;            /**< Maximum frame height in lines supported by hardware */
  u16 MaxDataWidth;         /**< Maximum data width in bits for each color channel */
  u16 Algorithm;            /**< Demosaicing interpolation algorithm used */
#ifdef SDT
  u16 IntrId;               /**< Interrupt ID for SDT builds */
  UINTPTR IntrParent;       /**< Interrupt parent: Bit[0]=type, Bit[64/32:1]=base address */
#endif
} XV_demosaic_Config;
#endif

/**
 * @brief Driver instance structure for the Demosaic IP.
 *
 * This structure contains the driver instance data. An instance must be
 * allocated for each Demosaic core in use.
 */
typedef struct {
    XV_demosaic_Config Config;           /**< Hardware configuration parameters */
    u32 IsReady;                         /**< Device initialization status (XIL_COMPONENT_IS_READY when ready) */
    XVDemosaic_Callback FrameDoneCallback;  /**< Callback function for frame processing done event */
    void *CallbackDoneRef;               /**< User reference pointer passed to done callback */
    XVDemosaic_Callback FrameReadyCallback; /**< Callback function for frame ready event */
    void *CallbackReadyRef;              /**< User reference pointer passed to ready callback */
} XV_demosaic;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
/** Writes a value to a Demosaic IP register (bare-metal/RTOS) */
#define XV_demosaic_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/** Reads a value from a Demosaic IP register (bare-metal/RTOS) */
#define XV_demosaic_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
/** Writes a value to a Demosaic IP register (Linux) */
#define XV_demosaic_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)

/** Reads a value from a Demosaic IP register (Linux) */
#define XV_demosaic_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

/** Assertion macro for void functions (Linux) */
#define Xil_AssertVoid(expr)    assert(expr)

/** Assertion macro for non-void functions (Linux) */
#define Xil_AssertNonvoid(expr) assert(expr)

/** Return value indicating successful operation */
#define XST_SUCCESS             0

/** Return value indicating device not found */
#define XST_DEVICE_NOT_FOUND    2

/** Return value indicating device open failed */
#define XST_OPEN_DEVICE_FAILED  3

/** Component ready status flag */
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifndef SDT
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, u16 DeviceId);
XV_demosaic_Config* XV_demosaic_LookupConfig(u16 DeviceId);
#else
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, UINTPTR BaseAddress);
XV_demosaic_Config* XV_demosaic_LookupConfig(UINTPTR BaseAddress);
#endif
int XV_demosaic_CfgInitialize(XV_demosaic *InstancePtr,
                              XV_demosaic_Config *ConfigPtr,
                              UINTPTR EffectiveAddr);
#else
int XV_demosaic_Initialize(XV_demosaic *InstancePtr, const char* InstanceName);
int XV_demosaic_Release(XV_demosaic *InstancePtr);
#endif

void XV_demosaic_Start(XV_demosaic *InstancePtr);
u32 XV_demosaic_IsDone(XV_demosaic *InstancePtr);
u32 XV_demosaic_IsIdle(XV_demosaic *InstancePtr);
u32 XV_demosaic_IsReady(XV_demosaic *InstancePtr);
void XV_demosaic_EnableAutoRestart(XV_demosaic *InstancePtr);
void XV_demosaic_DisableAutoRestart(XV_demosaic *InstancePtr);

void XV_demosaic_Set_HwReg_width(XV_demosaic *InstancePtr, u32 Data);
u32 XV_demosaic_Get_HwReg_width(XV_demosaic *InstancePtr);
void XV_demosaic_Set_HwReg_height(XV_demosaic *InstancePtr, u32 Data);
u32 XV_demosaic_Get_HwReg_height(XV_demosaic *InstancePtr);
void XV_demosaic_Set_HwReg_bayer_phase(XV_demosaic *InstancePtr, u32 Data);
u32 XV_demosaic_Get_HwReg_bayer_phase(XV_demosaic *InstancePtr);

void XV_demosaic_InterruptGlobalEnable(XV_demosaic *InstancePtr);
void XV_demosaic_InterruptGlobalDisable(XV_demosaic *InstancePtr);
void XV_demosaic_InterruptEnable(XV_demosaic *InstancePtr, u32 Mask);
void XV_demosaic_InterruptDisable(XV_demosaic *InstancePtr, u32 Mask);
void XV_demosaic_InterruptClear(XV_demosaic *InstancePtr, u32 Mask);
u32 XV_demosaic_InterruptGetEnabled(XV_demosaic *InstancePtr);
u32 XV_demosaic_InterruptGetStatus(XV_demosaic *InstancePtr);

void XVDemosaic_SetCallback(XV_demosaic *InstancePtr, u32 HandlerType,
		void *CallbackFunc, void *CallbackRef);
void XVDemosaic_InterruptHandler(XV_demosaic *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
