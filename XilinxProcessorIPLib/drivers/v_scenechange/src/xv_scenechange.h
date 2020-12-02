// ==============================================================
// Copyright (c) 1986-2020 Xilinx, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifndef XV_SCENECHANGE_H
#define XV_SCENECHANGE_H

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
#include "xv_scenechange_hw.h"
#include "sleep.h"

#define XV_SCD_LAYER_OFFSET		(0x100)
#define XV_SCD_IP_MAX_STREAMS		8
#define XV_SCD_WAIT_FOR_FLUSH_DONE	(25)
#define XV_SCD_WAIT_FOR_FLUSH_DELAY	(2000)
#define XV_SCD_IDLE_TIMEOUT		(1000000)
#define XV_SCD_MEMORY_MODE		1
#define XV_SCD_STREAM_MODE		0

/**************************** Type Definitions ******************************/

/* Please check Xilinx SceneChange PG*/
typedef enum {
	XV_SCD_HAS_Y8  = 24,
	XV_SCD_HAS_Y10 = 25
} XVScdClrFmt;

#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    UINTPTR Ctrl_BaseAddress;
    u8	MemoryBased;
    u8	NumStreams;
    u32	HistogramBits;
    u8  EnableY8;
    u8  EnableY10;
    u32 Cols;
    u32	Rows;
} XV_scenechange_Config;
#endif

/**
* Callback type for interrupt.
*
* @param    CallbackRef is a callback reference passed in by the upper
*           layer when setting the callback functions, and passed back to
*           the upper layer when the callback is invoked.
*
* @return   None.
*
* @note     None.
*
*/

typedef void (*XVSceneChange_Callback)(void *InstancePtr);
typedef struct {
    u64 BufferAddr;
    u32 SAD;
    u32 Threshold;
    u32 Width;
    u32 Height;
    u32 Stride;
    u32 SubSample;
    u8  LayerId;
    u8  StreamEnable;
    XVScdClrFmt VFormat;
} XVScdLayerConfig;
typedef struct {
    UINTPTR Ctrl_BaseAddress;
    u32 IsReady;
    u32 ScdDetLayerId;
    u32 ScdLayerDetSAD;
    void *CallbackRef;
    XV_scenechange_Config *ScdConfig;
    XVScdLayerConfig LayerConfig[XV_SCD_IP_MAX_STREAMS];
    XVSceneChange_Callback FrameDoneCallback;
} XV_scenechange;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_scenechange_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_scenechange_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_scenechange_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_scenechange_ReadReg(BaseAddress, RegOffset) \
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
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, u16 DeviceId);
XV_scenechange_Config* XV_scenechange_LookupConfig(u16 DeviceId);
int XV_scenechange_CfgInitialize(XV_scenechange *InstancePtr, XV_scenechange_Config *ConfigPtr);
#else
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, const char* InstanceName);
int XV_scenechange_Release(XV_scenechange *InstancePtr);
#endif

void XV_scenechange_Start(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsDone(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsIdle(XV_scenechange *InstancePtr);
u32 XV_scenechange_IsReady(XV_scenechange *InstancePtr);
void XV_scenechange_EnableAutoRestart(XV_scenechange *InstancePtr);
void XV_scenechange_DisableAutoRestart(XV_scenechange *InstancePtr);

void XV_scenechange_Set_HwReg_width_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_0(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_0(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_0(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad0(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad0_vld(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_1(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_1(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad1(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_2(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_2(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad2(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_3(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_3(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad3(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_4(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_4(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad4(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_5(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_5(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad5(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_6(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_6(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad6(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_width_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_width_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_height_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_height_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stride_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stride_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_video_format_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_video_format_7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_subsample_7(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_subsample_7(XV_scenechange *InstancePtr);
u32 XV_scenechange_Get_HwReg_sad7(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr);
void XV_scenechange_Set_HwReg_stream_enable(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Get_HwReg_stream_enable(XV_scenechange *InstancePtr);

void XV_scenechange_InterruptGlobalEnable(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptGlobalDisable(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptEnable(XV_scenechange *InstancePtr, u32 Mask);
void XV_scenechange_InterruptDisable(XV_scenechange *InstancePtr, u32 Mask);
void XV_scenechange_InterruptClear(XV_scenechange *InstancePtr, u32 Mask);
u32 XV_scenechange_InterruptGetEnabled(XV_scenechange *InstancePtr);
u32 XV_scenechange_InterruptGetStatus(XV_scenechange *InstancePtr);
void XV_scenechange_InterruptHandler(void *InstancePtr);
void XV_scenechange_EnableInterrupts(void *InstancePtr);
void XV_scenechange_SetCallback(XV_scenechange *InstancePtr, void *CallbackFunc,
			       void *CallbackRef);
int XV_scenechange_Layer_config(XV_scenechange *InstancePtr, u8 layerid);
void XV_scenechange_Layer_stream_enable(XV_scenechange *InstancePtr, u32 Data);
u32 XV_scenechange_Stop(XV_scenechange *InstancePtr);
u32 XV_scenechange_WaitForIdle(XV_scenechange *InstancePtr);
#ifdef __cplusplus
}
#endif

#endif
