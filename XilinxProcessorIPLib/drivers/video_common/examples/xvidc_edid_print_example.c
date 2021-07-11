/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvidc_edid_print_example.c
 *
 * Contains an example that, given a supplied base Extended Display
 * Identification Data (EDID) structure, will parse, decode, and print its
 * contents.
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

/******************************* Include Files ********************************/

#include "string.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xvidc_edid_print_example.h"

/******************* Macros (Inline Functions) Definitions ********************/

#define FLOAT_FRAC_TO_U32(V, D) ((u32)(V * D) - (((u32)V) * D))

/**************************** Function Prototypes *****************************/

static void Edid_PrintBaseVPId(u8 *EdidRaw);
static void Edid_PrintBaseVerRev(u8 *EdidRaw);
static void Edid_PrintBaseBasicDisp(u8 *EdidRaw);
static void Edid_PrintColorChar(u8 *EdidRaw);
static void Edid_PrintEstTimings(u8 *EdidRaw);
static void Edid_PrintStdTimings(u8 *EdidRaw);
static void Edid_PrintPtm(u8 *EdidRaw);
static u8 Edid_CalculateChecksum(u8 *Data, u8 Size);

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure and prints its contents.
 *
 * @param	EdidRaw is the base EDID structure to decode and print.
 *
 * @return
 *		- XST_SUCCESS if the supplied base EDID has a valid EDID header.
 *		- XST_FAILURE otherwise. The EDID wasn't decoded further.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Edid_PrintDecodeBase(u8 *EdidRaw)
{
	/* Check valid header. */
	if (XVidC_EdidIsHeaderValid(EdidRaw)) {
		xil_printf("\nEDID header is: 00 FF FF FF FF FF FF 00\n");
	}
	else {
		xil_printf("\nNot an EDID base block...\n");
		return XST_FAILURE;
	}

	/* Obtain vendor and product identification information. */
	Edid_PrintBaseVPId(EdidRaw);
	Edid_PrintBaseVerRev(EdidRaw);
	Edid_PrintBaseBasicDisp(EdidRaw);
	Edid_PrintColorChar(EdidRaw);
	Edid_PrintEstTimings(EdidRaw);
	Edid_PrintStdTimings(EdidRaw);

	xil_printf("Descriptors:\n");
	xil_printf("\tFirst tag: 0x%02lx 0x%02lx\n", EdidRaw[0x36],
								EdidRaw[0x38]);
	xil_printf("\tSecond tag: 0x%02lx 0x%02lx\n", EdidRaw[0x48],
								EdidRaw[0x4A]);
	xil_printf("\tThird tag: 0x%02lx 0x%02lx\n", EdidRaw[0x5A],
								EdidRaw[0x5C]);
	xil_printf("\tFourth tag: 0x%02lx 0x%02lx\n", EdidRaw[0x6C],
								EdidRaw[0x6E]);

	Edid_PrintPtm(EdidRaw);

	xil_printf("Number of extensions:\t%d\n",
					XVidC_EdidGetExtBlkCount(EdidRaw));
	xil_printf("Checksum:\t\t0x%02lx -> Calculated sum = 0x%02lx\n",
					XVidC_EdidGetChecksum(EdidRaw),
					Edid_CalculateChecksum(EdidRaw, 128));

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for supported video timing modes and prints all video
 * timing modes that are both supported in the EDID and listed in the
 * XVidC_VideoTimingModes table.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void Edid_PrintSuppVmTable(u8 *EdidRaw)
{
	u8 Index;

	xil_printf("Supported resolutions from video mode table:\n");
	for (Index = 0; Index < XVIDC_VM_NUM_SUPPORTED; Index++) {
		if (XVidC_EdidIsVideoTimingSupported(EdidRaw,
				&XVidC_VideoTimingModes[Index]) ==
				XST_SUCCESS) {
			xil_printf("\t%s\n",
					XVidC_VideoTimingModes[Index].Name);
		}
	}
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for the vendor and product ID and prints this information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintBaseVPId(u8 *EdidRaw)
{
	char ManName[4];
	XVidC_EdidGetManName(EdidRaw, ManName);

	/* Vendor and product identification. */
	xil_printf("Vendor and product identification:\n");
	xil_printf("\tID manufacturer name:\t%s\n", ManName);
	xil_printf("\tID product code:\t0x%04lx\n",
					XVidC_EdidGetIdProdCode(EdidRaw));
	xil_printf("\tID serial number:\t0x%08lx\n",
						XVidC_EdidGetIdSn(EdidRaw));
	if (XVidC_EdidIsYearModel(EdidRaw)) {
		xil_printf("\tModel year:\t\t%d\n",
					XVidC_EdidGetModManYear(EdidRaw));
	}
	else if (XVidC_EdidGetManWeek(EdidRaw) == 0x00) {
		xil_printf("\tManufactured:\t\tYear = %d ; Week N/A\n",
					XVidC_EdidGetModManYear(EdidRaw));
	}
	else {
		xil_printf("\tManufactured:\t\tYear = %d ; Week = %d\n",
					XVidC_EdidGetModManYear(EdidRaw),
					XVidC_EdidGetManWeek(EdidRaw));
	}
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for the EDID version and revision and prints this
 * information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintBaseVerRev(u8 *EdidRaw)
{
	/* EDID structure version and revision. */
	xil_printf("EDID structure version and revision: %d.%d\n",
					XVidC_EdidGetStructVer(EdidRaw),
					XVidC_EdidGetStructRev(EdidRaw));
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for basic display parameters and features and prints this
 * information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintBaseBasicDisp(u8 *EdidRaw)
{
	/* Basic display parameters and features. */
	xil_printf("Basic display parameters and features:\n");
	if (XVidC_EdidIsDigitalSig(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("\tVideo signal interface is digital.\n");

		if (XVidC_EdidGetColorDepth(EdidRaw) != XVIDC_BPC_UNKNOWN) {
			xil_printf("\tColor bit depth:\t%d\n",
					XVidC_EdidGetColorDepth(EdidRaw));
		}
		else {
			xil_printf("\tColor bit depth is undefined.\n");
		}

		switch (XVidC_EdidGetDigitalSigIfaceStd(EdidRaw)) {
			case XVIDC_EDID_BDISP_VID_DIG_VIS_DVI:
				xil_printf("\tDVI is supported.\n");
				break;
			case XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIA:
				xil_printf("\tHDMI-a is supported.\n");
				break;
			case XVIDC_EDID_BDISP_VID_DIG_VIS_HDMIB:
				xil_printf("\tHDMI-b is supported.\n");
				break;
			case XVIDC_EDID_BDISP_VID_DIG_VIS_MDDI:
				xil_printf("\tMDDI is supported.\n");
				break;
			case XVIDC_EDID_BDISP_VID_DIG_VIS_DP:
				xil_printf("\tDisplayPort is supported.\n");
				break;
			default:
				xil_printf("\tDigital interface undefined.\n");
				break;
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("\tVideo signal interface is analog.\n");

		xil_printf("\tSignal level standard:\t");

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
		xil_printf("(Video : Sync : Total)\n");

		xil_printf("\tVideo setup:\t\t");
		if (XVidC_EdidGetAnalogSigVidSetup(EdidRaw)) {
			xil_printf("Blank-to-black setup or pedestal.\n");
		}
		else {
			xil_printf("Blank level = black level.\n");
		}

		xil_printf("\tSynchronization types:\n");
		xil_printf("\t\tSeparate sync H & V signals ");
		if (XVidC_EdidSuppAnalogSigSepSyncHv(EdidRaw)) {
			xil_printf("are supported.\n");
		}
		else {
			xil_printf("are not supported.\n");
		}
		xil_printf("\t\tComposite sync signal on horizontal ");
		if (XVidC_EdidSuppAnalogSigCompSyncH(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}
		xil_printf("\t\tComposite sync signal on green video ");
		if (XVidC_EdidSuppAnalogSigCompSyncG(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}

		xil_printf("\tSerrations on the vertical sync ");
		if (XVidC_EdidSuppAnalogSigSerrVsync(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}
	}

	if (XVidC_EdidIsSsArSs(EdidRaw)) {
		xil_printf("\tScreen size (HxV):\t%dx%d(cm)\n",
						XVidC_EdidGetSsArH(EdidRaw),
						XVidC_EdidGetSsArV(EdidRaw));
	}
	else if (XVidC_EdidIsSsArArL(EdidRaw)) {
		xil_printf("\tAspect ratio (H:V):\t");
		switch (XVidC_EdidGetSsArH(EdidRaw)) {
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
					FLOAT_FRAC_TO_U32(
					XVidC_EdidGetSsArArL(EdidRaw), 1000));
				break;
		}
		xil_printf("(landscape)\n");
	}
	else if (XVidC_EdidIsSsArArP(EdidRaw)) {
		xil_printf("\tAspect ratio (H:V):\t");
		switch(XVidC_EdidGetSsArV(EdidRaw)) {
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
					FLOAT_FRAC_TO_U32(
					XVidC_EdidGetSsArArP(EdidRaw),
					1000));
				break;
		}
		xil_printf("(portrait)\n");
	}
	else {
		xil_printf("\tScreen size and aspect ratio are undefined.\n");
	}

	if (XVidC_EdidIsGammaInExt(EdidRaw)) {
		xil_printf("\tGamma is defined in an extension block.\n");
	}
	else {
		xil_printf("\tGamma:\t\t\t%d.%02d\n",
			(u32)XVidC_EdidGetGamma(EdidRaw),
			FLOAT_FRAC_TO_U32(XVidC_EdidGetGamma(EdidRaw),
			100));
	}

	xil_printf("\tDisplay power management:\n");
	xil_printf("\t\t\t\tStandby mode ");
	if (XVidC_EdidSuppFeaturePmStandby(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}
	xil_printf("\t\t\t\tSuspend mode ");
	if (XVidC_EdidSuppFeaturePmSuspend(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}
	xil_printf("\t\t\t\tActive off = very low power ");
	if (XVidC_EdidSuppFeaturePmOffVlp(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}

	if (XVidC_EdidIsDigitalSig(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("\tSupported color encoding format(s):\n");
		xil_printf("\t\t\t\tRGB 4:4:4\n");
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb444(EdidRaw)) {
			xil_printf("\t\t\t\tYCrCb 4:4:4\n");
		}
		if (XVidC_EdidSuppFeatureDigColorEncYCrCb422(EdidRaw)) {
			xil_printf("\t\t\t\tYCrCb 4:2:2\n");
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("\tDisplay color type:\t");
		switch (XVidC_EdidGetFeatureAnaColorType(EdidRaw)) {
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_MCG:
			xil_printf("Monochrome or grayscale display.\n");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_RGB:
			xil_printf("RGB color display.\n");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB:
			xil_printf("Non-RGB color display.\n");
			break;
		case XVIDC_EDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF:
		default:
			xil_printf("Display color type is undefined.\n");
			break;
		}
	}

	xil_printf("\tOther supported features:\n");
	/* sRGB standard is the default color space. */
	xil_printf("\t\tsRGB standard ");
	if (XVidC_EdidIsFeaturePtmInc(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is not ");
	}
	xil_printf("the default color space.\n");
	/* Preferred timing mode includes. */
	xil_printf("\t\tPtm ");
	if (XVidC_EdidIsFeaturePtmInc(EdidRaw)) {
		xil_printf("includes ");
	}
	else {
		xil_printf("does not include ");
	}
	xil_printf("the native pixel format and preferred refresh rate.\n");
	/* Continuous frequency. */
	xil_printf("\t\tDisplay ");
	if (XVidC_EdidIsFeatureContFreq(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is non-");
	}
	xil_printf("continuous frequency.\n");
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for color characteristics and prints this information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintColorChar(u8 *EdidRaw)
{
	xil_printf("Color characterisitics:\n");
	xil_printf("\tRed_x:\t\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcRedX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcRedX(EdidRaw), 1000000000));
	xil_printf("\tRed_y:\t\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcRedY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcRedY(EdidRaw), 1000000000));
	xil_printf("\tGreen_x:\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcGreenX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcGreenX(EdidRaw), 1000000000));
	xil_printf("\tGreen_y:\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcGreenY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcGreenY(EdidRaw), 1000000000));
	xil_printf("\tBlue_x:\t\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcBlueX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcBlueX(EdidRaw), 1000000000));
	xil_printf("\tBlue_y:\t\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcBlueY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcBlueY(EdidRaw), 1000000000));
	xil_printf("\tWhite_x:\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcWhiteX(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcWhiteX(EdidRaw), 1000000000));
	xil_printf("\tWhite_y:\t\t%d.%09d +- 0.0005\n",
		(u32)XVidC_EdidGetCcWhiteY(EdidRaw),
		FLOAT_FRAC_TO_U32(XVidC_EdidGetCcWhiteY(EdidRaw), 1000000000));
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for supported established timings and prints this
 * information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintEstTimings(u8 *EdidRaw)
{
	xil_printf("Established timings:\n");
	if (XVidC_EdidSuppEstTimings720x400_70(EdidRaw)) {
		xil_printf("\t720x400 @ 70Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings720x400_88(EdidRaw)) {
		xil_printf("\t720x400 @ 88Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings640x480_60(EdidRaw)) {
		xil_printf("\t640x480 @ 60Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings640x480_67(EdidRaw)) {
		xil_printf("\t640x480 @ 67Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings640x480_72(EdidRaw)) {
		xil_printf("\t640x480 @ 72Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings640x480_75(EdidRaw)) {
		xil_printf("\t640x480 @ 75Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings800x600_56(EdidRaw)) {
		xil_printf("\t800x600 @ 56Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings800x600_60(EdidRaw)) {
		xil_printf("\t800x600 @ 60Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings800x600_72(EdidRaw)) {
		xil_printf("\t800x600 @ 72Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings800x600_75(EdidRaw)) {
		xil_printf("\t800x600 @ 75Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings832x624_75(EdidRaw)) {
		xil_printf("\t832x624 @ 75Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1024x768_87(EdidRaw)) {
		xil_printf("\t1024x768 @ 87Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1024x768_60(EdidRaw)) {
		xil_printf("\t1024x768 @ 60Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1024x768_70(EdidRaw)) {
		xil_printf("\t1024x768 @ 70Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1024x768_75(EdidRaw)) {
		xil_printf("\t1024x768 @ 75Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1280x1024_75(EdidRaw)) {
		xil_printf("\t1280x1024 @ 75Hz supported.\n");
	}
	if (XVidC_EdidSuppEstTimings1152x870_75(EdidRaw)) {
		xil_printf("\t1152x870 @ 75Hz supported.\n");
	}
	xil_printf("\tManufacturer specified timings field: 0x%02lx.\n",
					XVidC_EdidGetTimingsMan(EdidRaw));
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for supported standard timings and prints this information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintStdTimings(u8 *EdidRaw)
{
	u8 Index;

	xil_printf("Standard timings:\n");
	for (Index = 0; Index < 8; Index++) {
		if (EdidRaw[XVIDC_EDID_STD_TIMINGS_H(Index + 1)] <= 1) {
			/* Not a valid standard timing. */
			continue;
		}

		xil_printf("\t%dx%d @ %dHz supported.\n",
				XVidC_EdidGetStdTimingsH(EdidRaw, Index + 1),
				XVidC_EdidGetStdTimingsV(EdidRaw, Index + 1),
				XVidC_EdidGetStdTimingsFrr(EdidRaw, Index + 1));
	}
}

/******************************************************************************/
/**
 * This function decodes the supplied base Extended Display Identification Data
 * (EDID) structure for the preferred timing mode (PTM) and prints its timing
 * information.
 *
 * @param	EdidRaw is the base EDID structure to decode.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void Edid_PrintPtm(u8 *EdidRaw)
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

	u32 PixelClkKhz = ((Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_MSB] <<
			8) | Ptm[XVIDC_EDID_DTD_PTM_PIXEL_CLK_KHZ_LSB]) * 10;

	u32 HFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_MASK) >>
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HFPORCH_SHIFT) << 8)
			| Ptm[XVIDC_EDID_DTD_PTM_HFPORCH_LSB];

	u32 HSyncWidth = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_MASK) >>
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_HSPW_SHIFT) << 8) |
			Ptm[XVIDC_EDID_DTD_PTM_HSPW_LSB];

	u32 VFrontPorch = (((Ptm[XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2] &
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_MASK) >>
			XVIDC_EDID_DTD_PTM_XFPORCH_XSPW_U2_VFPORCH_SHIFT) << 8)
			| ((Ptm[XVIDC_EDID_DTD_PTM_VFPORCH_VSPW_L4] &
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

	xil_printf("Preferred timing mode:\n");
	xil_printf("\tHorizontal resolution:\t%d px\n"
			"\tVertical resolution:\t%d lines\n"
			"\tPixel clock:\t\t%d KHz\n"
			"\tHorizontal front porch:\t%d px\n"
			"\tHorizontal sync width:\t%d px\n"
			"\tHorizontal back porch:\t%d px\n"
			"\tHorizontal blanking:\t%d px\n"
			"\tHorizontal polarity:\t%d\n"
			"\tVertical front porch:\t%d px\n"
			"\tVertical sync width:\t%d px\n"
			"\tVertical back porch:\t%d px\n"
			"\tVertical blanking:\t%d px\n"
			"\tVertical polarity:\t%d\n",
			HActive, VActive, PixelClkKhz,
			HFrontPorch, HSyncWidth, HBackPorch, HBlank, HPolarity,
			VFrontPorch, VSyncWidth, VBackPorch, VBlank, VPolarity);

	xil_printf("\tInterlaced:\t\t%s\n",
					XVidC_EdidIsDtdPtmInterlaced(EdidRaw) ?
					"Yes." : "No (progressive).");
}

/******************************************************************************/
/**
 * This function calculates a checksum.
 *
 * @param	Data is a pointer to the data to calculate the checksum on.
 * @param	Size the number of bytes of Data to calculate the checksum on.
 *
 * @return	The checksum value (truncated to a 8 bits).
 *
 * @note	None.
 *
*******************************************************************************/
static u8 Edid_CalculateChecksum(u8 *Data, u8 Size)
{
	u8 Index;
	u8 Sum = 0;

	for (Index = 0; Index < Size; Index++) {
		Sum += Data[Index];
	}

	return Sum;
}
