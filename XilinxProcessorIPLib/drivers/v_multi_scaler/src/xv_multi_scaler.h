/*************************************************************************
 * Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XV_MULTI_SCALER_H
#define XV_MULTI_SCALER_H

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
#include "xv_multi_scaler_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
#define XV_MAX_OUTS 8
#define XV_MULTI_SCALER_CLEAR_BIT_MASK 0x0
#define XV_MULTI_SCALER_AP_START_BIT_MASK 0x1
#define XV_MULTI_SCALER_AP_DONE_OFFSET 1
#define XV_MULTI_SCALER_AP_DONE_BIT_MASK \
	(1 << XV_MULTI_SCALER_AP_DONE_OFFSET)
#define XV_MULTI_SCALER_AP_IDLE_OFFSET 2
#define XV_MULTI_SCALER_AP_IDLE_BIT_MASK \
	(1 << XV_MULTI_SCALER_AP_IDLE_OFFSET)
#define XV_MULTI_SCALER_AP_READY_OFFSET 3
#define XV_MULTI_SCALER_AP_READY_BIT_MASK \
	(1 << XV_MULTI_SCALER_AP_READY_OFFSET)
#define XV_MULTI_SCALER_AP_AUTO_RESTART_OFFSET 7
#define XV_MULTI_SCALER_AP_AUTO_RESTART_BIT_MASK \
	(1 << XV_MULTI_SCALER_AP_AUTO_RESTART_OFFSET)
#define XV_MULTI_SCALER_ISR_DONE_BIT_MASK 0x1
#define XV_MULTI_SCALER_ISR_READY_OFFSET 1
#define XV_MULTI_SCALER_ISR_READY_BIT_MASK \
	(1 << XV_MULTI_SCALER_ISR_READY_OFFSET)
#define XV_MULTI_SCALER_WIDTH_IN(x) XV_multi_scaler_Set_HwReg_WidthIn_##x
#define XV_MULTI_SCALER_WIDTH_OUT(x) XV_multi_scaler_Set_HwReg_WidthOut_##x
#define XV_MULTI_SCALER_HEIGHT_IN(x) XV_multi_scaler_Set_HwReg_HeightIn_##x
#define XV_MULTI_SCALER_HEIGHT_OUT(x) XV_multi_scaler_Set_HwReg_HeightOut_##x
#define XV_MULTI_SCALER_STRIDE_IN(x) XV_multi_scaler_Set_HwReg_InStride_##x
#define XV_MULTI_SCALER_STRIDE_OUT(x) XV_multi_scaler_Set_HwReg_OutStride_##x
#define XV_MULTI_SCALER_LINE_RATE(x) XV_multi_scaler_Set_HwReg_LineRate_##x
#define XV_MULTI_SCALER_PIXEL_RATE(x) XV_multi_scaler_Set_HwReg_PixelRate_##x
#define XV_MULTI_SCALER_IN_PIXEL_FMT(x) \
				XV_multi_scaler_Set_HwReg_InPixelFmt_##x
#define XV_MULTI_SCALER_OUT_PIXEL_FMT(x) \
				XV_multi_scaler_Set_HwReg_OutPixelFmt_##x
#define XV_MULTI_SCALER_SRC_IMG_BUF_0(x) \
				XV_multi_scaler_Set_HwReg_srcImgBuf0_##x##_V
#define XV_MULTI_SCALER_SRC_IMG_BUF_1(x) \
				XV_multi_scaler_Set_HwReg_srcImgBuf1_##x##_V
#define XV_MULTI_SCALER_DST_IMG_BUF_0(x) \
				XV_multi_scaler_Set_HwReg_dstImgBuf0_##x##_V
#define XV_MULTI_SCALER_DST_IMG_BUF_1(x) \
				XV_multi_scaler_Set_HwReg_dstImgBuf1_##x##_V
typedef struct {
    u16 DeviceId;
    u32 Ctrl_BaseAddress;
    u32 SamplesPerClock;
    u32 MaxDataWidth;
    u32 MaxCols;
    u32 MaxRows;
    u32 PhaseShift;
    u32 ScaleMode;
    u32 NumTaps;
    u32 MaxOuts;
} XV_multi_scaler_Config;
extern XV_multi_scaler_Config XV_multi_scaler_ConfigTable[];
#endif

typedef void (*XVMultiScaler_Callback)(void *CallbackRef);
typedef struct {
    u32 Ctrl_BaseAddress;
    u32 IsReady;
    u32 SamplesPerClock;
    u32 MaxDataWidth;
    u32 MaxRows;
    u32 MaxCols;
    u32 MaxOuts;
    u32 NumTaps;
    u32 PhaseShift;
    u32 ScaleMode;
    XVMultiScaler_Callback FrameDoneCallback;
    void *CallbackRef;
    u8 OutBitMask;
} XV_multi_scaler;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_multi_scaler_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_multi_scaler_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_multi_scaler_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_multi_scaler_ReadReg(BaseAddress, RegOffset) \
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
int XV_multi_scaler_Initialize(XV_multi_scaler *InstancePtr, u16 DeviceId);
XV_multi_scaler_Config* XV_multi_scaler_LookupConfig(u16 DeviceId);
int XV_multi_scaler_CfgInitialize(XV_multi_scaler *InstancePtr,
	XV_multi_scaler_Config *ConfigPtr);
#else
int XV_multi_scaler_Initialize(XV_multi_scaler *InstancePtr,
	const char* InstanceName);
int XV_multi_scaler_Release(XV_multi_scaler *InstancePtr);
#endif

void XV_multi_scaler_Start(XV_multi_scaler *InstancePtr);
u32 XV_multi_scaler_IsDone(XV_multi_scaler *InstancePtr);
u32 XV_multi_scaler_IsIdle(XV_multi_scaler *InstancePtr);
u32 XV_multi_scaler_IsReady(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_EnableAutoRestart(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_DisableAutoRestart(XV_multi_scaler *InstancePtr);

void XV_multi_scaler_Set_HwReg_num_outs(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_num_outs(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_0(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_0(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_1(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_1(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_2(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_2(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_3(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_3(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_4(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_4(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_5(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_5(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_6(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_6(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_LineRate_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_LineRate_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_InStride_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_InStride_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_OutStride_7(XV_multi_scaler *InstancePtr,
	u32 Data);
u32 XV_multi_scaler_Get_HwReg_OutStride_7(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_Set_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data);
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length);
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length);

void XV_multi_scaler_InterruptGlobalEnable(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_InterruptGlobalDisable(XV_multi_scaler *InstancePtr);
void XV_multi_scaler_InterruptEnable(XV_multi_scaler *InstancePtr,
	u32 Mask);
void XV_multi_scaler_InterruptDisable(XV_multi_scaler *InstancePtr,
	u32 Mask);
void XV_multi_scaler_InterruptClear(XV_multi_scaler *InstancePtr,
	u32 Mask);
u32 XV_multi_scaler_InterruptGetEnabled(XV_multi_scaler *InstancePtr);
u32 XV_multi_scaler_InterruptGetStatus(XV_multi_scaler *InstancePtr);
void * XV_MultiScalerIntrHandler(void *data);
void XVMultiScaler_SetCallback(XV_multi_scaler *InstancePtr,
	void *CallbackFunc, void *CallbackRef);

#ifdef __cplusplus
}
#endif

#endif
