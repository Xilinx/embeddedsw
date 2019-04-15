/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*
******************************************************************************/
#include "xdptxss_zcu102_tx.h"


lane_link_rate_struct lane_link_table[]=
{
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_162GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_270GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_540GBPS},
	{XDP_TX_LANE_COUNT_SET_1,XDP_TX_LINK_BW_SET_810GBPS},
	{XDP_TX_LANE_COUNT_SET_2,XDP_TX_LINK_BW_SET_810GBPS},
	{XDP_TX_LANE_COUNT_SET_4,XDP_TX_LINK_BW_SET_810GBPS},

};

#define AUTO_TEST_MODE        1 //Enables testing after software download without user inputs
#define NO_OF_TEST_ITERATIONS 7 //5 when testing with Monitors as 422 is visual check only with Monitors (Special CRC constraint)
                                //7 when testing with Xilinx Sink
#define NO_OF_TRAINING_COMBOS 12
#define TEST_RETRY_MODE       1 //Retries Failed Test as per TEST_RETRY_COUNT
#define TEST_RETRY_COUNT      2 //Test will be run TEST_RETRY_COUNT+1 times, if it fails

#define TEST_RGB_FORMAT XVIDC_CSF_RGB
#define TEST_YCBCR444_FORMAT XVIDC_CSF_YCRCB_444
#define TEST_YCBCR422_FORMAT XVIDC_CSF_YCRCB_422
#define TEST_VID_COLOR_RAMP 1
#define TEST_VID_COLOR_SQUARES 3

//Test Frame CRC - DPCD
#define DPCD_MAX_LINE_RATE   0x001
#define DPCD_MAX_LANE_COUNT  0x002
#define DPCD_LINE_RATE       0x100
#define DPCD_LANE_COUNT      0x101
#define DPCD_TEST_CRC_R_Cr   0x240
#define DPCD_TEST_SINK_MISC  0x246
#define DPCD_TEST_SINK_START 0x270
#define CRC_AVAIL_TIMEOUT    1000

//extern XVidC_VideoMode resolution_table[];
// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_7680x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_VM_5120x1080_60_P ,
	XVIDC_VM_1920x3240_60_P,
	XVIDC_VM_1920x1080_60_P_RB,
	XVIDC_VM_1920x4320_60_P_RB,
	XVIDC_VM_3840x2160_120_P_RB2,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_DP_24_P,
	XVIDC_VM_7680x4320_DP_25_P,
	XVIDC_VM_7680x4320_30_P_RB,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_3840x4320_60_P_RB,
	XVIDC_VM_2560x1440_60_P_RB,
	XVIDC_VM_5120x2880_60_P_RB2,
	XVIDC_VM_7680x4320_30_P_MSTAR,
	XVIDC_VM_5120x2880_60_P_MSTAR,
	XVIDC_VM_3840x2160_120_P_MSTAR,
	XVIDC_MAX_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

// CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_AdditionalTimingModes[
					(XVIDC_MAX_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
    { XVIDC_VM_7680x1080_60_P_RB, "7680x1080@60Hz (RB)", XVIDC_FR_60HZ,
        {7680, 48, 32, 80, 7840, 1,
         1080, 3, 10, 18, 1111, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_5120x1080_60_P, "5120x1080@60Hz", XVIDC_FR_60HZ,
         {5120, 328, 544, 872, 6864, 1,
          1080, 3, 10, 27, 1120, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_1920x3240_60_P, "1920x3240@60Hz", XVIDC_FR_60HZ,
         {1920, 168, 208, 376, 2672, 1,
          3240, 3, 10, 101, 3354, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz(RB)", XVIDC_FR_60HZ,
         {1920, 48, 32, 80, 2080, 1,
          1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_1920x4320_60_P_RB, "1920x4320@60Hz(RB)", XVIDC_FR_60HZ,
         {1920, 48, 32, 80, 2080, 1,
          4320, 3, 10, 110, 4443, 0, 0, 0, 0, 0}
    },

    { XVIDC_VM_3840x2160_120_P_RB2, "3840x2160@120Hz(RB2)", XVIDC_FR_120HZ,
	{3840, 8, 32, 40, 3920, 1,
	2160, 113, 8, 6, 2287, 0, 0, 0, 0, 1} }, 

    { XVIDC_VM_3840x2160_100_P_RB2, "3840x2160@100Hz(RB2)", XVIDC_FR_100HZ,
	{3840, 8, 32, 40, 3920, 0,
	2160, 91, 8, 6, 2265, 0, 0, 0, 0, 1} },

    { XVIDC_VM_7680x4320_DP_24_P, "7680x4320@24Hz", XVIDC_FR_24HZ,
	{7680, 352, 176, 592, 8800, 1,
	4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1} },

    { XVIDC_VM_7680x4320_DP_25_P, "7680x4320@25Hz", XVIDC_FR_25HZ,
	{7680, 352, 176, 592, 8800, 1,
	4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1} },

    { XVIDC_VM_7680x4320_30_P_RB, "7680x4320@30Hz(RB)", XVIDC_FR_30HZ,
	{7680, 8, 32, 40, 7760, 0,
	4320, 47, 8, 6, 4381, 0, 0, 0, 0, 1} },

    { XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
        {7680, 48, 32, 80, 7840, 0,
         4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}
    },

    { XVIDC_VM_3840x4320_60_P_RB, "3840x4320@60Hz", XVIDC_FR_60HZ,
        {3840, 48, 32, 80, 4000, 0,
         4320, 3, 10, 110, 4443, 0, 0, 0, 0, 1} },

    { XVIDC_VM_2560x1440_60_P_RB, "2560x1440@60Hz", XVIDC_FR_60HZ,
        {2560, 48, 32, 80, 2720, 1,
         1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0} },

    { XVIDC_VM_5120x2880_60_P_RB2, "5120x2880@60Hz(RB2)", XVIDC_FR_60HZ,
        {5120, 8, 32, 40, 5200, 0,
         2880, 68, 8, 6, 2962, 0, 0, 0, 0, 1} },
   
    { XVIDC_VM_7680x4320_30_P_MSTAR, "7680x4320@30Hz(MSTAR)", XVIDC_FR_30HZ,
        {7680, 25, 97, 239, 8041, 0,
         4320, 48, 9, 5, 4382, 0, 0, 0, 0, 1} },  

    { XVIDC_VM_5120x2880_60_P_MSTAR, "5120x2880@60Hz(MSTAR)", XVIDC_FR_60HZ,
        {5120, 25, 97, 239, 5481, 0,
         2880, 48, 9, 5, 2942, 0, 0, 0, 0, 1} }, 

    { XVIDC_VM_3840x2160_120_P_MSTAR, "3840x2160@120Hz(MSTAR)", XVIDC_FR_120HZ,
        {3840, 48, 34, 79, 4001, 0,
         2160, 4, 6, 53, 2223, 0, 0, 0, 0, 1} }, 	 
};

const XVidC_VideoTimingMode XVidC_VideoTimingModes_Tests[XVIDC_VM_NUM_SUPPORTED] =
{
	/* Interlaced modes. */
		{ XVIDC_VM_720x480_60_I, "720x480@60Hz (I)", XVIDC_FR_60HZ,
			{720, 19, 62, 57, 858, 0,
			240, 4, 3, 15, 262, 5, 3, 15, 263, 0} },

	{ XVIDC_VM_720x480_60_I, "720x480@60Hz (I)", XVIDC_FR_60HZ,
		{720, 19, 62, 57, 858, 0,
		240, 4, 3, 15, 262, 5, 3, 15, 263, 0} },
	{ XVIDC_VM_720x576_50_I, "720x576@50Hz (I)", XVIDC_FR_50HZ,
		{720, 12, 63, 69, 864, 0,
		288, 2, 3, 19, 312, 3, 3, 19, 313, 0} },
	{ XVIDC_VM_1440x480_60_I, "1440x480@60Hz (I)", XVIDC_FR_60HZ,
		{1440, 38, 124, 114, 1716, 0,
		240, 4, 3, 15, 262, 5, 3, 15, 263, 0} },
	{ XVIDC_VM_1440x576_50_I, "1440x576@50Hz (I)", XVIDC_FR_50HZ,
		{1440, 24, 126, 138, 1728, 0,
		288, 2, 3, 19, 312, 3, 3, 19, 313, 0} },
	{ XVIDC_VM_1920x1080_48_I, "1920x1080@48Hz (I)", XVIDC_FR_48HZ,
		{1920, 371, 88, 371, 2750, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_1920x1080_50_I, "1920x1080@50Hz (I)", XVIDC_FR_50HZ,
		{1920, 528, 44, 148, 2640, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_1920x1080_60_I, "1920x1080@60Hz (I)", XVIDC_FR_60HZ,
		{1920, 88, 44, 148, 2200, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_1920x1080_96_I, "1920x1080@96Hz (I)", XVIDC_FR_96HZ,
		{1920, 371, 88, 371, 2750, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },
	{ XVIDC_VM_1920x1080_100_I, "1920x1080@100Hz (I)", XVIDC_FR_100HZ,
		{1920, 528, 44, 148, 2640, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },
	{ XVIDC_VM_1920x1080_120_I, "1920x1080@120Hz (I)", XVIDC_FR_120HZ,
		{1920, 88, 44, 148, 2200, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },
	{ XVIDC_VM_2048x1080_48_I, "2048x1080@48Hz (I)", XVIDC_FR_48HZ,
		{2048, 329, 44, 329, 2750, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_2048x1080_50_I, "2048x1080@50Hz (I)", XVIDC_FR_50HZ,
		{2048, 274, 44, 274, 2640, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_2048x1080_60_I, "2048x1080@60Hz (I)", XVIDC_FR_60HZ,
		{2048, 66, 20, 66, 2200, 1,
		540, 2, 5, 15, 562, 3, 5, 15, 563, 1} },
	{ XVIDC_VM_2048x1080_96_I, "2048x1080@96Hz (I)", XVIDC_FR_96HZ,
		{2048, 329, 44, 329, 2750, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },
	{ XVIDC_VM_2048x1080_100_I, "2048x1080@100Hz (I)", XVIDC_FR_100HZ,
		{2048, 274, 44, 274, 2640, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },
	{ XVIDC_VM_2048x1080_120_I, "2048x1080@120Hz (I)", XVIDC_FR_120HZ,
		{2048, 66, 20, 66, 2200, 1,
		1080, 4, 10, 30, 1124, 6, 10, 30, 1126, 1} },


	/* Progressive modes. */
	{ XVIDC_VM_640x350_85_P, "640x350@85Hz", XVIDC_FR_85HZ,
		{640, 32, 64, 96, 832, 1,
		350, 32, 3, 60, 445, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_640x480_60_P, "640x480@60Hz", XVIDC_FR_60HZ,
		{640, 8+8, 96, 40+8, 800, 0,
		480, 2+8, 2, 25+8, 525, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_640x480_72_P, "640x480@72Hz", XVIDC_FR_72HZ,
		{640, 8+16, 40, 120+8, 832, 0,
		480, 8+1, 3, 20+8, 520, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_640x480_75_P, "640x480@75Hz", XVIDC_FR_75HZ,
		{640, 16, 64, 120, 840, 0,
		480, 1, 3, 16, 500, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_640x480_85_P, "640x480@85Hz", XVIDC_FR_85HZ,
		{640, 56, 56, 80, 832, 0,
		480, 1, 3, 25, 509, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_720x400_85_P, "720x400@85Hz", XVIDC_FR_85HZ,
		{720, 36, 72, 108, 936, 0,
		400, 1, 3, 42, 446, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_720x480_60_P, "720x480@60Hz", XVIDC_FR_60HZ,
		{720, 16, 62, 60, 858, 0,
		480, 9, 6, 30, 525, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_720x576_50_P, "720x576@50Hz", XVIDC_FR_50HZ,
		{720, 12, 64, 68, 864, 0,
		576, 5, 5, 39, 625, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_800x600_56_P, "800x600@56Hz", XVIDC_FR_56HZ,
		{800, 24, 72, 128, 1024, 1,
		600, 1, 2, 22, 625, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_800x600_60_P, "800x600@60Hz", XVIDC_FR_60HZ,
		{800, 40, 128, 88, 1056, 1,
		600, 1, 4, 23, 628, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_800x600_72_P, "800x600@72Hz", XVIDC_FR_72HZ,
		{800, 56, 120, 64, 1040, 1,
		600, 37, 6, 23, 666, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_800x600_75_P, "800x600@75Hz", XVIDC_FR_75HZ,
		{800, 16, 80, 160, 1056, 1,
		600, 1, 3, 21, 625, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_800x600_85_P, "800x600@85Hz", XVIDC_FR_85HZ,
		{800, 32, 64, 152, 1048, 1,
		600, 1, 3, 27, 631, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_800x600_120_P_RB, "800x600@120Hz (RB)", XVIDC_FR_120HZ,
		{800, 48, 32, 80, 960, 1,
		600, 3, 4, 29, 636, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_848x480_60_P, "848x480@60Hz", XVIDC_FR_60HZ,
		{848, 16, 112, 112, 1088, 1,
		480, 6, 8, 23, 517, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1024x768_60_P, "1024x768@60Hz", XVIDC_FR_60HZ,
		{1024, 24, 136, 160, 1344, 0,
		768, 3, 6, 29, 806, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1024x768_70_P, "1024x768@70Hz", XVIDC_FR_70HZ,
		{1024, 24, 136, 144, 1328, 0,
		768, 3, 6, 29, 806, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1024x768_75_P, "1024x768@75Hz", XVIDC_FR_75HZ,
		{1024, 16, 96, 176, 1312, 1,
		768, 1, 3, 28, 800, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1024x768_85_P, "1024x768@85Hz", XVIDC_FR_85HZ,
		{1024, 48, 96, 208, 1376, 1,
		768, 1, 3, 36, 808, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1024x768_120_P_RB, "1024x768@120Hz (RB)", XVIDC_FR_120HZ,
		{1024, 48, 32, 80, 1184, 1,
		768, 3, 4, 38, 813, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1152x864_75_P, "1152x864@75Hz", XVIDC_FR_75HZ,
		{1152, 64, 128, 256, 1600, 1,
		864, 1, 3, 32, 900, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x720_24_P, "1280x720@24Hz", XVIDC_FR_24HZ,
		{1280, 970, 905, 970, 4125, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x720_25_P, "1280x720@25Hz", XVIDC_FR_25HZ,
		{1280, 970, 740, 970, 3960, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x720_30_P, "1280x720@30Hz", XVIDC_FR_30HZ,
		{1280, 970, 80, 970, 3300, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x720_50_P, "1280x720@50Hz", XVIDC_FR_50HZ,
		{1280, 440, 40, 220, 1980, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x720_60_P, "1280x720@60Hz", XVIDC_FR_60HZ,
		{1280, 110, 40, 220, 1650, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x768_60_P, "1280x768@60Hz", XVIDC_FR_60HZ,
		{1280, 64, 128, 192, 1664, 0,
		768, 3, 7, 20, 798, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x768_60_P_RB, "1280x768@60Hz (RB)", XVIDC_FR_60HZ,
		{1280, 48, 32, 80, 1440, 1,
		768, 3, 7, 12, 790, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1280x768_75_P, "1280x768@75Hz", XVIDC_FR_75HZ,
		{1280, 80, 128, 208, 1696, 0,
		768, 3, 7, 27, 805, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x768_85_P, "1280x768@85Hz", XVIDC_FR_85HZ,
		{1280, 80, 136, 216, 1712, 0,
		768, 3, 7, 31, 809, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x768_120_P_RB, "1280x768@120Hz (RB)", XVIDC_FR_120HZ,
		{1280, 48, 32, 80, 1440, 1,
		768, 3, 7, 35, 813, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1280x800_60_P, "1280x800@60Hz", XVIDC_FR_60HZ,
		{1280, 72, 128, 200, 1680, 0,
		800, 3, 6, 22, 831, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x800_60_P_RB, "1280x800@60Hz (RB)", XVIDC_FR_60HZ,
		{1280, 48, 32, 80, 1440, 1,
		800, 3, 6, 14, 823, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1280x800_75_P, "1280x800@75Hz", XVIDC_FR_75HZ,
		{1280, 80, 128, 208, 1696, 0,
		800, 3, 6, 29, 838, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x800_85_P, "1280x800@85Hz", XVIDC_FR_85HZ,
		{1280, 80, 136, 216, 1712, 0,
		800, 3, 6, 34, 843, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x800_120_P_RB, "1280x800@120Hz (RB)", XVIDC_FR_120HZ,
		{1280, 48, 32, 80, 1440, 1,
		800, 3, 6, 38, 847, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1280x960_60_P, "1280x960@60Hz", XVIDC_FR_60HZ,
		{1280, 96, 112, 312, 1800, 1,
		960, 1, 3, 36, 1000, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x960_85_P, "1280x960@85Hz", XVIDC_FR_85HZ,
		{1280, 64, 160, 224, 1728, 1,
		960, 1, 3, 47, 1011, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x960_120_P_RB, "1280x960@120Hz (RB)", XVIDC_FR_120HZ,
		{1280, 48, 32, 80, 1440, 1,
		960, 3, 4, 50, 1017, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1280x1024_60_P, "1280x1024@60Hz", XVIDC_FR_60HZ,
		{1280, 48, 112, 248, 1688, 1,
		1024, 1, 3, 38, 1066, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x1024_75_P, "1280x1024@75Hz", XVIDC_FR_75HZ,
		{1280, 16, 144, 248, 1688, 1,
		1024, 1, 3, 38, 1066, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x1024_85_P, "1280x1024@85Hz", XVIDC_FR_85HZ,
		{1280, 64, 160, 224, 1728, 1,
		1024, 1, 3, 44, 1072, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1280x1024_120_P_RB, "1280x1024@120Hz (RB)", XVIDC_FR_120HZ,
		{1280, 48, 32, 80, 1440, 1,
		1024, 3, 7, 50, 1084, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1360x768_60_P, "1360x768@60Hz", XVIDC_FR_60HZ,
		{1360, 64, 112, 256, 1792, 1,
		768, 3, 6, 18, 795, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1360x768_120_P_RB, "1360x768@120Hz (RB)", XVIDC_FR_120HZ,
		{1360, 48, 32, 80, 1520, 1,
		768, 3, 5, 37, 813, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1366x768_60_P, "1366x768@60Hz", XVIDC_FR_60HZ,
		{1366, 14, 56, 64, 1500, 1,
		768, 1, 3, 28, 800, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1400x1050_60_P, "1400x1050@60Hz", XVIDC_FR_60HZ,
		{1400, 88, 144, 232, 1864, 0,
		1050, 3, 4, 32, 1089, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1400x1050_60_P_RB, "1400x1050@60Hz (RB)", XVIDC_FR_60HZ,
		{1400, 48, 32, 80, 1560, 1,
		1050, 3, 4, 23, 1080, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1400x1050_75_P, "1400x1050@75Hz", XVIDC_FR_75HZ,
		{1400, 104, 144, 248, 1896, 0,
		1050, 3, 4, 42, 1099, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1400x1050_85_P, "1400x1050@85Hz", XVIDC_FR_85HZ,
		{1400, 104, 152, 256, 1912, 0,
		1050, 3, 4, 48, 1105, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1400x1050_120_P_RB, "1400x1050@120Hz (RB)", XVIDC_FR_120HZ,
		{1400, 48, 32, 80, 1560, 1,
		1050, 3, 4, 55, 1112, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1440x240_60_P, "1440x240@60Hz", XVIDC_FR_60HZ,
		{1440, 38, 124, 114, 1716, 0,
		240, 14, 3, 4, 262, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1440x900_60_P, "1440x900@60Hz", XVIDC_FR_60HZ,
		{1440, 80, 152, 232, 1904, 0,
		900, 3, 6, 25, 934, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1440x900_60_P_RB, "1440x900@60Hz (RB)", XVIDC_FR_60HZ,
		{1440, 48, 32, 80, 1600, 1,
		900, 3, 6, 17, 926, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1440x900_75_P, "1440x900@75Hz", XVIDC_FR_75HZ,
		{1440, 96, 152, 248, 1936, 0,
		900, 3, 6, 33, 942, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1440x900_85_P, "1440x900@85Hz", XVIDC_FR_85HZ,
		{1440, 104, 152, 256, 1952, 0,
		900, 3, 6, 39, 948, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1440x900_120_P_RB, "1440x900@120Hz (RB)", XVIDC_FR_120HZ,
		{1440, 48, 32, 80, 1600, 1,
		900, 3, 6, 44, 953, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1600x1200_60_P, "1600x1200@60Hz", XVIDC_FR_60HZ,
		{1600, 64, 192, 304, 2160, 1,
		1200, 1, 3, 46, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1600x1200_65_P, "1600x1200@65Hz", XVIDC_FR_65HZ,
		{1600, 64, 192, 304, 2160, 1,
		1200, 1, 3, 46, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1600x1200_70_P, "1600x1200@70Hz", XVIDC_FR_70HZ,
		{1600, 64, 192, 304, 2160, 1,
		1200, 1, 3, 46, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1600x1200_75_P, "1600x1200@75Hz", XVIDC_FR_75HZ,
		{1600, 64, 192, 304, 2160, 1,
		1200, 1, 3, 46, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1600x1200_85_P, "1600x1200@85Hz", XVIDC_FR_85HZ,
		{1600, 64, 192, 304, 2160, 1,
		1200, 1, 3, 46, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1600x1200_120_P_RB, "1600x1200@120Hz (RB)", XVIDC_FR_120HZ,
		{1600, 48, 32, 80, 1760, 1,
		1200, 3, 4, 64, 1271, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1680x720_50_P, "1680x720@50Hz", XVIDC_FR_50HZ,
		{1680, 260, 40, 220, 2200, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x720_60_P, "1680x720@60Hz", XVIDC_FR_60HZ,
		{1680, 260, 40, 220, 2200, 1,
		720, 5, 5, 20, 750, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x720_100_P, "1680x720@100Hz", XVIDC_FR_100HZ,
		{1680, 60, 40, 220, 2000, 1,
		720, 5, 5, 95, 825, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x720_120_P, "1680x720@120Hz", XVIDC_FR_120HZ,
		{1680, 60, 40, 220, 2000, 1,
		720, 5, 5, 95, 825, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x1050_50_P, "1680x1050@50Hz", XVIDC_FR_50HZ,
		{1680, 88, 176, 264, 2208, 0,
		1050, 3, 6, 24, 1083, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x1050_60_P, "1680x1050@60Hz", XVIDC_FR_60HZ,
		{1680, 104, 176, 280, 2240, 0,
		1050, 3, 6, 30, 1089, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x1050_60_P_RB, "1680x1050@60Hz (RB)", XVIDC_FR_60HZ,
		{1680, 48, 32, 80, 1840, 1,
		1050, 3, 6, 21, 1080, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1680x1050_75_P, "1680x1050@75Hz", XVIDC_FR_75HZ,
		{1680, 120, 176, 296, 2272, 0,
		1050, 3, 6, 40, 1099, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x1050_85_P, "1680x1050@85Hz", XVIDC_FR_85HZ,
		{1680, 128, 176, 304, 2288, 0,
		1050, 3, 6, 46, 1105, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1680x1050_120_P_RB, "1680x1050@120Hz (RB)", XVIDC_FR_120HZ,
		{1680, 48, 32, 80, 1840, 1,
		1050, 3, 6, 53, 1112, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1792x1344_60_P, "1792x1344@60Hz", XVIDC_FR_60HZ,
		{1792, 128, 200, 328, 2448, 0,
		1344, 1, 3, 46, 1394, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1792x1344_75_P, "1792x1344@75Hz", XVIDC_FR_75HZ,
		{1792, 96, 216, 352, 2456, 0,
		1344, 1, 3, 69, 1417, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1792x1344_120_P_RB, "1792x1344@120Hz (RB)", XVIDC_FR_120HZ,
		{1792, 48, 32, 80, 1952, 1,
		1344, 3, 4, 72, 1423, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1856x1392_60_P, "1856x1392@60Hz", XVIDC_FR_60HZ,
		{1856, 96, 224, 352, 2528, 0,
		1392, 1, 3, 43, 1439, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1856x1392_75_P, "1856x1392@75Hz", XVIDC_FR_75HZ,
		{1856, 128, 224, 352, 2560, 0,
		1392, 1, 3, 104, 1500, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1856x1392_120_P_RB, "1856x1392@120Hz (RB)", XVIDC_FR_120HZ,
		{1856, 48, 32, 80, 2016, 1,
		1392, 3, 4, 75, 1474, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1920x1080_24_P, "1920x1080@24Hz", XVIDC_FR_24HZ,
		{1920, 638, 44, 148, 2750, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_25_P, "1920x1080@25Hz", XVIDC_FR_25HZ,
		{1920, 528, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_30_P, "1920x1080@30Hz", XVIDC_FR_30HZ,
		{1920, 88, 44, 148, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_48_P, "1920x1080@48Hz", XVIDC_FR_48HZ,
		{1920, 638, 44, 148, 2750, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_50_P, "1920x1080@50Hz", XVIDC_FR_50HZ,
		{1920, 528, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_60_P, "1920x1080@60Hz", XVIDC_FR_60HZ,
		{1920, 88, 44, 148, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_100_P, "1920x1080@100Hz", XVIDC_FR_100HZ,
		{1920, 528, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1080_120_P, "1920x1080@120Hz", XVIDC_FR_120HZ,
		{1920, 88, 44, 148, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1200_60_P, "1920x1200@60Hz", XVIDC_FR_60HZ,
		{1920, 136, 200, 336, 2592, 0,
		1200, 3, 6, 36, 1245, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1200_60_P_RB, "1920x1200@60Hz (RB)", XVIDC_FR_60HZ,
		{1920, 48, 32, 80, 2080, 1,
		1200, 3, 6, 26, 1235, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1920x1200_75_P, "1920x1200@75Hz", XVIDC_FR_75HZ,
		{1920, 136, 208, 344, 2608, 0,
		1200, 3, 6, 46, 1255, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1200_85_P, "1920x1200@85Hz", XVIDC_FR_85HZ,
		{1920, 144, 208, 352, 2624, 0,
		1200, 3, 6, 53, 1262, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1200_120_P_RB, "1920x1200@120Hz (RB)", XVIDC_FR_120HZ,
		{1920, 48, 32, 80, 2080, 1,
		1200, 3, 6, 62, 1271, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1920x1440_60_P, "1920x1440@60Hz", XVIDC_FR_60HZ,
		{1920, 128, 208, 344, 2600, 0,
		1440, 1, 3, 56, 1500, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1440_75_P, "1920x1440@75Hz", XVIDC_FR_75HZ,
		{1920, 144, 224, 352, 2640, 0,
		1440, 1, 3, 56, 1500, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_1920x1440_120_P_RB, "1920x1440@120Hz (RB)", XVIDC_FR_120HZ,
		{1920, 48, 32, 80, 2080, 1,
		1440, 3, 4, 78, 1525, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_1920x2160_60_P, "1920x2160@60Hz", XVIDC_FR_60HZ,
		{1920, 88, 44, 148, 2200, 1,
		2160, 20, 10, 60, 2250, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_2048x1080_24_P, "2048x1080@24Hz", XVIDC_FR_24HZ,
		{2048, 510, 44, 148, 2750, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_25_P, "2048x1080@25Hz", XVIDC_FR_25HZ,
		{2048, 400, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_30_P, "2048x1080@30Hz", XVIDC_FR_30HZ,
		{2048, 66, 20, 66, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_48_P, "2048x1080@48Hz", XVIDC_FR_48HZ,
		{2048, 510, 44, 148, 2750, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_50_P, "2048x1080@50Hz", XVIDC_FR_50HZ,
		{2048, 400, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_60_P, "2048x1080@60Hz", XVIDC_FR_60HZ,
		{2048, 88, 44, 20, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_100_P, "2048x1080@100Hz", XVIDC_FR_100HZ,
		{2048, 528, 44, 148, 2640, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2048x1080_120_P, "2048x1080@120Hz", XVIDC_FR_120HZ,
		{2048, 88, 44, 148, 2200, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1080_50_P, "2560x1080@50Hz", XVIDC_FR_50HZ,
		{2560, 548, 44, 148, 3300, 1,
		1080, 4, 5, 36, 1125, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1080_60_P, "2560x1080@60Hz", XVIDC_FR_60HZ,
		{2560, 248, 44, 148, 3000, 1,
		1080, 4, 5, 11, 1100, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1080_100_P, "2560x1080@100Hz", XVIDC_FR_100HZ,
		{2560, 218, 44, 148, 2970, 1,
		1080, 4, 5, 161, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1080_120_P, "2560x1080@120Hz", XVIDC_FR_120HZ,
		{2560, 548, 44, 148, 3300, 1,
		1080, 4, 5, 161, 1250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1600_60_P, "2560x1600@60Hz", XVIDC_FR_60HZ,
		{2560, 192, 280, 472, 3504, 0,
		1600, 3, 6, 49, 1658, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1600_60_P_RB, "2560x1600@60Hz (RB)", XVIDC_FR_60HZ,
		{2560, 48, 32, 80, 2720, 1,
		1600, 3, 6, 37, 1646, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_2560x1600_75_P, "2560x1600@75Hz", XVIDC_FR_75HZ,
		{2560, 208, 280, 488, 3536, 0,
		1600, 3, 6, 63, 1672, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1600_85_P, "2560x1600@85Hz", XVIDC_FR_85HZ,
		{2560, 208, 280, 488, 3536, 0,
		1600, 3, 6, 73, 1682, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_2560x1600_120_P_RB, "2560x1600@120Hz (RB)", XVIDC_FR_120HZ,
		{2560, 48, 32, 80, 2720, 1,
		1600, 3, 6, 85, 1694, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_3840x2160_24_P, "3840x2160@24Hz", XVIDC_FR_24HZ,
		{3840, 1276, 88, 296, 5500, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_3840x2160_25_P, "3840x2160@25Hz", XVIDC_FR_25HZ,
		{3840, 1056, 88, 296, 5280, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_3840x2160_30_P, "3840x2160@30Hz", XVIDC_FR_30HZ,
		{3840, 176, 88, 296, 4400, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_3840x2160_48_P, "3840x2160@48Hz", XVIDC_FR_48HZ,
		{3840, 1276, 88, 296, 5500, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_3840x2160_50_P, "3840x2160@50Hz", XVIDC_FR_50HZ,
		{3840, 1056, 88, 296, 5280, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_3840x2160_60_P, "3840x2160@60Hz", XVIDC_FR_60HZ,
		{3840, 176, 88, 296, 4400, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
    { XVIDC_VM_3840x2160_60_P_RB, "3840x2160@60Hz (RB)", XVIDC_FR_60HZ,
        {3840, 48, 32, 80, 4000, 1,
        2160, 3, 5, 54, 2222, 0, 0, 0, 0, 0} },
	{ XVIDC_VM_4096x2160_24_P, "4096x2160@24Hz", XVIDC_FR_24HZ,
		{4096, 1020, 88, 296, 5500, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_25_P, "4096x2160@25Hz", XVIDC_FR_25HZ,
		{4096, 968, 88, 128, 5280, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_30_P, "4096x2160@30Hz", XVIDC_FR_30HZ,
		{4096, 88, 88, 128, 4400, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_48_P, "4096x2160@48Hz", XVIDC_FR_48HZ,
		{4096, 1020, 88, 296, 5500, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_50_P, "4096x2160@50Hz", XVIDC_FR_50HZ,
		{4096, 968, 88, 128, 5280, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_60_P, "4096x2160@60Hz", XVIDC_FR_60HZ,
		{4096, 88, 88, 128, 4400, 1,
		2160, 8, 10, 72, 2250, 0, 0, 0, 0, 1} },
	{ XVIDC_VM_4096x2160_60_P_RB, "4096x2160@60Hz (RB)", XVIDC_FR_60HZ,
		{4096, 8, 32, 40, 4176, 1,
		2160, 48, 8, 6, 2222, 0, 0, 0, 0, 0} },
};


XVidC_VideoMode resolution_table[] =
{
                XVIDC_VM_640x480_60_P,
                XVIDC_VM_480_60_P,
                XVIDC_VM_800x600_60_P,
                XVIDC_VM_1024x768_60_P,
                XVIDC_VM_720_60_P,
                XVIDC_VM_1600x1200_60_P,
                XVIDC_VM_1366x768_60_P,
                XVIDC_VM_1080_60_P,
                XVIDC_VM_UHD_30_P,
                XVIDC_VM_UHD_60_P,
                XVIDC_VM_2560x1600_60_P,
                XVIDC_VM_1280x1024_60_P,
                XVIDC_VM_1792x1344_60_P,
                XVIDC_VM_848x480_60_P,
                XVIDC_VM_1280x960_60_P,
                XVIDC_VM_1920x1440_60_P,
                XVIDC_VM_USE_EDID_PREFERRED,
				//CUSTOM_TIMING: User need to add new timing here
				XVIDC_VM_1920x1080_60_P_RB,
				//CUSTOM_TIMING: User need to add new timing here
				XVIDC_VM_3840x2160_60_P_RB,
				XVIDC_VM_3840x2160_120_P_RB2,
				XVIDC_VM_7680x4320_30_P_RB,
				XVIDC_VM_7680x4320_30_DELL

};


static char inbyte_local(void);
static u32 xil_gethex(u8 num_chars);
static char XUartPs_RecvByte_NonBlocking(void);
int XVidFrameCrc_Compare(u16 comp1, u16 comp2, u16 comp3,
		XVidC_VideoTimingMode TimingTable, user_config_struct user_config, u8 timeout,
		u8 LineRate, u8 LaneCount);

void DpPt_LaneLinkRateHelpMenu(void)
{
	xil_printf("Choose test option for Lane count and Link rate change\r\n"
	"0 --> train link @ 1.62G 1 lane\r\n"
	"1 --> train link @ 1.62G 2 lanes\r\n"
	"2 --> train link @ 1.62G 4 lanes\r\n"
	"3 --> train link @  2.7G 1 lane\r\n"
	"4 --> train link @  2.7G 2 lanes\r\n"
	"5 --> train link @  2.7G 4 lanes\r\n"
	"6 --> train link @  5.4G 1 lane\r\n"
	"7 --> train link @  5.4G 2 lanes\r\n"
	"8 --> train link @  5.4G 4 lanes\r\n"
	"9 --> train link @  8.1G 1 lane\r\n"
	"a --> train link @  8.1G 2 lanes\r\n"
	"b --> train link @  8.1G 4 lanes\r\n"
	"\r\n"
	"Press 'x' to return to main menu\r\n"
	"Press any key to display this menu again\r\n"

	);
}



void bpc_help_menu(int DPTXSS_BPC_int)
{
xil_printf("Choose Video Bits per color option\r\n"
			"1 -->  8 bpc (24bpp)\r\n");
if (DPTXSS_BPC_int >= 10){
xil_printf("2 --> 10 bpc (30bpp)\r\n");
if (DPTXSS_BPC_int >= 12){
xil_printf("3 --> 12 bpc (36bpp)\r\n");
if (DPTXSS_BPC_int >= 16)
xil_printf("4 --> 16 bpc (48bpp)\r\n");
}
}
	xil_printf(
			"\r\n"
			"Press 'x' to return to main menu\r\n"
			"Press any key to display this menu again\r\n"
	);
}

void format_help_menu(void)
{
	xil_printf("Choose Video Format \r\n"
			   "For YCbCr - Only Color Square Pattern is Supported \r\n"
			   "0 -->  RGB 			\r\n"
			   "1 -->  YCbCR444		\r\n"
			   "2 -->  YCbCr422		\r\n"
			   "\r\n"
			   "Press 'x' to return to main menu\r\n"
			   "Press any key to show this menu again\r\n");
}

void resolution_help_menu(void)
{
xil_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - -"
		" - - - - - - - - - -\r\n"
"-                            Select an Option for Resolutio"
		"n                                      -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - "
		"- - - - - \r\n"
"0 640x480_60_P    |   1 720x480_60_P      |   2 800x600_60_P  \r\n"
"3 1024x768_60_P   |   4 1280x720_60_P     |   5 1600x1200_60_P        \r\n"
"6 1366x768_60_P   |   7 1920x1080_60_P    |   8 3840x2160_30_P\r\n"
"9 3840x2160_60_P  |   a 2560x1600_60_P    |   b 1280x1024_60_P\r\n"
"c 1792x1344_60_P  |   d 848x480_60_P      |   e 1280x960\r\n"
"f 1920x1440_60_P  |   i 3840x2160_60_P_RB |   j 3840x2160_120_P_RB\r\n"
"k 7680x4320_24_P  |   l 7680x4320_30_P"
"- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - -"
		" - - - - - -\r\n"

"Press 'x' to return to main menu \r\n"
"Press any key to show this menu again\r\n");
}

void test_pattern_gen_help()
{
	xil_printf("Choose Video pattern\r\n"
			   "1 -->  Vesa LLC pattern 			\r\n"
			   "3 -->  Vesa Color Squares			\r\n"
			   "4 -->  Flat Red  screen 			\r\n"
			   "5 -->  Flat Blue screen 			\r\n"
			   "6 -->  Flat Green screen 			\r\n"
			   "7 -->  Flat Purple screen 			\r\n"
			   "\r\n"
			   "Press 'x' to return to main menu\r\n"
			   "Press any key to show this menu again\r\n");
}

void app_help()
 {
	xil_printf("\n\n-----------------------------------------------------\r\n");
	xil_printf("--                       Menu                      --\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("\r\n");
	xil_printf(" Select option\r\n");
	xil_printf(" t = Activate Tx Only path (TX uses QPLL) \r\n");
	xil_printf("\r\n");
	xil_printf("-----------------------------------------------------\r\n");
 }

void sub_help_menu(void)
{
  xil_printf(
"- - - - -  -  - - - - - - - - - - - - - - - - - -\r\n"
"-          DisplayPort TX Only Demo Menu        -\r\n"
"- Press 'z' to get this main menu at any point  -\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - -\r\n"
"1 - Change Resolution \r\n"
"2 - Change Bits Per Color \r\n"
"3 - Change Number of Lanes, Link Rate \r\n"
"4 - Change Pattern \r\n"
"5 - Display MSA Values for Tx\r\n"
"6 - Change Format \r\n"
"7 - Display Link Configuration Status and user selected resolution, BPC\r\n"
"8 - Display DPCD register Configurations\r\n"
"9 - Read Auxiliary registers \r\n"
"a - Enable/Disable Audio\r\n"
"d - Power Up/Down sink\r\n"
"e - Read EDID from sink\r\n"
"m - Read CRC checker value\r\n"
"z - Display this Menu again\r\n"
"- - - - - - - - - - - - - - - - - - - - - - - - - \r\n");
}



void select_link_lane(void)
 {

	xil_printf("-----------------------------------------------------\r\n");
	xil_printf("--    Select the Link and Line Capabilities        --\r\n");
	xil_printf("-----------------------------------------------------\r\n");
	  xil_printf("Choose TX capability for Lane count and Link rate\r\n"
	   "0 --> Set TX capability @ 1.62G 1 lane\r\n"
	   "1 --> Set TX capability @ 1.62G 2 lanes\r\n"
	   "2 --> Set TX capability @ 1.62G 4 lanes\r\n"
	   "3 --> Set TX capability @  2.7G 1 lane\r\n"
	   "4 --> Set TX capability @  2.7G 2 lanes\r\n"
	   "5 --> Set TX capability @  2.7G 4 lanes\r\n"
	   "6 --> Set TX capability @  5.4G 1 lane\r\n"
	   "7 --> Set TX capability @  5.4G 2 lanes\r\n"
	   "8 --> Set TX capability @  5.4G 4 lanes\r\n"
	   "9 --> Set TX capability @  8.1G 1 lane\r\n"
	   "a --> Set TX capability @  8.1G 2 lanes\r\n"
	   "b --> Set TX capability @  8.1G 4 lanes\r\n");
	  xil_printf("\r\n");
	  xil_printf("Press 'x' to return to main menu\r\n");
	  xil_printf("Press any key to display this menu\r\n");
	  xil_printf("-----------------------------------------------------\r\n");
 }


void main_loop(){
	/*Test Automation Support Changes*/
	unsigned int pass_cnt;
	unsigned int skip_cnt;
	unsigned int link_oversubscribed;
	unsigned int test_itr_cnt;
	unsigned int test_train_cnt;
	unsigned int fail_cnt;
	unsigned int Total_Tests;
	unsigned int StreamBandwidth;
	unsigned int LinkBandwidth;
	unsigned int Sink_MaxLinkRate;
	unsigned int Sink_MaxLaneCount;
	unsigned int no_of_comp=3;
	u16 comp1_crc;
	u16 comp2_crc;
	u16 comp3_crc;
	u16 crc_wait_cnt;
	u8  crc_timeout;
	XVidC_VideoTimingMode XVidC_VideoTimingMode_Active;
	unsigned int auto_test_mode_exit=0;
	u8 test_retry_cnt=0;
	/*--------------------------------*/

	int i;
	u32 Status;
//	XDpTxSs_Config *ConfigPtr;
	u8 exit = 0;
	char CommandKey;
	char CmdKey[2];
	unsigned int Command;
	u8 LaneCount;
	u8 LineRate;
	u8 LineRate_init_tx = 0;
	u8 Edid_org[128], Edid1_org[128];
	u8 done=0;
	u32 user_tx_LaneCount , user_tx_LineRate;
	u32 aux_reg_address, num_of_aux_registers;
	u8 Data[8];
	u8 audio_on=0;
	XilAudioInfoFrame *xilInfoFrame;
	int m_aud, n_aud;
	u8 in_pwr_save = 0;
	u16 DrpVal =0;
	u8 C_VideoUserStreamPattern[8] = {0x10, 0x11, 0x12, 0x13, 0x14,
												0x15, 0x16, 0x17}; //Duplicate

	unsigned char bpc_table[] = {6,8,10,12,16};


	user_config_struct user_config;
	user_config.user_bpc=8;
	user_config.VideoMode_local=XVIDC_VM_800x600_60_P;
	user_config.user_pattern=1;
	user_config.user_format = XVIDC_CSF_RGB;


	xilInfoFrame = 0; // initialize


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_MAX_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_AdditionalTimingModes,
							(XVIDC_MAX_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}



	sub_help_menu ();

	while (1) { // for menu loop
		if (tx_is_reconnected == 1) {
			hpd_con(&DpTxSsInst, Edid_org, Edid1_org,
					user_config.VideoMode_local);
			tx_is_reconnected = 0;
		}

		if(hpd_pulse_con_event == 1){
			hpd_pulse_con_event = 0;
			hpd_pulse_con(&DpTxSsInst);
			if(!XDpTxSs_CheckLinkStatus(&DpTxSsInst)){
				sink_power_cycle();
			}
		}


		CmdKey[0] = 0;
		CommandKey = 0;

                /*Exit auto test mode after executing the tests once and shift to interactive mode*/
                if(AUTO_TEST_MODE==1 && auto_test_mode_exit==0)
		{
		  CommandKey = 't';
		  auto_test_mode_exit = 1;
		}
		else
		{
		  CommandKey = xil_getc(0xff);
		}

		Command = atoi(&CommandKey);
		if (Command != 0) {
			xil_printf("You have selected command %d\r\n", Command);
		}

		switch (CommandKey){
		case 'e':
			xil_printf ("EDID read is :\r\n");

			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid_org, 0);
			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid_org[i]);
			}

			xil_printf ("\r\r\n\n");
			//reading the second block of EDID
			XDp_TxGetEdidBlock(DpTxSsInst.DpPtr, Edid1_org, 1);

			for (i=0;i<128;i++) {
				if(i%16==0 && i != 0)
					xil_printf("\r\n");
				xil_printf ("%02x ", Edid1_org[i]);
			}
			xil_printf ("\r\nEDID read over =======\r\n");

			break;

		case 'd' :

			if (in_pwr_save == 0) {
				sink_power_down();
				in_pwr_save = 1;
				xil_printf (
					"\r\n==========power down===========\r\n");
			} else {
				sink_power_up();
				in_pwr_save = 0;
				xil_printf (
					"\r\n==========power up===========\r\n");

				hpd_con(&DpTxSsInst, Edid1_org, Edid1_org,
				user_config.VideoMode_local);
			}
			break;


#if ENABLE_AUDIO
		case 'a' :
			audio_on = XDp_ReadReg(
					DpTxSsInst.DpPtr->Config.BaseAddr,
					XDP_TX_AUDIO_CONTROL);
			if (audio_on == 0) {
				xilInfoFrame->audio_channel_count = 0;
				xilInfoFrame->audio_coding_type = 0;
				xilInfoFrame->channel_allocation = 0;
				xilInfoFrame->downmix_inhibit = 0;
				xilInfoFrame->info_length = 27;
				xilInfoFrame->level_shift = 0;
				xilInfoFrame->sample_size = 1;//16 bits
				xilInfoFrame->sampling_frequency = 3; //48 Hz
				xilInfoFrame->type = 4;
				xilInfoFrame->version = 1;
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				sendAudioInfoFrame(xilInfoFrame);
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CHANNELS, 0x1);
				switch(LineRate)
				{
					case  6:m_aud = 24576; n_aud = 162000; break;
					case 10:m_aud = 24576; n_aud = 270000; break;
					case 20:m_aud = 24576; n_aud = 540000; break;
				}
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_MAUD,  m_aud );
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_NAUD,  n_aud );

				Vpg_Audio_start();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x1);
				xil_printf ("Audio enabled\r\n");
				audio_on = 1;
			} else {
				Vpg_Audio_stop();
				XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_AUDIO_CONTROL, 0x0);
				xil_printf ("Audio disabled\r\n");
				audio_on = 0;
			}
			break;
#endif
		case '1' :
			//resolution menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			resolution_help_menu();
			exit = 0;
//			CmdKey[0] = inbyte_local();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];

					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
					xil_printf("You have selected command '%c'\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
							done = 1;
						}

						else if (Command > 47 && Command < 58) {
							Command = Command - 48;
							done = 1;
						}
						else if (Command >= 58 || Command <= 47) {
							resolution_help_menu();
							done = 0;
							break;
						}
						xil_printf ("\r\nSetting resolution...\r\n");
						audio_on = 0;
						user_config.VideoMode_local =
											resolution_table[Command];


						start_tx (LineRate,LaneCount,user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();

						exit = done;
						break;
					}
				}
			}

			sub_help_menu ();
			break;

		case '2' :
			// BPC menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();

			exit = 0;
			bpc_help_menu(DPTXSS_BPC);
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_bpc = bpc_table[Command];
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command>4) || (Command < 0))
							{
								bpc_help_menu(DPTXSS_BPC);
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting BPC of %d\r\n",
												user_config.user_bpc);
								done = 1;
							}
							start_tx (LineRate, LaneCount,user_config);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '3' :
			xil_printf("Select the Link and Lane count\r\n");
			exit = 0;
			select_link_lane();
			while (exit == 0) {
				CmdKey[0] = 0;
				Command = 0;
				CmdKey[0] = inbyte_local();
				if(CmdKey[0]!=0){
					Command = (int)CmdKey[0];
					Command = Command - 48;
					switch  (CmdKey[0])
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
						xil_printf("You have selected command %c\r\n",
															CmdKey[0]);
						if(CmdKey[0] >= 'a' && CmdKey[0] <= 'z'){
							Command = CmdKey[0] -'a' + 10;
						}

						if((Command>=0)&&(Command<12))
						{
							user_tx_LaneCount =
									lane_link_table[Command].lane_count;
							user_tx_LineRate =
									lane_link_table[Command].link_rate;
							if(lane_link_table[Command].lane_count
									> DpTxSsInst.Config.MaxLaneCount)
							{
								xil_printf(
					"This Lane Count is not supported by Sink \r\n");
								xil_printf(
					"Max Supported Lane Count is 0x%x \r\n",
										DpTxSsInst.Config.MaxLaneCount);
								xil_printf(
					"Training at Supported Lane count  \r\n");
							LaneCount = DpTxSsInst.Config.MaxLaneCount;
							}
							done = 1;
						}
						else
						{
							xil_printf(
							"!!!Warning: You have selected wrong option"
							" for lane count and link rate\r\n");
							select_link_lane();
							done = 0;
							break;
						}
						// Disabling TX interrupts
						XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
										XDP_TX_INTERRUPT_MASK, 0xFFF);
						LineRate_init_tx = user_tx_LineRate;
						Status = set_vphy(LineRate_init_tx);

						XDpTxSs_Stop(&DpTxSsInst);
						audio_on = 0;
						xil_printf(
					"TX Link & Lane Capability is set to %x, %x\r\n",
					user_tx_LineRate, user_tx_LaneCount);
						xil_printf(
					"Setting TX to 8 BPC and 800x600 resolution\r\n");
						XDpTxSs_Reset(&DpTxSsInst);
						user_config.user_bpc=8;
						user_config.VideoMode_local
										=XVIDC_VM_800x600_60_P;
						user_config.user_pattern=1;
						user_config.user_format = XVIDC_CSF_RGB;
						start_tx (user_tx_LineRate, user_tx_LaneCount,
												user_config);
						LineRate = get_LineRate();
						LaneCount = get_Lanecounts();
						exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '4' :
			//pattern menu;
			test_pattern_gen_help();
			exit = 0;
			while (exit == 0) {
				CommandKey = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					Command = Command - 48;
					switch  (CommandKey)
					{
						case 'x' :
							exit = 1;
							sub_help_menu ();
							break;

						default :

							if(Command>0 && Command<8)
							{
								xil_printf(
								"You have selected video pattern %d "
								"from the pattern list \r\n", Command);
								done = 1;
							}
							else
							{
								xil_printf(
						"!!!Warning : Invalid pattern selected \r\n");
								test_pattern_gen_help();
								done = 0;
								break;
							}
							user_config.user_pattern = Command;
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							exit = done;
							break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '5' :
			//MSA;
			XDpTxSs_ReportMsaInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '6' :
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();
			exit = 0;
			format_help_menu();
			while (exit == 0) {
				CommandKey = 0;
				Command = 0;
				CommandKey = inbyte_local();
				if(CommandKey!=0){
					Command = (int)CommandKey;
					switch  (CommandKey)
					{
					   case 'x' :
					   exit = 1;
					   sub_help_menu ();
					   break;

					   default :
							Command = Command - 48;
							user_config.user_format = Command;
							xil_printf("You have selected %c\r\n",
															CommandKey);
							if((Command<=0)||(Command>3))
							{
								format_help_menu();
								done = 0;
								break;
							}
							else
							{
								xil_printf("Setting Format of %d\r\n",
											user_config.user_format);
								done = 1;
							}
							if(user_config.user_format!=1)
							{
							//Only Color Square is supported for YCbCr
								user_config.user_pattern = 3;
							}
							else
							{
								//Set Color Ramp for RGB (default)
								user_config.user_pattern = 1;
							}
							Vpg_VidgenSetUserPattern(DpTxSsInst.DpPtr,
									C_VideoUserStreamPattern[
											user_config.user_pattern]);
							start_tx (LineRate, LaneCount,user_config);
							LineRate = get_LineRate();
							LaneCount = get_Lanecounts();
							exit = done;
						break;
					}
				}
			}
			sub_help_menu ();
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '7' :
			//Link config and status
			XDpTxSs_ReportLinkInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '8' :
			//Display DPCD reg
			XDpTxSs_ReportSinkCapInfo(&DpTxSsInst);
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		case '9' :
			//"9 - Read Aux registers\r\n"
			xil_printf(
		"\r\n Give 4 bit Hex value of base register 0x");
			aux_reg_address = xil_gethex(4);
			xil_printf(
			  "\r\n Give msb 2 bit Hex value of base register 0x");
			aux_reg_address |= ((xil_gethex(2)<<16) & 0xFFFFFF);
			xil_printf("\r\n Give number of registers that you "
								"want to read (1 to 9): ");
			num_of_aux_registers = xil_gethex(1);
			if((num_of_aux_registers<1)||(num_of_aux_registers>9))
			{
					xil_printf("\r\n!!!Warning: Invalid number "
				   "selected, hence reading only one register\r\n");
					num_of_aux_registers = 1;
			}
			xil_printf("\r\nGiven base address offset is 0x%x\r\n",
										aux_reg_address);
			for(i=0;i<num_of_aux_registers;i++)
			{
					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
						(aux_reg_address+i), 1, &Data);
					if(Status == XST_SUCCESS)
					{
							xil_printf("Value at address offset "
						"0x%x, is = 0x%x\r\n",
										(aux_reg_address+i),
										((Data[0]) & 0xFF));
					} else {
							xil_printf("Aux Read failure\r\n");
							break;
					}
			}
			break;

			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */

		// Display VideoPHY status
		case 'b':
			xil_printf("Video PHY Config/Status --->\r\n");
			xil_printf(" RCS (0x10) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_REF_CLK_SEL_REG));
			xil_printf(" PR  (0x14) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_RESET_REG));
			xil_printf(" PLS (0x18) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_PLL_LOCK_STATUS_REG));
			xil_printf(" TXI (0x1C) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_REG));
			xil_printf(" TXIS(0x20) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_TX_INIT_STATUS_REG));
			xil_printf(" RXI (0x24) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_REG));
			xil_printf(" RXIS(0x28) = 0x%x\r\n",
			  XVphy_ReadReg(VPhyInst.Config.BaseAddr,
					  XVPHY_RX_INIT_STATUS_REG));

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_FBDIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_FBDIV) "
				"= 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_FBDIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_CPLL_REFCLK_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_CPLL_REFCLK_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_CPLL_REFCLK_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_RXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_RXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_RXOUT_DIV,
				DrpVal
			);

			Status = XVphy_DrpRd(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
					XVPHY_DRP_TXOUT_DIV, &DrpVal);
			xil_printf(" GT DRP Addr (XVPHY_DRP_TXOUT_DIV)"
				" = 0x%x, Val = 0x%x\r\n",XVPHY_DRP_TXOUT_DIV,
				DrpVal
			);

			break;


			// CRC read
		case 'm' :
			XVidFrameCrc_Report();
			/* Start TEST_CRC in sink*/
			Data[0] = 0x1;
			Status = XDp_TxAuxWrite(DpTxSsInst.DpPtr,
					DPCD_TEST_SINK_START, 1, &Data);

			/*Wait till CRC is available or timeout*/
			crc_wait_cnt=0;
			while(1)
			{
				/* Read CRC availability every few ms*/
				Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					DPCD_TEST_SINK_MISC, 1, &Data);
				DpPt_CustomWaitUs(&DpTxSsInst,10000);
				if((Data[0]&0x0F)!=0)
				{
					xil_printf("Sink CRC - Available...\r\n");
					crc_timeout = 0;
					break;
				}
				else if(crc_wait_cnt==CRC_AVAIL_TIMEOUT)
				{
					xil_printf("Sink CRC - Timed Out...\r\n");
					crc_timeout = 1;
					break;
				}
				else
				{
					crc_wait_cnt++;
				}

			}

			/*Wait time so that Tx and Rx has enough time to calculate CRC values*/
			DpPt_CustomWaitUs(&DpTxSsInst,100000);

			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					DPCD_TEST_CRC_R_Cr, 6, &Data);

			comp1_crc = Data[0] | (Data[1]<<8);
			comp2_crc = Data[2] | (Data[3]<<8);
			comp3_crc = Data[4] | (Data[5]<<8);

			xil_printf ("**** Sink CRC Values ****\r\n");
			xil_printf ("CRC - R/Cr   =  0x%x\r\n", comp1_crc);
			xil_printf ("CRC - G/Y    =  0x%x\r\n", comp2_crc);
			xil_printf ("CRC - B/Cb   =  0x%x\r\n", comp3_crc);
			
			break;

		case 't' :
			//resolution menu
			LineRate = get_LineRate();
			LaneCount = get_Lanecounts();

			xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
									(XVIDC_MAX_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
			Status = XVidC_RegisterCustomTimingModes(XVidC_AdditionalTimingModes,
									(XVIDC_MAX_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
			if (Status != XST_SUCCESS) {
				xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
			}

			/*Use it to enable BS-BS timing*/
			/*Not a default option*/
//			DpTxSsInst.UsrOpt.VtcAdjustBs = 1;

			/* Read Sink Capablity*/
			Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
					DPCD_MAX_LINE_RATE, 2, &Data);
			Sink_MaxLinkRate = Data[0];
			Sink_MaxLaneCount = Data[1]&0x1F;

			user_config.user_bpc = XVIDC_BPC_8;
			user_config.user_format = TEST_RGB_FORMAT;
			user_config.user_pattern = TEST_VID_COLOR_RAMP;

			Command = XVIDC_VM_640x350_85_P; //Progressive with Standard and RB timings

			Total_Tests = ((XVIDC_VM_NUM_SUPPORTED-XVIDC_VM_640x350_85_P-1)+ //Video tests from table
					    (XVIDC_MAX_NUM_SUPPORTED-XVIDC_VM_CUSTOM-1))* //Video tests - Custom
							(NO_OF_TEST_ITERATIONS-1) +
							NO_OF_TRAINING_COMBOS;      //Training tests
			pass_cnt = 0;
			fail_cnt = 0;
			skip_cnt = 0;
			crc_timeout = 0;
			link_oversubscribed = 0;
			test_itr_cnt=0;
			test_train_cnt=0;
			xil_printf("VID_RESULT, result, id, hres, vres, refresh, bpc, format, src_comp1, "
					"src_comp2, src_comp3, snk_comp1, snk_comp2, snk_comp3, LineRate, LaneCount\r\n");
			exit = 0;
			CommandKey = 0;
			while(exit==0)
			{
				/*Logic to break the loop*/
				CommandKey = inbyte_local();
				if(CommandKey=='x')
				{
					xil_printf("**** User Stopped Test Routine ****\r\n");
					exit = 1;
				}

					/*Test Iteration
					 * 422 mode is not automated since Sinks calculate CRC differently and some does not support
					 * For 422, industry uses visual check
					 * 0: Training Tests (All Combos)
					 * 1: RGB, 8 BPC
					 * 2: YCbCr444, 8 BPC
					 * 3: RGB, 10 BPC
					 * 4: YCbCr444, 10 BPC
					 * 5: YCbCr422, 8 BPC
					 * 6: YCbCr422, 10 BPC
					 * 7: Exit
					 * */

				    if(test_itr_cnt==0)
				    {
				    	/*Shift to Video Tests*/
				    	if(test_train_cnt==NO_OF_TRAINING_COMBOS)
				    	{
							LaneCount = Sink_MaxLaneCount;
							LineRate  = Sink_MaxLinkRate;
							Command = XVIDC_VM_640x350_85_P; //Progressive with Standard and RB timings
							test_itr_cnt = 1;
				    	}
				    	else
				    	{
							LaneCount = lane_link_table[test_train_cnt].lane_count;
							LineRate  = lane_link_table[test_train_cnt].link_rate;
							Command = XVIDC_VM_640x480_60_P; //Default
				    	}
				    	test_train_cnt++;
				    }

				    if(Command==XVIDC_MAX_NUM_SUPPORTED)
					{

						test_itr_cnt++;
						/*Max supported BPC in design examples is 10.
						 * User can change to MAX value as per system*/
						if(test_itr_cnt == NO_OF_TEST_ITERATIONS)
						{
							xil_printf("************** TESTS COMPLETED **************\r\n");
							xil_printf("************** %d / %d Passed************\r\n",
									pass_cnt,Total_Tests-skip_cnt);
							xil_printf("*********************************************\r\n");
							xil_printf("VID_RESULT, Pass=> %d/%d , Fail=> %d/%d\r\n",
									pass_cnt,Total_Tests-skip_cnt,
									fail_cnt-skip_cnt,Total_Tests-skip_cnt);
							xil_printf("VID_RESULT, %d Percent PASSED\r\n",
									(pass_cnt*100/(Total_Tests-skip_cnt)));
							break;
						}

						Command = XVIDC_VM_640x350_85_P; //Progressive with Standard and RB timings
						LaneCount = Sink_MaxLaneCount;
						LineRate  = Sink_MaxLinkRate;

						switch(test_itr_cnt)
						{
							case 1:
								user_config.user_bpc = XVIDC_BPC_8;
								user_config.user_format = TEST_RGB_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_RAMP;
							break;

							case 2:
								user_config.user_bpc = XVIDC_BPC_8;
								user_config.user_format = TEST_YCBCR444_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_SQUARES;
							break;

							case 3:
								user_config.user_bpc = XVIDC_BPC_10;
								user_config.user_format = TEST_RGB_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_RAMP;
							break;

							case 4:
								user_config.user_bpc = XVIDC_BPC_10;
								user_config.user_format = TEST_YCBCR444_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_SQUARES;
							break;

							case 5:
								user_config.user_bpc = XVIDC_BPC_8;
								user_config.user_format = TEST_YCBCR422_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_SQUARES;
							break;

							case 6:
								user_config.user_bpc = XVIDC_BPC_10;
								user_config.user_format = TEST_YCBCR422_FORMAT;
								user_config.user_pattern = TEST_VID_COLOR_SQUARES;
							break;

							default:
								xil_printf("Wrong Test Option!!!\r\n");
							break;
						}
					}

				    /*Not applicable for training tests as 640x480 is used as default*/
				    if(test_itr_cnt!=0)
				    {
						/*Special case for 1366 as it is not multiple of 4.*/
						if(Command==XVIDC_VM_1366x768_60_P && LaneCount==4)
						{
							LaneCount = 2;
						}
						else
						{
							LaneCount = Sink_MaxLaneCount;
						}
						LineRate  = Sink_MaxLinkRate;
				    }

				    if(Command>=XVIDC_VM_CUSTOM)
				    {
						XVidC_VideoTimingMode_Active.FrameRate = XVidC_AdditionalTimingModes[Command-XVIDC_VM_CUSTOM-1].FrameRate;
						XVidC_VideoTimingMode_Active.Timing = XVidC_AdditionalTimingModes[Command-XVIDC_VM_CUSTOM-1].Timing;
						XVidC_VideoTimingMode_Active.VmId = XVidC_AdditionalTimingModes[Command-XVIDC_VM_CUSTOM-1].VmId;
				    }
				    else
				    {
						XVidC_VideoTimingMode_Active.FrameRate = XVidC_VideoTimingModes_Tests[Command].FrameRate;
						XVidC_VideoTimingMode_Active.Timing = XVidC_VideoTimingModes_Tests[Command].Timing;
						XVidC_VideoTimingMode_Active.VmId = XVidC_VideoTimingModes_Tests[Command].VmId;
				    }

					/*Stream Bandwidth Calculation*/
				    if(user_config.user_format == TEST_YCBCR422_FORMAT)
				    {
				    	no_of_comp = 2;
				    }
				    else
				    {
				    	no_of_comp = 3;
				    }

					StreamBandwidth = ( ((XVidC_VideoTimingMode_Active.Timing.HTotal *
							XVidC_VideoTimingMode_Active.Timing.F0PVTotal *
							XVidC_VideoTimingMode_Active.FrameRate)/1000000) *
							(user_config.user_bpc * no_of_comp))/8/LaneCount;

					user_config.VideoMode_local =
							XVidC_VideoTimingMode_Active.VmId;

					/*Train Sink and send video*/
					Status = start_tx (LineRate,LaneCount,user_config);
					if(Status == XST_FAILURE)
					{
						xil_printf("VID_RESULT, Transmitter Training Failed"
								"@ LineRate=0x%x, LaneCount=0x%x...\r\n",
								LineRate, LaneCount);
						break;
					}

					/*Skip the test in report, if over subscribed*/
					LinkBandwidth   = (LineRate*27);
					xil_printf("StreamBandwidth=%d, LinkBandwidth=%d\r\n",
							StreamBandwidth,LinkBandwidth);
					if(StreamBandwidth>LinkBandwidth)
					{
						link_oversubscribed = 1;
						skip_cnt++;
					}
					else
					{
						link_oversubscribed = 0;
					}

					/* Read Trained Config*/
					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							DPCD_LINE_RATE, 2, &Data);
					LineRate = Data[0];
					LaneCount = Data[1]&0x1F;
			

					/* Start TEST_CRC in sink*/
					Data[0] = 0x1;
					Status = XDp_TxAuxWrite(DpTxSsInst.DpPtr,
							DPCD_TEST_SINK_START, 1, &Data);

					/*Wait till CRC is available or timeout*/
					crc_wait_cnt=0;
					while(1)
					{
						/* Read CRC availability every few ms*/
						Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							DPCD_TEST_SINK_MISC, 1, &Data);
						DpPt_CustomWaitUs(&DpTxSsInst,10000);
						if((Data[0]&0x0F)!=0)
						{
							xil_printf("Sink CRC - Available...\r\n");
							crc_timeout = (link_oversubscribed)?2:0;
							break;
						}
						else if(crc_wait_cnt==CRC_AVAIL_TIMEOUT)
						{
							xil_printf("Sink CRC - Timed Out...\r\n");
							crc_timeout = (link_oversubscribed)?2:1;
							break;
						}
						else
						{
							crc_wait_cnt++;
						}

					}

					/*Wait time so that Tx and Rx has enough time to calculate CRC values*/
					DpPt_CustomWaitUs(&DpTxSsInst,100000);

					Status = XDp_TxAuxRead(DpTxSsInst.DpPtr,
							DPCD_TEST_CRC_R_Cr, 6, &Data);

					comp1_crc = Data[0] | (Data[1]<<8);
					comp2_crc = Data[2] | (Data[3]<<8);
					comp3_crc = Data[4] | (Data[5]<<8);


					/*Name is defined as constant in Struct of video timing and hence using if-else*/
				    if(Command>=XVIDC_VM_CUSTOM)
				    {
						Status = XVidFrameCrc_Compare(comp1_crc, comp2_crc, comp3_crc,
								XVidC_AdditionalTimingModes[Command-XVIDC_VM_CUSTOM-1],
								user_config, crc_timeout, LineRate, LaneCount);
				    }
				    else
				    {
						Status = XVidFrameCrc_Compare(comp1_crc, comp2_crc, comp3_crc,
								XVidC_VideoTimingModes_Tests[Command],
								user_config, crc_timeout, LineRate, LaneCount);
				    }

				    /*If test failed, start retry counter with Max limit to TEST_RETRY_COUNT*/
				    test_retry_cnt = (test_retry_cnt==TEST_RETRY_COUNT ||
				    		Status==XST_SUCCESS || crc_timeout==2)
				    				?0:test_retry_cnt+1;

				    /*Go to next resolution only after retry expired OR retry mode is not enabled*/
				    if ( (test_retry_cnt==0 && TEST_RETRY_MODE==1) || (TEST_RETRY_MODE==0))
				    {
				    	Command++;
				    }

				    /*Update total test count as per retries*/
				    if(test_retry_cnt!=0)
				    {
				    	Total_Tests = Total_Tests + 1;
				    }

				    /*Start Custom Resolutions*/
				    if(Command==XVIDC_VM_NUM_SUPPORTED-1)
				    {
				    	Command = XVIDC_VM_7680x1080_60_P_RB;
				    }

					if(Status==XST_SUCCESS)
					{
						pass_cnt++;
					}
					else
					{
						fail_cnt++;
					}

			                /*Print Video Config Data - Used for Debug*/
			                XDpTxSs_ReportMsaInfo(&DpTxSsInst);
					xil_printf("XDP_TX_LINE_RESET_DISABLE = 0x%x\r\n",
							XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
									XDP_TX_LINE_RESET_DISABLE));					

					xil_printf("\r\n---> Test Status: PASS:: %d / %d , FAIL:: %d / %d, SKIPPED:: %d\r\n",
							pass_cnt,Total_Tests-skip_cnt,fail_cnt-skip_cnt,Total_Tests-skip_cnt,skip_cnt);
					xil_printf("--------------------------------------------------------------\r\n");

					/* Stop TEST_CRC in sink*/
					Data[0] = 0x0;
					Status = XDp_TxAuxWrite(DpTxSsInst.DpPtr,
							DPCD_TEST_SINK_START, 1, &Data);

					/*Wait time and then start next video*/
					DpPt_CustomWaitUs(&DpTxSsInst,100000);

			}//end while


			break;


			/* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^* *^*  */
		case 'z' :
			sub_help_menu ();
			break;

		} //end of switch (CmdKey[0])
	}
}


static char inbyte_local(void){
	char c=0;
	c = XUartPs_RecvByte_NonBlocking();
	return c;
}


/*****************************************************************************/
/**
*
* This function to convert integer to hex
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
static u32 xil_gethex(u8 num_chars){
	u32 data;
	u32 i;
	u8 term_key;
	data = 0;

	for(i=0;i<num_chars;i++){
		term_key = xil_getc(0);
		xil_printf("%c",term_key);
		if(term_key >= 'a') {
			term_key = term_key - 'a' + 10;
		} else if(term_key >= 'A') {
				term_key = term_key - 'A' + 10;
		} else {
			term_key = term_key - '0';
		}
		data = (data << 4) + term_key;
	}
	return data;
}


/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
static char XUartPs_RecvByte_NonBlocking(){
    u32 RecievedByte;
    RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
    /* Return the byte received */
    return (u8)RecievedByte;
}



/*****************************************************************************/
/**
*
* This function to get uart input from user
*
* @param	timeout_ms
*
* @return
*		- received charactor
*
* @note		None.
*
******************************************************************************/
char xil_getc(u32 timeout_ms){
	char c;
	u32 timeout = 0;

	extern XTmrCtr TmrCtr;

	// Reset and start timer
	if ( timeout_ms > 0 && timeout_ms != 0xff ){
	  XTmrCtr_Start(&TmrCtr, 0);
	}


	while((!XUartPs_IsReceiveData(STDIN_BASEADDRESS)) && (timeout == 0)){
		if ( timeout_ms == 0 ){ // no timeout - wait for ever
		   timeout = 0;
		} else if ( timeout_ms == 0xff ) { // no wait - special case
		   timeout = 1;
		} else if(timeout_ms > 0){
			if(XTmrCtr_GetValue(&TmrCtr, 0)
										> ( timeout_ms * (100000000 / 1000) )){
				timeout = 1;
			}
		}
	}
	if(timeout == 1){
		c = 0;
	} else {
		c = XUartPs_RecvByte_NonBlocking();
	}
	return c;
}

/*****************************************************************************/
/**
*
* This function compares CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int XVidFrameCrc_Compare(u16 comp1, u16 comp2, u16 comp3,
		XVidC_VideoTimingMode TimingTable, user_config_struct user_config,
		u8 timeout, u8 LineRate, u8 LaneCount)
{
	u16 calc_comp1;
	u16 calc_comp2;
	u16 calc_comp3;

	u16 src_comp1;
	u16 src_comp2;
	u16 src_comp3;

	u16 snk_comp1;
	u16 snk_comp2;
	u16 snk_comp3;

	u16 crc_match;

	calc_comp1 = XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
			VIDEO_FRAME_CRC_VALUE_G_R)
			& VIDEO_FRAME_CRC_R_Y_COMP_MASK;

	calc_comp2 = (XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
			VIDEO_FRAME_CRC_VALUE_G_R)
			& VIDEO_FRAME_CRC_G_CR_COMP_MASK)
			>> VIDEO_FRAME_CRC_G_CR_COMP_SHIFT;

	calc_comp3 = XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
			VIDEO_FRAME_CRC_VALUE_B)
			& VIDEO_FRAME_CRC_B_CB_COMP_MASK;

	/*Special Case - For 422, CRCs are calculated on AXI4S and
	 * need to map as per Sink Mapping*/
	if(user_config.user_format == TEST_YCBCR422_FORMAT)
	{
		src_comp1 = calc_comp1;
		src_comp2 = 0x0;
		src_comp3 = calc_comp3;

		snk_comp1 = comp3;
		snk_comp2 = 0x0;
		snk_comp3 = comp2;
	}
	else
	{
		src_comp1 = calc_comp1;
		src_comp2 = calc_comp2;
		src_comp3 = calc_comp3;

		snk_comp1 = comp1;
		snk_comp2 = comp2;
		snk_comp3 = comp3;
	}

	if((src_comp1==snk_comp1) && (src_comp2==snk_comp2) && (src_comp3==snk_comp3))
		crc_match = 1;
	else
		crc_match = 0;

	if( (crc_match==1) && (timeout==0))
	{
		xil_printf("VID_RESULT, PASSED, %s, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, %d\r\n",
				TimingTable.Name, TimingTable.Timing.HActive, TimingTable.Timing.VActive,
				TimingTable.FrameRate, user_config.user_bpc, user_config.user_format,
				src_comp1,src_comp2,src_comp3, snk_comp1, snk_comp2,snk_comp3,LineRate,LaneCount);
		return (XST_SUCCESS);
	}
	else if((timeout==2))
	{
		xil_printf("VID_RESULT, SKIPPED, %s, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, %d\r\n",
				TimingTable.Name, TimingTable.Timing.HActive, TimingTable.Timing.VActive,
				TimingTable.FrameRate, user_config.user_bpc, user_config.user_format,
				src_comp1,src_comp2,src_comp3, snk_comp1, snk_comp2,snk_comp3,LineRate,LaneCount);
		return (XST_FAILURE);
	}
	else
	{
		xil_printf("VID_RESULT, FAILED, %s, %d, %d, %d, %d, %d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, %d\r\n",
				TimingTable.Name, TimingTable.Timing.HActive, TimingTable.Timing.VActive,
				TimingTable.FrameRate, user_config.user_bpc, user_config.user_format,
				src_comp1,src_comp2,src_comp3, snk_comp1, snk_comp2,snk_comp3,LineRate,LaneCount);
		return (XST_FAILURE);
	}

}
