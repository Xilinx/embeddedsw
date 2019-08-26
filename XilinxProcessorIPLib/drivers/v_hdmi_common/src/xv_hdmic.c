/*******************************************************************************
 *
 * Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xhdmic.c
 * @addtogroup hdmi_common_v1_2
 * @{
 *
 * Contains common utility functions that are typically used by hdmi-related
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   EB  21/12/17 Initial release.
 * 1.1   EB  10/04/18 Fixed a bug in XV_HdmiC_ParseAudioInfoFrame
 * 1.2   EB   15/08/19 Added enumeration for HDMI 2.1 Support
 *       mmo  15/08/19 Updated the VIC table to support HDMI 2.1 Resolution
 *                     Added Audio ACR CTS/N Enumeration and Library
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/
#include "xv_hdmic.h"

/************************** Constant Definitions ******************************/

/*****************************************************************************/
/**
* This table contains the attributes for various standard resolutions.
* Each entry is of the format:
* 1) Resolution ID
* 2) Video Identification Code.
*/
const XHdmiC_VicTable VicTable[VICTABLE_SIZE] = {
    {XVIDC_VM_640x480_60_P, 1},      /* Vic 1 */
    {XVIDC_VM_720x480_60_P, 2},      /* Vic 2 */
    {XVIDC_VM_720x480_60_P, 3},      /* Vic 3 */
    {XVIDC_VM_1280x720_60_P, 4},     /* Vic 4 */
    {XVIDC_VM_1920x1080_60_I, 5},    /* Vic 5 */
    {XVIDC_VM_1440x480_60_I, 6},     /* Vic 6 */
    {XVIDC_VM_1440x480_60_I, 7},     /* Vic 7 */

    {XVIDC_VM_1920x1080_60_P, 16},   /* Vic 16 */
    {XVIDC_VM_720x576_50_P, 17},     /* Vic 17 */
    {XVIDC_VM_720x576_50_P, 18},     /* Vic 18 */
    {XVIDC_VM_1280x720_50_P, 19},    /* Vic 19 */
    {XVIDC_VM_1920x1080_50_I, 20},   /* Vic 20 */
    {XVIDC_VM_1440x576_50_I, 21},    /* Vic 21 */
    {XVIDC_VM_1440x576_50_I, 22},    /* Vic 22 */

    /* 1280 x 720 */
    {XVIDC_VM_1280x720_24_P, 65},    /* Vic 65 */
    {XVIDC_VM_1280x720_25_P, 66},    /* Vic 66 */
    {XVIDC_VM_1280x720_30_P, 67},    /* Vic 67 */
    {XVIDC_VM_1280x720_50_P, 68},    /* Vic 68 */
    {XVIDC_VM_1280x720_60_P, 69},    /* Vic 69 */
    {XVIDC_VM_1280x720_100_P, 70},   /* Vic 70 */
    {XVIDC_VM_1280x720_120_P, 71},   /* Vic 71 */

    /* 1680 x 720 */
    {XVIDC_VM_1680x720_50_P, 82},    /* Vic 82 */
    {XVIDC_VM_1680x720_60_P, 83},    /* Vic 83 */
    {XVIDC_VM_1680x720_100_P, 84},   /* Vic 84 */
    {XVIDC_VM_1680x720_120_P, 85},   /* Vic 85 */

    /* 1920 x 1080 */
    {XVIDC_VM_1920x1080_24_P, 32},   /* Vic 32 */
    {XVIDC_VM_1920x1080_25_P, 33},   /* Vic 33 */
    {XVIDC_VM_1920x1080_30_P, 34},   /* Vic 34 */
    {XVIDC_VM_1920x1080_50_P, 31},   /* Vic 31 */
    {XVIDC_VM_1920x1080_100_P, 64},  /* Vic 64 */
    {XVIDC_VM_1920x1080_120_P, 63},  /* Vic 63 */

    /* 2560 x 1080 */
    {XVIDC_VM_2560x1080_24_P, 86},   /* Vic 86 */
    {XVIDC_VM_2560x1080_25_P, 87},   /* Vic 87 */
    {XVIDC_VM_2560x1080_30_P, 88},   /* Vic 88 */
    {XVIDC_VM_2560x1080_50_P, 89},   /* Vic 89 */
    {XVIDC_VM_2560x1080_60_P, 90},   /* Vic 90 */
    {XVIDC_VM_2560x1080_100_P, 91},  /* Vic 91 */
    {XVIDC_VM_2560x1080_120_P, 92},  /* Vic 92 */

    /* 3840 x 2160 */
    {XVIDC_VM_3840x2160_24_P, 93},   /* Vic 93  */
    {XVIDC_VM_3840x2160_25_P, 94},   /* Vic 94  */
    {XVIDC_VM_3840x2160_30_P, 95},   /* Vic 95  */
    {XVIDC_VM_3840x2160_50_P, 96},   /* Vic 96  */
    {XVIDC_VM_3840x2160_60_P, 97},   /* Vic 97  */
    {XVIDC_VM_3840x2160_100_P, 117}, /* Vic 117 */
    {XVIDC_VM_3840x2160_120_P, 118}, /* Vic 118 */

    /* 4096 x 2160 */
    {XVIDC_VM_4096x2160_24_P, 98},   /* Vic 98  */
    {XVIDC_VM_4096x2160_25_P, 99},   /* Vic 99  */
    {XVIDC_VM_4096x2160_30_P, 100},  /* Vic 100 */
    {XVIDC_VM_4096x2160_50_P, 101},  /* Vic 101 */
    {XVIDC_VM_4096x2160_60_P, 102},  /* Vic 102 */
    {XVIDC_VM_4096x2160_100_P, 218}, /* Vic 218 */
    {XVIDC_VM_4096x2160_120_P, 219}, /* Vic 219 */

    /* 5120 x 2160 */
    {XVIDC_VM_5120x2160_24_P, 121},  /* Vic 121 */
    {XVIDC_VM_5120x2160_25_P, 122},  /* Vic 122 */
    {XVIDC_VM_5120x2160_30_P, 123},  /* Vic 123 */
    {XVIDC_VM_5120x2160_50_P, 125},  /* Vic 125 */
    {XVIDC_VM_5120x2160_60_P, 126},  /* Vic 126 */
    {XVIDC_VM_5120x2160_100_P, 127}, /* Vic 127 */
    {XVIDC_VM_5120x2160_120_P, 193}, /* Vic 193 */

    /* 7680 x 4320 */
    {XVIDC_VM_7680x4320_24_P, 194},  /* Vic 194 */
    {XVIDC_VM_7680x4320_25_P, 195},  /* Vic 195 */
    {XVIDC_VM_7680x4320_30_P, 196},  /* Vic 196 */
    {XVIDC_VM_7680x4320_50_P, 198},  /* Vic 198 */
    {XVIDC_VM_7680x4320_60_P, 199},  /* Vic 199 */
    {XVIDC_VM_7680x4320_100_P, 200}, /* Vic 200 */
    {XVIDC_VM_7680x4320_120_P, 201}, /* Vic 201 */

    /* 10240 x 4320 */
    {XVIDC_VM_10240x4320_24_P, 210},  /* Vic 210 */
    {XVIDC_VM_10240x4320_25_P, 211},  /* Vic 211 */
    {XVIDC_VM_10240x4320_30_P, 212},  /* Vic 212 */
    {XVIDC_VM_10240x4320_50_P, 214},  /* Vic 214 */
    {XVIDC_VM_10240x4320_60_P, 215},  /* Vic 215 */
    {XVIDC_VM_10240x4320_100_P, 216}, /* Vic 216 */
    {XVIDC_VM_10240x4320_120_P, 217}, /* Vic 217 */
};

/**
* This table contains the attributes for various FRL Rate.
* Each entry is of the format:
* 1) Number of lanes
* 2) Line Rate.
*/
const XHdmiC_FrlRate FrlRateTable[] = {
    {3, 0},	/* XHDMIC_MAXFRLRATE_NOT_SUPPORTED */
    {3, 3},	/* XHDMIC_MAXFRLRATE_3X3GBITSPS */
    {3, 6},	/* XHDMIC_MAXFRLRATE_3X6GBITSPS */
    {4, 6},	/* XHDMIC_MAXFRLRATE_4X6GBITSPS */
    {4, 8},	/* XHDMIC_MAXFRLRATE_4X8GBITSPS */
    {4, 10},	/* XHDMIC_MAXFRLRATE_4X10GBITSPS */
    {4, 12},	/* XHDMIC_MAXFRLRATE_4X12GBITSPS */
};

/**
* This table contains the CTS and N value of each TMDS character rate.
* Each entry is of the format:
* 1) Sample Frequency TMDS CLk, (32kHz, 44.1kHz, and 48kHz)
*/
const XHdmiC_TMDS_N_Table TMDSChar_N_Table[] =
{
	/* TMDSClk    32k   44k1   48k   88k2    96k  176k4   192k */
	{        0, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 25200000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 27000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 31500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 33750000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 37800000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 40500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 50400000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 54000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 67500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 74250000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 81000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 92812500, { 8192, 6272, 12288, 12544, 24576, 25088, 49152}},
	{108000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{111375000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{148500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{185625000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{222750000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{297000000, { 3072, 4704,  5120,  9408, 10240, 18816, 20480}},
	{371250000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{445500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{594000000, { 3072, 9408,  6144, 18816, 12288, 37632, 24576}}
};

/**
* This returns the the N Value based Audio Sampling Rate and TMDS
* Character Rate
*/
u32 XHdmiC_TMDS_GetNVal(u32 TMDSCharRate,
	XHdmiC_SamplingFrequency AudSampleFreq)
{
  XHdmiC_TMDS_N_Table const *item;
  u32 i = 0;

  /* Proceed only with supported Audio Sampling Frequency */
  if (AudSampleFreq == XHDMIC_SAMPLING_FREQUENCY ||
                  AudSampleFreq > XHDMIC_SAMPLING_FREQUENCY_192K) {
          /* Since AudSampFreq not valid, return N Value = 0 */
          return 0;
  }

  for(i = 0; i < sizeof(TMDSChar_N_Table)/sizeof(XHdmiC_TMDS_N_Table); i++) {
    item = &TMDSChar_N_Table[i];
    /* 10kHz  Tolerance */
	if(TMDSCharRate <= (item->TMDSCharRate + 10000) &&
			(TMDSCharRate >= (item->TMDSCharRate - 10000))) {
      return item->ACR_NVal[AudSampleFreq-1];
    }
  }

  /* If TMDS character rate could not be found return default values */
  item = &TMDSChar_N_Table[0];

  return item->ACR_NVal[AudSampleFreq-1];
}

/**
* This returns the Audio Sampling Rate and TMDS
* Character Rate
*/
XHdmiC_SamplingFrequency XHdmiC_TMDS_GetAudSampFreq(u32 TMDSCharRate,
		u32 N)
{
  XHdmiC_TMDS_N_Table const *item;
  u8 i = 0;
  u8 j = 0;

  for(i = 0; i < sizeof(TMDSChar_N_Table)/sizeof(XHdmiC_TMDS_N_Table); i++) {
		item = &TMDSChar_N_Table[i];
		if(TMDSCharRate <= (item->TMDSCharRate + 10000) &&
			(TMDSCharRate >= (item->TMDSCharRate - 10000))) {
			for (j = 0; j < 7; j++) {
				if(N == item->ACR_NVal[j]) {
					return j;
					break;
				}
			}
		}
  }
  /* Return non-valid sample frequency */
  return XHDMIC_SAMPLING_FREQUENCY;
}

/**
* Top level data structure for XHdmiC_FRL_CTS_N_Table_t
*/
typedef struct {
	/* 5 = FRL Rate 12,10,8,6,3 */
	XHdmiC_FRL_CTS_N_Val CTS_NVal[5];
} XHdmiC_FRL_CTS_N_Table_t;


/**
* This table contains the CTS and N value of each FRL character rate.
* Each entry is of the format:
* 1) Sample Frequency (32kHz, 44.1kHz, and 48kHz)
*/
const XHdmiC_FRL_CTS_N_Table_t FRL_CTS_N_Table[] =
{
	{/* 32kHz */
		{	/* 3 Gbps */
			{171875, {4224, 8448, 16896, 33792, 67584, 135168}},
			/* 6 Gbps */
			{328125, {4032, 8064, 16128, 32256, 64512, 129024}},
			/* 8 Gbps */
			{437500, {4032, 8064, 16128, 32256, 64512, 129024}},
			/* 10 Gbps */
			{468750, {3456, 6912, 13824, 27648, 55296, 110592}},
			/* 12 Gbps */
			{500000, {3072, 6144, 12288, 24576, 49152, 98304 }},
		},
	},
	{/* 44.1kHz */
		{
			{156250 ,{5292 ,10584 ,21168 ,42336 ,84672 ,169344}},
			{312500 ,{5292 ,10584 ,21168 ,42336 ,84672 ,169344}},
			{312500 ,{3969 ,7938  ,15876 ,31752 ,63504 ,127008}},
			{390625 ,{3969 ,7938  ,15876 ,31752 ,63504 ,127008}},
			{468750 ,{3969 ,7938  ,15876 ,31752 ,63504 ,127008}},
		},
	},
	{/* 48kHz */
		{
			{156250 ,{5760 ,11520 ,23040 ,46080 ,92160 ,184320}},
			{328125 ,{6048 ,12096 ,24192 ,48384 ,96768 ,193536}},
			{437500 ,{6048 ,12096 ,24192 ,48384 ,96768 ,193536}},
			{468750 ,{5184 ,10368 ,20736 ,41472 ,82944 ,165888}},
			{515625 ,{4752 ,9504  ,19008 ,38016 ,76032 ,152064}},
		},
	},
};

/**
* This returns the N value in FRL mode based on its Audio Sample Frequency
*/
u32 XHdmiC_FRL_GetNVal(XHdmiC_FRLCharRate FRLCharRate,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal)
{
	XHdmiC_FRL_CTS_N_Table_t const *item;
	u8 SampleFreq;
	u8 MultSampleFreq;
	u8 FrlAudioCharRate = FRLCharRate;

	/* Check for valid FRL Character Rate */
	if (FrlAudioCharRate > R_666_667 ) {
		/* Since AudSampFreq not valid, return N Value = 0 */
		FrlAudioCharRate = R_166_667;
	}

	if (AudSampleFreqVal%XHDMIC_SAMPLING_FREQ_48K == 0) {
		SampleFreq = 2;
		MultSampleFreq = AudSampleFreqVal/XHDMIC_SAMPLING_FREQ_48K;
	} else if (AudSampleFreqVal%XHDMIC_SAMPLING_FREQ_44_1K == 0) {
		SampleFreq = 1;
		MultSampleFreq = AudSampleFreqVal/XHDMIC_SAMPLING_FREQ_44_1K;
	} else if (AudSampleFreqVal%XHDMIC_SAMPLING_FREQ_32K == 0) {
		SampleFreq = 0;
		MultSampleFreq = AudSampleFreqVal/XHDMIC_SAMPLING_FREQ_32K;
	} else {
		/* Not Valid Scenario */
		SampleFreq = 0;
		MultSampleFreq = 0;
	}

	/* MultSampleFreq is the divisible value from the 3 base
	 * Sample Frequency.
	 * However the N value array for FRL are based on the power of 2.
	 * {base_freq*(2^0),base_freq*(2^1),base_freq*(2^2),base_freq*(2^3),..}
	 */
	switch (MultSampleFreq) {
		case 1:
		case 2:
			MultSampleFreq = MultSampleFreq - 1;
			break;

		case 4:
			MultSampleFreq = 2;
			break;

		case 8:
			MultSampleFreq = 3;
			break;

		case 16:
			MultSampleFreq = 4;
			break;

		case 32:
			MultSampleFreq = 5;
			break;

		default:
			MultSampleFreq = 0;
			break;
	}

	item = &FRL_CTS_N_Table[SampleFreq];
	return item->CTS_NVal[FrlAudioCharRate].ACR_NVal[MultSampleFreq];

}

/**
* This returns the Audio Sample Frequency for FRL
*/
XHdmiC_SamplingFrequencyVal
	XHdmiC_FRL_GetAudSampFreq(XHdmiC_FRLCharRate FRLCharRate,
					u32 CTS, u32 N)
{
    XHdmiC_FRL_CTS_N_Table_t const *item;
    u8 Index_0 = 0;
    u8 Index_1 = 0;
    XHdmiC_SamplingFrequencyVal SampleFreq = XHDMIC_SAMPLING_FREQ;

    item = &FRL_CTS_N_Table[SampleFreq];
    for (Index_0 = 0; Index_0 < 3; Index_0++) {
        item = &FRL_CTS_N_Table[Index_0];
        /* Adding tolerance of 10k CTS */
        if ((CTS <  (item->CTS_NVal[FRLCharRate].ACR_CTSVal + 10000)) &&
        (CTS >  (item->CTS_NVal[FRLCharRate].ACR_CTSVal - 10000))) {
		for (Index_1 = 0; Index_1 < 6; Index_1++) {
			if (N ==
				item->CTS_NVal[FRLCharRate].ACR_NVal[Index_1]){
				if (Index_0 == 0) {
					SampleFreq =
					XHDMIC_SAMPLING_FREQ_32K << Index_1;
				} else if (Index_0 == 1) {
					SampleFreq =
					XHDMIC_SAMPLING_FREQ_44_1K << Index_1;
				} else {
					SampleFreq =
					XHDMIC_SAMPLING_FREQ_48K << Index_1;
				}
				break;
			}
		}
        }
    }
    return SampleFreq;
}

/**
* This converts the Aud. Sampling Freq. Value to
* Audio Info-frame Sampling Freq. Format.
*/
XHdmiC_SamplingFrequency
	XHdmiC_GetAudIFSampFreq (XHdmiC_SamplingFrequencyVal AudSampFreqVal)
{
	XHdmiC_SamplingFrequency ExtACRSampFreq;

	switch (AudSampFreqVal) {
		case XHDMIC_SAMPLING_FREQ_32K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_32K;
		break;

		case XHDMIC_SAMPLING_FREQ_44_1K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_44_1K;
		break;

		case XHDMIC_SAMPLING_FREQ_48K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_48K;
		break;

		case XHDMIC_SAMPLING_FREQ_88_2K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_88_2K;
		break;

		case XHDMIC_SAMPLING_FREQ_96K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_96K;
		break;

		case XHDMIC_SAMPLING_FREQ_176_4K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_176_4K;
		break;

		case XHDMIC_SAMPLING_FREQ_192K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQUENCY_192K;
		break;

		default:
			ExtACRSampFreq =
					XHDMIC_SAMPLING_FREQ;
		break;
	}

	return ExtACRSampFreq;
}

/**
* This converts the Audio Info-frame Sampling Freq. Format. to
* Audio Sampling Freq. Value.
*/
XHdmiC_SamplingFrequencyVal
	XHdmiC_GetAudSampFreqVal(XHdmiC_SamplingFrequency AudSampFreqVal)
{
	XHdmiC_SamplingFrequencyVal ExtACRSampFreq;

	switch (AudSampFreqVal) {
		case XHDMIC_SAMPLING_FREQUENCY_32K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_32K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_44_1K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_44_1K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_48K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_48K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_88_2K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_88_2K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_96K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_96K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_176_4K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_176_4K;
		break;

		case XHDMIC_SAMPLING_FREQUENCY_192K:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_192K;
		break;

		default:
			ExtACRSampFreq =
				XHDMIC_SAMPLING_FREQ_32K;
		break;
	}

	return ExtACRSampFreq;
}

/*************************** Function Definitions *****************************/

/**
*
* This function retrieves the Auxiliary Video Information Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseAVIInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AVI_InfoFrame *infoFramePtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_AVI_INFOFRAME_TYPE) {

		/* Header, Version */
		infoFramePtr->Version = AuxPtr->Header.Byte[1];

		/* PB1 */
		infoFramePtr->ColorSpace = (AuxPtr->Data.Byte[1] >> 5) & 0x7;
		infoFramePtr->ActiveFormatDataPresent = (AuxPtr->Data.Byte[1] >> 4) & 0x1;
		infoFramePtr->BarInfo = (AuxPtr->Data.Byte[1] >> 2) & 0x3;
		infoFramePtr->ScanInfo = AuxPtr->Data.Byte[1] & 0x3;

		/* PB2 */
		infoFramePtr->Colorimetry = (AuxPtr->Data.Byte[2] >> 6) & 0x3;
		infoFramePtr->PicAspectRatio = (AuxPtr->Data.Byte[2] >> 4) & 0x3;
		infoFramePtr->ActiveAspectRatio = AuxPtr->Data.Byte[2] & 0xf;

		/* PB3 */
		infoFramePtr->Itc = (AuxPtr->Data.Byte[3] >> 7) & 0x1;
		infoFramePtr->ExtendedColorimetry = (AuxPtr->Data.Byte[3] >> 4) & 0x7;
		infoFramePtr->QuantizationRange = (AuxPtr->Data.Byte[3] >> 2) & 0x3;
		infoFramePtr->NonUniformPictureScaling = AuxPtr->Data.Byte[3] & 0x3;

		/* PB4 */
		infoFramePtr->VIC = AuxPtr->Data.Byte[4] & 0x7f;

		/* PB5 */
		infoFramePtr->YccQuantizationRange = (AuxPtr->Data.Byte[5] >> 6) & 0x3;
		infoFramePtr->ContentType = (AuxPtr->Data.Byte[5] >> 4) & 0x3;
		infoFramePtr->PixelRepetition = AuxPtr->Data.Byte[5] & 0xf;

		/* PB6/7 */
		infoFramePtr->TopBar = (AuxPtr->Data.Byte[8] << 8) | AuxPtr->Data.Byte[6];

		/* PB8/9 */
		infoFramePtr->BottomBar = (AuxPtr->Data.Byte[10] << 8) | AuxPtr->Data.Byte[9];

		/* PB10/11 */
		infoFramePtr->LeftBar = (AuxPtr->Data.Byte[12] << 8) | AuxPtr->Data.Byte[11];

		/* PB12/13 */
		infoFramePtr->RightBar = (AuxPtr->Data.Byte[14] << 8) | AuxPtr->Data.Byte[13];

	}
}

/*****************************************************************************/
/**
*
* This function retrieves the General Control Packet.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseGCP(XHdmiC_Aux *AuxPtr, XHdmiC_GeneralControlPacket *GcpPtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_GENERAL_CONTROL_PACKET_TYPE) {

		/* SB0 */
		GcpPtr->Clear_AVMUTE = (AuxPtr->Data.Byte[0] >> 4) & 0x1;
		GcpPtr->Set_AVMUTE = AuxPtr->Data.Byte[0] & 0x1;

		/* SB1 */
		GcpPtr->PixelPackingPhase = (AuxPtr->Data.Byte[1] >> 4) & 0xf;
		GcpPtr->ColorDepth = AuxPtr->Data.Byte[1] & 0xf;

		/* SB2 */
		GcpPtr->Default_Phase = AuxPtr->Data.Byte[2] & 0x1;
	}
}

/*****************************************************************************/
/**
*
* This function retrieves the Audio Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseAudioInfoFrame(XHdmiC_Aux *AuxPtr, XHdmiC_AudioInfoFrame *AudIFPtr)
{
	if (AuxPtr->Header.Byte[0] == AUX_AUDIO_INFOFRAME_TYPE) {

		/* HB1, Version */
		AudIFPtr->Version = AuxPtr->Header.Byte[1];

		/* PB1 */
		AudIFPtr->CodingType = (AuxPtr->Data.Byte[1] >> 4) & 0xf;
		AudIFPtr->ChannelCount = (AuxPtr->Data.Byte[1]) & 0x7;

		/* PB2 */
		AudIFPtr->SampleFrequency = (AuxPtr->Data.Byte[2] >> 2) & 0x7;
		AudIFPtr->SampleSize = AuxPtr->Data.Byte[2] & 0x3;

		/* PB4 */
		AudIFPtr->ChannelAllocation = AuxPtr->Data.Byte[4];

		/* PB5 */
		AudIFPtr->Downmix_Inhibit = (AuxPtr->Data.Byte[5] >> 7) & 0x1;
		AudIFPtr->LevelShiftVal = (AuxPtr->Data.Byte[5] >> 4) & 0xf;
		AudIFPtr->LFE_Playback_Level = AuxPtr->Data.Byte[5] & 0x3;
	}
}

/*****************************************************************************/
/**
*
* This function generates and sends Auxilliary Video Infoframes
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_AVIIF_GeneratePacket(XHdmiC_AVI_InfoFrame *infoFramePtr)
{
//	Xil_AssertNonvoid(infoFramePtr != NULL);

	u8 Index;
	u8 Crc;
	XHdmiC_Aux aux;

	/* Header, Packet type*/
	aux.Header.Byte[0] = AUX_AVI_INFOFRAME_TYPE;

	/* Version */
	aux.Header.Byte[1] = infoFramePtr->Version;

	/* Length */
	aux.Header.Byte[2] = 13;

	/* Checksum (this will be calculated by the HDMI TX IP) */
	aux.Header.Byte[3] = 0;

	/* PB1 */
   aux.Data.Byte[1] = (infoFramePtr->ColorSpace & 0x7) << 5 |
		   ((infoFramePtr->ActiveFormatDataPresent << 4) & 0x10) |
		   ((infoFramePtr->BarInfo << 2) & 0xc) |
		   (infoFramePtr->ScanInfo & 0x3);

   /* PB2 */
   aux.Data.Byte[2] = ((infoFramePtr->Colorimetry & 0x3) << 6  |
		   ((infoFramePtr->PicAspectRatio << 4) & 0x30) |
		   (infoFramePtr->ActiveAspectRatio & 0xf));

   /* PB3 */
   aux.Data.Byte[3] = (infoFramePtr->Itc & 0x1) << 7 |
		   ((infoFramePtr->ExtendedColorimetry << 4) & 0x70) |
		   ((infoFramePtr->QuantizationRange << 2) & 0xc) |
		   (infoFramePtr->NonUniformPictureScaling & 0x3);

   /* PB4 */
   aux.Data.Byte[4] = infoFramePtr->VIC;

   /* PB5 */
   aux.Data.Byte[5] = (infoFramePtr->YccQuantizationRange & 0x3) << 6 |
		   ((infoFramePtr->ContentType << 4) & 0x30) |
		   (infoFramePtr->PixelRepetition & 0xf);

   /* PB6 */
   aux.Data.Byte[6] = infoFramePtr->TopBar & 0xff;

   aux.Data.Byte[7] = 0;

   /* PB8 */
   aux.Data.Byte[8] = (infoFramePtr->TopBar & 0xff00) >> 8;

   /* PB9 */
   aux.Data.Byte[9] = infoFramePtr->BottomBar & 0xff;

   /* PB10 */
   aux.Data.Byte[10] = (infoFramePtr->BottomBar & 0xff00) >> 8;

   /* PB11 */
   aux.Data.Byte[11] = infoFramePtr->LeftBar & 0xff;

   /* PB12 */
   aux.Data.Byte[12] = (infoFramePtr->LeftBar & 0xff00) >> 8;

   /* PB13 */
   aux.Data.Byte[13] = infoFramePtr->RightBar & 0xff;

   /* PB14 */
   aux.Data.Byte[14] = (infoFramePtr->RightBar & 0xff00) >> 8;

   /* Index references the length to calculate start of loop from where values are reserved */
   for (Index = aux.Header.Byte[2] + 2; Index < 32; Index++) {
	   aux.Data.Byte[Index] = 0;
   }

   /* Calculate AVI infoframe checksum */
   Crc = 0;

   /* Header */
   for (Index = 0; Index < 3; Index++) {
     Crc += aux.Header.Byte[Index];
   }

   /* Data */
   for (Index = 1; Index < (aux.Header.Byte[2] + 2); Index++) {
     Crc += aux.Data.Byte[Index];
   }

   Crc = 256 - Crc;

   /* PB0 */
   aux.Data.Byte[0] = Crc;

   return aux;
}

/*****************************************************************************/
/**
*
* This function generates and sends Audio Infoframes
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_AudioIF_GeneratePacket(XHdmiC_AudioInfoFrame *AudioInfoFrame)
{
//	Xil_AssertNonvoid(AudioInfoFrame != NULL);

	u8 Index;
	u8 Crc;
	XHdmiC_Aux aux;

	/* Header, Packet Type */
	aux.Header.Byte[0] = AUX_AUDIO_INFOFRAME_TYPE;

	/* Version */
	aux.Header.Byte[1] = 0x01;

	/* Length */
	aux.Header.Byte[2] = 0x0A;

	aux.Header.Byte[3] = 0;

	/* PB1 */
	aux.Data.Byte[1] = AudioInfoFrame->CodingType << 4 |
			(AudioInfoFrame->ChannelCount & 0x7);

	/* PB2 */
	aux.Data.Byte[2] = ((AudioInfoFrame->SampleFrequency << 2) & 0x1c) |
			(AudioInfoFrame->SampleSize & 0x3);

	/* PB3 */
	aux.Data.Byte[3] = 0;


	/* PB4 */
	aux.Data.Byte[4] = AudioInfoFrame->ChannelAllocation;

	/* PB5 */
	aux.Data.Byte[5] = (AudioInfoFrame->Downmix_Inhibit << 7) |
			((AudioInfoFrame->LevelShiftVal << 3) & 0x78) |
			(AudioInfoFrame->LFE_Playback_Level & 0x3);

	for (Index = 6; Index < 32; Index++)
	{
		aux.Data.Byte[Index] = 0;
	}

	/* Calculate Audio infoframe checksum */
	  Crc = 0;

	  /* Header */
	  for (Index = 0; Index < 3; Index++) {
	    Crc += aux.Header.Byte[Index];
	  }

	  /* Data */
	  for (Index = 1; Index < aux.Header.Byte[2] + 1; Index++) {
		  Crc += aux.Data.Byte[Index];
	  }

	  Crc = 256 - Crc;

	  aux.Data.Byte[0] = Crc;

	  return aux;
}

/*****************************************************************************/
/**
*
* This function converts the XVidC_ColorFormat to XHdmiC_Colorspace
*
* @param  ColorFormat is the XVidC_ColorFormat value to be converted
*
* @return XHdmiC_Colorspace value.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Colorspace XV_HdmiC_XVidC_To_IfColorformat(XVidC_ColorFormat ColorFormat) {
	XHdmiC_Colorspace Colorspace;

	switch(ColorFormat) {
		case XVIDC_CSF_RGB :
			Colorspace = XHDMIC_COLORSPACE_RGB;
			break;

		case XVIDC_CSF_YCRCB_422 :
			Colorspace = XHDMIC_COLORSPACE_YUV422;
			break;

		case XVIDC_CSF_YCRCB_444 :
			Colorspace = XHDMIC_COLORSPACE_YUV444;
			break;

		case XVIDC_CSF_YCRCB_420 :
			Colorspace = XHDMIC_COLORSPACE_YUV420;
			break;

		default:
			Colorspace = XHDMIC_COLORSPACE_RESERVED;
			break;
	}

	return Colorspace;
}

/*****************************************************************************/
/**
*
* This function converts the XVidC_ColorDepth to XHdmiC_ColorDepth
*
* @param  Bpc is the XVidC_ColorDepth value to be converted
*
* @return XHdmiC_ColorDepth value.
*
* @note   None.
*
******************************************************************************/
XVidC_AspectRatio XV_HdmiC_IFAspectRatio_To_XVidC(XHdmiC_PicAspectRatio AR) {
	XVidC_AspectRatio AspectRatio;

	switch(AR) {
		case XHDMIC_PIC_ASPECT_RATIO_4_3 :
			AspectRatio = XVIDC_AR_4_3;
			break;

		case XHDMIC_PIC_ASPECT_RATIO_16_9 :
			AspectRatio = XVIDC_AR_16_9;
			break;

		default:
			AspectRatio = XVIDC_AR_16_9;
			break;
	}

	return AspectRatio;
}
