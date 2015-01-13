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
 * @file xedid.h
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
 * </pre>
 *
*******************************************************************************/

#ifndef XEDID_H_
/* Prevent circular inclusions by using protection macros. */
#define XEDID_H_

/******************************* Include Files ********************************/

#include "xstatus.h"
#include "xvid.h"

/************************** Constant Definitions ******************************/

/** @name Address mapping for the base EDID block.
  * @{
  */
#define XEDID_HEADER				0x00
/* Vendor and product identification. */
#define XEDID_VPI_ID_MAN_NAME0			0x08
#define XEDID_VPI_ID_MAN_NAME1			0x09
#define XEDID_VPI_ID_PROD_CODE_LSB		0x0A
#define XEDID_VPI_ID_PROD_CODE_MSB		0x0B
#define XEDID_VPI_ID_SN0			0x0C
#define XEDID_VPI_ID_SN1			0x0D
#define XEDID_VPI_ID_SN2			0x0E
#define XEDID_VPI_ID_SN3			0x0F
#define XEDID_VPI_WEEK_MAN			0x10
#define XEDID_VPI_YEAR				0x11
/* EDID structure version and revision. */
#define XEDID_STRUCT_VER			0x12
#define XEDID_STRUCT_REV			0x13
/* Basic display parameters and features. */
#define XEDID_BDISP_VID				0x14
#define XEDID_BDISP_H_SSAR			0x15
#define XEDID_BDISP_V_SSAR			0x16
#define XEDID_BDISP_GAMMA			0x17
#define XEDID_BDISP_FEATURE			0x18
/* Color characterisitics (display x,y chromaticity coordinates). */
#define XEDID_CC_RG_LOW				0x19
#define XEDID_CC_BW_LOW				0x1A
#define XEDID_CC_REDX_HIGH			0x1B
#define XEDID_CC_REDY_HIGH			0x1C
#define XEDID_CC_GREENX_HIGH			0x1D
#define XEDID_CC_GREENY_HIGH			0x1E
#define XEDID_CC_BLUEX_HIGH			0x1F
#define XEDID_CC_BLUEY_HIGH			0x20
#define XEDID_CC_WHITEX_HIGH			0x21
#define XEDID_CC_WHITEY_HIGH			0x22
/* Established timings. */
#define XEDID_EST_TIMINGS_I			0x23
#define XEDID_EST_TIMINGS_II			0x24
#define XEDID_EST_TIMINGS_MAN			0x25
/* Standard timings. */
#define XEDID_STD_TIMINGS_H(N)			(0x26 + 2 * (N - 1))
#define XEDID_STD_TIMINGS_AR_FRR(N)		(0x27 + 2 * (N - 1))
/* 18 byte descriptors. */
#define XEDID_18BYTE_DESCRIPTOR(N)		(0x36 + 18 * (N - 1))
#define XEDID_PTM				(XEDID_18BYTE_DESCRIPTOR(1))
/* - Detailed timing descriptor (DTD) / Preferred timing mode (PTM). */
#define XEDID_DTD_PTM_PIXEL_CLK_KHZ_LSB		0x00
#define XEDID_DTD_PTM_PIXEL_CLK_KHZ_MSB		0x01
#define XEDID_DTD_PTM_HRES_LSB			0x02
#define XEDID_DTD_PTM_HBLANK_LSB		0x03
#define XEDID_DTD_PTM_HRES_HBLANK_U4		0x04
#define XEDID_DTD_PTM_VRES_LSB			0x05
#define XEDID_DTD_PTM_VBLANK_LSB		0x06
#define XEDID_DTD_PTM_VRES_VBLANK_U4		0x07
#define XEDID_DTD_PTM_HFPORCH_LSB		0x08
#define XEDID_DTD_PTM_HSPW_LSB			0x09
#define XEDID_DTD_PTM_VFPORCH_VSPW_L4		0x0A
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2		0x0B
#define XEDID_DTD_PTM_HIMGSIZE_MM_LSB		0x0C
#define XEDID_DTD_PTM_VIMGSIZE_MM_LSB		0x0D
#define XEDID_DTD_PTM_XIMGSIZE_MM_U4		0x0E
#define XEDID_DTD_PTM_HBORDER			0x0F
#define XEDID_DTD_PTM_VBORDER			0x10
#define XEDID_DTD_PTM_SIGNAL			0x11

/* Extension block count. */
#define XEDID_EXT_BLK_COUNT			0x7E
/* Checksum. */
#define XEDID_CHECKSUM				0x7F
/* @} */

/******************************************************************************/

/** @name Extended Display Identification Data: Masks, shifts, and values.
  * @{
  */
#define XEDID_VPI_ID_MAN_NAME0_CHAR0_SHIFT	2
#define XEDID_VPI_ID_MAN_NAME0_CHAR0_MASK	(0x1F << 2)
#define XEDID_VPI_ID_MAN_NAME0_CHAR1_MASK	0x03
#define XEDID_VPI_ID_MAN_NAME0_CHAR1_POS	3
#define XEDID_VPI_ID_MAN_NAME1_CHAR1_SHIFT	5
#define XEDID_VPI_ID_MAN_NAME1_CHAR2_MASK	0x1F

/* Basic display parameters and features: Video input definition. */
#define XEDID_BDISP_VID_VSI_SHIFT		7
#define XEDID_BDISP_VID_VSI_MASK		(0x01 << 7)
#define XEDID_BDISP_VID_ANA_SLS_SHIFT		5
#define XEDID_BDISP_VID_ANA_SLS_MASK		(0x03 << 5)
#define XEDID_BDISP_VID_ANA_SLS_0700_0300_1000	0x0
#define XEDID_BDISP_VID_ANA_SLS_0714_0286_1000	0x1
#define XEDID_BDISP_VID_ANA_SLS_1000_0400_1400	0x2
#define XEDID_BDISP_VID_ANA_SLS_0700_0000_0700	0x3
#define XEDID_BDISP_VID_ANA_VID_SETUP_MASK	(0x01 << 4)
#define XEDID_BDISP_VID_ANA_SEP_SYNC_HV_MASK	(0x01 << 3)
#define XEDID_BDISP_VID_ANA_COMP_SYNC_H_MASK	(0x01 << 2)
#define XEDID_BDISP_VID_ANA_COMP_SYNC_G_MASK	(0x01 << 1)
#define XEDID_BDISP_VID_ANA_SERR_V_SYNC_MASK	(0x01)
#define XEDID_BDISP_VID_DIG_BPC_SHIFT		4
#define XEDID_BDISP_VID_DIG_BPC_MASK		(0x7 << 4)
#define XEDID_BDISP_VID_DIG_BPC_UNDEF		0x0
#define XEDID_BDISP_VID_DIG_BPC_6		0x1
#define XEDID_BDISP_VID_DIG_BPC_8		0x2
#define XEDID_BDISP_VID_DIG_BPC_10		0x3
#define XEDID_BDISP_VID_DIG_BPC_12		0x4
#define XEDID_BDISP_VID_DIG_BPC_14		0x5
#define XEDID_BDISP_VID_DIG_BPC_16		0x6
#define XEDID_BDISP_VID_DIG_VIS_MASK		0xF
#define XEDID_BDISP_VID_DIG_VIS_UNDEF		0x0
#define XEDID_BDISP_VID_DIG_VIS_DVI		0x1
#define XEDID_BDISP_VID_DIG_VIS_HDMIA		0x2
#define XEDID_BDISP_VID_DIG_VIS_HDMIB		0x3
#define XEDID_BDISP_VID_DIG_VIS_MDDI		0x4
#define XEDID_BDISP_VID_DIG_VIS_DP		0x5

/* Basic display parameters and features: Feature support. */
#define XEDID_BDISP_FEATURE_PM_STANDBY_MASK	(0x1 << 7)
#define XEDID_BDISP_FEATURE_PM_SUSPEND_MASK	(0x1 << 6)
#define XEDID_BDISP_FEATURE_PM_OFF_VLP_MASK	(0x1 << 5)
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_SHIFT	3
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_MASK	(0x3 << 3)
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_MCG	0x0
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_RGB	0x1
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB	0x2
#define XEDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF	0x3
#define XEDID_BDISP_FEATURE_DIG_COLORENC_YCRCB444_MASK	(0x1 << 3)
#define XEDID_BDISP_FEATURE_DIG_COLORENC_YCRCB422_MASK	(0x1 << 4)
#define XEDID_BDISP_FEATURE_SRGB_DEF_MASK	(0x1 << 2)
#define XEDID_BDISP_FEATURE_PTM_INC_MASK	(0x1 << 1)
#define XEDID_BDISP_FEATURE_CONTFREQ_MASK	(0x1)

/* Color characterisitics (display x,y chromaticity coordinates). */
#define XEDID_CC_HIGH_SHIFT			2
#define XEDID_CC_RBX_LOW_SHIFT			6
#define XEDID_CC_RBY_LOW_SHIFT			4
#define XEDID_CC_RBY_LOW_MASK			(0x3 << 4)
#define XEDID_CC_GWX_LOW_SHIFT			2
#define XEDID_CC_GWX_LOW_MASK			(0x3 << 2)
#define XEDID_CC_GWY_LOW_MASK			(0x3)
#define XEDID_CC_GREENY_HIGH			0x1E
#define XEDID_CC_BLUEX_HIGH			0x1F
#define XEDID_CC_BLUEY_HIGH			0x20
#define XEDID_CC_WHITEX_HIGH			0x21
#define XEDID_CC_WHITEY_HIGH			0x22

/* Established timings. */
#define XEDID_EST_TIMINGS_I_720x400_70_MASK	(0x1 << 7)
#define XEDID_EST_TIMINGS_I_720x400_88_MASK	(0x1 << 6)
#define XEDID_EST_TIMINGS_I_640x480_60_MASK	(0x1 << 5)
#define XEDID_EST_TIMINGS_I_640x480_67_MASK	(0x1 << 4)
#define XEDID_EST_TIMINGS_I_640x480_72_MASK	(0x1 << 3)
#define XEDID_EST_TIMINGS_I_640x480_75_MASK	(0x1 << 2)
#define XEDID_EST_TIMINGS_I_800x600_56_MASK	(0x1 << 1)
#define XEDID_EST_TIMINGS_I_800x600_60_MASK	(0x1)
#define XEDID_EST_TIMINGS_II_800x600_72_MASK	(0x1 << 7)
#define XEDID_EST_TIMINGS_II_800x600_75_MASK	(0x1 << 6)
#define XEDID_EST_TIMINGS_II_832x624_75_MASK	(0x1 << 5)
#define XEDID_EST_TIMINGS_II_1024x768_87_MASK	(0x1 << 4)
#define XEDID_EST_TIMINGS_II_1024x768_60_MASK	(0x1 << 3)
#define XEDID_EST_TIMINGS_II_1024x768_70_MASK	(0x1 << 2)
#define XEDID_EST_TIMINGS_II_1024x768_75_MASK	(0x1 << 1)
#define XEDID_EST_TIMINGS_II_1280x1024_75_MASK	(0x1)
#define XEDID_EST_TIMINGS_MAN_1152x870_75_MASK	(0x1 << 7)
#define XEDID_EST_TIMINGS_MAN_MASK		(0x7F)

/* Standard timings. */
#define XEDID_STD_TIMINGS_AR_SHIFT		6
#define XEDID_STD_TIMINGS_AR_16_10		0x0
#define XEDID_STD_TIMINGS_AR_4_3		0x1
#define XEDID_STD_TIMINGS_AR_5_4		0x2
#define XEDID_STD_TIMINGS_AR_16_9		0x3
#define XEDID_STD_TIMINGS_FRR_MASK		(0x3F)

/* Detailed timing descriptor (DTD) / Preferred timing mode (PTM). */
#define XEDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK	0x0F
#define XEDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK		0xF0
#define XEDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT		4
#define XEDID_DTD_PTM_VFPORCH_VSPW_L4_VSPW_MASK		0x0F
#define XEDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_MASK	0xF0
#define XEDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_SHIFT	4
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_MASK	0xC0
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_MASK		0x30
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_MASK	0x0C
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_VSPW_MASK		0x03
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_SHIFT	6
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_SHIFT	4
#define XEDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_SHIFT	2
#define XEDID_DTD_PTM_XIMGSIZE_MM_U4_VIMGSIZE_MM_MASK	0x0F
#define XEDID_DTD_PTM_XIMGSIZE_MM_U4_HIMGSIZE_MM_MASK	0xF0
#define XEDID_DTD_PTM_XIMGSIZE_MM_U4_HIMGSIZE_MM_SHIFT	4
#define XEDID_DTD_PTM_SIGNAL_INTERLACED_MASK		0x80
#define XEDID_DTD_PTM_SIGNAL_INTERLACED_SHIFT		7
#define XEDID_DTD_PTM_SIGNAL_HPOLARITY_MASK		0x02
#define XEDID_DTD_PTM_SIGNAL_VPOLARITY_MASK		0x04
#define XEDID_DTD_PTM_SIGNAL_HPOLARITY_SHIFT		1
#define XEDID_DTD_PTM_SIGNAL_VPOLARITY_SHIFT		2
/* @} */

/******************* Macros (Inline Functions) Definitions ********************/

#define XEDID_IS_HEADER_VALID(E) \
			!memcmp(E, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8)

/* Vendor and product identification: ID manufacturer name. */
void XEDID_GET_VPI_ID_MAN_NAME(u8 *EdidRaw, char ManName[4]);
/* Vendor and product identification: ID product code. */
#define XEDID_GET_VPI_ID_PROD_CODE(E) \
	((u16)((E[XEDID_VPI_ID_PROD_CODE_MSB] << 8) | \
	E[XEDID_VPI_ID_PROD_CODE_LSB]))
/* Vendor and product identification: ID serial number. */
#define XEDID_GET_VPI_SN(E) \
	((u32)((E[XEDID_VPI_ID_SN3] << 24) | (E[XEDID_VPI_ID_SN2] << 16) | \
	(E[XEDID_VPI_ID_SN1] << 8) | E[XEDID_VPI_ID_SN0]))
/* Vendor and product identification: Week and year of manufacture or model
 * year. */
#define XEDID_GET_VPI_WEEK_MAN(E)	(E[XEDID_VPI_WEEK_MAN])
#define XEDID_GET_VPI_YEAR(E)		(E[XEDID_VPI_YEAR] + 1990)
#define XEDID_IS_VPI_YEAR_MODEL(E)	(XEDID_GET_VPI_WEEK_MAN(E) == 0xFF)
#define XEDID_IS_VPI_YEAR_MAN(E)	(XEDID_GET_VPI_WEEK_MAN(E) != 0xFF)

/* EDID structure version and revision. */
#define XEDID_GET_STRUCT_VER(E)		(E[XEDID_STRUCT_VER])
#define XEDID_GET_STRUCT_REV(E)		(E[XEDID_STRUCT_REV])

/* Basic display parameters and features: Video input definition. */
#define XEDID_IS_BDISP_VID_VSI_DIGITAL(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_VSI_MASK) != 0)
#define XEDID_IS_BDISP_VID_VSI_ANALOG(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_VSI_MASK) == 0)
#define XEDID_GET_BDISP_VID_ANA_SLS(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_SLS_MASK) >> \
	XEDID_BDISP_VID_ANA_SLS_SHIFT)
#define XEDID_SUPP_BDISP_VID_ANA_VID_SETUP(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_VID_SETUP_MASK) != 0)
#define XEDID_SUPP_BDISP_VID_ANA_SEP_SYNC_HV(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_SEP_SYNC_HV_MASK) != 0)
#define XEDID_SUPP_BDISP_VID_ANA_COMP_SYNC_H(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_COMP_SYNC_H_MASK) != 0)
#define XEDID_SUPP_BDISP_VID_ANA_COMP_SYNC_G(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_COMP_SYNC_G_MASK) != 0)
#define XEDID_SUPP_BDISP_VID_ANA_SERR_V_SYNC(E) \
	((E[XEDID_BDISP_VID] & XEDID_BDISP_VID_ANA_SERR_V_SYNC_MASK) != 0)
u8 XEDID_GET_BDISP_VID_DIG_BPC(u8 *EdidRaw);
#define XEDID_GET_BDISP_VID_DIG_VIS(E) \
	(E[XEDID_BDISP_VID] & XEDID_BDISP_VID_DIG_VIS_MASK)

/* Basic display parameters and features: Horizontal and vertical screen size or
 * aspect ratio. */
#define XEDID_IS_BDISP_SSAR_DEFINED(E) \
	((E[XEDID_BDISP_H_SSAR] | E[XEDID_BDISP_V_SSAR]) != 0)
#define XEDID_GET_BDISP_SSAR_H(E)	E[XEDID_BDISP_H_SSAR]
#define XEDID_GET_BDISP_SSAR_V(E)	E[XEDID_BDISP_V_SSAR]
#define XEDID_IS_BDISP_SSAR_SS(E) \
	((XEDID_GET_BDISP_SSAR_H(E) != 0) && (XEDID_GET_BDISP_SSAR_V(E) != 0))
#define XEDID_IS_BDISP_SSAR_AR_L(E) \
	((XEDID_GET_BDISP_SSAR_H(E) != 0) && (XEDID_GET_BDISP_SSAR_V(E) == 0))
#define XEDID_IS_BDISP_SSAR_AR_P(E) \
	((XEDID_GET_BDISP_SSAR_H(E) == 0) && (XEDID_GET_BDISP_SSAR_V(E) != 0))
#define XEDID_GET_BDISP_SSAR_AR_L(E) \
	((float)((XEDID_GET_BDISP_SSAR_H(E) + 99.0) / 100.0))
#define XEDID_GET_BDISP_SSAR_AR_P(E) \
	((float)(100.0 / (XEDID_GET_BDISP_SSAR_V(E) + 99.0)))

/* Basic display parameters and features: Gamma. */
#define XEDID_IS_BDISP_GAMMA_IN_EXT(E)	(E[XEDID_BDISP_GAMMA] == 0xFF)
#define XEDID_GET_BDISP_GAMMA(E) \
	((float)((E[XEDID_BDISP_GAMMA] + 100.0) / 100.0))

/* Basic display parameters and features: Feature support. */
#define XEDID_SUPP_BDISP_FEATURE_PM_STANDBY(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_PM_STANDBY_MASK) != 0)
#define XEDID_SUPP_BDISP_FEATURE_PM_SUSPEND(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_PM_SUSPEND_MASK) != 0)
#define XEDID_SUPP_BDISP_FEATURE_PM_OFF_VLP(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_PM_OFF_VLP_MASK) != 0)
#define XEDID_GET_BDISP_FEATURE_ANA_COLORTYPE(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_ANA_COLORTYPE_MASK) >> \
	XEDID_BDISP_FEATURE_ANA_COLORTYPE_SHIFT)
#define XEDID_SUPP_BDISP_FEATURE_DIG_COLORENC_YCRCB444(E) \
	((E[XEDID_BDISP_FEATURE] & \
	XEDID_BDISP_FEATURE_DIG_COLORENC_YCRCB444_MASK) != 0)
#define XEDID_SUPP_BDISP_FEATURE_DIG_COLORENC_YCRCB422(E) \
	((E[XEDID_BDISP_FEATURE] & \
	XEDID_BDISP_FEATURE_DIG_COLORENC_YCRCB422_MASK) != 0)
#define XEDID_IS_BDISP_FEATURE_SRGB_DEF(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_SRGB_DEF_MASK) != 0)
#define XEDID_IS_BDISP_FEATURE_PTM_INC(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_PTM_INC_MASK) != 0)
#define XEDID_IS_BDISP_FEATURE_CONTFREQ(E) \
	((E[XEDID_BDISP_FEATURE] & XEDID_BDISP_FEATURE_CONTFREQ_MASK) != 0)

/* Color characterisitics (display x,y chromaticity coordinates). */
float XEDID_GET_CC_REDX(u8 *EdidRaw);
float XEDID_GET_CC_REDY(u8 *EdidRaw);
float XEDID_GET_CC_GREENX(u8 *EdidRaw);
float XEDID_GET_CC_GREENY(u8 *EdidRaw);
float XEDID_GET_CC_BLUEX(u8 *EdidRaw);
float XEDID_GET_CC_BLUEY(u8 *EdidRaw);
float XEDID_GET_CC_WHITEX(u8 *EdidRaw);
float XEDID_GET_CC_WHITEY(u8 *EdidRaw);

/* Established timings. */
#define XEDID_SUPP_EST_TIMINGS_720x400_70(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_720x400_70_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_720x400_88(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_720x400_88_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_640x480_60(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_640x480_60_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_640x480_67(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_640x480_67_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_640x480_72(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_640x480_72_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_640x480_75(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_640x480_75_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_800x600_56(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_800x600_56_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_800x600_60(E) \
	((E[XEDID_EST_TIMINGS_I] & XEDID_EST_TIMINGS_I_800x600_60_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_800x600_72(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_800x600_72_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_800x600_75(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_800x600_75_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_832x624_75(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_832x624_75_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1024x768_87(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_1024x768_87_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1024x768_60(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_1024x768_60_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1024x768_70(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_1024x768_70_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1024x768_75(E) \
	((E[XEDID_EST_TIMINGS_II] & XEDID_EST_TIMINGS_II_1024x768_75_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1280x1024_75(E) \
	((E[XEDID_EST_TIMINGS_II] & \
	XEDID_EST_TIMINGS_II_1280x1024_75_MASK) != 0)
#define XEDID_SUPP_EST_TIMINGS_1152x870_75(E) \
	((E[XEDID_EST_TIMINGS_MAN] & \
	XEDID_EST_TIMINGS_MAN_1152x870_75_MASK) != 0)
#define XEDID_GET_TIMINGS_MAN(E) \
	(E[XEDID_EST_TIMINGS_MAN] & XEDID_EST_TIMINGS_MAN_MASK)

/* Standard timings. */
#define XEDID_GET_STD_TIMINGS_H(E, N)	((E[XEDID_STD_TIMINGS_H(N)] + 31) * 8)
#define XEDID_GET_STD_TIMINGS_AR(E, N) \
	(E[XEDID_STD_TIMINGS_AR_FRR(N)] >> XEDID_STD_TIMINGS_AR_SHIFT)
#define XEDID_GET_STD_TIMINGS_FRR(E, N) \
	((E[XEDID_STD_TIMINGS_AR_FRR(N)] & XEDID_STD_TIMINGS_FRR_MASK) + 60)
u16 XEDID_GET_STD_TIMINGS_V(u8 *EdidRaw, u8 StdTimingsNum);

#define XEDID_IS_DTD_PTM_INTERLACED(E) \
	((E[XEDID_PTM + XEDID_DTD_PTM_SIGNAL] & \
	XEDID_DTD_PTM_SIGNAL_INTERLACED_MASK) >> \
	XEDID_DTD_PTM_SIGNAL_INTERLACED_SHIFT)

/* Extension block count. */
#define XEDID_GET_EXT_BLK_COUNT(E)	(E[XEDID_EXT_BLK_COUNT])

/* Checksum. */
#define XEDID_GET_CHECKSUM(E)		(E[XEDID_CHECKSUM])

/* Utility functions. */
u32 XEdid_IsVideoTimingSupported(u8 *EdidRaw, XVid_VideoTimingMode *VtMode);

#endif /* XEDID_H_ */
