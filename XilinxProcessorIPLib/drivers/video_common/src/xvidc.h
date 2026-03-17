/*******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved
* Copyright 2023-2026 Advanced Micro Devices, Inc. All Rights Reserved..
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvidc.h
 * @addtogroup video_common Overview
 * @{
 * @details
 *
 * Contains common structures, definitions, macros, and utility functions that
 * are typically used by video-related drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   rc,  01/10/15 Initial release.
 *       als
 * 2.0   als  08/14/15 Added new video timings.
 * 2.2   als  02/01/16 Functions with pointer arguments that don't modify
 *                     contents now const.
 *                     Added ability to insert a custom video timing table:
 *                         XVidC_RegisterCustomTimingModes
 *                         XVidC_UnregisterCustomTimingMode
 *       yh            Added 3D support.
 * 3.0   aad  05/13/16 Added API to search for RB video modes.
 *       als  05/16/16 Added Y-only to color format enum.
 * 3.1   rco  07/26/17 Moved timing table extern definition to xvidc.c
 *                     Added video-in-memory color formats
 *                     Updated XVidC_RegisterCustomTimingModes API signature
 * 4.1   rco  11/23/17 Added new memory formats
 *                     Added xil_printf include statement
 *                     Added new API XVidC_GetVideoModeIdWBlanking
 *                     Fix C++ warnings
 * 4.2   jsr  07/22/17 Added new video modes, framerates, color formats for SDI
 *                     New member AspectRatio is added to video stream structure
 *                     Reordered XVidC_VideoMode enum variables and corrected the
 *                     memory format enums
 *       aad  07/10/17 Add XVIDC_VM_3840x2160_60_P_RB video format
 *       vyc  10/04/17 Added new streaming alpha formats and new memory formats
 *       aad  09/05/17 Add XVIDC_VM_1366x768_60_P_RB resolution
 * 4.3   eb   26/01/18 Added API XVidC_GetVideoModeIdExtensive
 *       jsr  02/22/18 Added XVIDC_CSF_YCBCR_420 color space format
 *       vyc  04/04/18 Added BGR8 memory format
 * 4.5   jsr  07/03/18 Added XVIDC_VM_720x486_60_I video format
 * 4.5   yas  03/08/19 Added support for frame rates 144HZ and 240HZ
 * 4.6   mmo  02/14/19 Added 5k, 8k, 10k and Low Resolution with 200Hz, 240Hz
 * 4.12  kp   15/07/21 Added new 3planar video formats and video timing modes
         kp   24/08/21 Added new video timing modes related to different VTotal
 * </pre>
 *
*******************************************************************************/

#ifndef XVIDC_H_  /* Prevent circular inclusions by using protection macros. */
#define XVIDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/

#include "xil_types.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/**
 * This typedef enumerates the list of available standard display monitor
 * timings as specified in the xvidc_timings_table.c file. The naming format is:
 *
 * XVIDC_VM_<RESOLUTION>_<REFRESH RATE (HZ)>_<P|I>(_RB)
 *
 * Where RB stands for reduced blanking.
 */
typedef enum {
	/* Interlaced modes. */
	XVIDC_VM_720x480_60_I = 0,  /**< 720x480 @ 60Hz interlaced */
	XVIDC_VM_720x486_60_I,  /**< 720x486 @ 60Hz interlaced */
	XVIDC_VM_720x576_50_I,  /**< 720x576 @ 50Hz interlaced */
	XVIDC_VM_1440x480_60_I,  /**< 1440x480 @ 60Hz interlaced */
	XVIDC_VM_1440x480_120_I,  /**< 1440x480 @ 120Hz interlaced */
	XVIDC_VM_1440x480_240_I,  /**< 1440x480 @ 240Hz interlaced */
	XVIDC_VM_1440x576_50_I,  /**< 1440x576 @ 50Hz interlaced */
	XVIDC_VM_1440x576_100_I,  /**< 1440x576 @ 100Hz interlaced */
	XVIDC_VM_1440x576_200_I,  /**< 1440x576 @ 200Hz interlaced */
	XVIDC_VM_1920x1080_48_I,  /**< 1920x1080 @ 48Hz interlaced */
	XVIDC_VM_1920x1080_50_I,  /**< 1920x1080 @ 50Hz interlaced */
	XVIDC_VM_1920x1080_50_I_VT1250,  /**< 1920x1080 @ 50Hz interlaced (VT=1250) */
	XVIDC_VM_1920x1080_60_I,  /**< 1920x1080 @ 60Hz interlaced */
	XVIDC_VM_1920x1080_96_I,  /**< 1920x1080 @ 96Hz interlaced */
	XVIDC_VM_1920x1080_100_I,  /**< 1920x1080 @ 100Hz interlaced */
	XVIDC_VM_1920x1080_120_I,  /**< 1920x1080 @ 120Hz interlaced */
	XVIDC_VM_2048x1080_48_I,  /**< 2048x1080 @ 48Hz interlaced */
	XVIDC_VM_2048x1080_50_I,  /**< 2048x1080 @ 50Hz interlaced */
	XVIDC_VM_2048x1080_60_I,  /**< 2048x1080 @ 60Hz interlaced */
	XVIDC_VM_2048x1080_96_I,  /**< 2048x1080 @ 96Hz interlaced */
	XVIDC_VM_2048x1080_100_I,  /**< 2048x1080 @ 100Hz interlaced */
	XVIDC_VM_2048x1080_120_I,  /**< 2048x1080 @ 120Hz interlaced */
	XVIDC_VM_2880x480_60_I,  /**< 2880x480 @ 60Hz interlaced */
	XVIDC_VM_2880x576_50_I,  /**< 2880x576 @ 50Hz interlaced */


	/* Progressive modes. */
	XVIDC_VM_640x350_85_P,  /**< 640x350 @ 85Hz progressive */
	XVIDC_VM_640x480_60_P,  /**< 640x480 @ 60Hz progressive */
	XVIDC_VM_640x480_72_P,  /**< 640x480 @ 72Hz progressive */
	XVIDC_VM_640x480_75_P,  /**< 640x480 @ 75Hz progressive */
	XVIDC_VM_640x480_85_P,  /**< 640x480 @ 85Hz progressive */
	XVIDC_VM_720x400_85_P,  /**< 720x400 @ 85Hz progressive */
	XVIDC_VM_720x480_60_P,  /**< 720x480 @ 60Hz progressive */
	XVIDC_VM_720x480_120_P,  /**< 720x480 @ 120Hz progressive */
	XVIDC_VM_720x480_240_P,  /**< 720x480 @ 240Hz progressive */
	XVIDC_VM_720x576_50_P,  /**< 720x576 @ 50Hz progressive */
	XVIDC_VM_720x576_100_P,  /**< 720x576 @ 100Hz progressive */
	XVIDC_VM_720x576_200_P,  /**< 720x576 @ 200Hz progressive */
	XVIDC_VM_800x600_56_P,  /**< 800x600 @ 56Hz progressive */
	XVIDC_VM_800x600_60_P,  /**< 800x600 @ 60Hz progressive */
	XVIDC_VM_800x600_72_P,  /**< 800x600 @ 72Hz progressive */
	XVIDC_VM_800x600_75_P,  /**< 800x600 @ 75Hz progressive */
	XVIDC_VM_800x600_85_P,  /**< 800x600 @ 85Hz progressive */
	XVIDC_VM_800x600_120_P_RB,  /**< 800x600 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_848x480_60_P,  /**< 848x480 @ 60Hz progressive */
	XVIDC_VM_1024x768_60_P,  /**< 1024x768 @ 60Hz progressive */
	XVIDC_VM_1024x768_70_P,  /**< 1024x768 @ 70Hz progressive */
	XVIDC_VM_1024x768_75_P,  /**< 1024x768 @ 75Hz progressive */
	XVIDC_VM_1024x768_85_P,  /**< 1024x768 @ 85Hz progressive */
	XVIDC_VM_1024x768_120_P_RB,  /**< 1024x768 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1152x864_75_P,  /**< 1152x864 @ 75Hz progressive */
	XVIDC_VM_1280x720_24_P,  /**< 1280x720 @ 24Hz progressive */
	XVIDC_VM_1280x720_25_P,  /**< 1280x720 @ 25Hz progressive */
	XVIDC_VM_1280x720_30_P,  /**< 1280x720 @ 30Hz progressive */
	XVIDC_VM_1280x720_48_P,  /**< 1280x720 @ 48Hz progressive */
	XVIDC_VM_1280x720_50_P,  /**< 1280x720 @ 50Hz progressive */
	XVIDC_VM_1280x720_60_P,  /**< 1280x720 @ 60Hz progressive */
	XVIDC_VM_1280x720_100_P,  /**< 1280x720 @ 100Hz progressive */
	XVIDC_VM_1280x720_120_P,  /**< 1280x720 @ 120Hz progressive */
	XVIDC_VM_1280x768_60_P,  /**< 1280x768 @ 60Hz progressive */
	XVIDC_VM_1280x768_60_P_RB,  /**< 1280x768 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1280x768_75_P,  /**< 1280x768 @ 75Hz progressive */
	XVIDC_VM_1280x768_85_P,  /**< 1280x768 @ 85Hz progressive */
	XVIDC_VM_1280x768_120_P_RB,  /**< 1280x768 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1280x800_60_P,  /**< 1280x800 @ 60Hz progressive */
	XVIDC_VM_1280x800_60_P_RB,  /**< 1280x800 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1280x800_75_P,  /**< 1280x800 @ 75Hz progressive */
	XVIDC_VM_1280x800_85_P,  /**< 1280x800 @ 85Hz progressive */
	XVIDC_VM_1280x800_120_P_RB,  /**< 1280x800 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1280x960_60_P,  /**< 1280x960 @ 60Hz progressive */
	XVIDC_VM_1280x960_85_P,  /**< 1280x960 @ 85Hz progressive */
	XVIDC_VM_1280x960_120_P_RB,  /**< 1280x960 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1280x1024_60_P,  /**< 1280x1024 @ 60Hz progressive */
	XVIDC_VM_1280x1024_75_P,  /**< 1280x1024 @ 75Hz progressive */
	XVIDC_VM_1280x1024_85_P,  /**< 1280x1024 @ 85Hz progressive */
	XVIDC_VM_1280x1024_120_P_RB,  /**< 1280x1024 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1360x768_60_P,  /**< 1360x768 @ 60Hz progressive */
	XVIDC_VM_1360x768_120_P_RB,  /**< 1360x768 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1366x768_60_P,  /**< 1366x768 @ 60Hz progressive */
	XVIDC_VM_1366x768_60_P_RB,  /**< 1366x768 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1400x1050_60_P,  /**< 1400x1050 @ 60Hz progressive */
	XVIDC_VM_1400x1050_60_P_RB,  /**< 1400x1050 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1400x1050_75_P,  /**< 1400x1050 @ 75Hz progressive */
	XVIDC_VM_1400x1050_85_P,  /**< 1400x1050 @ 85Hz progressive */
	XVIDC_VM_1400x1050_120_P_RB,  /**< 1400x1050 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1440x240_60_P,  /**< 1440x240 @ 60Hz progressive */
	XVIDC_VM_1440x240_60_P_VT263,  /**< 1440x240 @ 60Hz progressive (VT=263) */
	XVIDC_VM_1440x288_50_P,  /**< 1440x288 @ 50Hz progressive */
	XVIDC_VM_1440x288_50_P_VT314,  /**< 1440x288 @ 50Hz progressive (VT=314) */
	XVIDC_VM_1440x288_50_P_VT312,  /**< 1440x288 @ 50Hz progressive (VT=312) */
	XVIDC_VM_1440x480_60_P,  /**< 1440x480 @ 60Hz progressive */
	XVIDC_VM_1440x576_50_P,  /**< 1440x576 @ 50Hz progressive */
	XVIDC_VM_1440x900_60_P,  /**< 1440x900 @ 60Hz progressive */
	XVIDC_VM_1440x900_60_P_RB,  /**< 1440x900 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1440x900_75_P,  /**< 1440x900 @ 75Hz progressive */
	XVIDC_VM_1440x900_85_P,  /**< 1440x900 @ 85Hz progressive */
	XVIDC_VM_1440x900_120_P_RB,  /**< 1440x900 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1600x1200_60_P,  /**< 1600x1200 @ 60Hz progressive */
	XVIDC_VM_1600x1200_65_P,  /**< 1600x1200 @ 65Hz progressive */
	XVIDC_VM_1600x1200_70_P,  /**< 1600x1200 @ 70Hz progressive */
	XVIDC_VM_1600x1200_75_P,  /**< 1600x1200 @ 75Hz progressive */
	XVIDC_VM_1600x1200_85_P,  /**< 1600x1200 @ 85Hz progressive */
	XVIDC_VM_1600x1200_120_P_RB,  /**< 1600x1200 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1680x720_24_P,  /**< 1680x720 @ 24Hz progressive */
	XVIDC_VM_1680x720_25_P,  /**< 1680x720 @ 25Hz progressive */
	XVIDC_VM_1680x720_30_P,  /**< 1680x720 @ 30Hz progressive */
	XVIDC_VM_1680x720_48_P,  /**< 1680x720 @ 48Hz progressive */
	XVIDC_VM_1680x720_50_P,  /**< 1680x720 @ 50Hz progressive */
	XVIDC_VM_1680x720_60_P,  /**< 1680x720 @ 60Hz progressive */
	XVIDC_VM_1680x720_100_P,  /**< 1680x720 @ 100Hz progressive */
	XVIDC_VM_1680x720_120_P,  /**< 1680x720 @ 120Hz progressive */
	XVIDC_VM_1680x1050_50_P,  /**< 1680x1050 @ 50Hz progressive */
	XVIDC_VM_1680x1050_60_P,  /**< 1680x1050 @ 60Hz progressive */
	XVIDC_VM_1680x1050_60_P_RB,  /**< 1680x1050 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1680x1050_75_P,  /**< 1680x1050 @ 75Hz progressive */
	XVIDC_VM_1680x1050_85_P,  /**< 1680x1050 @ 85Hz progressive */
	XVIDC_VM_1680x1050_120_P_RB,  /**< 1680x1050 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1792x1344_60_P,  /**< 1792x1344 @ 60Hz progressive */
	XVIDC_VM_1792x1344_75_P,  /**< 1792x1344 @ 75Hz progressive */
	XVIDC_VM_1792x1344_120_P_RB,  /**< 1792x1344 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1856x1392_60_P,  /**< 1856x1392 @ 60Hz progressive */
	XVIDC_VM_1856x1392_75_P,  /**< 1856x1392 @ 75Hz progressive */
	XVIDC_VM_1856x1392_120_P_RB,  /**< 1856x1392 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1920x1080_24_P,  /**< 1920x1080 @ 24Hz progressive */
	XVIDC_VM_1920x1080_25_P,  /**< 1920x1080 @ 25Hz progressive */
	XVIDC_VM_1920x1080_30_P,  /**< 1920x1080 @ 30Hz progressive */
	XVIDC_VM_1920x1080_48_P,  /**< 1920x1080 @ 48Hz progressive */
	XVIDC_VM_1920x1080_50_P,  /**< 1920x1080 @ 50Hz progressive */
	XVIDC_VM_1920x1080_60_P,  /**< 1920x1080 @ 60Hz progressive */
	XVIDC_VM_1920x1080_100_P,  /**< 1920x1080 @ 100Hz progressive */
	XVIDC_VM_1920x1080_120_P,  /**< 1920x1080 @ 120Hz progressive */
	XVIDC_VM_1920x1200_60_P,  /**< 1920x1200 @ 60Hz progressive */
	XVIDC_VM_1920x1200_60_P_RB,  /**< 1920x1200 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_1920x1200_75_P,  /**< 1920x1200 @ 75Hz progressive */
	XVIDC_VM_1920x1200_85_P,  /**< 1920x1200 @ 85Hz progressive */
	XVIDC_VM_1920x1200_120_P_RB,  /**< 1920x1200 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1920x1440_60_P,  /**< 1920x1440 @ 60Hz progressive */
	XVIDC_VM_1920x1440_75_P,  /**< 1920x1440 @ 75Hz progressive */
	XVIDC_VM_1920x1440_120_P_RB,  /**< 1920x1440 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_1920x2160_60_P,  /**< 1920x2160 @ 60Hz progressive */
	XVIDC_VM_2048x1080_24_P,  /**< 2048x1080 @ 24Hz progressive */
	XVIDC_VM_2048x1080_25_P,  /**< 2048x1080 @ 25Hz progressive */
	XVIDC_VM_2048x1080_30_P,  /**< 2048x1080 @ 30Hz progressive */
	XVIDC_VM_2048x1080_48_P,  /**< 2048x1080 @ 48Hz progressive */
	XVIDC_VM_2048x1080_50_P,  /**< 2048x1080 @ 50Hz progressive */
	XVIDC_VM_2048x1080_60_P,  /**< 2048x1080 @ 60Hz progressive */
	XVIDC_VM_2048x1080_100_P,  /**< 2048x1080 @ 100Hz progressive */
	XVIDC_VM_2048x1080_120_P,  /**< 2048x1080 @ 120Hz progressive */
	XVIDC_VM_2560x1080_24_P,  /**< 2560x1080 @ 24Hz progressive */
	XVIDC_VM_2560x1080_25_P,  /**< 2560x1080 @ 25Hz progressive */
	XVIDC_VM_2560x1080_30_P,  /**< 2560x1080 @ 30Hz progressive */
	XVIDC_VM_2560x1080_48_P,  /**< 2560x1080 @ 48Hz progressive */
	XVIDC_VM_2560x1080_50_P,  /**< 2560x1080 @ 50Hz progressive */
	XVIDC_VM_2560x1080_60_P,  /**< 2560x1080 @ 60Hz progressive */
	XVIDC_VM_2560x1080_100_P,  /**< 2560x1080 @ 100Hz progressive */
	XVIDC_VM_2560x1080_120_P,  /**< 2560x1080 @ 120Hz progressive */
	XVIDC_VM_2560x1600_60_P,  /**< 2560x1600 @ 60Hz progressive */
	XVIDC_VM_2560x1600_60_P_RB,  /**< 2560x1600 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_2560x1600_75_P,  /**< 2560x1600 @ 75Hz progressive */
	XVIDC_VM_2560x1600_85_P,  /**< 2560x1600 @ 85Hz progressive */
	XVIDC_VM_2560x1600_120_P_RB,  /**< 2560x1600 @ 120Hz progressive (reduced blanking) */
	XVIDC_VM_2880x240_60_P,  /**< 2880x240 @ 60Hz progressive */
	XVIDC_VM_2880x240_60_P_VT263,  /**< 2880x240 @ 60Hz progressive (VT=263) */
	XVIDC_VM_2880x288_50_P,  /**< 2880x288 @ 50Hz progressive */
	XVIDC_VM_2880x288_50_P_VT314,  /**< 2880x288 @ 50Hz progressive (VT=314) */
	XVIDC_VM_2880x288_50_P_VT312,  /**< 2880x288 @ 50Hz progressive (VT=312) */
	XVIDC_VM_2880x480_60_P,  /**< 2880x480 @ 60Hz progressive */
	XVIDC_VM_2880x576_50_P,  /**< 2880x576 @ 50Hz progressive */
	XVIDC_VM_3840x2160_24_P,  /**< 3840x2160 @ 24Hz progressive */
	XVIDC_VM_3840x2160_25_P,  /**< 3840x2160 @ 25Hz progressive */
	XVIDC_VM_3840x2160_30_P,  /**< 3840x2160 @ 30Hz progressive */
	XVIDC_VM_3840x2160_48_P,  /**< 3840x2160 @ 48Hz progressive */
	XVIDC_VM_3840x2160_50_P,  /**< 3840x2160 @ 50Hz progressive */
	XVIDC_VM_3840x2160_60_P,  /**< 3840x2160 @ 60Hz progressive */
	XVIDC_VM_3840x2160_60_P_RB,  /**< 3840x2160 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_3840x2160_100_P,  /**< 3840x2160 @ 100Hz progressive */
	XVIDC_VM_3840x2160_120_P,  /**< 3840x2160 @ 120Hz progressive */
	XVIDC_VM_4096x2160_24_P,  /**< 4096x2160 @ 24Hz progressive */
	XVIDC_VM_4096x2160_25_P,  /**< 4096x2160 @ 25Hz progressive */
	XVIDC_VM_4096x2160_30_P,  /**< 4096x2160 @ 30Hz progressive */
	XVIDC_VM_4096x2160_48_P,  /**< 4096x2160 @ 48Hz progressive */
	XVIDC_VM_4096x2160_50_P,  /**< 4096x2160 @ 50Hz progressive */
	XVIDC_VM_4096x2160_60_P,  /**< 4096x2160 @ 60Hz progressive */
	XVIDC_VM_4096x2160_60_P_RB,  /**< 4096x2160 @ 60Hz progressive (reduced blanking) */
	XVIDC_VM_4096x2160_100_P,  /**< 4096x2160 @ 100Hz progressive */
	XVIDC_VM_4096x2160_120_P,  /**< 4096x2160 @ 120Hz progressive */
	XVIDC_VM_5120x2160_24_P,  /**< 5120x2160 @ 24Hz progressive */
	XVIDC_VM_5120x2160_25_P,  /**< 5120x2160 @ 25Hz progressive */
	XVIDC_VM_5120x2160_30_P,  /**< 5120x2160 @ 30Hz progressive */
	XVIDC_VM_5120x2160_48_P,  /**< 5120x2160 @ 48Hz progressive */
	XVIDC_VM_5120x2160_50_P,  /**< 5120x2160 @ 50Hz progressive */
	XVIDC_VM_5120x2160_60_P,  /**< 5120x2160 @ 60Hz progressive */
	XVIDC_VM_5120x2160_100_P,  /**< 5120x2160 @ 100Hz progressive */
	XVIDC_VM_5120x2160_120_P,  /**< 5120x2160 @ 120Hz progressive */
	XVIDC_VM_7680x4320_24_P,  /**< 7680x4320 @ 24Hz progressive */
	XVIDC_VM_7680x4320_25_P,  /**< 7680x4320 @ 25Hz progressive */
	XVIDC_VM_7680x4320_30_P,  /**< 7680x4320 @ 30Hz progressive */
	XVIDC_VM_7680x4320_48_P,  /**< 7680x4320 @ 48Hz progressive */
	XVIDC_VM_7680x4320_50_P,  /**< 7680x4320 @ 50Hz progressive */
	XVIDC_VM_7680x4320_60_P,  /**< 7680x4320 @ 60Hz progressive */
	XVIDC_VM_7680x4320_120_P,  /**< 7680x4320 @ 120Hz progressive */
	XVIDC_VM_7680x4320_100_P,  /**< 7680x4320 @ 100Hz progressive */
	XVIDC_VM_10240x4320_24_P,  /**< 10240x4320 @ 24Hz progressive */
	XVIDC_VM_10240x4320_25_P,  /**< 10240x4320 @ 25Hz progressive */
	XVIDC_VM_10240x4320_30_P,  /**< 10240x4320 @ 30Hz progressive */
	XVIDC_VM_10240x4320_48_P,  /**< 10240x4320 @ 48Hz progressive */
	XVIDC_VM_10240x4320_50_P,  /**< 10240x4320 @ 50Hz progressive */
	XVIDC_VM_10240x4320_60_P,  /**< 10240x4320 @ 60Hz progressive */
	XVIDC_VM_10240x4320_100_P,  /**< 10240x4320 @ 100Hz progressive */
	XVIDC_VM_10240x4320_120_P,  /**< 10240x4320 @ 120Hz progressive */

	XVIDC_VM_NUM_SUPPORTED,  /**< Number of supported video modes */
	XVIDC_VM_USE_EDID_PREFERRED,  /**< Use EDID preferred mode */
	XVIDC_VM_NO_INPUT,  /**< No input detected */
	XVIDC_VM_NOT_SUPPORTED,  /**< Video mode not supported */
	XVIDC_VM_CUSTOM,  /**< Custom video mode */

	/* Marks beginning/end of interlaced/progressive modes in the table. */
	XVIDC_VM_INTL_START = XVIDC_VM_720x480_60_I,  /**< First interlaced mode in table */
	XVIDC_VM_PROG_START = XVIDC_VM_640x350_85_P,  /**< First progressive mode in table */
	XVIDC_VM_INTL_END = (XVIDC_VM_PROG_START - 1),  /**< Last interlaced mode in table */
	XVIDC_VM_PROG_END = (XVIDC_VM_NUM_SUPPORTED - 1),  /**< Last progressive mode in table */

	/* Common naming. */
	XVIDC_VM_480_60_I = XVIDC_VM_720x480_60_I,  /**< 480i alias */
	XVIDC_VM_486_60_I = XVIDC_VM_720x486_60_I,  /**< 486i alias */
	XVIDC_VM_576_50_I = XVIDC_VM_720x576_50_I,  /**< 576i alias */
	XVIDC_VM_1080_50_I = XVIDC_VM_1920x1080_50_I,  /**< 1080i 50Hz alias */
	XVIDC_VM_1080_60_I = XVIDC_VM_1920x1080_60_I,  /**< 1080i 60Hz alias */
	XVIDC_VM_VGA_60_P = XVIDC_VM_640x480_60_P,  /**< VGA alias */
	XVIDC_VM_480_60_P = XVIDC_VM_720x480_60_P,  /**< 480p alias */
	XVIDC_VM_SVGA_60_P = XVIDC_VM_800x600_60_P,  /**< SVGA alias */
	XVIDC_VM_XGA_60_P = XVIDC_VM_1024x768_60_P,  /**< XGA alias */
	XVIDC_VM_720_50_P = XVIDC_VM_1280x720_50_P,  /**< 720p 50Hz alias */
	XVIDC_VM_720_60_P = XVIDC_VM_1280x720_60_P,  /**< 720p 60Hz alias */
	XVIDC_VM_WXGA_60_P = XVIDC_VM_1366x768_60_P,  /**< WXGA alias */
	XVIDC_VM_UXGA_60_P = XVIDC_VM_1600x1200_60_P,  /**< UXGA alias */
	XVIDC_VM_WSXGA_60_P = XVIDC_VM_1680x1050_60_P,  /**< WSXGA alias */
	XVIDC_VM_1080_24_P = XVIDC_VM_1920x1080_24_P,  /**< 1080p 24Hz alias */
	XVIDC_VM_1080_25_P = XVIDC_VM_1920x1080_25_P,  /**< 1080p 25Hz alias */
	XVIDC_VM_1080_30_P = XVIDC_VM_1920x1080_30_P,  /**< 1080p 30Hz alias */
	XVIDC_VM_1080_50_P = XVIDC_VM_1920x1080_50_P,  /**< 1080p 50Hz alias */
	XVIDC_VM_1080_60_P = XVIDC_VM_1920x1080_60_P,  /**< 1080p 60Hz alias */
	XVIDC_VM_WUXGA_60_P = XVIDC_VM_1920x1200_60_P,  /**< WUXGA alias */
	XVIDC_VM_UHD2_60_P = XVIDC_VM_1920x2160_60_P,  /**< UHD2 alias */
	XVIDC_VM_UHD_24_P = XVIDC_VM_3840x2160_24_P,  /**< UHD 24Hz alias */
	XVIDC_VM_UHD_25_P = XVIDC_VM_3840x2160_25_P,  /**< UHD 25Hz alias */
	XVIDC_VM_UHD_30_P = XVIDC_VM_3840x2160_30_P,  /**< UHD 30Hz alias */
	XVIDC_VM_UHD_60_P = XVIDC_VM_3840x2160_60_P,  /**< UHD 60Hz alias */
	XVIDC_VM_4K2K_60_P = XVIDC_VM_4096x2160_60_P,  /**< 4K2K 60Hz alias */
	XVIDC_VM_4K2K_60_P_RB = XVIDC_VM_4096x2160_60_P_RB,  /**< 4K2K 60Hz RB alias */
} XVidC_VideoMode;

/**
 * Progressive/interlaced video format.
 */
typedef enum {
	XVIDC_VF_PROGRESSIVE = 0,  /**< Progressive scan */
	XVIDC_VF_INTERLACED,       /**< Interlaced scan */
	XVIDC_VF_UNKNOWN           /**< Unknown video format */
} XVidC_VideoFormat;

/**
 * Frame rate.
 */
typedef enum {
	XVIDC_FR_24HZ = 24,           /**< 24 Hz refresh rate */
	XVIDC_FR_25HZ = 25,           /**< 25 Hz refresh rate */
	XVIDC_FR_30HZ = 30,           /**< 30 Hz refresh rate */
	XVIDC_FR_48HZ = 48,           /**< 48 Hz refresh rate */
	XVIDC_FR_50HZ = 50,           /**< 50 Hz refresh rate */
	XVIDC_FR_56HZ = 56,           /**< 56 Hz refresh rate */
	XVIDC_FR_60HZ = 60,           /**< 60 Hz refresh rate */
	XVIDC_FR_65HZ = 65,           /**< 65 Hz refresh rate */
	XVIDC_FR_67HZ = 67,           /**< 67 Hz refresh rate */
	XVIDC_FR_70HZ = 70,           /**< 70 Hz refresh rate */
	XVIDC_FR_72HZ = 72,           /**< 72 Hz refresh rate */
	XVIDC_FR_75HZ = 75,           /**< 75 Hz refresh rate */
	XVIDC_FR_85HZ = 85,           /**< 85 Hz refresh rate */
	XVIDC_FR_87HZ = 87,           /**< 87 Hz refresh rate */
	XVIDC_FR_88HZ = 88,           /**< 88 Hz refresh rate */
	XVIDC_FR_96HZ = 96,           /**< 96 Hz refresh rate */
	XVIDC_FR_100HZ = 100,         /**< 100 Hz refresh rate */
	XVIDC_FR_120HZ = 120,         /**< 120 Hz refresh rate */
	XVIDC_FR_144HZ = 144,         /**< 144 Hz refresh rate */
	XVIDC_FR_200HZ = 200,         /**< 200 Hz refresh rate */
	XVIDC_FR_240HZ = 240,         /**< 240 Hz refresh rate */
	XVIDC_FR_NUM_SUPPORTED = 21,  /**< Number of supported frame rates */
	XVIDC_FR_UNKNOWN              /**< Unknown frame rate */
} XVidC_FrameRate;

/**
 * Color depth - bits per color component.
 */
typedef enum {
	XVIDC_BPC_6 = 6,              /**< 6 bits per component */
	XVIDC_BPC_8 = 8,              /**< 8 bits per component */
	XVIDC_BPC_10 = 10,            /**< 10 bits per component */
	XVIDC_BPC_12 = 12,            /**< 12 bits per component */
	XVIDC_BPC_14 = 14,            /**< 14 bits per component */
	XVIDC_BPC_16 = 16,            /**< 16 bits per component */
	XVIDC_BPC_NUM_SUPPORTED = 6,  /**< Number of supported color depths */
	XVIDC_BPC_UNKNOWN             /**< Unknown color depth */
} XVidC_ColorDepth;

/**
 * Pixels per clock.
 */
typedef enum {
	XVIDC_PPC_1 = 1,              /**< 1 pixel per clock */
	XVIDC_PPC_2 = 2,              /**< 2 pixels per clock */
	XVIDC_PPC_4 = 4,              /**< 4 pixels per clock */
	XVIDC_PPC_8 = 8,              /**< 8 pixels per clock */
	XVIDC_PPC_NUM_SUPPORTED = 4   /**< Number of supported PPC values */
} XVidC_PixelsPerClock;

/**
 * Color space format.
 */
typedef enum {
	/* Streaming video formats */
	XVIDC_CSF_RGB = 0,  /**< RGB streaming format */
	XVIDC_CSF_YCRCB_444,  /**< YCrCb 4:4:4 streaming format */
	XVIDC_CSF_YCRCB_422,  /**< YCrCb 4:2:2 streaming format */
	XVIDC_CSF_YCRCB_420,  /**< YCrCb 4:2:0 streaming format */
	XVIDC_CSF_YONLY,  /**< Y only (luma) streaming format */
	XVIDC_CSF_RGBA,  /**< RGBA streaming format */
	XVIDC_CSF_YCRCBA_444,  /**< YCrCbA 4:4:4 streaming format with alpha */

	/*
	 * 3 empty slots reserved for video formats for future
	 * extension
	 */

	/* Video in memory formats */
	XVIDC_CSF_MEM_RGBX8 = 10,   /**< Memory format [31:0] x:B:G:R 8:8:8:8 */
	XVIDC_CSF_MEM_YUVX8,        /**< Memory format [31:0] x:V:U:Y 8:8:8:8 */
	XVIDC_CSF_MEM_YUYV8,        /**< Memory format [31:0] V:Y:U:Y 8:8:8:8 */
	XVIDC_CSF_MEM_RGBA8,        /**< Memory format [31:0] A:B:G:R 8:8:8:8 */
	XVIDC_CSF_MEM_YUVA8,        /**< Memory format [31:0] A:V:U:Y 8:8:8:8 */
	XVIDC_CSF_MEM_RGBX10,       /**< Memory format [31:0] x:B:G:R 2:10:10:10 */
	XVIDC_CSF_MEM_YUVX10,       /**< Memory format [31:0] x:V:U:Y 2:10:10:10 */
	XVIDC_CSF_MEM_RGB565,       /**< Memory format [15:0] B:G:R 5:6:5 */
	XVIDC_CSF_MEM_Y_UV8,        /**< Memory format [15:0] Y:Y 8:8, [15:0] V:U 8:8 */
	XVIDC_CSF_MEM_Y_UV8_420,    /**< Memory format [15:0] Y:Y 8:8, [15:0] V:U 8:8 (4:2:0) */
	XVIDC_CSF_MEM_RGB8,         /**< Memory format [23:0] B:G:R 8:8:8 */
	XVIDC_CSF_MEM_YUV8,         /**< Memory format [24:0] V:U:Y 8:8:8 */
	XVIDC_CSF_MEM_Y_UV10,       /**< Memory format [31:0] x:Y:Y:Y 2:10:10:10 [31:0] x:U:V:U 2:10:10:10 */
	XVIDC_CSF_MEM_Y_UV10_420,   /**< Memory format [31:0] x:Y:Y:Y 2:10:10:10 [31:0] x:U:V:U 2:10:10:10 (4:2:0) */
	XVIDC_CSF_MEM_Y8,           /**< Memory format [31:0] Y:Y:Y:Y 8:8:8:8 */
	XVIDC_CSF_MEM_Y10,          /**< Memory format [31:0] x:Y:Y:Y 2:10:10:10 */
	XVIDC_CSF_MEM_BGRA8,        /**< Memory format [31:0] A:R:G:B 8:8:8:8 */
	XVIDC_CSF_MEM_BGRX8,        /**< Memory format [31:0] X:R:G:B 8:8:8:8 */
	XVIDC_CSF_MEM_UYVY8,        /**< Memory format [31:0] Y:V:Y:U 8:8:8:8 */
	XVIDC_CSF_MEM_BGR8,         /**< Memory format [23:0] R:G:B 8:8:8 */
	XVIDC_CSF_MEM_RGBX12,       /**< Memory format [39:0] x:R:G:B 4:12:12:12 */
	XVIDC_CSF_MEM_YUVX12,       /**< Memory format [39:0] x:V:U:Y 4:12:12:12 */
	XVIDC_CSF_MEM_Y_UV12,       /**< Memory format [23:0] Y:Y 12:12, [23:0] V:U 12:12 */
	XVIDC_CSF_MEM_Y_UV12_420,   /**< Memory format [23:0] Y:Y 12:12, [23:0] V:U 12:12 (4:2:0) */
	XVIDC_CSF_MEM_Y12,          /**< Memory format [39:0] x:Y2:Y1:Y0 4:12:12:12 */
	XVIDC_CSF_MEM_RGB16,        /**< Memory format [47:0] R:G:B 16:16:16 */
	XVIDC_CSF_MEM_YUV16,        /**< Memory format [47:0] V:U:Y 16:16:16 */
	XVIDC_CSF_MEM_Y_UV16,       /**< Memory format [31:0] Y:Y 16:16, [31:0] V:U 16:16 */
	XVIDC_CSF_MEM_Y_UV16_420,   /**< Memory format [31:0] Y:Y 16:16, [31:0] V:U 16:16 (4:2:0) */
	XVIDC_CSF_MEM_Y16,          /**< Memory format [47:0] Y2:Y1:Y0 16:16:16 */
	XVIDC_CSF_MEM_R_G_B8,       /**< Memory format [7:0] R:8, [7:0] G:8, [7:0] B:8 */
	XVIDC_CSF_MEM_Y_U_V8_420,   /**< Memory format [15:0] Y:Y 8:8, [7:0] U:8, [7:0] V:8 (4:2:0) */
	XVIDC_CSF_MEM_Y_U_V8,       /**< Memory format [7:0] Y:8, [7:0] U:8, [7:0] V:8 */
	XVIDC_CSF_MEM_Y_U_V10,      /**< Memory format [9:0] Y:10, [9:0] U:10, [9:0] V:10 */
	XVIDC_CSF_MEM_Y_U_V12,      /**< Memory format [11:0] Y:12, [11:0] U:12, [11:0] V:12 */
	XVIDC_CSF_MEM_Y_U_V10_L16LE,      /**< Memory format [31:0] Y1:Y0 16:16 [31:0] U1:U0 16:16 [31:0] V1:V0 16:16 (10-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_UV10_L16LE,       /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (10-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_UV10_420_L16LE,   /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (10-bit LSB Aligned, 4:2:0) */
	XVIDC_CSF_MEM_Y10_L16LE,          /**< Memory format [31:0] Y1:Y0 16:16 (10-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_U_V12_L16LE,      /**< Memory format [31:0] Y1:Y0 16:16 [31:0] U1:U0 16:16 [31:0] V1:V0 16:16 (12-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_UV12_L16LE,       /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (12-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_UV12_420_L16LE,   /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (12-bit LSB Aligned, 4:2:0) */
	XVIDC_CSF_MEM_Y12_L16LE,          /**< Memory format [31:0] Y1:Y0 16:16 (12-bit LSB Aligned) */
	XVIDC_CSF_MEM_Y_U_V10_M16LE,      /**< Memory format [31:0] Y1:Y0 16:16 [31:0] U1:U0 16:16 [31:0] V1:V0 16:16 (10-bit MSB Aligned) */
	XVIDC_CSF_MEM_Y_UV10_M16LE,       /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (10-bit MSB Aligned) */
	XVIDC_CSF_MEM_Y_UV10_420_M16LE,   /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (10-bit MSB Aligned, 4:2:0) */
	XVIDC_CSF_MEM_Y10_M16LE,          /**< Memory format [31:0] Y1:Y0 16:16 (10-bit MSB Aligned) */
	XVIDC_CSF_MEM_Y_U_V12_M16LE,      /**< Memory format [31:0] Y1:Y0 16:16 [31:0] U1:U0 16:16 [31:0] V1:V0 16:16 (12-bit MSB Aligned) */
	XVIDC_CSF_MEM_Y_UV12_M16LE,       /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (12-bit MSB Aligned) */
	XVIDC_CSF_MEM_Y_UV12_420_M16LE,   /**< Memory format [31:0] Y1:Y0 16:16 [31:0] V0:U0 16:16 (12-bit MSB Aligned, 4:2:0) */
	XVIDC_CSF_MEM_Y12_M16LE,          /**< Memory format [31:0] Y1:Y0 16:16 (12-bit MSB Aligned) */
	XVIDC_CSF_MEM_END,          /**< End of memory formats */

	/* Streaming formats with components re-ordered */
	XVIDC_CSF_YCBCR_422 = 64,  /**< YCbCr 4:2:2 streaming format (reordered components) */
	XVIDC_CSF_YCBCR_420,  /**< YCbCr 4:2:0 streaming format (reordered components) */
	XVIDC_CSF_YCBCR_444,  /**< YCbCr 4:4:4 streaming format (reordered components) */

	XVIDC_CSF_NUM_SUPPORTED,    /**< Number of supported color formats (includes reserved slots) */
	XVIDC_CSF_UNKNOWN,  /**< Unknown color format */
	XVIDC_CSF_STRM_START = XVIDC_CSF_RGB,  /**< First streaming format */
	XVIDC_CSF_STRM_END   = XVIDC_CSF_YONLY,  /**< Last streaming format */
	XVIDC_CSF_MEM_START  = XVIDC_CSF_MEM_RGBX8,  /**< First memory format */
	XVIDC_CSF_NUM_STRM   = (XVIDC_CSF_STRM_END - XVIDC_CSF_STRM_START + 1),  /**< Number of streaming formats */
	XVIDC_CSF_NUM_MEM    = (XVIDC_CSF_MEM_END - XVIDC_CSF_MEM_START)  /**< Number of memory formats */
} XVidC_ColorFormat;

/**
 * Tile Size in Tile format.
 */
typedef enum {
	XVIDC_TILE_32 = 32,           /**< 32-byte tile size */
	XVIDC_TILE_64 = 64,           /**< 64-byte tile size */
	XVIDC_TILE_NUM_SUPPORTED = 2  /**< Number of supported tile sizes */
} XVidC_TileSize;

/**
 * Image Aspect Ratio.
 */
typedef enum {
	XVIDC_AR_4_3 = 0,   /**< 4:3 aspect ratio */
	XVIDC_AR_16_9 = 1   /**< 16:9 aspect ratio */
} XVidC_AspectRatio;

/**
 * Color space conversion standard.
 */
typedef enum {
	XVIDC_BT_2020 = 0,         /**< ITU-R BT.2020 color standard */
	XVIDC_BT_709,              /**< ITU-R BT.709 color standard */
	XVIDC_BT_601,              /**< ITU-R BT.601 color standard */
	XVIDC_BT_NUM_SUPPORTED,    /**< Number of supported color standards */
	XVIDC_BT_UNKNOWN           /**< Unknown color standard */
} XVidC_ColorStd;

/**
 * Color conversion output range.
 */
typedef enum {
	XVIDC_CR_16_235 = 0,       /**< Limited range 16-235 */
	XVIDC_CR_16_240,           /**< Limited range 16-240 */
	XVIDC_CR_0_255,            /**< Full range 0-255 */
	XVIDC_CR_NUM_SUPPORTED,    /**< Number of supported color ranges */
	XVIDC_CR_UNKNOWN_RANGE     /**< Unknown color range */
} XVidC_ColorRange;

/**
 * 3D formats.
 */
typedef enum {
	XVIDC_3D_FRAME_PACKING = 0,	/**< Frame packing.         */
	XVIDC_3D_FIELD_ALTERNATIVE,	/**< Field alternative.     */
	XVIDC_3D_LINE_ALTERNATIVE,	/**< Line alternative.      */
	XVIDC_3D_SIDE_BY_SIDE_FULL,	/**< Side-by-side (full).   */
	XVIDC_3D_TOP_AND_BOTTOM_HALF,	/**< Top-and-bottom (half). */
	XVIDC_3D_SIDE_BY_SIDE_HALF,	/**< Side-by-side (half).   */
	XVIDC_3D_UNKNOWN		/**< Unknown 3D format */
} XVidC_3DFormat;

/**
 * 3D Sub-sampling methods.
 */
typedef enum {
	XVIDC_3D_SAMPLING_HORIZONTAL = 0, /**< Horizontal sub-sampling. */
	XVIDC_3D_SAMPLING_QUINCUNX,	  /**< Quincunx matrix.         */
	XVIDC_3D_SAMPLING_UNKNOWN	  /**< Unknown sampling method */
} XVidC_3DSamplingMethod;

/**
 * 3D Sub-sampling positions.
 */
typedef enum {
	XVIDC_3D_SAMPPOS_OLOR = 0,	/**< Odd/Left,  Odd/Right.  */
	XVIDC_3D_SAMPPOS_OLER,		/**< Odd/Left,  Even/Right. */
	XVIDC_3D_SAMPPOS_ELOR,		/**< Even/Left, Odd/Right.  */
	XVIDC_3D_SAMPPOS_ELER,		/**< Even/Left, Even/Right. */
	XVIDC_3D_SAMPPOS_UNKNOWN	/**< Unknown sampling position */
} XVidC_3DSamplingPosition;

/****************************** Type Definitions ******************************/

/**
 * Video timing structure.
 */
typedef struct {
	u16 HActive;          /**< Horizontal active pixels */
	u16 HFrontPorch;      /**< Horizontal front porch pixels */
	u16 HSyncWidth;       /**< Horizontal sync width pixels */
	u16 HBackPorch;       /**< Horizontal back porch pixels */
	u16 HTotal;           /**< Horizontal total pixels */
	u8 HSyncPolarity;     /**< Horizontal sync polarity (0=negative, 1=positive) */
	u16 VActive;          /**< Vertical active lines */
	u16 F0PVFrontPorch;   /**< Field 0/Progressive vertical front porch lines */
	u16 F0PVSyncWidth;    /**< Field 0/Progressive vertical sync width lines */
	u16 F0PVBackPorch;    /**< Field 0/Progressive vertical back porch lines */
	u16 F0PVTotal;        /**< Field 0/Progressive vertical total lines */
	u16 F1VFrontPorch;    /**< Field 1 vertical front porch lines */
	u16 F1VSyncWidth;     /**< Field 1 vertical sync width lines */
	u16 F1VBackPorch;     /**< Field 1 vertical back porch lines */
	u16 F1VTotal;         /**< Field 1 vertical total lines */
	u8 VSyncPolarity;     /**< Vertical sync polarity (0=negative, 1=positive) */
} XVidC_VideoTiming;

/**
 * 3D Sampling info structure.
 */
typedef struct {
	XVidC_3DSamplingMethod   Method;    /**< 3D sampling method */
	XVidC_3DSamplingPosition Position;  /**< 3D sampling position */
} XVidC_3DSamplingInfo;

/**
 * 3D info structure.
 */
typedef struct {
	XVidC_3DFormat		  Format;    /**< 3D format type */
	XVidC_3DSamplingInfo  Sampling;  /**< 3D sampling information */
} XVidC_3DInfo;

/**
 * Electro Optical Transfer Function
 *
 * Based on CTA861-G
 */
typedef enum {
	/* TG - Traditional Gamma */
	XVIDC_EOTF_TG_SDR = 0,     /**< Traditional Gamma SDR */
	XVIDC_EOTF_TG_HDR,         /**< Traditional Gamma HDR */
	XVIDC_EOTF_SMPTE2084,      /**< SMPTE ST 2084 (PQ) */
	XVIDC_EOTF_HLG,            /**< Hybrid Log-Gamma (HLG) */
	XVIDC_EOTF_NUM_SUPPORTED,  /**< Number of supported EOTFs */
	XVIDC_EOTF_UNKNOWN,        /**< Unknown EOTF */
} XVidC_Eotf;

/**
 * Video stream structure.
 */
typedef struct {
	XVidC_ColorFormat	ColorFormatId;         /**< Color format identifier */
	XVidC_ColorDepth	ColorDepth;            /**< Color depth in bits per component */
	XVidC_PixelsPerClock	PixPerClk;         /**< Pixels per clock */
	XVidC_FrameRate		FrameRate;             /**< Frame rate */
	XVidC_AspectRatio	AspectRatio;          /**< Aspect ratio */
	u8			IsInterlaced;                      /**< Interlaced flag (0=progressive, 1=interlaced) */
	u8			Is3D;                              /**< 3D flag (0=2D, 1=3D) */
	XVidC_3DInfo		Info_3D;               /**< 3D format information */
	XVidC_VideoMode		VmId;                  /**< Video mode identifier */
	XVidC_VideoTiming	Timing;                /**< Video timing parameters */
	XVidC_Eotf		Eotf;                       /**< Electro-optical transfer function */
	XVidC_ColorStd		ColorStd;              /**< Color standard/space */
	XVidC_FrameRate		BaseFrameRate;         /**< Base frame rate */
	XVidC_VideoTiming	BaseTiming;            /**< Base timing parameters */
	/* For DSC streams */
	u8			IsDSCompressed;                    /**< DSC compression flag (0=uncompressed, 1=compressed) */
	u8			DynamicRange;                      /**< Dynamic range */
	XVidC_VideoTiming	UncompressedTiming;    /**< Uncompressed timing for DSC streams */
} XVidC_VideoStream;

/**
 * Video window structure.
 */
typedef struct {
	u32 StartX;   /**< Window starting X coordinate */
	u32 StartY;   /**< Window starting Y coordinate */
	u32 Width;    /**< Window width in pixels */
	u32 Height;   /**< Window height in pixels */
} XVidC_VideoWindow;

/**
 * Video timing mode from the video timing table.
 */
typedef struct {
	XVidC_VideoMode		VmId;       /**< Video mode identifier */
	char			Name[21];           /**< Video mode name string */
	XVidC_FrameRate		FrameRate;  /**< Frame rate for this mode */
	XVidC_VideoTiming	Timing;     /**< Timing parameters for this mode */
} XVidC_VideoTimingMode;

/**
 * Callback type which represents a custom timer wait handler. This is only
 * used for Microblaze since it doesn't have a native sleep function. To avoid
 * dependency on a hardware timer, the default wait functionality is implemented
 * using loop iterations; this isn't too accurate. Therefore a custom timer
 * handler is used, the user may implement their own wait implementation.
 *
 * @param	TimerPtr is a pointer to the timer instance.
 * @param	Delay is the duration (msec/usec) to be passed to the timer
 *		function.
 *
*******************************************************************************/
typedef void (*XVidC_DelayHandler)(void *TimerPtr, u32 Delay);

/**************************** Function Prototypes *****************************/

u32 XVidC_RegisterCustomTimingModes(const XVidC_VideoTimingMode *CustomTable,
		                            u16 NumElems);
void XVidC_UnregisterCustomTimingModes(void);
u64 XVidC_GetPixelClockHzByHVFr(u32 HTotal, u32 VTotal, u8 FrameRate);
u64 XVidC_GetPixelClockHzByVmId(XVidC_VideoMode VmId);
XVidC_VideoFormat XVidC_GetVideoFormat(XVidC_VideoMode VmId);
u8 XVidC_IsInterlaced(XVidC_VideoMode VmId);
const XVidC_VideoTimingMode* XVidC_GetVideoModeData(XVidC_VideoMode VmId);
const char *XVidC_GetVideoModeStr(XVidC_VideoMode VmId);
const char *XVidC_GetFrameRateStr(XVidC_VideoMode VmId);
const char *XVidC_GetColorFormatStr(XVidC_ColorFormat ColorFormatId);
XVidC_FrameRate XVidC_GetFrameRate(XVidC_VideoMode VmId);
const XVidC_VideoTiming* XVidC_GetTimingInfo(XVidC_VideoMode VmId);
void XVidC_ReportStreamInfo(const XVidC_VideoStream *Stream);
void XVidC_ReportTiming(const XVidC_VideoTiming *Timing, u8 IsInterlaced);
const char *XVidC_Get3DFormatStr(XVidC_3DFormat Format);
u32 XVidC_SetVideoStream(XVidC_VideoStream *VidStrmPtr, XVidC_VideoMode VmId,
			             XVidC_ColorFormat ColorFormat, XVidC_ColorDepth Bpc,
			             XVidC_PixelsPerClock Ppc);
u32 XVidC_Set3DVideoStream(XVidC_VideoStream *VidStrmPtr, XVidC_VideoMode VmId,
			               XVidC_ColorFormat ColorFormat, XVidC_ColorDepth Bpc,
			               XVidC_PixelsPerClock Ppc, XVidC_3DInfo *Info3DPtr);
XVidC_VideoMode XVidC_GetVideoModeId(u32 Width, u32 Height, u32 FrameRate,
		                             u8 IsInterlaced);
XVidC_VideoMode XVidC_GetVideoModeIdExtensive(XVidC_VideoTiming *Timing,
											  u32 FrameRate,
											  u8 IsInterlaced,
											  u8 IsExtensive);
XVidC_VideoMode XVidC_GetVideoModeIdRb(u32 Width, u32 Height, u32 FrameRate,
		                               u8 IsInterlaced, u8 RbN);
XVidC_VideoMode XVidC_GetVideoModeIdWBlanking(const XVidC_VideoTiming *Timing,
		                                      u32 FrameRate, u8 IsInterlaced);

/******************* Macros (Inline Functions) Definitions ********************/

/*****************************************************************************/
/**
 * This macro check if video stream is 3D or 2D.
 *
 * @param	VidStreamPtr is a pointer to the XVidC_VideoStream structure.
 *
 * @return	3D(1)/2D(0)
 *
 * @note	C-style signature:
 *		u8 XDp_IsStream3D(XVidC_VideoStream *VidStreamPtr)
 *
 *****************************************************************************/
#define XVidC_IsStream3D(VidStreamPtr)       ((VidStreamPtr)->Is3D)

/*************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XVIDC_H_ */
/** @} */
