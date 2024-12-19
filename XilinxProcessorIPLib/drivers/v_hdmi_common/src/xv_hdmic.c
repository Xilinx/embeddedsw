/*******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmic.c
 * @addtogroup hdmi_common Overview
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
 * 1.3   EB  29/10/19 Fixed a bug where XV_HdmiC_AVIIF_GeneratePacket and
 *                        XV_HdmiC_AudioIF_GeneratePacket return incorrect Aux
 *           02/12/19 Added 3D Audio Enumerations and APIs
 * 1.6   kp  15/07/21 Updated the VIC table as per latest CTA spec
             30/08/21 Added missing video resolution IDs
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
	{ XVIDC_VM_640x480_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 1 },
	{ XVIDC_VM_720x480_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 2 },
	{ XVIDC_VM_720x480_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 3 },
	{ XVIDC_VM_1280x720_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 4 },
	{ XVIDC_VM_1920x1080_60_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 5 },
	{ XVIDC_VM_1440x480_60_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 6 },
	{ XVIDC_VM_1440x480_60_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 7 },
	{ XVIDC_VM_1440x240_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 8 },
	{ XVIDC_VM_1440x240_60_P_VT263, XHDMIC_PIC_ASPECT_RATIO_4_3, 8},
	{ XVIDC_VM_1440x240_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 9 },
	{ XVIDC_VM_1440x240_60_P_VT263, XHDMIC_PIC_ASPECT_RATIO_16_9, 9},
	{ XVIDC_VM_2880x480_60_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 10 },
	{ XVIDC_VM_2880x480_60_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 11 },
	{ XVIDC_VM_2880x240_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 12 },
	{ XVIDC_VM_2880x240_60_P_VT263, XHDMIC_PIC_ASPECT_RATIO_4_3, 12},
	{ XVIDC_VM_2880x240_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 13 },
	{ XVIDC_VM_2880x240_60_P_VT263, XHDMIC_PIC_ASPECT_RATIO_16_9, 13},
	{ XVIDC_VM_1440x480_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 14 },
	{ XVIDC_VM_1440x480_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 15 },
	{ XVIDC_VM_1920x1080_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 16 },
	{ XVIDC_VM_720x576_50_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 17 },
	{ XVIDC_VM_720x576_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 18 },
	{ XVIDC_VM_1280x720_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 19 },
	{ XVIDC_VM_1920x1080_50_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 20 },
	{ XVIDC_VM_1440x576_50_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 21 },
	{ XVIDC_VM_1440x576_50_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 22 },
	{ XVIDC_VM_1440x288_50_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 23 },
	{ XVIDC_VM_1440x288_50_P_VT312, XHDMIC_PIC_ASPECT_RATIO_4_3, 23},
	{ XVIDC_VM_1440x288_50_P_VT314, XHDMIC_PIC_ASPECT_RATIO_4_3, 23},
	{ XVIDC_VM_1440x288_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 24 },
	{ XVIDC_VM_1440x288_50_P_VT312, XHDMIC_PIC_ASPECT_RATIO_16_9, 24},
	{ XVIDC_VM_1440x288_50_P_VT314, XHDMIC_PIC_ASPECT_RATIO_16_9, 24},
	{ XVIDC_VM_2880x576_50_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 25 },
	{ XVIDC_VM_2880x576_50_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 26 },
	{ XVIDC_VM_2880x288_50_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 27 },
	{ XVIDC_VM_2880x288_50_P_VT312, XHDMIC_PIC_ASPECT_RATIO_4_3, 27},
	{ XVIDC_VM_2880x288_50_P_VT314, XHDMIC_PIC_ASPECT_RATIO_4_3, 27},
	{ XVIDC_VM_2880x288_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 28 },
	{ XVIDC_VM_2880x288_50_P_VT312, XHDMIC_PIC_ASPECT_RATIO_16_9, 28},
	{ XVIDC_VM_2880x288_50_P_VT314, XHDMIC_PIC_ASPECT_RATIO_16_9, 28},
	{ XVIDC_VM_1440x576_50_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 29 },
	{ XVIDC_VM_1440x576_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 30 },
	{ XVIDC_VM_1920x1080_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 31 },
	{ XVIDC_VM_1920x1080_24_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 32 },
	{ XVIDC_VM_1920x1080_25_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 33 },
	{ XVIDC_VM_1920x1080_30_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 34 },
	{ XVIDC_VM_2880x480_60_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 35 },
	{ XVIDC_VM_2880x480_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 36 },
	{ XVIDC_VM_2880x576_50_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 37 },
	{ XVIDC_VM_2880x576_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 38 },
	{ XVIDC_VM_1920x1080_50_I_VT1250, XHDMIC_PIC_ASPECT_RATIO_16_9, 39 },
	{ XVIDC_VM_1920x1080_100_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 40 },
	{ XVIDC_VM_1280x720_100_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 41 },
	{ XVIDC_VM_720x576_100_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 42 },
	{ XVIDC_VM_720x576_100_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 43 },
	{ XVIDC_VM_1440x576_100_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 44 },
	{ XVIDC_VM_1440x576_100_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 45 },
	{ XVIDC_VM_1920x1080_120_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 46 },
	{ XVIDC_VM_1280x720_120_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 47 },
	{ XVIDC_VM_720x480_120_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 48 },
	{ XVIDC_VM_720x480_120_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 49 },
	{ XVIDC_VM_1440x480_120_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 50 },
	{ XVIDC_VM_1440x480_120_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 51 },
	{ XVIDC_VM_720x576_200_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 52 },
	{ XVIDC_VM_720x576_200_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 53 },
	{ XVIDC_VM_1440x576_200_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 54 },
	{ XVIDC_VM_1440x576_200_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 55 },
	{ XVIDC_VM_720x480_240_P, XHDMIC_PIC_ASPECT_RATIO_4_3, 56 },
	{ XVIDC_VM_720x480_240_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 57 },
	{ XVIDC_VM_1440x480_240_I, XHDMIC_PIC_ASPECT_RATIO_4_3, 58 },
	{ XVIDC_VM_1440x480_240_I, XHDMIC_PIC_ASPECT_RATIO_16_9, 59 },
	{ XVIDC_VM_1280x720_24_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 60 },
	{ XVIDC_VM_1280x720_25_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 61 },
	{ XVIDC_VM_1280x720_30_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 62 },
	{ XVIDC_VM_1920x1080_120_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 63 },
	{ XVIDC_VM_1920x1080_100_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 64 },
	{ XVIDC_VM_1280x720_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 65 },
	{ XVIDC_VM_1280x720_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 66 },
	{ XVIDC_VM_1280x720_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 67 },
	{ XVIDC_VM_1280x720_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 68 },
	{ XVIDC_VM_1280x720_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 69 },
	{ XVIDC_VM_1280x720_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 70 },
	{ XVIDC_VM_1280x720_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 71 },
	{ XVIDC_VM_1920x1080_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 72 },
	{ XVIDC_VM_1920x1080_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 73 },
	{ XVIDC_VM_1920x1080_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 74 },
	{ XVIDC_VM_1920x1080_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 75 },
	{ XVIDC_VM_1920x1080_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 76 },
	{ XVIDC_VM_1920x1080_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 77 },
	{ XVIDC_VM_1920x1080_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 78 },
	{ XVIDC_VM_1680x720_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 79 },
	{ XVIDC_VM_1680x720_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 80 },
	{ XVIDC_VM_1680x720_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 81 },
	{ XVIDC_VM_1680x720_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 82 },
	{ XVIDC_VM_1680x720_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 83 },
	{ XVIDC_VM_1680x720_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 84 },
	{ XVIDC_VM_1680x720_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 85 },
	{ XVIDC_VM_2560x1080_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 86 },
	{ XVIDC_VM_2560x1080_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 87 },
	{ XVIDC_VM_2560x1080_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 88 },
	{ XVIDC_VM_2560x1080_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 89 },
	{ XVIDC_VM_2560x1080_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 90 },
	{ XVIDC_VM_2560x1080_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 91 },
	{ XVIDC_VM_2560x1080_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 92 },
	{ XVIDC_VM_3840x2160_24_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 93 },
	{ XVIDC_VM_3840x2160_25_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 94 },
	{ XVIDC_VM_3840x2160_30_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 95 },
	{ XVIDC_VM_3840x2160_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 96 },
	{ XVIDC_VM_3840x2160_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 97 },
	{ XVIDC_VM_4096x2160_24_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 98 },
	{ XVIDC_VM_4096x2160_25_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 99 },
	{ XVIDC_VM_4096x2160_30_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 100 },
	{ XVIDC_VM_4096x2160_50_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 101 },
	{ XVIDC_VM_4096x2160_60_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 102 },
	{ XVIDC_VM_3840x2160_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 103 },
	{ XVIDC_VM_3840x2160_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 104 },
	{ XVIDC_VM_3840x2160_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 105 },
	{ XVIDC_VM_3840x2160_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 106 },
	{ XVIDC_VM_3840x2160_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 107 },
	{ XVIDC_VM_1280x720_48_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 108 },
	{ XVIDC_VM_1280x720_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 109 },
	{ XVIDC_VM_1680x720_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 110 },
	{ XVIDC_VM_1920x1080_48_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 111 },
	{ XVIDC_VM_1920x1080_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 112 },
	{ XVIDC_VM_2560x1080_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 113 },
	{ XVIDC_VM_3840x2160_48_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 114 },
	{ XVIDC_VM_4096x2160_48_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 115 },
	{ XVIDC_VM_3840x2160_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 116 },
	{ XVIDC_VM_3840x2160_100_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 117 },
	{ XVIDC_VM_3840x2160_120_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 118 },
	{ XVIDC_VM_3840x2160_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 119 },
	{ XVIDC_VM_3840x2160_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 120 },
	{ XVIDC_VM_5120x2160_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 121 },
	{ XVIDC_VM_5120x2160_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 122 },
	{ XVIDC_VM_5120x2160_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 123 },
	{ XVIDC_VM_5120x2160_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 124 },
	{ XVIDC_VM_5120x2160_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 125 },
	{ XVIDC_VM_5120x2160_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 126 },
	{ XVIDC_VM_5120x2160_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 127 },
	{ XVIDC_VM_5120x2160_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 193 },
	{ XVIDC_VM_7680x4320_24_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 194 },
	{ XVIDC_VM_7680x4320_25_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 195 },
	{ XVIDC_VM_7680x4320_30_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 196 },
	{ XVIDC_VM_7680x4320_48_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 197 },
	{ XVIDC_VM_7680x4320_50_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 198 },
	{ XVIDC_VM_7680x4320_60_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 199 },
	{ XVIDC_VM_7680x4320_100_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 200 },
	{ XVIDC_VM_7680x4320_120_P, XHDMIC_PIC_ASPECT_RATIO_16_9, 201 },
	{ XVIDC_VM_7680x4320_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 202 },
	{ XVIDC_VM_7680x4320_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 203 },
	{ XVIDC_VM_7680x4320_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 204 },
	{ XVIDC_VM_7680x4320_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 205 },
	{ XVIDC_VM_7680x4320_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 206 },
	{ XVIDC_VM_7680x4320_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 207 },
	{ XVIDC_VM_7680x4320_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 208 },
	{ XVIDC_VM_7680x4320_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 209 },
	{ XVIDC_VM_10240x4320_24_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 210 },
	{ XVIDC_VM_10240x4320_25_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 211 },
	{ XVIDC_VM_10240x4320_30_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 212 },
	{ XVIDC_VM_10240x4320_48_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 213 },
	{ XVIDC_VM_10240x4320_50_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 214 },
	{ XVIDC_VM_10240x4320_60_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 215 },
	{ XVIDC_VM_10240x4320_100_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 216 },
	{ XVIDC_VM_10240x4320_120_P, XHDMIC_PIC_ASPECT_RATIO_64_27, 217 },
	{ XVIDC_VM_4096x2160_100_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 218 },
	{ XVIDC_VM_4096x2160_120_P, XHDMIC_PIC_ASPECT_RATIO_256_135, 219 },
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
* Character Rate.
*
* This function is not expected to be called for CTS/N as 0.
*/
XHdmiC_SamplingFrequency XHdmiC_TMDS_GetAudSampFreq(u32 TMDSCharRate,
		u32 N, u32 CTSVal)
{
  XHdmiC_TMDS_N_Table const *item;
  u8 i = 0;
  u8 j = 0;
  u64 fs;
  u32 FsTol = 1000; /* Max Fs tolerance limit */

  /* Look for standard TMDS rates */
  for(i = 0; i < sizeof(TMDSChar_N_Table)/sizeof(XHdmiC_TMDS_N_Table); i++) {
		item = &TMDSChar_N_Table[i];
		if(TMDSCharRate <= (item->TMDSCharRate + 10000) &&
			(TMDSCharRate >= (item->TMDSCharRate - 10000))) {
			for (j = 0; j < 7; j++) {
				if(N == item->ACR_NVal[j]) {
					/*
					 * starts from
					 * XHDMIC_SAMPLING_FREQUENCY_32K
					 */
					return (j + 1);
					break;
				}
			}
		}
  }

  if (!CTSVal)
          return XHDMIC_SAMPLING_FREQUENCY;

 /* compute and approximate */
  fs = ((u64)TMDSCharRate * (u64)N) ;
  fs = fs / (128 * CTSVal);

  if (((XHDMIC_SAMPLING_FREQ_32K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_32K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_32K;
  else if (((XHDMIC_SAMPLING_FREQ_44_1K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_44_1K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_44_1K;
  else if (((XHDMIC_SAMPLING_FREQ_48K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_48K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_48K;
  else if (((XHDMIC_SAMPLING_FREQ_88_2K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_88_2K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_88_2K;
  else if (((XHDMIC_SAMPLING_FREQ_96K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_96K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_96K;
  else if (((XHDMIC_SAMPLING_FREQ_176_4K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_176_4K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_176_4K;
  else if (((XHDMIC_SAMPLING_FREQ_192K - FsTol) <= fs) &&
		  (fs <= (XHDMIC_SAMPLING_FREQ_192K + FsTol)))
	  return XHDMIC_SAMPLING_FREQUENCY_192K;
  else
	  return  XHDMIC_SAMPLING_FREQUENCY; /* invalid */
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

/*****************************************************************************/
/**
*
* This function is used to Generate CVTEM Packet
*
* @param
*
* @return: None
*
* @note
*
******************************************************************************/
void XV_HdmiC_GenerateCvtem(XHdmiC_Aux *DscAuxFifo, XHdmiC_DscPpsData *DscPpsPtr,
		u16 HCact_Bytes, u16 HFrontPorch, u16 HBackPorch, u16 HSyncWidth)
{
  u32 i = 0, j = 0, k = 0, pps_idx=0;

  /* Packet -0 : Header */
  DscAuxFifo[0].Header.Byte[0] = 0x7F;
  DscAuxFifo[0].Header.Byte[1] = 0x80;
  DscAuxFifo[0].Header.Byte[2] = 0x0;

  /* Packet -0 : Data */
  DscAuxFifo[0].Data.Byte[0] =  0x06; //PB0 -  new(0),end(0),ds_type(00),afr(0),vfr(1),sync(1),rsvd
  DscAuxFifo[0].Data.Byte[1] =  0x00; //PB1 -RSVD
  DscAuxFifo[0].Data.Byte[2] =  0x01; //PB2  Organization ID
  DscAuxFifo[0].Data.Byte[3] =  0x00; //PB3  Data_Set_Tag(MSB)
  DscAuxFifo[0].Data.Byte[4] =  0x02; //PB4  Data_Set_Tag(LSB)
  DscAuxFifo[0].Data.Byte[5] =  0x00; //PB5  Data_Set_Length(MSB)
  DscAuxFifo[0].Data.Byte[6] =  0x88; //PB6  Data_Set_Length(LSB)

  /* PB7 to PB 27 / PPS 0 to PPS 20 */
  for(j = 1; j < 4; j++) {
	  for(k = 0; k < 7; k++) {
		  DscAuxFifo[0].Data.Byte[(j*8)+k] = DscPpsPtr->Byte[pps_idx++];
	  }
  }

  /*Packet 1 to 3 */
  for(i = 1; i < 4; i++) {
	  /* Header */
	  DscAuxFifo[i].Header.Byte[0] = 0x7F;
	  DscAuxFifo[i].Header.Byte[1] = 0x00;
	  DscAuxFifo[i].Header.Byte[2] = i;
	  /* PB0 to PB 27, PPS 21 to PPS 104 */
	  for(j = 0; j < 4; j++) {
		  for(k = 0; k < 7; k++) {
			  DscAuxFifo[i].Data.Byte[(j*8)+k] = DscPpsPtr->Byte[pps_idx++];
		  }
	  }
  }

  /* Packet - 4 Header  */
  DscAuxFifo[4].Header.Byte[0] = 0x7F;
  DscAuxFifo[4].Header.Byte[1] = 0x00;
  DscAuxFifo[4].Header.Byte[2] = 0x4;

  /* PB0 to PB 20, PPS 105 to PPS 125 */
  for(j = 0; j < 3; j++) {
	  for(k = 0; k < 7; k++) {
		  DscAuxFifo[4].Data.Byte[(j*8)+k] = DscPpsPtr->Byte[pps_idx++];
	  }
  }

  DscAuxFifo[4].Data.Byte[24] = DscPpsPtr->Byte[pps_idx++]; // PB21/PPS126
  DscAuxFifo[4].Data.Byte[25] = DscPpsPtr->Byte[pps_idx++]; // PB22/PPS125
  DscAuxFifo[4].Data.Byte[26] = HFrontPorch & 0xFF; // PB23/HFront[7:0]
  DscAuxFifo[4].Data.Byte[27] = (HFrontPorch>>8) & 0xFF; // PB24/HFront[15:8]
  DscAuxFifo[4].Data.Byte[28] = HSyncWidth & 0xFF; // PB25/HSync[7:0]
  DscAuxFifo[4].Data.Byte[29] = (HSyncWidth>>8) & 0xFF;  // PB26/HSync[15:8]
  DscAuxFifo[4].Data.Byte[30] = HBackPorch & 0xFF; // PB27/HBack[7:0]

  /* Packet - 5 Header  */
  DscAuxFifo[5].Header.Byte[0] = 0x7F;
  DscAuxFifo[5].Header.Byte[1] = 0x40;
  DscAuxFifo[5].Header.Byte[2] = 0x5;

  /* Packet 5 Data */
  DscAuxFifo[5].Data.Byte[0] = (HBackPorch>>8) & 0XFF; // PB0/HBack[15:8]
  DscAuxFifo[5].Data.Byte[1] = HCact_Bytes & 0xFF; // PB1/HCactive_Bytes[7:0]
  DscAuxFifo[5].Data.Byte[2] = (HCact_Bytes>>8) & 0xFF; // PB2/HCactive_Bytes[7:0]

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

		/* Data Length */
		infoFramePtr->DataLength = AuxPtr->Header.Byte[2];

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
		infoFramePtr->VIC = AuxPtr->Data.Byte[4] & 0xff;

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

		/* PB14*/
		infoFramePtr->AdditionalColorimetryExtension = (AuxPtr->Data.Byte[16] >> 4) & 0x0f;

		infoFramePtr->FrameRateCode = ((AuxPtr->Data.Byte[16] & 0x0f) |
					 ((AuxPtr->Data.Byte[17] & 0x40) >> 2));

		/* PB15 */
		infoFramePtr->ResolutionID = AuxPtr->Data.Byte[17] & 0x3f;
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
void XV_HdmiC_ParseAudioInfoFrame(XHdmiC_Aux *AuxPtr,
		XHdmiC_AudioInfoFrame *AudIFPtr)
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
* This function retrieves the Audio Metadata.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseAudioMetadata(XHdmiC_Aux *AuxPtr,
		XHdmiC_AudioMetadata *AudMetadata)
{
	if (AuxPtr->Header.Byte[0] == AUX_AUDIO_METADATA_PACKET_TYPE) {
		/* 3D Audio */
		AudMetadata->Audio3D = AuxPtr->Header.Byte[1] & 0x1;

		/* HB2 */
		AudMetadata->Num_Audio_Str =
				(AuxPtr->Header.Byte[2] >> 2) & 0x3;
		AudMetadata->Num_Views = AuxPtr->Header.Byte[2] & 0x3;

		/* PB0 */
		AudMetadata->Audio3D_ChannelCount =
				AuxPtr->Data.Byte[0] & 0x1F;

		/* PB1 */
		AudMetadata->ACAT = AuxPtr->Data.Byte[1] & 0x0F;

		/* PB2 */
		AudMetadata->Audio3D_ChannelAllocation =
				AuxPtr->Data.Byte[2] & 0xFF;
	}
}

/*****************************************************************************/
/**
 *
 * This function retrieves the SPD Infoframes.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
******************************************************************************/
void XV_HdmiC_ParseSPDIF(XHdmiC_Aux *AuxPtr, XHdmiC_SPDInfoFrame *SPDInfoFrame)
{
	if (AuxPtr->Header.Byte[0] == AUX_SPD_INFOFRAME_TYPE) {
		/* 3D Audio */
		SPDInfoFrame->Version = AuxPtr->Header.Byte[1];

		/* Vendor Name Characters */
		SPDInfoFrame->VN1 = AuxPtr->Data.Byte[1];
		SPDInfoFrame->VN2 = AuxPtr->Data.Byte[2];
		SPDInfoFrame->VN3 = AuxPtr->Data.Byte[3];
		SPDInfoFrame->VN4 = AuxPtr->Data.Byte[4];
		SPDInfoFrame->VN5 = AuxPtr->Data.Byte[5];
		SPDInfoFrame->VN6 = AuxPtr->Data.Byte[6];
		SPDInfoFrame->VN7 = AuxPtr->Data.Byte[8];
		SPDInfoFrame->VN8 = AuxPtr->Data.Byte[9];

		/* Product Description Character */
		SPDInfoFrame->PD1 = AuxPtr->Data.Byte[10];
		SPDInfoFrame->PD2 = AuxPtr->Data.Byte[11];
		SPDInfoFrame->PD3 = AuxPtr->Data.Byte[12];
		SPDInfoFrame->PD4 = AuxPtr->Data.Byte[13];
		SPDInfoFrame->PD5 = AuxPtr->Data.Byte[14];
		SPDInfoFrame->PD6 = AuxPtr->Data.Byte[16];
		SPDInfoFrame->PD7 = AuxPtr->Data.Byte[17];
		SPDInfoFrame->PD8 = AuxPtr->Data.Byte[18];
		SPDInfoFrame->PD9 = AuxPtr->Data.Byte[19];
		SPDInfoFrame->PD10 = AuxPtr->Data.Byte[20];
		SPDInfoFrame->PD11 = AuxPtr->Data.Byte[21];
		SPDInfoFrame->PD12 = AuxPtr->Data.Byte[22];
		SPDInfoFrame->PD13 = AuxPtr->Data.Byte[24];
		SPDInfoFrame->PD14 = AuxPtr->Data.Byte[25];
		SPDInfoFrame->PD15 = AuxPtr->Data.Byte[26];
		SPDInfoFrame->PD16 = AuxPtr->Data.Byte[27];

		/* Source Information */
		SPDInfoFrame->SourceInfo = AuxPtr->Data.Byte[28];
	}
}

/*****************************************************************************/
/**
*
* This function retrieves the DRM Infoframes.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiC_ParseDRMIF(XHdmiC_Aux *AuxPtr, XHdmiC_DRMInfoFrame *DRMInfoFrame)
{
	if (AuxPtr->Header.Byte[0] == AUX_DRM_INFOFRAME_TYPE) {

		/* Vendor Name Characters */
		DRMInfoFrame->EOTF = AuxPtr->Data.Byte[1] & 0x7;

		DRMInfoFrame->Static_Metadata_Descriptor_ID =
				AuxPtr->Data.Byte[2] & 0x7;

		DRMInfoFrame->disp_primaries[0].x =
				(AuxPtr->Data.Byte[3] & 0xFF) |
				(AuxPtr->Data.Byte[4] << 8);

		DRMInfoFrame->disp_primaries[0].y =
				(AuxPtr->Data.Byte[5] & 0xFF) |
				(AuxPtr->Data.Byte[6] << 8);

		DRMInfoFrame->disp_primaries[1].x =
				(AuxPtr->Data.Byte[8] & 0xFF) |
				(AuxPtr->Data.Byte[9] << 8);

		DRMInfoFrame->disp_primaries[1].y =
				(AuxPtr->Data.Byte[10] & 0xFF) |
				(AuxPtr->Data.Byte[11] << 8);

		DRMInfoFrame->disp_primaries[2].x =
				(AuxPtr->Data.Byte[12] & 0xFF) |
				(AuxPtr->Data.Byte[13] << 8);

		DRMInfoFrame->disp_primaries[2].y =
				(AuxPtr->Data.Byte[14] & 0xFF) |
				(AuxPtr->Data.Byte[16] << 8);

		DRMInfoFrame->white_point.x =
				(AuxPtr->Data.Byte[17] & 0xFF) |
				(AuxPtr->Data.Byte[18] << 8);

		DRMInfoFrame->white_point.y =
				(AuxPtr->Data.Byte[19] & 0xFF) |
				(AuxPtr->Data.Byte[20] << 8);

		DRMInfoFrame->Max_Disp_Mastering_Luminance =
				(AuxPtr->Data.Byte[21] & 0xFF) |
				(AuxPtr->Data.Byte[22] << 8);

		DRMInfoFrame->Min_Disp_Mastering_Luminance =
				(AuxPtr->Data.Byte[24] & 0xFF) |
				(AuxPtr->Data.Byte[25] << 8);

		DRMInfoFrame->Max_Content_Light_Level =
				(AuxPtr->Data.Byte[26] & 0xFF) |
				(AuxPtr->Data.Byte[27] << 8);

		DRMInfoFrame->Max_Frame_Average_Light_Level =
				(AuxPtr->Data.Byte[28] & 0xFF) |
				(AuxPtr->Data.Byte[29] << 8);
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

	(void)memset((void *)&aux, 0, sizeof(XHdmiC_Aux));

	/* Header, Packet type*/
	aux.Header.Byte[0] = AUX_AVI_INFOFRAME_TYPE;

	if (infoFramePtr->FrameRateCode > XHDMIC_FR_NO_DATA ||
			infoFramePtr->ResolutionID > XHDMIC_VM_1280x720P) {
		infoFramePtr->Version = 4;
		infoFramePtr->DataLength = 15;
	} else if ((infoFramePtr->Colorimetry == XHDMIC_COLORIMETRY_EXTENDED) &&
			(infoFramePtr->ExtendedColorimetry ==
			 XHDMIC_EXTENDED_COLORIMETRY_RESERVED_3)) {
		infoFramePtr->Version = 4;
		infoFramePtr->DataLength = 14;
	} else if (infoFramePtr->VIC >= 128) {
		infoFramePtr->Version = 3;
		infoFramePtr->DataLength = 13;
	} else {
		infoFramePtr->Version = 2;
		infoFramePtr->DataLength = 13;
	}

	/* Version */
	aux.Header.Byte[1] = infoFramePtr->Version;

	/* Length */
	aux.Header.Byte[2] = infoFramePtr->DataLength;

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

	aux.Data.Byte[15] = 0;

	/*PB14*/
	if (infoFramePtr->DataLength >= 14) {
		aux.Data.Byte[16] = ((infoFramePtr->AdditionalColorimetryExtension & 0xf) << 4) |
				(infoFramePtr->FrameRateCode & 0xf);
	} else {
		aux.Data.Byte[16] = 0;
	}

	/*PB15*/
	if (infoFramePtr->DataLength == 15) {
		aux.Data.Byte[17] = (infoFramePtr->ResolutionID & 0x3f) |
				((infoFramePtr->FrameRateCode & 0x10) << 2);
	} else {
		aux.Data.Byte[17] = 0;
	}

	/* Index references the length to calculate start of loop from
	 * where values are reserved */
	for (Index = aux.Header.Byte[2] + 3; Index < 32; Index++)
		aux.Data.Byte[Index] = 0;

	/* Calculate AVI infoframe checksum */
	Crc = 0;

	/* Header */
	for (Index = 0; Index < 3; Index++)
		Crc += aux.Header.Byte[Index];

	/* Data */
	for (Index = 1; Index < (aux.Header.Byte[2] + 3); Index++)
		Crc += aux.Data.Byte[Index];

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

	(void)memset((void *)&aux, 0, sizeof(XHdmiC_Aux));

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
* This function generates and sends Audio Metadata Packet
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux XV_HdmiC_AudioMetadata_GeneratePacket(XHdmiC_AudioMetadata
		*AudMetadata)
{
	u8 Index;
	XHdmiC_Aux aux;

	(void)memset((void *)&aux, 0, sizeof(XHdmiC_Aux));

	/* Header, Packet Type */
	aux.Header.Byte[0] = AUX_AUDIO_METADATA_PACKET_TYPE;

	/* 3D Audio */
	aux.Header.Byte[1] = AudMetadata->Audio3D & 0x1;

	/* HB2 */
	aux.Header.Byte[2] = ((AudMetadata->Num_Audio_Str & 0x3) << 2) |
			(AudMetadata->Num_Views & 0x3);

	/* HB3 */
	aux.Header.Byte[3] = 0;

	/* PB0 */
	aux.Data.Byte[0] = AudMetadata->Audio3D_ChannelCount & 0x1F;

	/* PB1 */
	aux.Data.Byte[1] = AudMetadata->ACAT & 0x0F;

	/* PB2 */
	aux.Data.Byte[2] = AudMetadata->Audio3D_ChannelAllocation & 0xFF;

	for (Index = 3; Index < 32; Index++)
		aux.Data.Byte[Index] = 0;

	return aux;
}

/*****************************************************************************/
/**
 *
 * This function generates and sends SPD Infoframes
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
 *
 * @return None.
 *
 * @note   None.
 *
******************************************************************************/
XHdmiC_Aux XV_HdmiC_SPDIF_GeneratePacket(XHdmiC_SPDInfoFrame *SPDInfoFrame)
{
	u8 Index;
	u8 Crc;
	XHdmiC_Aux aux;

	(void)memset((void *)&aux, 0, sizeof(XHdmiC_Aux));

	/* Header, Packet Type */
	aux.Header.Byte[0] = AUX_SPD_INFOFRAME_TYPE;

	/* 3D Audio */
	aux.Header.Byte[1] = SPDInfoFrame->Version;

	/* Length of SPD InfoFrame */
	aux.Header.Byte[2] = 25;

	/* HB3 */
	aux.Header.Byte[3] = 0; /* CRC */

	/* Vendor Name Characters */
	aux.Data.Byte[0] = 0; /* CRC */
	aux.Data.Byte[1] = SPDInfoFrame->VN1;
	aux.Data.Byte[2] = SPDInfoFrame->VN2;
	aux.Data.Byte[3] = SPDInfoFrame->VN3;
	aux.Data.Byte[4] = SPDInfoFrame->VN4;
	aux.Data.Byte[5] = SPDInfoFrame->VN5;
	aux.Data.Byte[6] = SPDInfoFrame->VN6;
	aux.Data.Byte[7] = 0; /* ECC */
	aux.Data.Byte[8] = SPDInfoFrame->VN7;
	aux.Data.Byte[9] = SPDInfoFrame->VN8;

	/* Product Description Character */
	aux.Data.Byte[10] = SPDInfoFrame->PD1;
	aux.Data.Byte[11] = SPDInfoFrame->PD2;
	aux.Data.Byte[12] = SPDInfoFrame->PD3;
	aux.Data.Byte[13] = SPDInfoFrame->PD4;
	aux.Data.Byte[14] = SPDInfoFrame->PD5;
	aux.Data.Byte[15] = 0; /* ECC */
	aux.Data.Byte[16] = SPDInfoFrame->PD6;
	aux.Data.Byte[17] = SPDInfoFrame->PD7;
	aux.Data.Byte[18] = SPDInfoFrame->PD8;
	aux.Data.Byte[19] = SPDInfoFrame->PD9;
	aux.Data.Byte[20] = SPDInfoFrame->PD10;
	aux.Data.Byte[21] = SPDInfoFrame->PD11;
	aux.Data.Byte[22] = SPDInfoFrame->PD12;
	aux.Data.Byte[23] = 0; /* ECC */
	aux.Data.Byte[24] = SPDInfoFrame->PD13;
	aux.Data.Byte[25] = SPDInfoFrame->PD14;
	aux.Data.Byte[26] = SPDInfoFrame->PD15;
	aux.Data.Byte[27] = SPDInfoFrame->PD16;

	/* Source Information */
	aux.Data.Byte[28] = SPDInfoFrame->SourceInfo;
	aux.Data.Byte[29] = 0;
	aux.Data.Byte[30] = 0;
	aux.Data.Byte[31] = 0; /* ECC */

	/* Calculate SPD infoframe checksum */
	Crc = 0;

	/* Header */
	for (Index = 0; Index < 3; Index++)
		Crc += aux.Header.Byte[Index];

	/* Data */
	for (Index = 1; Index < aux.Header.Byte[2] + 4; Index++)
		Crc += aux.Data.Byte[Index];

	Crc = 256 - Crc;

	aux.Data.Byte[0] = Crc;

	return aux;
}

/*****************************************************************************/
/**
 *
 * This function generates and sends DRM Infoframes
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem instance.
 *
 * @return None.
 *
 * @note   None.
 *
******************************************************************************/
void XV_HdmiC_DRMIF_GeneratePacket(XHdmiC_DRMInfoFrame *DRMInfoFrame, XHdmiC_Aux *aux)
{
	u8 Index;
	u8 Crc;

	memset(aux, 0, sizeof(XHdmiC_Aux));

	/* Header, Packet Type */
	aux->Header.Byte[0] = AUX_DRM_INFOFRAME_TYPE;

	/* Version Refer CEA-861-G */
	aux->Header.Byte[1] = 0x1;

	/* Length of DRM InfoFrame */
	aux->Header.Byte[2] = 26;

	/* HB3 */
	aux->Header.Byte[3] = 0; /* CRC */

	/* Vendor Name Characters */
	aux->Data.Byte[0] = 0; /* CRC */
	aux->Data.Byte[1] = DRMInfoFrame->EOTF & 0x7;

	aux->Data.Byte[2] = DRMInfoFrame->Static_Metadata_Descriptor_ID & 0x7;

	aux->Data.Byte[3] = DRMInfoFrame->disp_primaries[0].x & 0xFF;
	aux->Data.Byte[4] = DRMInfoFrame->disp_primaries[0].x >> 8;

	aux->Data.Byte[5] = DRMInfoFrame->disp_primaries[0].y & 0xFF;
	aux->Data.Byte[6] = DRMInfoFrame->disp_primaries[0].y >> 8;
	aux->Data.Byte[7] = 0; /* ECC */

	aux->Data.Byte[8] = DRMInfoFrame->disp_primaries[1].x & 0xFF;
	aux->Data.Byte[9] = DRMInfoFrame->disp_primaries[1].x >> 8;

	aux->Data.Byte[10] = DRMInfoFrame->disp_primaries[1].y & 0xFF;
	aux->Data.Byte[11] = DRMInfoFrame->disp_primaries[1].y >> 8;

	aux->Data.Byte[12] = DRMInfoFrame->disp_primaries[2].x & 0xFF;
	aux->Data.Byte[13] = DRMInfoFrame->disp_primaries[2].x >> 8;

	aux->Data.Byte[14] = DRMInfoFrame->disp_primaries[2].y & 0xFF;
	aux->Data.Byte[15] = 0; /* ECC */
	aux->Data.Byte[16] = DRMInfoFrame->disp_primaries[2].y >> 8;

	aux->Data.Byte[17] = DRMInfoFrame->white_point.x & 0xFF;
	aux->Data.Byte[18] = DRMInfoFrame->white_point.x >> 8;

	aux->Data.Byte[19] = DRMInfoFrame->white_point.y & 0xFF;
	aux->Data.Byte[20] = DRMInfoFrame->white_point.y >> 8;

	aux->Data.Byte[21] = DRMInfoFrame->Max_Disp_Mastering_Luminance & 0xFF;
	aux->Data.Byte[22] = DRMInfoFrame->Max_Disp_Mastering_Luminance >> 8;
	aux->Data.Byte[23] = 0; /* ECC */

	aux->Data.Byte[24] = DRMInfoFrame->Min_Disp_Mastering_Luminance & 0xFF;
	aux->Data.Byte[25] = DRMInfoFrame->Min_Disp_Mastering_Luminance >> 8;

	aux->Data.Byte[26] = DRMInfoFrame->Max_Content_Light_Level & 0xFF;
	aux->Data.Byte[27] = DRMInfoFrame->Max_Content_Light_Level >> 8;

	aux->Data.Byte[28] = DRMInfoFrame->Max_Frame_Average_Light_Level & 0xFF;
	aux->Data.Byte[29] = DRMInfoFrame->Max_Frame_Average_Light_Level >> 8;

	aux->Data.Byte[30] = 0;
	aux->Data.Byte[31] = 0; /* ECC */

	/* Calculate DRM infoframe checksum */
	Crc = 0;

	/* Header */
	for (Index = 0; Index < 3; Index++)
		Crc += aux->Header.Byte[Index];

	/* Data */
	for (Index = 1; Index < aux->Header.Byte[2] + 4; Index++)
		Crc += aux->Data.Byte[Index];

	Crc = 256 - Crc;

	aux->Data.Byte[0] = Crc;
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
			Colorspace = XHDMIC_COLORSPACE_RESERVED6;
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
