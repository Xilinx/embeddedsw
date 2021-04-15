/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xedid_print_example.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include "string.h"
#include "xvidc_edid.h"
#include "xedid_print_example.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "dppt.h"
#include "xparameters.h"
#include "xuartlite_l.h"

//#include "../inc/xdptx_example_common.h"

#define FLOAT_FRAC_TO_U32(V, D) ((u32)(V * D) - (((u32)V) * D))

static void Edid_Print_BaseVPId(u8 *EdidRaw);
static void Edid_Print_BaseVerRev(u8 *EdidRaw);
static void Edid_Print_BaseBasicDisp(u8 *EdidRaw);
static void Edid_Print_ColorChar(u8 *EdidRaw);
static void Edid_Print_EstTimings(u8 *EdidRaw);
static void Edid_Print_StdTimings(u8 *EdidRaw);
static void Edid_Print_Ptm(u8 *EdidRaw);
static u8 Edid_CalculateChecksum(u8 *Data, u8 Size);

u8 support_640_480_60;
u8 support_800_600_60;

u32 Edid_PrintDecodeBase(u8 *EdidRaw)
{
	/* Check valid header. */
	if (XVidC_EdidIsHeaderValid(EdidRaw)) {
		xil_printf("\n\rEDID header is: 00 FF FF FF FF FF FF 00\n\r");
	} else {
		xil_printf("\n\rNot an EDID base block...\n\r");
		return XST_FAILURE;
	}

	/* Obtain vendor and product identification information. */
	Edid_Print_BaseVPId(EdidRaw);
	Edid_Print_BaseVerRev(EdidRaw);
	Edid_Print_BaseBasicDisp(EdidRaw);
	Edid_Print_ColorChar(EdidRaw);
	Edid_Print_EstTimings(EdidRaw);
	Edid_Print_StdTimings(EdidRaw);

	xil_printf("Descriptors:\n\r");
	xil_printf("First tag: 0x%02lx 0x%02lx\n\r",
			EdidRaw[0x36], EdidRaw[0x38]);
	xil_printf("Second tag: 0x%02lx 0x%02lx\n\r",
			EdidRaw[0x48], EdidRaw[0x4A]);
	xil_printf("Third tag: 0x%02lx 0x%02lx\n\r",
			EdidRaw[0x5A], EdidRaw[0x5C]);
	xil_printf("Fourth tag: 0x%02lx 0x%02lx\n\r",
			EdidRaw[0x6C], EdidRaw[0x6E]);

	Edid_Print_Ptm(EdidRaw);

	xil_printf("Number of extensions:%d\n\r",
			XVidC_EdidGetExtBlkCount(EdidRaw));
	xil_printf("Checksum:0x%02lx -> Calculated sum = 0x%02lx\n\r",
			XVidC_EdidGetChecksum(EdidRaw),
			Edid_CalculateChecksum(EdidRaw, 128));

	return XST_SUCCESS;
}

void Edid_Print_Supported_VideoModeTable(u8 *EdidRaw)
{
//	u8 Index;

//	xil_printf("Supported resolutions from video mode table:\n\r");
//	for (Index = 0; Index < XVIDC_VM_NUM_SUPPORTED; Index++) {
//		if (XVidC_EdidIsVideoTimingSupported(EdidRaw,
//				&XVidC_VideoTimingModes[Index]) == XST_SUCCESS) {
//			xil_printf("%s\n\r", XVidC_VideoTimingModes[Index].Name);
//		}
//	}
}

static void Edid_Print_BaseVPId(u8 *EdidRaw)
{
	char ManName[4];
	XVidC_EdidGetManName(EdidRaw, ManName);

	/* Vendor and product identification. */
	xil_printf("Vendor and product identification:\n\r");
	xil_printf("ID manufacturer name:%s\n\r", ManName);
	xil_printf("ID product code:0x%04lx\n\r",
			XVidC_EdidGetIdProdCode(EdidRaw));
	xil_printf("ID serial number:0x%08lx\n\r",
			XVidC_EdidGetIdSn(EdidRaw));
	if (XVidC_EdidIsYearModel(EdidRaw)) {
		xil_printf("Model year:%d\n\r",
			XVidC_EdidGetModManYear(EdidRaw));
	}
	else if (XVidC_EdidGetManWeek(EdidRaw) == 0x00) {
		xil_printf("Manufactured:Year = %d ; Week N/A\n\r",
			XVidC_EdidGetModManYear(EdidRaw));
	}
	else {
		xil_printf("Manufactured:Year = %d ; Week = %d\n\r",
			XVidC_EdidGetModManYear(EdidRaw),
			XVidC_EdidGetManWeek(EdidRaw));
	}
}

static void Edid_Print_BaseVerRev(u8 *EdidRaw)
{
	/* EDID structure version and revision. */
	xil_printf("EDID structure version and revision: %d.%d\n\r",
			XVidC_EdidGetStructVer(EdidRaw),
			XVidC_EdidGetStructRev(EdidRaw));
}

static void Edid_Print_BaseBasicDisp(u8 *EdidRaw)
{
	/* Basic display parameters and features. */
	xil_printf("Basic display parameters and features:\n\r");
	if (XVidC_EdidIsAnalogSig(EdidRaw)) {	//XEDID_IS_BDISP_VID_VSI_DIGITAL
		/* Input is a digital video signal interface. */
		xil_printf("Video signal interface is digital.\n\r");

		if (XVidC_EdidGetDigitalSigIfaceStd(EdidRaw) !=
		    XVIDC_EDID_BDISP_VID_DIG_BPC_UNDEF) {
			xil_printf("Color bit depth:%d\n\r",
				   XVidC_EdidGetDigitalSigIfaceStd(EdidRaw));
		} else {
			xil_printf("Color bit depth is undefined.\n\r");
		}

		switch (XVidC_EdidGetDigitalSigIfaceStd(EdidRaw)) {
		case XVIDC_EDID_BDISP_VID_DIG_VIS_DVI:
			xil_printf("DVI is supported.\n\r");
			break;
		case XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIA:
			xil_printf("HDMI-a is supported.\n\r");
			break;
		case XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIB:
			xil_printf("HDMI-b is supported.\n\r");
			break;
		case XVIDC_EDID_BDISP_VID_DIG_VIS_MDDI:
			xil_printf("MDDI is supported.\n\r");
			break;
		case XVIDC_EDID_BDISP_VID_DIG_VIS_DP:
			xil_printf("DisplayPort is supported.\n\r");
			break;
		default:
			xil_printf("Digital interface undefined.\n\r");
			break;
		}
	} else {
		/* Input is an analog video signal interface. */
		xil_printf("Video signal interface is analog.\n\r");

		xil_printf("Signal level standard:");

		switch (XVidC_EdidGetAnalogSigLvlStd(EdidRaw)) {
		case XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0300_1000:
			xil_printf("0.700 : 0.300 : 1.000 Vp-p ");
			break;
		case XVIDC_EDID_BDISP_VID_ANA_SLS_0714_0286_1000:
			xil_printf("0.714 : 0.286 : 1.000 Vp-p ");
			break;
		case XVIDC_EDID_BDISP_VID_ANA_SLS_1000_0400_1400:
			xil_printf("1.000 : 0.400 : 1.400 Vp-p ");
			break;
		case XVIDC_EDID_BDISP_VID_ANA_SLS_0700_0000_0700:
		default:
			xil_printf("0.700 : 0.000 : 0.700 V p-p");
			break;
		}
		xil_printf("(Video : Sync : Total)\n\r");

		xil_printf("Video setup:");
		if (XVidC_EdidGetAnalogSigVidSetup(EdidRaw)) {
			xil_printf("Blank-to-black setup or pedestal.\n\r");
		} else {
			xil_printf("Blank level = black level.\n\r");
		}

		xil_printf("Synchronization types:\n\r");
		xil_printf("Separate sync H & V signals ");
		if (XVidC_EdidSuppAnalogSigSepSyncHv(EdidRaw)) {
			xil_printf("are supported.\n\r");
		} else {
			xil_printf("are not supported.\n\r");
		}

		xil_printf("Composite sync signal on horizontal ");
		if (XVidC_EdidSuppAnalogSigCompSyncH(EdidRaw)) {
			xil_printf("is supported.\n\r");
		} else {
			xil_printf("is not supported.\n\r");
		}

		xil_printf("Composite sync signal on green video ");
		if (XVidC_EdidSuppAnalogSigCompSyncG(EdidRaw)) {
			xil_printf("is supported.\n\r");
		} else {
			xil_printf("is not supported.\n\r");
		}

		xil_printf("Serrations on the vertical sync ");
		if (XVidC_EdidSuppAnalogSigSerrVsync(EdidRaw)) {
			xil_printf("is supported.\n\r");
		} else {
			xil_printf("is not supported.\n\r");
		}
	}

	if (XVidC_EdidIsSsArSs(EdidRaw)) {
		xil_printf("Screen size (HxV):%dx%d(cm)\n\r",
			   XVidC_EdidGetSsArH(EdidRaw),
			   XVidC_EdidGetSsArV(EdidRaw));
	} else if (XVidC_EdidIsSsArArL(EdidRaw)) {
		xil_printf("Aspect ratio (H:V):");
		switch(XVidC_EdidGetSsArH(EdidRaw)) {
		case 0x4F:
			xil_printf("16:9 ");
			break;
		case 0x3D:
			xil_printf("16:10 ");
			break;
		case 0x22:
			xil_printf("4:3 ");
			break;
		case 0x1A:
			xil_printf("5:4 ");
			break;
		default:
			xil_printf("%d.%03d:1 ",
				(u32)XVidC_EdidGetSsArArL(EdidRaw),
				FLOAT_FRAC_TO_U32(XVidC_EdidGetSsArArL(EdidRaw),
						  1000));
			break;
		}
		xil_printf("(landscape)\n\r");
	} else if (XVidC_EdidIsSsArDefined(EdidRaw)) {
		xil_printf("Aspect ratio (H:V):");
		switch (XVidC_EdidGetSsArV(EdidRaw)) {
		case 0x4F:
			xil_printf("9:16 ");
			break;
		case 0x3D:
			xil_printf("10:16 ");
			break;
		case 0x22:
			xil_printf("3:4 ");
			break;
		case 0x1A:
			xil_printf("4:5 ");
			break;
		default:
			xil_printf("%d.%03d:1 ",
				(u32)XVidC_EdidIsSsArArP(EdidRaw),
				FLOAT_FRAC_TO_U32(XVidC_EdidIsSsArArP(EdidRaw),
						  1000));
			break;
		}
		xil_printf("(portrait)\n\r");
	} else {
		xil_printf("Screen size and aspect ratio are undefined.\n\r");
	}

	if (XVidC_EdidIsGammaInExt(EdidRaw)) {
		xil_printf("Gamma is defined in an extension block.\n\r");
	} else {
		xil_printf("Gamma:%d.%02d\n\r",
			(u32)XVidC_EdidGetGamma(EdidRaw),
			FLOAT_FRAC_TO_U32(XVidC_EdidGetGamma(EdidRaw), 100));
	}

	xil_printf("Display power management:\n\r");
	xil_printf("Standby mode ");
	if (XVidC_EdidSuppFeaturePmStandby(EdidRaw)) {
		xil_printf("is supported.\n\r");
	} else {
		xil_printf("is not supported.\n\r");
	}
	xil_printf("Suspend mode ");
	if (XVidC_EdidSuppFeaturePmSuspend(EdidRaw)) {
		xil_printf("is supported.\n\r");
	} else {
		xil_printf("is not supported.\n\r");
	}
	xil_printf("Active off = very low power ");
	if (XVidC_EdidSuppFeaturePmOffVlp(EdidRaw)) {
		xil_printf("is supported.\n\r");
	} else {
		xil_printf("is not supported.\n\r");
	}

	if (XVidC_EdidIsDigitalSig(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("Supported color encoding format(s):\n\r");
		xil_printf("RGB 4:4:4\n\r");
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb444(EdidRaw)) {
			xil_printf("YCrCb 4:4:4\n\r");
		}
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb422(EdidRaw)) {
			xil_printf("YCrCb 4:2:2\n\r");
		}
	} else {
		/* Input is an analog video signal interface. */
		xil_printf("Display color type:");
		switch (XVidC_EdidGetFeatureAnaColorType(EdidRaw)) {
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MCG:
			xil_printf("Monochrome or grayscale display.\n\r");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_RGB:
			xil_printf("RGB color display.\n\r");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB:
			xil_printf("Non-RGB color display.\n\r");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF:
		default:
			xil_printf("Display color type is undefined.\n\r");
			break;
		}
	}

	xil_printf("Other supported features:\n\r");
	/* sRGB standard is the default color space. */
	xil_printf("sRGB standard ");
	if (XVidC_EdidIsFeatureSrgbDef(EdidRaw)) {
		xil_printf("is ");
	} else {
		xil_printf("is not ");
	}
	xil_printf("the default color space.\n\r");
	/* Preferred timing mode includes. */
	xil_printf("Ptm ");
	if (XVidC_EdidIsFeaturePtmInc(EdidRaw)) {
		xil_printf("includes ");
	} else {
		xil_printf("does not include ");
	}
	xil_printf("the native pixel format and preferred refresh rate.\n\r");
	/* Continuous frequency. */
	xil_printf("Display ");
	if (XVidC_EdidIsFeatureContFreq(EdidRaw)) {
		xil_printf("is ");
	} else {
		xil_printf("is non-");
	}
	xil_printf("continuous frequency.\n\r");
}

static void Edid_Print_ColorChar(u8 *EdidRaw)
{
//	xil_printf("Color characterisitics:\n\r");
//	xil_printf("Red_x:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_REDX(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_REDX(EdidRaw), 1000000000));
//	xil_printf("Red_y:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_REDY(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_REDY(EdidRaw), 1000000000));
//	xil_printf("Green_x:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_GREENX(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_GREENX(EdidRaw), 1000000000));
//	xil_printf("Green_y:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_GREENY(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_GREENY(EdidRaw), 1000000000));
//	xil_printf("Blue_x:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_BLUEX(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_BLUEX(EdidRaw), 1000000000));
//	xil_printf("Blue_y:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_BLUEY(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_BLUEY(EdidRaw), 1000000000));
//	xil_printf("White_x:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_WHITEX(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_WHITEX(EdidRaw), 1000000000));
//	xil_printf("White_y:%d.%09d +- 0.0005\n\r",
//		(u32)XEDID_GET_CC_WHITEY(EdidRaw),
//		FLOAT_FRAC_TO_U32(XEDID_GET_CC_WHITEY(EdidRaw), 1000000000));
}

static void Edid_Print_EstTimings(u8 *EdidRaw)
{
	xil_printf("Established timings:\n\r");
	if (XVidC_EdidSuppEstTimings720x400_70(EdidRaw)) {
		xil_printf("720x400 @ 70Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings720x400_88(EdidRaw)) {
		xil_printf("720x400 @ 88Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings640x480_60(EdidRaw)) {
		support_640_480_60 = 1;
		xil_printf("640x480 @ 60Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings640x480_67(EdidRaw)) {
		xil_printf("640x480 @ 67Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings640x480_72(EdidRaw)) {
		xil_printf("640x480 @ 72Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings640x480_75(EdidRaw)) {
		xil_printf("640x480 @ 75Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings800x600_56(EdidRaw)) {
		xil_printf("800x600 @ 56Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings800x600_60(EdidRaw)) {
		support_800_600_60 = 1;
		xil_printf("800x600 @ 60Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings800x600_72(EdidRaw)) {
		xil_printf("800x600 @ 72Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings800x600_75(EdidRaw)) {
		xil_printf("800x600 @ 75Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings832x624_75(EdidRaw)) {
		xil_printf("832x624 @ 75Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1024x768_87(EdidRaw)) {
		xil_printf("1024x768 @ 87Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1024x768_60(EdidRaw)) {
		xil_printf("1024x768 @ 60Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1024x768_70(EdidRaw)) {
		xil_printf("1024x768 @ 70Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1024x768_75(EdidRaw)) {
		xil_printf("1024x768 @ 75Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1280x1024_75(EdidRaw)) {
		xil_printf("1280x1024 @ 75Hz supported.\n\r");
	}
	if (XVidC_EdidSuppEstTimings1152x870_75(EdidRaw)) {
		xil_printf("1152x870 @ 75Hz supported.\n\r");
	}
	xil_printf("Manufacturer specified timings field: 0x%02lx.\n\r",
			XVidC_EdidGetTimingsMan(EdidRaw));
}

static void Edid_Print_StdTimings(u8 *EdidRaw)
{
	/* Standard timings. */
	u8 Index;

	xil_printf("Standard timings:\n\r");
	for (Index = 0; Index < 8; Index++) {
		if (EdidRaw[XVIDC_EDID_STD_TIMINGS_H(Index + 1)] <= 1) {
			/* Not a valid standard timing. */
			continue;
		}

		xil_printf("%dx%d @ %dHz supported.\n\r",
			   XVidC_EdidGetStdTimingsH(EdidRaw, Index + 1),
			   XVidC_EdidGetStdTimingsAr(EdidRaw, Index + 1),
			   XVidC_EdidGetStdTimingsFrr(EdidRaw, Index + 1));
	}
}

static void Edid_Print_Ptm(u8 *EdidRaw)
{
	u8 *Ptm;

	Ptm = &EdidRaw[XVIDC_EDID_PTM];

	u16 HBlank = ((Ptm[XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4] &
		       XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		      Ptm[XVIDC_EDID_DTD_PTM_HBLANK_LSB];

	u16 VBlank = ((Ptm[XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4] &
		       XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XBLANK_MASK) << 8) |
		      Ptm[XVIDC_EDID_DTD_PTM_VBLANK_LSB];

	u32 HActive = (((Ptm[XVIDC_EDID_DTD_PTM_HRES_HBLANK_U4] &
		         XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK) >>
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
		       Ptm[XVIDC_EDID_DTD_PTM_HRES_LSB];

	u32 VActive = (((Ptm[XVIDC_EDID_DTD_PTM_VRES_VBLANK_U4] &
			 XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_MASK) >>
			XVIDC_EDID_DTD_PTM_XRES_XBLANK_U4_XRES_SHIFT) << 8) |
		       Ptm[XVIDC_EDID_DTD_PTM_VRES_LSB];

	u32 PixelClkKhz = ((Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB] << 8) |
			    Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB]) * 10;

	u32 HFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			     XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			   XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8) |
			  Ptm[XVIDC_EDID_DTD_PTM_HFPORCH_LSB];

	u32 HSyncWidth = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			    XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_MASK) >>
			  XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			 Ptm[XVIDC_EDID_DTD_PTM_HSPW_LSB];

	u32 VFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			     XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
			    XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8) |
			  ((Ptm[XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4] &
			    XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_MASK) >>
			  XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VFPORCH_SHIFT);

	u32 VSyncWidth = ((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			   XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VSPW_MASK) << 8) |
			 (Ptm[XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4] &
			  XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4_VSPW_MASK);

	u32 HBackPorch = HBlank - (HFrontPorch + HSyncWidth);

	u32 VBackPorch = VBlank - (VFrontPorch + VSyncWidth);

	u8 HPolarity = (Ptm[XVIDC_EDID_DTD_PTM_SIGNAL] &
			XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_MASK) >>
		       XVIDC_EDID_DTD_PTM_SIGNAL_HPOLARITY_SHIFT;
	u8 VPolarity = (Ptm[XVIDC_EDID_DTD_PTM_SIGNAL] &
			XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_MASK) >>
		       XVIDC_EDID_DTD_PTM_SIGNAL_VPOLARITY_SHIFT;

	xil_printf("Preferred timing mode:\n\r");
	xil_printf("Horizontal resolution:%d px\n\r"
		   "Vertical resolution:%d lines\n\r"
		   "Pixel clock:%d KHz\n\r"
		   "Horizontal front porch:%d px\n\r"
		   "Horizontal sync width:%d px\n\r"
		   "Horizontal back porch:%d px\n\r"
		   "Horizontal blanking:%d px\n\r"
		   "Horizontal polarity:%d\n\r"
		   "Vertical front porch:%d px\n\r"
		   "Vertical sync width:%d px\n\r"
		   "Vertical back porch:%d px\n\r"
		   "Vertical blanking:%d px\n\r"
		   "Vertical polarity:%d\n\r", HActive, VActive, PixelClkKhz,
		   HFrontPorch, HSyncWidth, HBackPorch, HBlank, HPolarity,
		   VFrontPorch, VSyncWidth, VBackPorch, VBlank, VPolarity);

	xil_printf("Interlaced:%s\n\r",
		   XVidC_EdidIsDtdPtmInterlaced(EdidRaw) ?
		   "Yes." : "No (progressive).");
}

static u8 Edid_CalculateChecksum(u8 *Data, u8 Size)
{
	u8 Index;
	u8 Sum = 0;

	for (Index = 0; Index < Size; Index++) {
		Sum += Data[Index];
	}

	return Sum;
}

/******************************************************************************/
/**
 * This function prints the contents of the EDID.
 *
 * @param       InstancePtr is a pointer to the XDptx instance.
*******************************************************************************/
void XDptx_DbgPrintEdid(XDp *InstancePtr)
{
	u8 i = 0;
	u8 Edid[128];
	static u32 first_time=0;

	if (InstancePtr->Config.MstSupport == 0) {
		/* SST mode sink is accessed directly. */
		XDp_TxGetEdid(InstancePtr, Edid);
	} else {
		/* MST mode has remote sinks. */
		u8 SinkNum;

		xil_printf("%d ",InstancePtr->TxInstance.Topology.SinkTotal);
		xil_printf("sinks found, display EDID for which sink #? ");
		xil_printf("(Choices 0-%d):\n\r",
			   InstancePtr->TxInstance.Topology.SinkTotal - 1);
		if (first_time==0) {
			SinkNum = 0;
			first_time = 1;
		} else {
			SinkNum = XUartLite_RecvByte(STDIN_BASEADDRESS) - '0';
		}

		if ((SinkNum < 0) ||
		    (SinkNum > (InstancePtr->TxInstance.Topology.SinkTotal - 1))) {
			xil_printf("%d ", SinkNum);
			xil_printf("is an invalid choice. Returning "
				   "to main menu.\n\r");
			return;
		}

		XDp_TxTopologyNode *SelSink =
				InstancePtr->TxInstance.Topology.SinkList[SinkNum];

		xil_printf("> Sink #%d: LCT = %d ; RAD = ",
			   SinkNum, SelSink->LinkCountTotal);

		for (i = 0; i < SelSink->LinkCountTotal; i++) {
			xil_printf("%d ", SelSink->RelativeAddress[i]);
		}
		xil_printf("\n\r");

		/* Fetch the EDID base block from the 
		 * remote sink in the topology. */
		XDp_TxGetRemoteEdid(InstancePtr, SelSink->LinkCountTotal,
				    SelSink->RelativeAddress, Edid);
	}

	xil_printf("Edid contents:");
	for (i = 0; i < 128; i++) {
		if ((i % 16) == 0) {
			xil_printf("\n\r\t");
		}

		xil_printf("%02lX ", Edid[i]);
	}

	xil_printf("\n\r");

	Edid_PrintDecodeBase(Edid);
}
