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
 * @file xdptx_vidmodetable.c
 *
 * Contains display monitor timing (DMT) modes for various standard resolutions.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  05/17/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xil_types.h"
#include "xdptx.h"

/**************************** Variable Definitions ****************************/

/**
 * This table contains the main stream attributes for various standard
 * resolutions. Each entry is of the format:
 * 1) XDPTX_VM_<HRES>x<VRES>_<REFRESH (HZ)>_P(_RB = Reduced Blanking)
 * 2) Display Monitor Timing (DMT) ID
 * 3) Horizontal resolution (pixels)
 * 4) Vertical resolution (lines)
 * 5) Pixel clock (KHz)
 * 6) Interlaced (0=non-interlaced|1=interlaced)
 * 7) Horizontal sync polarity (0=positive|1=negative)
 * 8) Vertical sync polarity (0=positive|1=negative)
 * 9) Horizontal front porch (pixels)
 * 10) Horizontal sync time (pixels)
 * 11) Horizontal back porch (pixels)
 * 12) Vertical front porch (lines)
 * 13) Vertical sync time (lines)
 * 14) Vertical back porch (lines)
 */
XDptx_DmtMode XDptx_DmtModes[] =
{
	{XDPTX_VM_640x480_60_P,		0x04, 640, 480, 25175,
				0, 1, 1, 8, 96, 40, 2, 2, 25},
	{XDPTX_VM_800x600_60_P,		0x09, 800, 600, 40000,
				0, 0, 0, 40, 128, 88, 1, 4, 23},
	{XDPTX_VM_848x480_60_P,		0x0E, 848, 480, 33750,
				0, 0, 0, 16, 112, 112, 6, 8, 23},
	{XDPTX_VM_1024x768_60_P,	0x10, 1024, 768, 65000,
				0, 1, 1, 24, 136, 160, 3, 6, 29},
	{XDPTX_VM_1280x768_60_P_RB,	0x16, 1280, 768, 68250,
				0, 0, 1, 48, 32, 80, 3, 7, 12},
	{XDPTX_VM_1280x768_60_P,	0x17, 1280, 768, 79500,
				0, 1, 0, 64, 128, 192, 3, 7, 20},
	{XDPTX_VM_1280x800_60_P_RB,	0x1B, 1280, 800, 71000,
				0, 0, 1, 48, 32, 80, 3, 6, 14},
	{XDPTX_VM_1280x800_60_P,	0x1C, 1280, 800, 83500,
				0, 1, 0, 72, 128, 200, 3, 6, 22},
	{XDPTX_VM_1280x960_60_P,	0x20, 1280, 960, 108000,
				0, 0, 0, 96, 112, 312, 1, 3, 36},
	{XDPTX_VM_1280x1024_60_P,	0x23, 1280, 1024, 108000,
				0, 0, 0, 48, 112, 248, 1, 3, 38},
	{XDPTX_VM_1360x768_60_P,	0x27, 1360, 768, 85500,
				0, 0, 0, 64, 112, 256, 3, 6, 18},
	{XDPTX_VM_1400x1050_60_P_RB,	0x29, 1400, 1050, 101000,
				0, 0, 1, 48, 32, 80, 3, 4, 23},
	{XDPTX_VM_1400x1050_60_P,	0x2A, 1400, 1050, 121750,
				0, 1, 0, 88, 144, 232, 3, 4, 32},
	{XDPTX_VM_1440x900_60_P_RB,	0x2E, 1440, 900, 88750,
				0, 0, 1, 48, 32, 80, 3, 6, 17},
	{XDPTX_VM_1440x900_60_P,	0x2F, 1440, 900, 106500,
				0, 1, 0, 80, 152, 232, 3, 6, 25},
	{XDPTX_VM_1600x1200_60_P,	0x33, 1600, 1200, 162000,
				0, 0, 0, 64, 192, 304, 1, 3, 46},
	{XDPTX_VM_1680x1050_60_P_RB,	0x39, 1680, 1050, 119000,
				0, 1, 0, 48, 32, 80, 3, 6, 21},
	{XDPTX_VM_1680x1050_60_P,	0x3A, 1680, 1050, 146250,
				0, 1, 0, 104, 176, 280, 3, 6, 30},
	{XDPTX_VM_1792x1344_60_P,	0x3E, 1792, 1344, 204750,
				0, 1, 0, 128, 200, 328, 1, 3, 46},
	{XDPTX_VM_1856x1392_60_P,	0x41, 1856, 1392, 218250,
				0, 1, 0, 96, 224, 352, 1, 3, 43},
	{XDPTX_VM_1920x1200_60_P_RB,	0x44, 1920, 1200, 154000,
				0, 0, 1, 48, 32, 80, 3, 6, 26},
	{XDPTX_VM_1920x1200_60_P,	0x45, 1920, 1200, 193250,
				0, 1, 0, 136, 200, 336, 3, 6, 36},
	{XDPTX_VM_1920x1440_60_P,	0x49, 1920, 1440, 234000,
				0, 1, 0, 128, 208, 344, 1, 3, 56},
	{XDPTX_VM_2560x1600_60_P_RB,	0x4C, 2560, 1600, 268500,
				0, 0, 1, 48, 32, 80, 3, 6, 37},
	{XDPTX_VM_2560x1600_60_P,	0x4D, 2560, 1600, 348500,
				0, 1, 0, 192, 280, 472, 3, 6, 49},
	{XDPTX_VM_800x600_56_P,		0x08, 800, 600, 36000,
				0, 0, 0, 24, 72, 128, 1, 2, 22},
	{XDPTX_VM_1600x1200_65_P,	0x34, 1600, 1200, 175500,
				0, 0, 0, 64, 192, 304, 1, 3, 46},
	{XDPTX_VM_1600x1200_70_P,	0x35, 1600, 1200, 189000,
				0, 0, 0, 64, 192, 304, 1, 3, 46},
	{XDPTX_VM_1024x768_70_P,	0x11, 1024, 768, 75000,
				0, 1, 1, 24, 136, 144, 3, 6, 29},
	{XDPTX_VM_640x480_72_P,		0x05, 640, 480, 31500,
				0, 1, 1, 16, 40, 120, 1, 3, 20},
	{XDPTX_VM_800x600_72_P,		0x0A, 800, 600, 50000,
				0, 0, 0, 56, 120, 64, 37, 6, 23},
	{XDPTX_VM_640x480_75_P,		0x06, 640, 480, 31500,
				0, 1, 1, 16, 64, 120, 1, 3, 16},
	{XDPTX_VM_800x600_75_P,		0x0B, 800, 600, 49500,
				0, 0, 0, 16, 80, 160, 1, 3, 21},
	{XDPTX_VM_1024x768_75_P,	0x12, 1024, 768, 78750,
				0, 0, 0, 16, 96, 176, 1, 3, 28},
	{XDPTX_VM_1152x864_75_P,	0x15, 1152, 864, 108000,
				0, 0, 0, 64, 128, 256, 1, 3, 32},
	{XDPTX_VM_1280x768_75_P,	0x18, 1280, 768, 102250,
				0, 1, 0, 80, 128, 208, 3, 7, 27},
	{XDPTX_VM_1280x800_75_P,	0x1D, 1280, 800, 106500,
				0, 1, 0, 80, 128, 208, 3, 6, 29},
	{XDPTX_VM_1280x1024_75_P,	0x24, 1280, 1024, 135000,
				0, 0, 0, 16, 144, 248, 1, 3, 38},
	{XDPTX_VM_1400x1050_75_P,	0x2B, 1400, 1050, 156000,
				0, 1, 0, 104, 144, 248, 3, 4, 42},
	{XDPTX_VM_1440x900_75_P,	0x30, 1440, 900, 136750,
				0, 1, 0, 96, 152, 31, 3, 6, 33},
	{XDPTX_VM_1600x1200_75_P,	0x36, 1600, 1200, 202500,
				0, 0, 0, 64, 192, 304, 1, 3, 46},
	{XDPTX_VM_1680x1050_75_P,	0x3B, 1680, 1050, 187000,
				0, 1, 0, 120, 176, 37, 3, 6, 40},
	{XDPTX_VM_1792x1344_75_P,	0x3F, 1792, 1344, 261000,
				0, 1, 0, 96, 216, 352, 1, 3, 69},
	{XDPTX_VM_1856x1392_75_P,	0x42, 1856, 1392, 288000,
				0, 1, 0, 128, 224, 352, 1, 3, 104},
	{XDPTX_VM_1920x1200_75_P,	0x46, 1920, 1200, 245250,
				0, 1, 0, 136, 208, 344, 3, 6, 46},
	{XDPTX_VM_1920x1440_75_P,	0x4A, 1920, 1440, 297000,
				0, 1, 0, 144, 224, 352, 1, 3, 56},
	{XDPTX_VM_2560x1600_75_P,	0x4E, 2560, 1600, 443250,
				0, 1, 0, 208, 280, 488, 3, 6, 63},
	{XDPTX_VM_640x350_85_P,		0x01, 640, 350, 31500,
				0, 0, 1, 32, 64, 96, 32, 3, 60},
	{XDPTX_VM_640x400_85_P,		0x02, 640, 400, 31500,
				0, 1, 0, 32, 64, 96, 1, 3, 41},
	{XDPTX_VM_720x400_85_P,		0x03, 720, 400, 35500,
				0, 1, 0, 36, 72, 108, 1, 3, 42},
	{XDPTX_VM_640x480_85_P,		0x07, 640, 480, 36000,
				0, 1, 1, 56, 56, 80, 1, 3, 25},
	{XDPTX_VM_800x600_85_P,		0x0C, 800, 600, 56250,
				0, 0, 0, 32, 64, 152, 1, 3, 27},
	{XDPTX_VM_1024x768_85_P,	0x13, 1024, 768, 94500,
				0, 0, 0, 48, 96, 208, 1, 3, 36},
	{XDPTX_VM_1280x768_85_P,	0x19, 1280, 768, 117500,
				0, 1, 0, 80, 136, 216, 3, 7, 31},
	{XDPTX_VM_1280x800_85_P,	0x1E, 1280, 800, 122500,
				0, 1, 0, 80, 136, 216, 3, 6, 34},
	{XDPTX_VM_1280x960_85_P,	0x21, 1280, 960, 148500,
				0, 0, 0, 64, 160, 224, 1, 3, 47},
	{XDPTX_VM_1280x1024_85_P,	0x25, 1280, 1024, 157500,
				0, 0, 0, 64, 160, 224, 1, 3, 44},
	{XDPTX_VM_1400x1050_85_P,	0x2C, 1400, 1050, 179500,
				0, 1, 0, 104, 152, 256, 3, 4, 48},
	{XDPTX_VM_1440x900_85_P,	0x31, 1440, 900, 157000,
				0, 1, 0, 104, 152, 32, 3, 6, 39},
	{XDPTX_VM_1600x1200_85_P,	0x37, 1600, 1200, 229500,
				0, 0, 0, 64, 192, 304, 1, 3, 46},
	{XDPTX_VM_1680x1050_85_P,	0x3C, 1680, 1050, 214750,
				0, 1, 0, 128, 176, 304, 3, 6, 46},
	{XDPTX_VM_1920x1200_85_P,	0x47, 1920, 1200, 281250,
				0, 1, 0, 144, 208, 352, 3, 6, 53},
	{XDPTX_VM_2560x1600_85_P,	0x4F, 2560, 1600, 505250,
				0, 1, 0, 208, 280, 488, 3, 6, 73},
	{XDPTX_VM_800x600_120_P_RB,	0x0D, 800, 600, 73250,
				0, 0, 1, 48, 32, 80, 3, 4, 29},
	{XDPTX_VM_1024x768_120_P_RB,	0x14, 1024, 768, 115500,
				0, 0, 1, 48, 32, 80, 3, 4, 38},
	{XDPTX_VM_1280x768_120_P_RB,	0x1A, 1280, 768, 140250,
				0, 0, 1, 48, 32, 80, 3, 7, 35},
	{XDPTX_VM_1280x800_120_P_RB,	0x1F, 1280, 800, 146250,
				0, 0, 1, 48, 32, 80, 3, 6, 38},
	{XDPTX_VM_1280x960_120_P_RB,	0x22, 1280, 960, 175500,
				0, 0, 1, 48, 32, 80, 3, 4, 50},
	{XDPTX_VM_1280x1024_120_P_RB,	0x26, 1280, 1024, 187250,
				0, 0, 1, 48, 32, 80, 3, 7, 50},
	{XDPTX_VM_1360x768_120_P_RB,	0x28, 1360, 768, 148250,
				0, 0, 1, 48, 32, 80, 3, 5, 37},
	{XDPTX_VM_1400x1050_120_P_RB,	0x2D, 1400, 1050, 208000,
				0, 0, 1, 48, 32, 80, 3, 4, 55},
	{XDPTX_VM_1440x900_120_P_RB,	0x32, 1440, 900, 182750,
				0, 0, 1, 48, 32, 80, 3, 6, 44},
	{XDPTX_VM_1600x1200_120_P_RB,	0x38, 1600, 1200, 268250,
				0, 0, 1, 48, 32, 80, 3, 4, 64},
	{XDPTX_VM_1680x1050_120_P_RB,	0x3D, 1680, 1050, 245500,
				0, 0, 1, 48, 32, 80, 3, 6, 53},
	{XDPTX_VM_1792x1344_120_P_RB,	0x40, 1792, 1344, 333250,
				0, 0, 1, 48, 32, 80, 3, 4, 72},
	{XDPTX_VM_1856x1392_120_P_RB,	0x43, 1856, 1392, 356500,
				0, 0, 1, 48, 32, 80, 3, 4, 75},
	{XDPTX_VM_1920x1200_120_P_RB,	0x48, 1920, 1200, 317000,
				0, 0, 1, 48, 32, 80, 3, 6, 62},
	{XDPTX_VM_1920x1440_120_P_RB,	0x4B, 1920, 1440, 380500,
				0, 0, 1, 48, 32, 80, 3, 4, 78},
	{XDPTX_VM_2560x1600_120_P_RB,	0x50, 2560, 1600, 552750,
				0, 0, 1, 48, 32, 80, 3, 6, 85},
	{XDPTX_VM_1366x768_60_P,	0x00, 1366, 768, 72000,
				0, 0, 0, 14, 56, 64, 1, 3, 28},
	{XDPTX_VM_1920x1080_60_P,	0x00, 1920, 1080, 148500,
				0, 1, 1, 88, 44, 148, 4, 5, 36},
	{XDPTX_VM_UHD_30_P,		0x00, 3840, 2160, 297000,
				0, 0, 1, 176, 88, 296, 20, 10, 60},
	{XDPTX_VM_720_60_P,		0x00, 1280, 720, 74250,
				0, 1, 1, 110, 40, 220, 5, 5, 20},
	{XDPTX_VM_480_60_P,		0x00, 720, 480, 27027,
				0, 1, 1, 16, 62, 60, 9, 6, 30},
	{XDPTX_VM_UHD2_60_P,		0x00, 1920, 2160, 297000,
				0, 0, 1, 88, 44, 148, 20, 10, 60},
	{XDPTX_VM_UHD_60,		0x00, 3840, 2160, 594000,
				0, 0, 1, 176, 88, 296, 20, 10, 60}
};
