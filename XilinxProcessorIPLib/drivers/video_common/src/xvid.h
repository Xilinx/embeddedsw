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

/****************************** Type Definitions ******************************/

/**
 * This typedef enumerates the list of available standard display monitor
 * timings as specified in the xvid_dmt_table.c file. The naming format is:
 *
 * XVID_VM_<RESOLUTION>_<REFRESH RATE (HZ)>_<P|RB>
 *
 * Where RB stands for reduced blanking.
 */
typedef enum {
	XVID_VM_640x480_60_P,
	XVID_VM_800x600_60_P,
	XVID_VM_848x480_60_P,
	XVID_VM_1024x768_60_P,
	XVID_VM_1280x768_60_P_RB,
	XVID_VM_1280x768_60_P,
	XVID_VM_1280x800_60_P_RB,
	XVID_VM_1280x800_60_P,
	XVID_VM_1280x960_60_P,
	XVID_VM_1280x1024_60_P,
	XVID_VM_1360x768_60_P,
	XVID_VM_1400x1050_60_P_RB,
	XVID_VM_1400x1050_60_P,
	XVID_VM_1440x900_60_P_RB,
	XVID_VM_1440x900_60_P,
	XVID_VM_1600x1200_60_P,
	XVID_VM_1680x1050_60_P_RB,
	XVID_VM_1680x1050_60_P,
	XVID_VM_1792x1344_60_P,
	XVID_VM_1856x1392_60_P,
	XVID_VM_1920x1200_60_P_RB,
	XVID_VM_1920x1200_60_P,
	XVID_VM_1920x1440_60_P,
	XVID_VM_2560x1600_60_P_RB,
	XVID_VM_2560x1600_60_P,
	XVID_VM_800x600_56_P,
	XVID_VM_1600x1200_65_P,
	XVID_VM_1600x1200_70_P,
	XVID_VM_1024x768_70_P,
	XVID_VM_640x480_72_P,
	XVID_VM_800x600_72_P,
	XVID_VM_640x480_75_P,
	XVID_VM_800x600_75_P,
	XVID_VM_1024x768_75_P,
	XVID_VM_1152x864_75_P,
	XVID_VM_1280x768_75_P,
	XVID_VM_1280x800_75_P,
	XVID_VM_1280x1024_75_P,
	XVID_VM_1400x1050_75_P,
	XVID_VM_1440x900_75_P,
	XVID_VM_1600x1200_75_P,
	XVID_VM_1680x1050_75_P,
	XVID_VM_1792x1344_75_P,
	XVID_VM_1856x1392_75_P,
	XVID_VM_1920x1200_75_P,
	XVID_VM_1920x1440_75_P,
	XVID_VM_2560x1600_75_P,
	XVID_VM_640x350_85_P,
	XVID_VM_640x400_85_P,
	XVID_VM_720x400_85_P,
	XVID_VM_640x480_85_P,
	XVID_VM_800x600_85_P,
	XVID_VM_1024x768_85_P,
	XVID_VM_1280x768_85_P,
	XVID_VM_1280x800_85_P,
	XVID_VM_1280x960_85_P,
	XVID_VM_1280x1024_85_P,
	XVID_VM_1400x1050_85_P,
	XVID_VM_1440x900_85_P,
	XVID_VM_1600x1200_85_P,
	XVID_VM_1680x1050_85_P,
	XVID_VM_1920x1200_85_P,
	XVID_VM_2560x1600_85_P,
	XVID_VM_800x600_120_P_RB,
	XVID_VM_1024x768_120_P_RB,
	XVID_VM_1280x768_120_P_RB,
	XVID_VM_1280x800_120_P_RB,
	XVID_VM_1280x960_120_P_RB,
	XVID_VM_1280x1024_120_P_RB,
	XVID_VM_1360x768_120_P_RB,
	XVID_VM_1400x1050_120_P_RB,
	XVID_VM_1440x900_120_P_RB,
	XVID_VM_1600x1200_120_P_RB,
	XVID_VM_1680x1050_120_P_RB,
	XVID_VM_1792x1344_120_P_RB,
	XVID_VM_1856x1392_120_P_RB,
	XVID_VM_1920x1200_120_P_RB,
	XVID_VM_1920x1440_120_P_RB,
	XVID_VM_2560x1600_120_P_RB,
	XVID_VM_1366x768_60_P,
	XVID_VM_1920x1080_60_P,
	XVID_VM_UHD_30_P,
	XVID_VM_720_60_P,
	XVID_VM_480_60_P,
	XVID_VM_UHD2_60_P,
	XVID_VM_UHD_60,
	XVID_VM_USE_EDID_PREFERRED,
	XVID_VM_LAST = XVID_VM_USE_EDID_PREFERRED
} XVid_VideoMode;

/**
 * This typedef contains the display monitor timing attributes for a video mode.
 */
typedef struct {
	XVid_VideoMode VideoMode;	/**< Enumerated key. */
	u8 DmtId;			/**< Standard Display Monitor Timing
						(DMT) ID number. */
	u16 HResolution;		/**< Horizontal resolution (in
						pixels). */
	u16 VResolution;		/**< Vertical resolution (in lines). */
	u32 PixelClkKhz;		/**< Pixel frequency (in KHz). This is
						also the M value for the video
						stream (MVid). */
	u8 Interlaced;			/**< Input stream interlaced scan
						(0=non-interlaced/
						1=interlaced). */
	u8 HSyncPolarity;		/**< Horizontal synchronization polarity
						(0=positive/1=negative). */
	u8 VSyncPolarity;		/**< Vertical synchronization polarity
						(0=positive/1=negative). */
	u32 HFrontPorch;		/**< Horizontal front porch (in
						pixels). */
	u32 HSyncPulseWidth;		/**< Horizontal synchronization time
						(pulse width in pixels). */
	u32 HBackPorch;			/**< Horizontal back porch (in
						pixels). */
	u32 VFrontPorch;		/**< Vertical front porch (in lines). */
	u32 VSyncPulseWidth;		/**< Vertical synchronization time
						(pulse width in lines). */
	u32 VBackPorch;			/**< Vertical back porch (in lines). */
} XVid_DmtMode;

/*************************** Variable Declarations ****************************/

extern XVid_DmtMode XVid_DmtModes[];

/**************************** Function Prototypes *****************************/

#endif /* XVID_H_ */
