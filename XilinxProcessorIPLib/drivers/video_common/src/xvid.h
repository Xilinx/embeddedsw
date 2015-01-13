/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xvid.h
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
 * </pre>
 *
*******************************************************************************/

#ifndef XVID_H_
/* Prevent circular inclusions by using protection macros. */
#define XVID_H_

/******************************* Include Files ********************************/

#include "xil_types.h"

/************************** Constant Definitions ******************************/

/**
 * This typedef enumerates the list of available standard display monitor
 * timings as specified in the xvid_timings_table.c file. The naming format is:
 *
 * XVID_VM_<RESOLUTION>_<REFRESH RATE (HZ)>_<P|I>(_RB)
 *
 * Where RB stands for reduced blanking.
 */
typedef enum {
	XVID_VM_640x350_85_P = 0,
	XVID_VM_640x400_85_P,
	XVID_VM_640x480_60_P,
	XVID_VM_640x480_72_P,
	XVID_VM_640x480_75_P,
	XVID_VM_640x480_85_P,
	XVID_VM_720x400_85_P,
	XVID_VM_720x480_60_P,
	XVID_VM_800x600_56_P,
	XVID_VM_800x600_60_P,
	XVID_VM_800x600_72_P,
	XVID_VM_800x600_75_P,
	XVID_VM_800x600_85_P,
	XVID_VM_800x600_120_P_RB,
	XVID_VM_848x480_60_P,
	XVID_VM_1024x768_60_P,
	XVID_VM_1024x768_70_P,
	XVID_VM_1024x768_75_P,
	XVID_VM_1024x768_85_P,
	XVID_VM_1024x768_120_P_RB,
	XVID_VM_1152x864_75_P,
	XVID_VM_1280x720_50_P,
	XVID_VM_1280x720_60_P,
	XVID_VM_1280x768_60_P_RB,
	XVID_VM_1280x768_60_P,
	XVID_VM_1280x768_75_P,
	XVID_VM_1280x768_85_P,
	XVID_VM_1280x768_120_P_RB,
	XVID_VM_1280x800_60_P_RB,
	XVID_VM_1280x800_60_P,
	XVID_VM_1280x800_75_P,
	XVID_VM_1280x800_85_P,
	XVID_VM_1280x800_120_P_RB,
	XVID_VM_1280x960_60_P,
	XVID_VM_1280x960_85_P,
	XVID_VM_1280x960_120_P_RB,
	XVID_VM_1280x1024_60_P,
	XVID_VM_1280x1024_75_P,
	XVID_VM_1280x1024_85_P,
	XVID_VM_1280x1024_120_P_RB,
	XVID_VM_1360x768_60_P,
	XVID_VM_1360x768_120_P_RB,
	XVID_VM_1366x768_60_P,
	XVID_VM_1400x1050_60_P_RB,
	XVID_VM_1400x1050_60_P,
	XVID_VM_1400x1050_75_P,
	XVID_VM_1400x1050_85_P,
	XVID_VM_1400x1050_120_P_RB,
	XVID_VM_1440x900_60_P_RB,
	XVID_VM_1440x900_60_P,
	XVID_VM_1440x900_75_P,
	XVID_VM_1440x900_85_P,
	XVID_VM_1440x900_120_P_RB,
	XVID_VM_1600x1200_60_P,
	XVID_VM_1600x1200_65_P,
	XVID_VM_1600x1200_70_P,
	XVID_VM_1600x1200_75_P,
	XVID_VM_1600x1200_85_P,
	XVID_VM_1600x1200_120_P_RB,
	XVID_VM_1680x1050_60_P_RB,
	XVID_VM_1680x1050_60_P,
	XVID_VM_1680x1050_75_P,
	XVID_VM_1680x1050_85_P,
	XVID_VM_1680x1050_120_P_RB,
	XVID_VM_1792x1344_60_P,
	XVID_VM_1792x1344_75_P,
	XVID_VM_1792x1344_120_P_RB,
	XVID_VM_1856x1392_60_P,
	XVID_VM_1856x1392_75_P,
	XVID_VM_1856x1392_120_P_RB,
	XVID_VM_1920x1080_24_P,
	XVID_VM_1920x1080_25_P,
	XVID_VM_1920x1080_30_P,
	XVID_VM_1920x1080_50_P,
	XVID_VM_1920x1080_60_P,
	XVID_VM_1920x1200_60_P_RB,
	XVID_VM_1920x1200_60_P,
	XVID_VM_1920x1200_75_P,
	XVID_VM_1920x1200_85_P,
	XVID_VM_1920x1200_120_P_RB,
	XVID_VM_1920x1440_60_P,
	XVID_VM_1920x1440_75_P,
	XVID_VM_1920x1440_120_P_RB,
	XVID_VM_1920x2160_60_P,
	XVID_VM_2560x1600_60_P_RB,
	XVID_VM_2560x1600_60_P,
	XVID_VM_2560x1600_75_P,
	XVID_VM_2560x1600_85_P,
	XVID_VM_2560x1600_120_P_RB,
	XVID_VM_3840x2160_24_P,
	XVID_VM_3840x2160_25_P,
	XVID_VM_3840x2160_30_P,
	XVID_VM_3840x2160_60_P,

	XVID_VM_480_30_I,
	XVID_VM_576_25_I,
	XVID_VM_1080_25_I,
	XVID_VM_1080_30_I,

	XVID_VM_NUM_SUPPORT,
	XVID_VM_USE_EDID_PREFERRED,
	XVID_VM_NO_INPUT,

	XVID_VM_480_60_P = XVID_VM_720x480_60_P,
	XVID_VM_720_50_P = XVID_VM_1280x720_50_P,
	XVID_VM_720_60_P = XVID_VM_1280x720_60_P,
	XVID_VM_WXGA_60_P = XVID_VM_1366x768_60_P,
	XVID_VM_UXGA_60_P = XVID_VM_1600x1200_60_P,
	XVID_VM_WSXGA_60_P = XVID_VM_1680x1050_60_P,
	XVID_VM_1080_24_P = XVID_VM_1920x1080_24_P,
	XVID_VM_1080_25_P = XVID_VM_1920x1080_25_P,
	XVID_VM_1080_30_P = XVID_VM_1920x1080_30_P,
	XVID_VM_1080_50_P = XVID_VM_1920x1080_50_P,
	XVID_VM_1080_60_P = XVID_VM_1920x1080_60_P,
	XVID_VM_WUXGA_60_P = XVID_VM_1920x1200_60_P,
	XVID_VM_UHD2_60_P = XVID_VM_1920x2160_60_P,
	XVID_VM_UHD_24_P = XVID_VM_3840x2160_24_P,
	XVID_VM_UHD_25_P = XVID_VM_3840x2160_25_P,
	XVID_VM_UHD_30_P = XVID_VM_3840x2160_30_P
	XVID_VM_UHD_60_P = XVID_VM_3840x2160_60_P
} XVid_VideoMode;

/* Progressive/interlaced video format. */
typedef enum {
	XVid_VM_PROGRESSIVE = 0,
	XVid_VM_INTERLACED
} XVid_VideoFormat;

typedef enum {
	XVID_FR_24HZ = 24,
	XVID_FR_25HZ = 25,
	XVID_FR_30HZ = 30,
	XVID_FR_50HZ = 50,
	XVID_FR_56HZ = 56,
	XVID_FR_60HZ = 60,
	XVID_FR_65HZ = 65,
	XVID_FR_67HZ = 67,
	XVID_FR_70HZ = 70,
	XVID_FR_72HZ = 72,
	XVID_FR_75HZ = 75,
	XVID_FR_85HZ = 85,
	XVID_FR_87HZ = 87,
	XVID_FR_88HZ = 88,
	XVID_FR_100HZ = 100,
	XVID_FR_120HZ = 120,
	XVID_FR_NUM_SUPPORTED = 16,
	XVID_FR_UNKNOWN
} XVid_FrameRate;

/* Color depth / data width / bits per color component. */
typedef enum {
	XVID_BPC_6 = 6,
	XVID_BPC_8 = 8,
	XVID_BPC_10 = 10,
	XVID_BPC_12 = 12,
	XVID_BPC_16 = 16,
	XVID_BPC_NUM_SUPPORTED = 5,
	XVID_BPC_UNKNOWN
} XVid_DataWidth;

/* Color space and chroma format. */
typedef enum {
	XVID_CS_RGB = 0,
	XVID_CS_YCRCB_444,
	XVID_CS_YCRCB_422,
	XVID_CS_YCRCB_420,
	XVID_CS_NUM_SUPPORTED,
	XVID_CS_UNKNOWN
} XVid_ColorFormat;

/* Color space conversion standards. */
typedef enum {
	XVID_BT_2020 = 0,
	XVID_BT_709,
	XVID_BT_601,
	XVID_BT_NUM_SUPPORTED,
	XVID_BT_UNKNOWN
} XVid_ColorConversionStd;

/* Color conversion output range. */
typedef enum {
	XVID_CR_16_235 = 0,
	XVID_CR_16_240,
	XVID_CR_0_255,
	XVID_CR_NUM_SUPPORTED,
	XVID_CR_UNKNOWN_RANGE
} XVid_ColorRange;

/****************************** Type Definitions ******************************/

/**
 * Video timing structure.
 */
typedef struct {
	u32 HActive;
	u32 HFrontPorch;
	u32 HSyncWidth;
	u32 HBackPorch;
	u32 HTotal;
	u32 HSyncPolarity;
	u32 VActive;
	u32 F0PVFrontPorch;
	u32 F0PVSyncWidth;
	u32 F0PVBackPorch;
	u32 F0PVTotal;
	u32 F1VFrontPorch;
	u32 F1VSyncWidth;
	u32 F1VBackPorch;
	u32 F1VTotal;
	u32 VSyncPolarity;
} XVid_VideoTiming;

/**
 * Video stream structure.
 */
typedef struct {
	XVid_ColorFormat	ColorFormatId;
	XVid_DataWidth		Bpc;
	XVid_FrameRate		FrameRate;
	u32			IsInterlaced;
	XVid_VideoMode		ResId;
	XVid_VideoTiming	Timing;
} XVid_VideoStream;

/**
 * Video window structure.
 */
typedef struct {
	u32 StartX;
	u32 StartY;
	u32 Width;
	u32 Height;
} XVid_VideoWindow;

typedef struct {
	XVid_VideoMode		VmId;
	const char		Name[21];
	XVid_FrameRate		FrameRate;
	u32			PixelClkKhz;
	XVid_VideoTiming	Timing;
} XVid_VideoTimingMode;

/*************************** Variable Declarations ****************************/

extern const XVid_VideoTimingMode XVid_VideoTimingModes[XVID_VM_NUM_SUPPORT];

/**************************** Function Prototypes *****************************/

#endif /* XVID_H_ */
