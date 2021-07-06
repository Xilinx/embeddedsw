// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_GAMMA_LUT_H
#define XV_GAMMA_LUT_H

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
#include "xv_gamma_lut_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

typedef void (*XVGamma_Lut_Callback)(void *InstancePtr);

/************************** Constant Definitions *****************************/
#define XVGAMMA_LUT_IRQ_DONE_MASK            (0x01)
#define XVGAMMA_LUT_IRQ_READY_MASK           (0x02)

typedef enum {
  XVGAMMA_LUT_HANDLER_DONE = 1,  /**< Handler for ap_done */
  XVGAMMA_LUT_HANDLER_READY      /**< Handler for ap_ready */
} XVGAMMA_LUT_HandlerType;

/**
* This typedef contains configuration information for the gamma lut core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
    u16 DeviceId;          /**< Unique ID  of device */
    UINTPTR BaseAddress;   /**< The base address of the core instance. */
    u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
    u16 MaxWidth;          /**< Maximum columns supported by core instance */
    u16 MaxHeight;         /**< Maximum rows supported by core instance */
    u16 MaxDataWidth;      /**< Maximum Data width of each channel */
} XV_gamma_lut_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
    XV_gamma_lut_Config Config;  /**< Hardware Configuration */
    u32 IsReady;                 /**< Device is initialized and ready */
    XVGamma_Lut_Callback FrameDoneCallback;
    void *CallbackDoneRef;     /**< To be passed to the connect interrupt
                                callback */
    XVGamma_Lut_Callback FrameReadyCallback;
    void *CallbackReadyRef;     /**< To be passed to the connect interrupt
                                callback */
} XV_gamma_lut;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_gamma_lut_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_gamma_lut_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_gamma_lut_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_gamma_lut_ReadReg(BaseAddress, RegOffset) \
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
int XV_gamma_lut_Initialize(XV_gamma_lut *InstancePtr, u16 DeviceId);
XV_gamma_lut_Config* XV_gamma_lut_LookupConfig(u16 DeviceId);
int XV_gamma_lut_CfgInitialize(XV_gamma_lut *InstancePtr,
                               XV_gamma_lut_Config *ConfigPtr,
                               UINTPTR EffectiveAddr);
#else
int XV_gamma_lut_Initialize(XV_gamma_lut *InstancePtr, const char* InstanceName);
int XV_gamma_lut_Release(XV_gamma_lut *InstancePtr);
#endif

void XV_gamma_lut_Start(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_IsDone(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_IsIdle(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_IsReady(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_EnableAutoRestart(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_DisableAutoRestart(XV_gamma_lut *InstancePtr);

void XV_gamma_lut_Set_HwReg_width(XV_gamma_lut *InstancePtr, u32 Data);
u32 XV_gamma_lut_Get_HwReg_width(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_Set_HwReg_height(XV_gamma_lut *InstancePtr, u32 Data);
u32 XV_gamma_lut_Get_HwReg_height(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_Set_HwReg_video_format(XV_gamma_lut *InstancePtr, u32 Data);
u32 XV_gamma_lut_Get_HwReg_video_format(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_BaseAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_HighAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_TotalBytes(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_BitWidth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_Depth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_0_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_0_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_0_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_0_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_BaseAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_HighAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_TotalBytes(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_BitWidth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_Depth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_1_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_1_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_1_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_1_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_BaseAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_HighAddress(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_TotalBytes(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_BitWidth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_Depth(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_2_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_2_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length);
u32 XV_gamma_lut_Write_HwReg_gamma_lut_2_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);
u32 XV_gamma_lut_Read_HwReg_gamma_lut_2_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length);

void XV_gamma_lut_InterruptGlobalEnable(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_InterruptGlobalDisable(XV_gamma_lut *InstancePtr);
void XV_gamma_lut_InterruptEnable(XV_gamma_lut *InstancePtr, u32 Mask);
void XV_gamma_lut_InterruptDisable(XV_gamma_lut *InstancePtr, u32 Mask);
void XV_gamma_lut_InterruptClear(XV_gamma_lut *InstancePtr, u32 Mask);
u32 XV_gamma_lut_InterruptGetEnabled(XV_gamma_lut *InstancePtr);
u32 XV_gamma_lut_InterruptGetStatus(XV_gamma_lut *InstancePtr);

void XVGammaLut_SetCallback(XV_gamma_lut *InstancePtr, u32 HandlerType,
		void *CallbackFunc, void *CallbackRef);
void XVGammaLut_InterruptHandler(XV_gamma_lut *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
