/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_multi_scaler_l2.h
* @addtogroup v_multi_scaler Overview
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
#ifndef XV_MULTISCALER_L2_H 	 /* prevent circular inclusions by using protection macros  */
/* Define header guard macro to prevent multiple inclusion */
#define XV_MULTISCALER_L2_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xv_multi_scaler.h"

/************************** Constant Definitions *****************************/


/**
 * Maximum number of vertical taps supported by the scaler
 */
#define XV_MULTISCALER_MAX_V_TAPS 12

/**
 * Maximum number of vertical phases for polyphase filter
 */
#define XV_MULTISCALER_MAX_V_PHASES 64

/**
 * Bit mask for extracting output channel selection
 */
#define XV_MULTISCALER_OUTPUT_MASK 0xFF

/**
 * Maximum bytes per pixel (for RGBX/YUVX at 10-bit)
 */
#define XV_MAX_BYTES_PER_PIXEL 4

/**
 * Maximum buffer size calculation based on max dimensions and pixel format
 */
#define XV_MAX_BUF_SIZE XPAR_XV_MULTI_SCALER_0_MAX_COLS * \
		XPAR_XV_MULTI_SCALER_0_MAX_ROWS * \
		XV_MAX_BYTES_PER_PIXEL

/**
 * Hardware register offset for filter coefficient memory
 */
#define XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_FLTCOEFF_OFFSET 0x2000

/**
 * Fixed-point precision for scaling step calculation (2^16)
 */
#define STEP_PRECISION 65536

/**
 * Bit mask for extracting lower 16 bits
 */
#define XVSC_MASK_LOW_16BITS 0x0000FFFF

/**
 * Bit mask for extracting upper 16 bits
 */
#define XVSC_MASK_HIGH_16BITS 0xFFFF0000

/**************************** Type Definitions *******************************/
/**
 * Scaler type enumeration - defines the interpolation algorithm used
 */
typedef enum
{
	XV_MULTISCALER_BILINEAR = 0,  /**< Bilinear interpolation */
	XV_MULTISCALER_BICUBIC,       /**< Bicubic interpolation */
	XV_MULTISCALER_POLYPHASE      /**< Polyphase interpolation */
} XV_MULTISCALER_TYPE;

/**
 * Supported filter tap configurations for polyphase scaling
 */
typedef enum
{
	XV_MULTISCALER_TAPS_6  = 6,   /**< 6-tap filter */
	XV_MULTISCALER_TAPS_8  = 8,   /**< 8-tap filter */
	XV_MULTISCALER_TAPS_10 = 10,  /**< 10-tap filter */
	XV_MULTISCALER_TAPS_12 = 12   /**< 12-tap filter */
} XV_MULTISCALER_TAPS;

/**
 * Memory pixel format enumeration - defines supported color formats
 */
typedef enum
{
	XV_MULTI_SCALER_RGBX8  = 10,      /**< RGB with alpha, 8-bit per component */
	XV_MULTI_SCALER_YUVX8  = 11,      /**< YUV with alpha, 8-bit per component */
	XV_MULTI_SCALER_YUYV8 = 12,       /**< YUYV 4:2:2, 8-bit */
	XV_MULTI_SCALER_RGBX10 = 15,      /**< RGB with alpha, 10-bit per component */
	XV_MULTI_SCALER_YUVX10 = 16,      /**< YUV with alpha, 10-bit per component */
	XV_MULTI_SCALER_Y_UV8 = 18,       /**< Y/UV semi-planar 4:2:2, 8-bit */
	XV_MULTI_SCALER_Y_UV8_420 = 19,   /**< Y/UV semi-planar 4:2:0, 8-bit */
	XV_MULTI_SCALER_RGB8 = 20,        /**< RGB packed, 8-bit per component */
	XV_MULTI_SCALER_YUV8 = 21,        /**< YUV packed, 8-bit per component */
	XV_MULTI_SCALER_Y_UV10 = 22,      /**< Y/UV semi-planar 4:2:2, 10-bit */
	XV_MULTI_SCALER_Y_UV10_420 = 23,  /**< Y/UV semi-planar 4:2:0, 10-bit */
	XV_MULTI_SCALER_Y8 = 24,          /**< Y only (grayscale), 8-bit */
	XV_MULTI_SCALER_Y10 = 25,         /**< Y only (grayscale), 10-bit */
	XV_MULTI_SCALER_BGRX8 = 27,       /**< BGR with alpha, 8-bit per component */
	XV_MULTI_SCALER_UYVY8 = 28,       /**< UYVY 4:2:2, 8-bit */
	XV_MULTI_SCALER_BGR8 = 29,        /**< BGR packed, 8-bit per component */
} XV_MULTISCALER_MEMORY_FORMATS;

/**
 * Crop window configuration structure
 */
typedef struct {
	u32 Height;   /**< Height of the crop region in pixels */
	u32 Width;    /**< Width of the crop region in pixels */
	u32 StartX;   /**< Horizontal start position of crop window */
	u32 StartY;   /**< Vertical start position of crop window */
	u8 Crop;      /**< Enable/disable cropping (1=enable, 0=disable) */
} XV_multi_scaler_Crop_Window;

/**
 * Video channel configuration structure
 */
typedef struct {
	u32 ChannelId;                        /**< Output channel identifier (0-7) */
	UINTPTR SrcImgBuf0;                   /**< Source image buffer 0 address */
	UINTPTR SrcImgBuf1;                   /**< Source image buffer 1 address (for chroma) */
	u32 HeightIn;                         /**< Input image height in pixels */
	u32 HeightOut;                        /**< Output image height in pixels */
	u32 WidthIn;                          /**< Input image width in pixels */
	u32 WidthOut;                         /**< Output image width in pixels */
	u32 ColorFormatIn;                    /**< Input color format */
	u32 ColorFormatOut;                   /**< Output color format */
	u32 InStride;                         /**< Input buffer stride in bytes */
	u32 OutStride;                        /**< Output buffer stride in bytes */
	UINTPTR DstImgBuf0;                   /**< Destination image buffer 0 address */
	UINTPTR DstImgBuf1;                   /**< Destination image buffer 1 address (for chroma) */
	XV_multi_scaler_Crop_Window CropWin;  /**< Crop window configuration */
} XV_multi_scaler_Video_Config;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XV_MultiScalerStart(XV_multi_scaler *InstancePtr);
void XV_MultiScalerStop(XV_multi_scaler *InstancePtr);
u32 XV_MultiScalerGetNumOutputs(XV_multi_scaler *InstancePtr);
void XV_MultiScalerSetNumOutputs(XV_multi_scaler *InstancePtr, u32 NumOuts);
void XV_MultiScalerGetChannelConfig(XV_multi_scaler  *InstancePtr,
	XV_multi_scaler_Video_Config *multi_scaler_cfg);
void XV_MultiScalerSetChannelConfig(XV_multi_scaler  *InstancePtr,
	XV_multi_scaler_Video_Config *multi_scaler_cfg);

/************************** Variable Definitions *****************************/
/** Fixed coefficient tables for polyphase scaling filters */
extern const short XV_multiscaler_fixedcoeff_taps6_6C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_6];

/**
 * Fixed coefficient table for 6-tap filter with 12 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps6_12C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];

/**
 * Fixed coefficient table for 8-tap filter with 8 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps8_8C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_8];

/**
 * Fixed coefficient table for 8-tap filter with 12 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps8_12C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];

/**
 * Fixed coefficient table for 10-tap filter with 10 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps10_10C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_10];

/**
 * Fixed coefficient table for 10-tap filter with 12 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps10_12C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];

/**
 * Fixed coefficient table for 12-tap filter with 12 coefficients per phase
 */
extern const short XV_multiscaler_fixedcoeff_taps12_12C[XV_MULTISCALER_MAX_V_PHASES]
	[XV_MULTISCALER_TAPS_12];

#ifdef __cplusplus
}
#endif

#endif
/** @} */
