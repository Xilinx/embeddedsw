/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_multi_scaler_l2.h
* @addtogroup v_multiscaler_v1_0
* @{
*
* This header file contains layer 2 API's of the multi scaler core
* driver.The functions contained herein provides a high level implementation of
* features provided by the IP, abstracting away the register level details from
* the user
*
* <b> Interrupts </b>
*
* The driver does the interrupt handling, and dispatch to the user application
* through callback functions that user has registered. If there are no
* registered callback functions, then a stub callback function is called.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b>Limitations</b>
*
******************************************************************************/
#ifndef XV_MULTISCALER_L2_H 	 /* prevent circular inclusions */
#define XV_MULTISCALER_L2_H		 /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_multi_scaler.h"

/************************** Constant Definitions *****************************/
#define XV_MULTISCALER_MAX_V_TAPS 12
#define XV_MULTISCALER_MAX_V_PHASES 64
#define XV_MULTISCALER_OUTPUT_MASK 0xFF
#define XV_MAX_BYTES_PER_PIXEL 4
#define XV_MAX_BUF_SIZE XPAR_V_MULTI_SCALER_0_MAX_COLS * \
		XPAR_V_MULTI_SCALER_0_MAX_ROWS * \
		XV_MAX_BYTES_PER_PIXEL
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_FLTCOEFF_OFFSET 0x2000
#define STEP_PRECISION 65536
#define XVSC_MASK_LOW_16BITS 0x0000FFFF
#define XVSC_MASK_HIGH_16BITS 0xFFFF0000

/**************************** Type Definitions *******************************/
/**
 * This typedef eumerates the Scaler Type
 */
typedef enum
{
	XV_MULTISCALER_BILINEAR = 0,
	XV_MULTISCALER_BICUBIC,
	XV_MULTISCALER_POLYPHASE
} XV_MULTISCALER_TYPE;

/**
 * This typedef enumerates the supported taps
 */
typedef enum
{
	XV_MULTISCALER_TAPS_6  = 6,
	XV_MULTISCALER_TAPS_8  = 8,
	XV_MULTISCALER_TAPS_10 = 10,
	XV_MULTISCALER_TAPS_12 = 12
} XV_MULTISCALER_TAPS;

typedef enum
{
	XV_MULTI_SCALER_RGBX8  = 10,
	XV_MULTI_SCALER_YUVX8  = 11,
	XV_MULTI_SCALER_YUYV8 = 12,
	XV_MULTI_SCALER_RGBX10 = 15,
	XV_MULTI_SCALER_YUVX10 = 16,
	XV_MULTI_SCALER_Y_UV8 = 18,
	XV_MULTI_SCALER_Y_UV8_420 = 19,
	XV_MULTI_SCALER_RGB8 = 20,
	XV_MULTI_SCALER_YUV8 = 21,
	XV_MULTI_SCALER_Y_UV10 = 22,
	XV_MULTI_SCALER_Y_UV10_420 = 23,
	XV_MULTI_SCALER_Y8 = 24,
	XV_MULTI_SCALER_Y10 = 25,
	XV_MULTI_SCALER_BGRX8 = 27,
	XV_MULTI_SCALER_UYVY8 = 28,
	XV_MULTI_SCALER_BGR8 = 29,
} XV_MULTISCALER_MEMORY_FORMATS;

typedef struct {
	u32 Height;
	u32 Width;
	u32 StartX;
	u32 StartY;
	u8 Crop;
} XV_multi_scaler_Crop_Window;

typedef struct {
	u32 ChannelId;
	UINTPTR SrcImgBuf0;
	UINTPTR SrcImgBuf1;
	u32 HeightIn;
	u32 HeightOut;
	u32 WidthIn;
	u32 WidthOut;
	u32 ColorFormatIn;
	u32 ColorFormatOut;
	u32 InStride;
	u32 OutStride;
	UINTPTR DstImgBuf0;
	UINTPTR DstImgBuf1;
	XV_multi_scaler_Crop_Window CropWin;
} XV_multi_scaler_Video_Config;

extern const short XV_multiscaler_fixedcoeff_taps6[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];
extern const short XV_multiscaler_fixedcoeff_taps8[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];
extern const short XV_multiscaler_fixedcoeff_taps10[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];
extern const short XV_multiscaler_fixedcoeff_taps12[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];

/************************** Function Prototypes ******************************/
void XV_MultiScalerStart(XV_multi_scaler *InstancePtr);
void XV_MultiScalerStop(XV_multi_scaler *InstancePtr);
u32 XV_MultiScalerGetNumOutputs(XV_multi_scaler *InstancePtr);
void XV_MultiScalerSetNumOutputs(XV_multi_scaler *InstancePtr, u32 NumOuts);
void XV_MultiScalerGetChannelConfig(XV_multi_scaler  *InstancePtr,
	XV_multi_scaler_Video_Config *multi_scaler_cfg);
void XV_MultiScalerSetChannelConfig(XV_multi_scaler  *InstancePtr,
	XV_multi_scaler_Video_Config *multi_scaler_cfg);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
