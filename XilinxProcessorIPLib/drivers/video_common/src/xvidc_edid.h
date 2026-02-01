/*******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvidc_edid.h
 * @addtogroup video_common Overview
 * @{
 *
 * Contains macros, definitions, and function declarations related to the
 * Extended Display Identification Data (EDID) structure which is present in all
 * monitors. All content in this file is agnostic of communication interface
 * protocol.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  11/09/14 Initial release.
 * 2.2   als  02/01/16 Functions with pointer arguments that don't modify
 *                     contents now const.
 * 4.0   aad  10/26/16 Functions which return fixed point values instead of
 *		       float
 * </pre>
 *
*******************************************************************************/

#ifndef XVIDC_EDID_H_
/* Prevent circular inclusions by using protection macros. */
#define XVIDC_EDID_H_

#ifdef __cplusplus
extern "C" {
#endif
/******************************* Include Files ********************************/

#include "xstatus.h"
#include "xvidc.h"

/************************** Constant Definitions ******************************/

/** @name Address mapping for the base EDID block.
 * @{
 */
/** EDID header field offset */
#define XVIDC_EDID_HEADER				0x00
/* Vendor and product identification. */
/** Vendor/product ID manufacturer name byte 0 offset */
#define XVIDC_EDID_VPI_ID_MAN_NAME0			0x08
/** Vendor/product ID manufacturer name byte 1 offset */
#define XVIDC_EDID_VPI_ID_MAN_NAME1			0x09
/** Vendor/product ID product code LSB offset */
#define XVIDC_EDID_VPI_ID_PROD_CODE_LSB			0x0A
/** Vendor/product ID product code MSB offset */
#define XVIDC_EDID_VPI_ID_PROD_CODE_MSB			0x0B
/** Vendor/product ID serial number byte 0 offset */
#define XVIDC_EDID_VPI_ID_SN0				0x0C
/** Vendor/product ID serial number byte 1 offset */
#define XVIDC_EDID_VPI_ID_SN1				0x0D
/** Vendor/product ID serial number byte 2 offset */
#define XVIDC_EDID_VPI_ID_SN2				0x0E
/** Vendor/product ID serial number byte 3 offset */
#define XVIDC_EDID_VPI_ID_SN3				0x0F
/** Vendor/product ID week of manufacture offset */
#define XVIDC_EDID_VPI_WEEK_MAN				0x10
/** Vendor/product ID year of manufacture/model offset */
#define XVIDC_EDID_VPI_YEAR				0x11
/* EDID structure version and revision. */
/** EDID structure version number offset */
#define XVIDC_EDID_STRUCT_VER				0x12
/** EDID structure revision number offset */
#define XVIDC_EDID_STRUCT_REV				0x13
/* Basic display parameters and features. */
/** Basic display video input definition offset */
#define XVIDC_EDID_BDISP_VID				0x14
/** Basic display horizontal screen size or aspect ratio offset */
#define XVIDC_EDID_BDISP_H_SSAR				0x15
/** Basic display vertical screen size or aspect ratio offset */
#define XVIDC_EDID_BDISP_V_SSAR				0x16
/** Basic display gamma offset */
#define XVIDC_EDID_BDISP_GAMMA				0x17
/** Basic display feature support offset */
#define XVIDC_EDID_BDISP_FEATURE			0x18
/* Color characteristics (display x,y chromaticity coordinates). */
/** Color characteristics red/green low bits offset */
#define XVIDC_EDID_CC_RG_LOW				0x19
/** Color characteristics blue/white low bits offset */
#define XVIDC_EDID_CC_BW_LOW				0x1A
/** Color characteristics red X high bits offset */
#define XVIDC_EDID_CC_REDX_HIGH				0x1B
/** Color characteristics red Y high bits offset */
#define XVIDC_EDID_CC_REDY_HIGH				0x1C
/** Color characteristics green X high bits offset */
#define XVIDC_EDID_CC_GREENX_HIGH			0x1D
/** Color characteristics green Y high bits offset */
#define XVIDC_EDID_CC_GREENY_HIGH			0x1E
/** Color characteristics blue X high bits offset */
#define XVIDC_EDID_CC_BLUEX_HIGH			0x1F
/** Color characteristics blue Y high bits offset */
#define XVIDC_EDID_CC_BLUEY_HIGH			0x20
/** Color characteristics white X high bits offset */
#define XVIDC_EDID_CC_WHITEX_HIGH			0x21
/** Color characteristics white Y high bits offset */
#define XVIDC_EDID_CC_WHITEY_HIGH			0x22
/* Established timings. */
/** Established timings I offset */
#define XVIDC_EDID_EST_TIMINGS_I			0x23
/** Established timings II offset */
#define XVIDC_EDID_EST_TIMINGS_II			0x24
/** Manufacturer established timings offset */
#define XVIDC_EDID_EST_TIMINGS_MAN			0x25
/* Standard timings. */
/** Standard timing horizontal resolution offset (N=1-8) */
#define XVIDC_EDID_STD_TIMINGS_H(N)			(0x26 + 2 * (N - 1))
/** Standard timing aspect ratio and field refresh rate offset (N=1-8) */
#define XVIDC_EDID_STD_TIMINGS_AR_FRR(N)		(0x27 + 2 * (N - 1))
/* 18 byte descriptors. */
/** 18-byte descriptor block offset (N=1-4) */
#define XVIDC_EDID_18BYTE_DESCRIPTOR(N)			(0x36 + 18 * (N - 1))
/** Preferred timing mode offset (first 18-byte descriptor) */
#define XVIDC_EDID_PTM			(XVIDC_EDID_18BYTE_DESCRIPTOR(1))
/* - Detailed timing descriptor (DTD) / Preferred timing mode (PTM). */
/** DTD/PTM pixel clock in kHz LSB offset */
#define XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB		0x00
/** DTD/PTM pixel clock in kHz MSB offset */
#define XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB		0x01
/** DTD/PTM horizontal addressable pixels LSB offset */
#define XVIDC_EDID_DTD_PTM_HRES_LSB			0x02
/** DTD/PTM horizontal blanking pixels LSB offset */
#define XVIDC_EDID_DTD_PTM_HBLANK_LSB			0x03
/** DTD/PTM horizontal res/blanking upper 4 bits offset */
#define XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4		0x04
/** DTD/PTM vertical addressable lines LSB offset */
#define XVIDC_EDID_DTD_PTM_VRES_LSB			0x05
/** DTD/PTM vertical blanking lines LSB offset */
#define XVIDC_EDID_DTD_PTM_VBLANK_LSB			0x06
/** DTD/PTM vertical res/blanking upper 4 bits offset */
#define XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4		0x07
/** DTD/PTM horizontal front porch pixels LSB offset */
#define XVIDC_EDID_DTD_PTM_HFPORCH_LSB			0x08
/** DTD/PTM horizontal sync pulse width LSB offset */
#define XVIDC_EDID_DTD_PTM_HSPW_LSB			0x09
/** DTD/PTM vertical front porch and sync pulse width lower 4 bits offset */
#define XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4		0x0A
/** DTD/PTM horizontal/vertical front porch and sync pulse width upper 2 bits offset */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2		0x0B
/** DTD/PTM horizontal display size in mm LSB offset */
#define XVIDC_EDID_DTD_PTM_HIMGSIZE_MM_LSB		0x0C
/** DTD/PTM vertical display size in mm LSB offset */
#define XVIDC_EDID_DTD_PTM_VIMGSIZE_MM_LSB		0x0D
/** DTD/PTM horizontal/vertical display size upper 4 bits offset */
#define XVIDC_EDID_DTD_PTM_XIMGSIZE_MM_U4		0x0E
/** DTD/PTM horizontal border pixels offset */
#define XVIDC_EDID_DTD_PTM_HBORDER			0x0F
/** DTD/PTM vertical border lines offset */
#define XVIDC_EDID_DTD_PTM_VBORDER			0x10
/** DTD/PTM signal features offset */
#define XVIDC_EDID_DTD_PTM_SIGNAL			0x11

/* Extension block count. */
/** Extension block count offset */
#define XVIDC_EDID_EXT_BLK_COUNT			0x7E
/* Checksum. */
/** EDID checksum offset */
#define XVIDC_EDID_CHECKSUM				0x7F
/** @} */

/******************************************************************************/

/** @name Extended Display Identification Data: Masks, shifts, and values.
 * @{
 */
/** Manufacturer name character 0 bit shift */
#define XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR0_SHIFT		2
/** Manufacturer name character 0 bit mask */
#define XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR0_MASK		(0x1F << 2)
/** Manufacturer name character 1 low bits mask */
#define XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR1_MASK		0x03
/** Manufacturer name character 1 bit position */
#define XVIDC_EDID_VPI_ID_MAN_NAME0_CHAR1_POS		3
/** Manufacturer name character 1 high bits shift */
#define XVIDC_EDID_VPI_ID_MAN_NAME1_CHAR1_SHIFT		5
/** Manufacturer name character 2 bit mask */
#define XVIDC_EDID_VPI_ID_MAN_NAME1_CHAR2_MASK		0x1F

/* Basic display parameters and features: Video input definition. */
/** Video signal interface bit shift */
#define XVIDC_EDID_BDISP_VID_VSI_SHIFT			7
/** Video signal interface bit mask (0=analog, 1=digital) */
#define XVIDC_EDID_BDISP_VID_VSI_MASK			(0x01 << 7)
/** Analog signal level standard bit shift */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_SHIFT		5
/** Analog signal level standard bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_MASK		(0x03 << 5)
/** Analog signal level 0.700/0.300/1.000 V */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0300_1000	0x0
/** Analog signal level 0.714/0.286/1.000 V */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_0714_0286_1000	0x1
/** Analog signal level 1.000/0.400/1.400 V */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_1000_0400_1400	0x2
/** Analog signal level 0.700/0.000/0.700 V */
#define XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0000_0700	0x3
/** Analog blank-to-black setup expected bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_VID_SETUP_MASK		(0x01 << 4)
/** Analog separate sync H and V signals supported bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_SEP_SYNC_HV_MASK	(0x01 << 3)
/** Analog composite sync signal on H bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_COMP_SYNC_H_MASK	(0x01 << 2)
/** Analog composite sync signal on green bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_COMP_SYNC_G_MASK	(0x01 << 1)
/** Analog serration on V sync pulse bit mask */
#define XVIDC_EDID_BDISP_VID_ANA_SERR_V_SYNC_MASK	(0x01)
/** Digital bits per color bit shift */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_SHIFT		4
/** Digital bits per color bit mask */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_MASK		(0x7 << 4)
/** Digital bits per color undefined */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_UNDEF		0x0
/** Digital 6 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_6			0x1
/** Digital 8 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_8			0x2
/** Digital 10 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_10			0x3
/** Digital 12 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_12			0x4
/** Digital 14 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_14			0x5
/** Digital 16 bits per color */
#define XVIDC_EDID_BDISP_VID_DIG_BPC_16			0x6
/** Digital video interface standard bit mask */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_MASK		0xF
/** Digital video interface standard undefined */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_UNDEF		0x0
/** Digital video interface standard DVI */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_DVI		0x1
/** Digital video interface standard HDMI-A */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIA		0x2
/** Digital video interface standard HDMI-B */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIB		0x3
/** Digital video interface standard MDDI */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_MDDI		0x4
/** Digital video interface standard DisplayPort */
#define XVIDC_EDID_BDISP_VID_DIG_VIS_DP			0x5

/* Basic display parameters and features: Feature support. */
/** Feature support standby power management bit mask */
#define XVIDC_EDID_BDISP_FEATURE_PM_STANDBY_MASK	(0x1 << 7)
/** Feature support suspend power management bit mask */
#define XVIDC_EDID_BDISP_FEATURE_PM_SUSPEND_MASK	(0x1 << 6)
/** Feature support active-off/very low power bit mask */
#define XVIDC_EDID_BDISP_FEATURE_PM_OFF_VLP_MASK	(0x1 << 5)
/** Feature support analog display color type bit shift */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_SHIFT	3
/** Feature support analog display color type bit mask */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MASK	(0x3 << 3)
/** Analog display color type monochrome/grayscale */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MCG	0x0
/** Analog display color type RGB color */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_RGB	0x1
/** Analog display color type non-RGB color */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB	0x2
/** Analog display color type undefined */
#define XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF	0x3
/** Feature support digital YCrCb 4:4:4 color encoding bit mask */
#define XVIDC_EDID_BDISP_FEATURE_DIG_COLORENC_YCRCB444_MASK	(0x1 << 3)
/** Feature support digital YCrCb 4:2:2 color encoding bit mask */
#define XVIDC_EDID_BDISP_FEATURE_DIG_COLORENC_YCRCB422_MASK	(0x1 << 4)
/** Feature support sRGB is default color space bit mask */
#define XVIDC_EDID_BDISP_FEATURE_SRGB_DEF_MASK		(0x1 << 2)
/** Feature support preferred timing mode included bit mask */
#define XVIDC_EDID_BDISP_FEATURE_PTM_INC_MASK		(0x1 << 1)
/** Feature support continuous frequency operation bit mask */
#define XVIDC_EDID_BDISP_FEATURE_CONTFREQ_MASK		(0x1)

/* Color characteristics (display x,y chromaticity coordinates). */
/** Color characteristics high bits shift value */
#define XVIDC_EDID_CC_HIGH_SHIFT			2
/** Red/Blue X coordinate low bits shift */
#define XVIDC_EDID_CC_RBX_LOW_SHIFT			6
/** Red/Blue Y coordinate low bits shift */
#define XVIDC_EDID_CC_RBY_LOW_SHIFT			4
/** Red/Blue Y coordinate low bits mask */
#define XVIDC_EDID_CC_RBY_LOW_MASK			(0x3 << 4)
/** Green/White X coordinate low bits shift */
#define XVIDC_EDID_CC_GWX_LOW_SHIFT			2
/** Green/White X coordinate low bits mask */
#define XVIDC_EDID_CC_GWX_LOW_MASK			(0x3 << 2)
/** Green/White Y coordinate low bits mask */
#define XVIDC_EDID_CC_GWY_LOW_MASK			(0x3)
/** Color characteristics green Y high bits offset (duplicate definition) */
#define XVIDC_EDID_CC_GREENY_HIGH			0x1E
/** Color characteristics blue X high bits offset (duplicate definition) */
#define XVIDC_EDID_CC_BLUEX_HIGH			0x1F
/** Color characteristics blue Y high bits offset (duplicate definition) */
#define XVIDC_EDID_CC_BLUEY_HIGH			0x20
/** Color characteristics white point X high bits offset (duplicate definition) */
#define XVIDC_EDID_CC_WHITEX_HIGH			0x21
/** Color characteristics white point Y high bits offset (duplicate definition) */
#define XVIDC_EDID_CC_WHITEY_HIGH			0x22

/* Established timings. */
/** Established timing: 720x400 @ 70Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_720x400_70_MASK	(0x1 << 7)
/** Established timing: 720x400 @ 88Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_720x400_88_MASK	(0x1 << 6)
/** Established timing: 640x480 @ 60Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_640x480_60_MASK	(0x1 << 5)
/** Established timing: 640x480 @ 67Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_640x480_67_MASK	(0x1 << 4)
/** Established timing: 640x480 @ 72Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_640x480_72_MASK	(0x1 << 3)
/** Established timing: 640x480 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_640x480_75_MASK	(0x1 << 2)
/** Established timing: 800x600 @ 56Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_800x600_56_MASK	(0x1 << 1)
/** Established timing: 800x600 @ 60Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_I_800x600_60_MASK	(0x1)
/** Established timing: 800x600 @ 72Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_800x600_72_MASK	(0x1 << 7)
/** Established timing: 800x600 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_800x600_75_MASK	(0x1 << 6)
/** Established timing: 832x624 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_832x624_75_MASK	(0x1 << 5)
/** Established timing: 1024x768 @ 87Hz interlaced bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_1024x768_87_MASK	(0x1 << 4)
/** Established timing: 1024x768 @ 60Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_1024x768_60_MASK	(0x1 << 3)
/** Established timing: 1024x768 @ 70Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_1024x768_70_MASK	(0x1 << 2)
/** Established timing: 1024x768 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_1024x768_75_MASK	(0x1 << 1)
/** Established timing: 1280x1024 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_II_1280x1024_75_MASK	(0x1)
/** Manufacturer established timing: 1152x870 @ 75Hz bit mask */
#define XVIDC_EDID_EST_TIMINGS_MAN_1152x870_75_MASK	(0x1 << 7)
/** Manufacturer established timings bit mask */
#define XVIDC_EDID_EST_TIMINGS_MAN_MASK			(0x7F)

/* Standard timings. */
/** Standard timings aspect ratio bit shift */
#define XVIDC_EDID_STD_TIMINGS_AR_SHIFT			6
/** Standard timing aspect ratio: 16:10 */
#define XVIDC_EDID_STD_TIMINGS_AR_16_10			0x0
/** Standard timing aspect ratio: 4:3 */
#define XVIDC_EDID_STD_TIMINGS_AR_4_3			0x1
/** Standard timing aspect ratio: 5:4 */
#define XVIDC_EDID_STD_TIMINGS_AR_5_4			0x2
/** Standard timing aspect ratio: 16:9 */
#define XVIDC_EDID_STD_TIMINGS_AR_16_9			0x3
/** Standard timings field refresh rate bit mask */
#define XVIDC_EDID_STD_TIMINGS_FRR_MASK			(0x3F)

/* Detailed timing descriptor (DTD) / Preferred timing mode (PTM). */
/** DTD/PTM horizontal/vertical resolution and blanking upper 4 bits: blanking mask */
#define XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK		0x0F
/** DTD/PTM horizontal/vertical resolution and blanking upper 4 bits: resolution mask */
#define XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK		0xF0
/** DTD/PTM horizontal/vertical resolution and blanking upper 4 bits: resolution shift */
#define XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT		4
/** DTD/PTM vertical front porch and sync pulse width lower 4 bits: sync pulse width mask */
#define XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VSPW_MASK		0x0F
/** DTD/PTM vertical front porch and sync pulse width lower 4 bits: front porch mask */
#define XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_MASK		0xF0
/** DTD/PTM vertical front porch and sync pulse width lower 4 bits: front porch shift */
#define XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_SHIFT	4
/** DTD/PTM horizontal front porch upper 2 bits mask */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_MASK		0xC0
/** DTD/PTM horizontal sync pulse width upper 2 bits mask */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_MASK		0x30
/** DTD/PTM vertical front porch upper 2 bits mask */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_MASK		0x0C
/** DTD/PTM vertical sync pulse width upper 2 bits mask */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VSPW_MASK		0x03
/** DTD/PTM horizontal front porch upper 2 bits shift */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_SHIFT	6
/** DTD/PTM horizontal sync pulse width upper 2 bits shift */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_SHIFT		4
/** DTD/PTM vertical front porch upper 2 bits shift */
#define XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_SHIFT	2
/** DTD/PTM vertical image size upper 4 bits mask */
#define XVIDC_EDID_DTD_PTM_XIMGSIZE_MM_U4_VIMGSIZE_MM_MASK	0x0F
/** DTD/PTM horizontal image size upper 4 bits mask */
#define XVIDC_EDID_DTD_PTM_XIMGSIZE_MM_U4_HIMGSIZE_MM_MASK	0xF0
/** DTD/PTM horizontal image size upper 4 bits shift */
#define XVIDC_EDID_DTD_PTM_XIMGSIZE_MM_U4_HIMGSIZE_MM_SHIFT	4
/** DTD/PTM interlaced bit mask */
#define XVIDC_EDID_DTD_PTM_SIGNAL_INTERLACED_MASK		0x80
/** DTD/PTM interlaced bit shift */
#define XVIDC_EDID_DTD_PTM_SIGNAL_INTERLACED_SHIFT		7
/** DTD/PTM horizontal polarity bit mask */
#define XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_MASK		0x02
/** DTD/PTM vertical polarity bit mask */
#define XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_MASK		0x04
/** DTD/PTM horizontal polarity bit shift */
#define XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_SHIFT		1
/** DTD/PTM vertical polarity bit shift */
#define XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_SHIFT		2
/** @} */

/******************* Macros (Inline Functions) Definitions ********************/

/** Check if EDID header matches valid signature
 * @param E EDID data array */
#define XVidC_EdidIsHeaderValid(E) \
	!memcmp(E, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8)

/* Vendor and product identification: ID manufacturer name. */
/* void XVidC_EdidGetManName(const u8 *EdidRaw, char ManName[4]); */

/* Vendor and product identification: ID product code. */
/** Get product code from EDID
 * @param E EDID data array */
#define XVidC_EdidGetIdProdCode(E) \
	((u16)((E[XVIDC_EDID_VPI_ID_PROD_CODE_MSB] << 8) | \
	E[XVIDC_EDID_VPI_ID_PROD_CODE_LSB]))

/* Vendor and product identification: ID serial number. */
/** Extract 32-bit serial number from EDID
 * @param E EDID data array */
#define XVidC_EdidGetIdSn(E) \
	((u32)((E[XVIDC_EDID_VPI_ID_SN3] << 24) | \
	(E[XVIDC_EDID_VPI_ID_SN2] << 16) | (E[XVIDC_EDID_VPI_ID_SN1] << 8) | \
	E[XVIDC_EDID_VPI_ID_SN0]))

/* Vendor and product identification: Week and year of manufacture or model
 * year. */
/** Get week of manufacture from EDID
 * @param E EDID data array */
#define XVidC_EdidGetManWeek(E)		(E[XVIDC_EDID_VPI_WEEK_MAN])
/** Get model/manufacture year from EDID
 * @param E EDID data array */
#define XVidC_EdidGetModManYear(E)	(E[XVIDC_EDID_VPI_YEAR] + 1990)
/** Check if year field represents model year (week=0xFF)
 * @param E EDID data array */
#define XVidC_EdidIsYearModel(E)	(XVidC_EdidGetManWeek(E) == 0xFF)
/** Check if year field represents manufacture year (week!=0xFF)
 * @param E EDID data array */
#define XVidC_EdidIsYearMan(E)		(XVidC_EdidGetManWeek(E) != 0xFF)

/* EDID structure version and revision. */
/** Get EDID structure version number
 * @param E EDID data array */
#define XVidC_EdidGetStructVer(E)	(E[XVIDC_EDID_STRUCT_VER])
/** Get EDID structure revision number
 * @param E EDID data array */
#define XVidC_EdidGetStructRev(E)	(E[XVIDC_EDID_STRUCT_REV])

/* Basic display parameters and features: Video input definition. */
/** Check if display uses digital signal input
 * @param E EDID data array */
#define XVidC_EdidIsDigitalSig(E) \
	((E[XVIDC_EDID_BDISP_VID] & XVIDC_EDID_BDISP_VID_VSI_MASK) != 0)
/** Check if display uses analog signal input
 * @param E EDID data array */
#define XVidC_EdidIsAnalogSig(E) \
	((E[XVIDC_EDID_BDISP_VID] & XVIDC_EDID_BDISP_VID_VSI_MASK) == 0)
/** Get analog signal level standard
 * @param E EDID data array */
#define XVidC_EdidGetAnalogSigLvlStd(E) \
	((E[XVIDC_EDID_BDISP_VID] & XVIDC_EDID_BDISP_VID_ANA_SLS_MASK) >> \
	XVIDC_EDID_BDISP_VID_ANA_SLS_SHIFT)
/** Get analog video setup (blank-to-black setup/pedestal)
 * @param E EDID data array */
#define XVidC_EdidGetAnalogSigVidSetup(E) \
	((E[XVIDC_EDID_BDISP_VID] & \
	XVIDC_EDID_BDISP_VID_ANA_VID_SETUP_MASK) != 0)
/** Check if analog separate sync H and V supported
 * @param E EDID data array */
#define XVidC_EdidSuppAnalogSigSepSyncHv(E) \
	((E[XVIDC_EDID_BDISP_VID] & \
	XVIDC_EDID_BDISP_VID_ANA_SEP_SYNC_HV_MASK) != 0)
/** Check if analog composite sync on H supported
 * @param E EDID data array */
#define XVidC_EdidSuppAnalogSigCompSyncH(E) \
	((E[XVIDC_EDID_BDISP_VID] & \
	XVIDC_EDID_BDISP_VID_ANA_COMP_SYNC_H_MASK) != 0)
/** Check if analog composite sync on G (green) supported
 * @param E EDID data array */
#define XVidC_EdidSuppAnalogSigCompSyncG(E) \
	((E[XVIDC_EDID_BDISP_VID] & \
	XVIDC_EDID_BDISP_VID_ANA_COMP_SYNC_G_MASK) != 0)
/** Check if analog serration on V sync pulse supported
 * @param E EDID data array */
#define XVidC_EdidSuppAnalogSigSerrVsync(E) \
	((E[XVIDC_EDID_BDISP_VID] & \
	XVIDC_EDID_BDISP_VID_ANA_SERR_V_SYNC_MASK) != 0)
/* XVidC_ColorDepth XVidC_EdidGetColorDepth(const u8 *EdidRaw); */
/** Get digital signal video interface standard
 * @param E EDID data array */
#define XVidC_EdidGetDigitalSigIfaceStd(E) \
	(E[XVIDC_EDID_BDISP_VID] & XVIDC_EDID_BDISP_VID_DIG_VIS_MASK)

/* Basic display parameters and features: Horizontal and vertical screen size or
 * aspect ratio. */
/** Check if screen size or aspect ratio is defined
 * @param E EDID data array */
#define XVidC_EdidIsSsArDefined(E) \
	((E[XVIDC_EDID_BDISP_H_SSAR] | E[XVIDC_EDID_BDISP_V_SSAR]) != 0)
/** Get horizontal screen size or aspect ratio value
 * @param E EDID data array */
#define XVidC_EdidGetSsArH(E)	E[XVIDC_EDID_BDISP_H_SSAR]
/** Get vertical screen size or aspect ratio value
 * @param E EDID data array */
#define XVidC_EdidGetSsArV(E)	E[XVIDC_EDID_BDISP_V_SSAR]
/** Check if values represent screen size (both non-zero)
 * @param E EDID data array */
#define XVidC_EdidIsSsArSs(E) \
	((XVidC_EdidGetSsArH(E) != 0) && (XVidC_EdidGetSsArV(E) != 0))
/** Check if values represent landscape aspect ratio (H non-zero, V zero)
 * @param E EDID data array */
#define XVidC_EdidIsSsArArL(E) \
	((XVidC_EdidGetSsArH(E) != 0) && (XVidC_EdidGetSsArV(E) == 0))
/** Check if values represent portrait aspect ratio (H zero, V non-zero)
 * @param E EDID data array */
#define XVidC_EdidIsSsArArP(E) \
	((XVidC_EdidGetSsArH(E) == 0) && (XVidC_EdidGetSsArV(E) != 0))
/** Calculate landscape aspect ratio
 * @param E EDID data array */
#define XVidC_EdidGetSsArArL(E) \
	((float)((XVidC_EdidGetSsArH(E) + 99.0) / 100.0))
/** Calculate portrait aspect ratio
 * @param E EDID data array */
#define XVidC_EdidGetSsArArP(E) \
	((float)(100.0 / (XVidC_EdidGetSsArV(E) + 99.0)))

/* Basic display parameters and features: Gamma. */
/** Check if gamma value is stored in extension block
 * @param E EDID data array */
#define XVidC_EdidIsGammaInExt(E)	(E[XVIDC_EDID_BDISP_GAMMA] == 0xFF)
/** Calculate gamma value from EDID
 * @param E EDID data array */
#define XVidC_EdidGetGamma(E) \
	((float)((E[XVIDC_EDID_BDISP_GAMMA] + 100.0) / 100.0))

/* Basic display parameters and features: Feature support. */
/** Check if standby power management supported
 * @param E EDID data array */
#define XVidC_EdidSuppFeaturePmStandby(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_PM_STANDBY_MASK) != 0)
/** Check if suspend power management supported
 * @param E EDID data array */
#define XVidC_EdidSuppFeaturePmSuspend(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_PM_SUSPEND_MASK) != 0)
/** Check if active-off/very low power supported
 * @param E EDID data array */
#define XVidC_EdidSuppFeaturePmOffVlp(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_PM_OFF_VLP_MASK) != 0)
/** Get analog display color type
 * @param E EDID data array */
#define XVidC_EdidGetFeatureAnaColorType(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MASK) >> \
	XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_SHIFT)
/** Check if digital YCrCb 4:4:4 color encoding supported
 * @param E EDID data array */
#define XVidC_EdidSuppFeatureDigColorEncYCrCb444(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_DIG_COLORENC_YCRCB444_MASK) != 0)
/** Check if digital YCrCb 4:2:2 color encoding supported
 * @param E EDID data array */
#define XVidC_EdidSuppFeatureDigColorEncYCrCb422(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_DIG_COLORENC_YCRCB422_MASK) != 0)
/** Check if sRGB is default color space
 * @param E EDID data array */
#define XVidC_EdidIsFeatureSrgbDef(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_SRGB_DEF_MASK) != 0)
/** Check if preferred timing mode included
 * @param E EDID data array */
#define XVidC_EdidIsFeaturePtmInc(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_PTM_INC_MASK) != 0)
/** Check if continuous frequency operation supported
 * @param E EDID data array */
#define XVidC_EdidIsFeatureContFreq(E) \
	((E[XVIDC_EDID_BDISP_FEATURE] & \
	XVIDC_EDID_BDISP_FEATURE_CONTFREQ_MASK) != 0)

/* Established timings. */
/** Check if 720x400 @ 70Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings720x400_70(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_720x400_70_MASK) != 0)
/** Check if 720x400 @ 88Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings720x400_88(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_720x400_88_MASK) != 0)
/** Check if 640x480 @ 60Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings640x480_60(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_640x480_60_MASK) != 0)
/** Check if 640x480 @ 67Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings640x480_67(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_640x480_67_MASK) != 0)
/** Check if 640x480 @ 72Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings640x480_72(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_640x480_72_MASK) != 0)
/** Check if 640x480 @ 75Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings640x480_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_640x480_75_MASK) != 0)
/** Check if 800x600 @ 56Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings800x600_56(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_800x600_56_MASK) != 0)
/** Check if 800x600 @ 60Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings800x600_60(E) \
	((E[XVIDC_EDID_EST_TIMINGS_I] & \
	XVIDC_EDID_EST_TIMINGS_I_800x600_60_MASK) != 0)
/** Check if 800x600 @ 72Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings800x600_72(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_800x600_72_MASK) != 0)
/** Check if 800x600 @ 75Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings800x600_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_800x600_75_MASK) != 0)
/** Check if 832x624 @ 75Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings832x624_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_832x624_75_MASK) != 0)
/** Check if 1024x768 @ 87Hz interlaced timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1024x768_87(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_1024x768_87_MASK) != 0)
/** Check if 1024x768 @ 60Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1024x768_60(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_1024x768_60_MASK) != 0)
/** Check if 1024x768 @ 70Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1024x768_70(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_1024x768_70_MASK) != 0)
/** Check if 1024x768 @ 75Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1024x768_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_1024x768_75_MASK) != 0)
/** Check if 1280x1024 @ 75Hz timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1280x1024_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_II] & \
	XVIDC_EDID_EST_TIMINGS_II_1280x1024_75_MASK) != 0)
/** Check if 1152x870 @ 75Hz manufacturer timing supported
 * @param E EDID data array */
#define XVidC_EdidSuppEstTimings1152x870_75(E) \
	((E[XVIDC_EDID_EST_TIMINGS_MAN] & \
	XVIDC_EDID_EST_TIMINGS_MAN_1152x870_75_MASK) != 0)
/** Get manufacturer-specific established timings
 * @param E EDID data array */
#define XVidC_EdidGetTimingsMan(E) \
	(E[XVIDC_EDID_EST_TIMINGS_MAN] & XVIDC_EDID_EST_TIMINGS_MAN_MASK)

/* Standard timings. */
/** Get standard timing horizontal resolution
 * @param E EDID data array
 * @param N Standard timing index (0-7) */
#define XVidC_EdidGetStdTimingsH(E, N) \
	((E[XVIDC_EDID_STD_TIMINGS_H(N)] + 31) * 8)
/** Get standard timing aspect ratio
 * @param E EDID data array
 * @param N Standard timing index (0-7) */
#define XVidC_EdidGetStdTimingsAr(E, N) \
	(E[XVIDC_EDID_STD_TIMINGS_AR_FRR(N)] >> XVIDC_EDID_STD_TIMINGS_AR_SHIFT)
/** Get standard timing field refresh rate
 * @param E EDID data array
 * @param N Standard timing index (0-7) */
#define XVidC_EdidGetStdTimingsFrr(E, N) \
	((E[XVIDC_EDID_STD_TIMINGS_AR_FRR(N)] & \
	XVIDC_EDID_STD_TIMINGS_FRR_MASK) + 60)
/* u16 XVidC_EdidGetStdTimingsV(const u8 *EdidRaw, u8 StdTimingsNum); */
/** Check if DTD/PTM uses interlaced scan
 * @param E EDID data array */
#define XVidC_EdidIsDtdPtmInterlaced(E) \
	((E[XVIDC_EDID_PTM + XVIDC_EDID_DTD_PTM_SIGNAL] & \
	XVIDC_EDID_DTD_PTM_SIGNAL_INTERLACED_MASK) >> \
	XVIDC_EDID_DTD_PTM_SIGNAL_INTERLACED_SHIFT)

/* Extension block count. */
/** Get number of EDID extension blocks
 * @param E EDID data array */
#define XVidC_EdidGetExtBlkCount(E)	(E[XVIDC_EDID_EXT_BLK_COUNT])

/* Checksum. */
/** Get EDID base block checksum
 * @param E EDID data array */
#define XVidC_EdidGetChecksum(E)	(E[XVIDC_EDID_CHECKSUM])

/**************************** Function Prototypes *****************************/

/* Vendor and product identification: ID manufacturer name. */
void XVidC_EdidGetManName(const u8 *EdidRaw, char ManName[4]);

/* Basic display parameters and features: Video input definition. */
XVidC_ColorDepth XVidC_EdidGetColorDepth(const u8 *EdidRaw);

/* Color characteristics (display x,y chromaticity coordinates). */
int XVidC_EdidGetCcRedX(const u8 *EdidRaw);
int XVidC_EdidGetCcRedY(const u8 *EdidRaw);
int XVidC_EdidGetCcGreenX(const u8 *EdidRaw);
int XVidC_EdidGetCcGreenY(const u8 *EdidRaw);
int XVidC_EdidGetCcBlueX(const u8 *EdidRaw);
int XVidC_EdidGetCcBlueY(const u8 *EdidRaw);
int XVidC_EdidGetCcWhiteX(const u8 *EdidRaw);
int XVidC_EdidGetCcWhiteY(const u8 *EdidRaw);

/* Standard timings. */
u16 XVidC_EdidGetStdTimingsV(const u8 *EdidRaw, u8 StdTimingsNum);

/* Utility functions. */
u32 XVidC_EdidIsVideoTimingSupported(const u8 *EdidRaw,
		const XVidC_VideoTimingMode *VtMode);

#ifdef __cplusplus
}
#endif
#endif /* XVIDC_EDID_H_ */
/** @} */
