/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xvidc.h
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
 * </pre>
 *
*******************************************************************************/

#ifndef XVIDC_H_
/* Prevent circular inclusions by using protection macros. */
#define XVIDC_H_

/******************************* Include Files ********************************/

#include "xil_types.h"

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
	XVIDC_VM_480_60_I = 0,
	XVIDC_VM_576_50_I,
	XVIDC_VM_1080_50_I,
	XVIDC_VM_1080_60_I,

	/* Progressive modes. */
	XVIDC_VM_640x350_85_P,
	XVIDC_VM_640x480_60_P,
	XVIDC_VM_640x480_72_P,
	XVIDC_VM_640x480_75_P,
	XVIDC_VM_640x480_85_P,
	XVIDC_VM_720x400_85_P,
	XVIDC_VM_720x480_60_P,
	XVIDC_VM_720x576_50_P,
	XVIDC_VM_800x600_56_P,
	XVIDC_VM_800x600_60_P,
	XVIDC_VM_800x600_72_P,
	XVIDC_VM_800x600_75_P,
	XVIDC_VM_800x600_85_P,
	XVIDC_VM_800x600_120_P_RB,
	XVIDC_VM_848x480_60_P,
	XVIDC_VM_1024x768_60_P,
	XVIDC_VM_1024x768_70_P,
	XVIDC_VM_1024x768_75_P,
	XVIDC_VM_1024x768_85_P,
	XVIDC_VM_1024x768_120_P_RB,
	XVIDC_VM_1152x864_75_P,
	XVIDC_VM_1280x720_50_P,
	XVIDC_VM_1280x720_60_P,
	XVIDC_VM_1280x768_60_P,
	XVIDC_VM_1280x768_60_P_RB,
	XVIDC_VM_1280x768_75_P,
	XVIDC_VM_1280x768_85_P,
	XVIDC_VM_1280x768_120_P_RB,
	XVIDC_VM_1280x800_60_P,
	XVIDC_VM_1280x800_60_P_RB,
	XVIDC_VM_1280x800_75_P,
	XVIDC_VM_1280x800_85_P,
	XVIDC_VM_1280x800_120_P_RB,
	XVIDC_VM_1280x960_60_P,
	XVIDC_VM_1280x960_85_P,
	XVIDC_VM_1280x960_120_P_RB,
	XVIDC_VM_1280x1024_60_P,
	XVIDC_VM_1280x1024_75_P,
	XVIDC_VM_1280x1024_85_P,
	XVIDC_VM_1280x1024_120_P_RB,
	XVIDC_VM_1360x768_60_P,
	XVIDC_VM_1360x768_120_P_RB,
	XVIDC_VM_1366x768_60_P,
	XVIDC_VM_1400x1050_60_P,
	XVIDC_VM_1400x1050_60_P_RB,
	XVIDC_VM_1400x1050_75_P,
	XVIDC_VM_1400x1050_85_P,
	XVIDC_VM_1400x1050_120_P_RB,
	XVIDC_VM_1440x900_60_P,
	XVIDC_VM_1440x900_60_P_RB,
	XVIDC_VM_1440x900_75_P,
	XVIDC_VM_1440x900_85_P,
	XVIDC_VM_1440x900_120_P_RB,
	XVIDC_VM_1600x1200_60_P,
	XVIDC_VM_1600x1200_65_P,
	XVIDC_VM_1600x1200_70_P,
	XVIDC_VM_1600x1200_75_P,
	XVIDC_VM_1600x1200_85_P,
	XVIDC_VM_1600x1200_120_P_RB,
	XVIDC_VM_1680x1050_60_P,
	XVIDC_VM_1680x1050_60_P_RB,
	XVIDC_VM_1680x1050_75_P,
	XVIDC_VM_1680x1050_85_P,
	XVIDC_VM_1680x1050_120_P_RB,
	XVIDC_VM_1792x1344_60_P,
	XVIDC_VM_1792x1344_75_P,
	XVIDC_VM_1792x1344_120_P_RB,
	XVIDC_VM_1856x1392_60_P,
	XVIDC_VM_1856x1392_75_P,
	XVIDC_VM_1856x1392_120_P_RB,
	XVIDC_VM_1920x1080_24_P,
	XVIDC_VM_1920x1080_25_P,
	XVIDC_VM_1920x1080_30_P,
	XVIDC_VM_1920x1080_50_P,
	XVIDC_VM_1920x1080_60_P,
	XVIDC_VM_1920x1200_60_P,
	XVIDC_VM_1920x1200_60_P_RB,
	XVIDC_VM_1920x1200_75_P,
	XVIDC_VM_1920x1200_85_P,
	XVIDC_VM_1920x1200_120_P_RB,
	XVIDC_VM_1920x1440_60_P,
	XVIDC_VM_1920x1440_75_P,
	XVIDC_VM_1920x1440_120_P_RB,
	XVIDC_VM_1920x2160_60_P,
	XVIDC_VM_2560x1600_60_P,
	XVIDC_VM_2560x1600_60_P_RB,
	XVIDC_VM_2560x1600_75_P,
	XVIDC_VM_2560x1600_85_P,
	XVIDC_VM_2560x1600_120_P_RB,
	XVIDC_VM_3840x2160_24_P,
	XVIDC_VM_3840x2160_25_P,
	XVIDC_VM_3840x2160_30_P,
	XVIDC_VM_3840x2160_60_P,

	XVIDC_VM_NUM_SUPPORTED,
	XVIDC_VM_USE_EDID_PREFERRED,
	XVIDC_VM_NO_INPUT,
	XVIDC_VM_NOT_SUPPORTED,

	/* Marks beginning/end of interlaced/progressive modes in the table. */
	XVIDC_VM_INTL_START = XVIDC_VM_480_60_I,
	XVIDC_VM_PROG_START = XVIDC_VM_640x350_85_P,
	XVIDC_VM_INTL_END = (XVIDC_VM_PROG_START - 1),
	XVIDC_VM_PROG_END = (XVIDC_VM_NUM_SUPPORTED - 1),

	/* Common naming. */
	XVIDC_VM_VGA_60_P = XVIDC_VM_640x480_60_P,
	XVIDC_VM_480_60_P = XVIDC_VM_720x480_60_P,
	XVIDC_VM_SVGA_60_P = XVIDC_VM_800x600_60_P,
	XVIDC_VM_XGA_60_P = XVIDC_VM_1024x768_60_P,
	XVIDC_VM_720_50_P = XVIDC_VM_1280x720_50_P,
	XVIDC_VM_720_60_P = XVIDC_VM_1280x720_60_P,
	XVIDC_VM_WXGA_60_P = XVIDC_VM_1366x768_60_P,
	XVIDC_VM_UXGA_60_P = XVIDC_VM_1600x1200_60_P,
	XVIDC_VM_WSXGA_60_P = XVIDC_VM_1680x1050_60_P,
	XVIDC_VM_1080_24_P = XVIDC_VM_1920x1080_24_P,
	XVIDC_VM_1080_25_P = XVIDC_VM_1920x1080_25_P,
	XVIDC_VM_1080_30_P = XVIDC_VM_1920x1080_30_P,
	XVIDC_VM_1080_50_P = XVIDC_VM_1920x1080_50_P,
	XVIDC_VM_1080_60_P = XVIDC_VM_1920x1080_60_P,
	XVIDC_VM_WUXGA_60_P = XVIDC_VM_1920x1200_60_P,
	XVIDC_VM_UHD2_60_P = XVIDC_VM_1920x2160_60_P,
	XVIDC_VM_UHD_24_P = XVIDC_VM_3840x2160_24_P,
	XVIDC_VM_UHD_25_P = XVIDC_VM_3840x2160_25_P,
	XVIDC_VM_UHD_30_P = XVIDC_VM_3840x2160_30_P,
	XVIDC_VM_UHD_60_P = XVIDC_VM_3840x2160_60_P
} XVidC_VideoMode;

/**
 * Progressive/interlaced video format.
 */
typedef enum {
	XVIDC_VF_PROGRESSIVE = 0,
	XVIDC_VF_INTERLACED
} XVidC_VideoFormat;

/**
 * Frame rate.
 */
typedef enum {
	XVIDC_FR_24HZ = 24,
	XVIDC_FR_25HZ = 25,
	XVIDC_FR_30HZ = 30,
	XVIDC_FR_50HZ = 50,
	XVIDC_FR_56HZ = 56,
	XVIDC_FR_60HZ = 60,
	XVIDC_FR_65HZ = 65,
	XVIDC_FR_67HZ = 67,
	XVIDC_FR_70HZ = 70,
	XVIDC_FR_72HZ = 72,
	XVIDC_FR_75HZ = 75,
	XVIDC_FR_85HZ = 85,
	XVIDC_FR_87HZ = 87,
	XVIDC_FR_88HZ = 88,
	XVIDC_FR_100HZ = 100,
	XVIDC_FR_120HZ = 120,
	XVIDC_FR_NUM_SUPPORTED = 16,
	XVIDC_FR_UNKNOWN
} XVidC_FrameRate;

/**
 * Color depth - bits per color component.
 */
typedef enum {
	XVIDC_BPC_6 = 6,
	XVIDC_BPC_8 = 8,
	XVIDC_BPC_10 = 10,
	XVIDC_BPC_12 = 12,
	XVIDC_BPC_14 = 14,
	XVIDC_BPC_16 = 16,
	XVIDC_BPC_NUM_SUPPORTED = 6,
	XVIDC_BPC_UNKNOWN
} XVidC_ColorDepth;

/**
 * Pixels per clock.
 */
typedef enum {
	XVIDC_PPC_1 = 1,
	XVIDC_PPC_2 = 2,
	XVIDC_PPC_4 = 4,
	XVIDC_PPC_NUM_SUPPORTED = 3,
} XVidC_PixelsPerClock;

/**
 * Color space format.
 */
typedef enum {
	XVIDC_CSF_RGB = 0,
	XVIDC_CSF_YCRCB_444,
	XVIDC_CSF_YCRCB_422,
	XVIDC_CSF_YCRCB_420,
	XVIDC_CSF_NUM_SUPPORTED,
	XVIDC_CSF_UNKNOWN
} XVidC_ColorFormat;

/**
 * Color space conversion standard.
 */
typedef enum {
	XVIDC_BT_2020 = 0,
	XVIDC_BT_709,
	XVIDC_BT_601,
	XVIDC_BT_NUM_SUPPORTED,
	XVIDC_BT_UNKNOWN
} XVidC_ColorStd;

/**
 * Color conversion output range.
 */
typedef enum {
	XVIDC_CR_16_235 = 0,
	XVIDC_CR_16_240,
	XVIDC_CR_0_255,
	XVIDC_CR_NUM_SUPPORTED,
	XVIDC_CR_UNKNOWN_RANGE
} XVidC_ColorRange;

/****************************** Type Definitions ******************************/

/**
 * Video timing structure.
 */
typedef struct {
	u16 HActive;
	u16 HFrontPorch;
	u16 HSyncWidth;
	u16 HBackPorch;
	u16 HTotal;
	u8 HSyncPolarity;
	u16 VActive;
	u16 F0PVFrontPorch;
	u16 F0PVSyncWidth;
	u16 F0PVBackPorch;
	u16 F0PVTotal;
	u16 F1VFrontPorch;
	u16 F1VSyncWidth;
	u16 F1VBackPorch;
	u16 F1VTotal;
	u8 VSyncPolarity;
} XVidC_VideoTiming;

/**
 * Video stream structure.
 */
typedef struct {
	XVidC_ColorFormat	ColorFormatId;
	XVidC_ColorDepth	ColorDepth;
	XVidC_PixelsPerClock	PixPerClk;
	XVidC_FrameRate		FrameRate;
	u8			IsInterlaced;
	XVidC_VideoMode		VmId;
	XVidC_VideoTiming	Timing;
} XVidC_VideoStream;

/**
 * Video window structure.
 */
typedef struct {
	u32 StartX;
	u32 StartY;
	u32 Width;
	u32 Height;
} XVidC_VideoWindow;

/**
 * Video timing mode from the video timing table.
 */
typedef struct {
	XVidC_VideoMode		VmId;
	const char		Name[21];
	XVidC_FrameRate		FrameRate;
	XVidC_VideoTiming	Timing;
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

u32 XVidC_GetPixelClockHzByHVFr(u32 HTotal, u32 VTotal, u8 FrameRate);
u32 XVidC_GetPixelClockHzByVmId(XVidC_VideoMode VmId);
XVidC_VideoFormat XVidC_GetVideoFormat(XVidC_VideoMode VmId);
u8 XVidC_IsInterlaced(XVidC_VideoMode VmId);
XVidC_VideoMode XVidC_GetVideoModeId(u32 Width, u32 Height, u32 FrameRate,
					u8 IsInterlaced);
const XVidC_VideoTimingMode* XVidC_GetVideoModeData(XVidC_VideoMode VmId);
const char* XVidC_GetVideoModeStr(XVidC_VideoMode VmId);
char* XVidC_GetFrameRateStr(XVidC_VideoMode VmId);
char* XVidC_GetColorFormatStr(XVidC_ColorFormat ColorFormatId);
XVidC_FrameRate XVidC_GetFrameRate(XVidC_VideoMode VmId);
const XVidC_VideoTiming* XVidC_GetTimingInfo(XVidC_VideoMode VmId);
void XVidC_ReportStreamInfo(XVidC_VideoStream *Stream);
void XVidC_ReportTiming(XVidC_VideoTiming *Timing, u8 IsInterlaced);

/*************************** Variable Declarations ****************************/

const XVidC_VideoTimingMode XVidC_VideoTimingModes[XVIDC_VM_NUM_SUPPORTED];

#endif /* XVIDC_H_ */
