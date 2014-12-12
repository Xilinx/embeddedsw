/*
 * Edid_Print.c
 *
 *  Created on: Nov 9, 2014
 *      Author: andreis
 */



#include "string.h"
#include "xedid.h"
#include "xedid_print_example.h"
#include "xil_printf.h"
#include "xstatus.h"

#define FLOAT_FRAC_TO_U32(V, D) ((u32)(V * D) - (((u32)V) * D))

static void Edid_Print_BaseVPId(u8 *EdidRaw);
static void Edid_Print_BaseVerRev(u8 *EdidRaw);
static void Edid_Print_BaseBasicDisp(u8 *EdidRaw);
static void Edid_Print_ColorChar(u8 *EdidRaw);
static void Edid_Print_EstTimings(u8 *EdidRaw);
static void Edid_Print_StdTimings(u8 *EdidRaw);
static u8 Edid_CalculateChecksum(u8 *Data, u8 Size);

u32 Edid_PrintDecodeAll(XDptx *InstancePtr, u8 Lct, u8 *Rad)
{
	u32 Status;
	u8 EdidBase[128];
	u8 Index;
	u8 NumExt;

	xil_printf("*** EDID for: LCT = %d", Lct);
	if ((Lct - 1) > 0) {
		xil_printf("; Rad = ");
	}
	for (Index = 0; Index < (Lct - 1); Index++) {
		xil_printf("%d ", Rad[Index]);
	}
	xil_printf("***\n");

	Status = XDptx_GetRemoteEdidBlock(InstancePtr, EdidBase, 0, Lct, Rad);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	//DpTools_PrintData(EdidBase, 128);
	//Edid_PrintDecodeBase(EdidBase);

	NumExt = XEDID_GET_EXT_BLK_COUNT(EdidBase);

	/*
	u8 EdidExtBlocks[NumExt * 128];

	DpTools_GetEdidAllExtBlocks(InstancePtr, NumExt, EdidExtBlocks, Lct, Rad);

	xil_printf("\nExtension blocks ::::::::::::::::::::::\n");
	for (Index = 0; Index < NumExt; Index++) {
		//DpTools_PrintData(EdidExtBlocks + (Index * 128), 128);
		Status = dptools_edid_parse_dispid(EdidExtBlocks + (Index * 128));
		if (Status != XST_SUCCESS) {
			xil_printf("\tNot a DispID extension block (tag: 0x%02lx)\n", EdidExtBlocks[Index * 128]);
		}
		xil_printf("\n");
	}
	*/

	xil_printf("\n::::::::::::::::::::::::::::::::::::::::::::::::\n");

	return Status;
}

u32 Edid_PrintDecodeBase(u8 *EdidRaw)
{
	/* Check valid header. */
	if (XEDID_IS_HEADER_VALID(EdidRaw)) {
		xil_printf("\nEDID header is: 00 FF FF FF FF FF FF 00\n");
	}
	else {
		xil_printf("\nNot an EDID base block...\n");
		return XST_FAILURE;
	}

	/* Obtain vendor and product identification information. */
	Edid_Print_BaseVPId(EdidRaw);
	Edid_Print_BaseVerRev(EdidRaw);
	Edid_Print_BaseBasicDisp(EdidRaw);
	Edid_Print_ColorChar(EdidRaw);
	Edid_Print_EstTimings(EdidRaw);
	Edid_Print_StdTimings(EdidRaw);

	xil_printf("Descriptors:\n");
	xil_printf("\tFirst tag: 0x%02lx 0x%02lx\n", EdidRaw[0x36], EdidRaw[0x38]);
	xil_printf("\tSecond tag: 0x%02lx 0x%02lx\n", EdidRaw[0x48], EdidRaw[0x4A]);
	xil_printf("\tThird tag: 0x%02lx 0x%02lx\n", EdidRaw[0x5A], EdidRaw[0x5C]);
	xil_printf("\tFourth tag: 0x%02lx 0x%02lx\n", EdidRaw[0x6C], EdidRaw[0x6E]);

	xil_printf("Number of extensions:\t%d\n", XEDID_GET_EXT_BLK_COUNT(EdidRaw));
	xil_printf("Checksum:\t\t0x%02lx -> Calculated sum = 0x%02lx (== 0x00)\n",
			XEDID_GET_CHECKSUM(EdidRaw),
			Edid_CalculateChecksum(EdidRaw, 128));

	return XST_SUCCESS;
}

static void Edid_Print_BaseVPId(u8 *EdidRaw)
{
	char ManName[4];
	XEDID_GET_VPI_ID_MAN_NAME(EdidRaw, ManName);

	/* Vendor and product identification. */
	xil_printf("Vendor and product identification:\n");
	xil_printf("\tID manufacturer name:\t%s\n", ManName);
	xil_printf("\tID product code:\t0x%04lx\n", XEDID_GET_VPI_ID_PROD_CODE(EdidRaw));
	xil_printf("\tID serial number:\t0x%08lx\n", XEDID_GET_VPI_SN(EdidRaw));
	if (XEDID_IS_VPI_YEAR_MODEL(EdidRaw)) {
		xil_printf("\tModel year:\t\t%d\n", XEDID_GET_VPI_YEAR(EdidRaw));
	}
	else if (XEDID_GET_VPI_WEEK_MAN(EdidRaw) == 0x00) {
		xil_printf("\tManufactured:\t\tYear = %d ; Week not specified.\n", XEDID_GET_VPI_YEAR(EdidRaw));
	}
	else {
		xil_printf("\tManufactured:\t\tYear = %d ; Week = %d\n", XEDID_GET_VPI_YEAR(EdidRaw), XEDID_GET_VPI_WEEK_MAN(EdidRaw));
	}
}

static void Edid_Print_BaseVerRev(u8 *EdidRaw)
{
	/* EDID structure version and revision. */
	xil_printf("EDID structure version and revision: %d.%d\n", XEDID_GET_STRUCT_VER(EdidRaw), XEDID_GET_STRUCT_REV(EdidRaw));
}

static void Edid_Print_BaseBasicDisp(u8 *EdidRaw)
{
	/* Basic display parameters and features. */
	xil_printf("Basic display parameters and features:\n");
	if (XEDID_IS_BDISP_VID_VSI_DIGITAL(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("\tVideo signal interface is digital.\n");

		if (XEDID_GET_BDISP_VID_DIG_BPC(EdidRaw) != XEDID_BDISP_VID_DIG_BPC_UNDEF) {
			xil_printf("\tColor bit depth:\t%d\n", XEDID_GET_BDISP_VID_DIG_BPC(EdidRaw));
		}
		else {
			xil_printf("\tColor bit depth is undefined.\n");
		}

		switch (XEDID_GET_BDISP_VID_DIG_VIS(EdidRaw)) {
			case XEDID_BDISP_VID_DIG_VIS_DVI:
				xil_printf("\tDVI is supported.\n");
				break;
			case XEDID_BDISP_VID_DIG_VIS_HDMIA:
				xil_printf("\tHDMI-a is supported.\n");
				break;
			case XEDID_BDISP_VID_DIG_VIS_HDMIB:
				xil_printf("\tHDMI-b is supported.\n");
				break;
			case XEDID_BDISP_VID_DIG_VIS_MDDI:
				xil_printf("\tMDDI is supported.\n");
				break;
			case XEDID_BDISP_VID_DIG_VIS_DP:
				xil_printf("\tDisplayPort is supported.\n");
				break;
			default:
				xil_printf("\tDigital interface is not defined.\n");
				break;
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("\tVideo signal interface is analog.\n");

		xil_printf("\tSignal level standard:\t");

		switch (XEDID_GET_BDISP_VID_ANA_SLS(EdidRaw)) {
			case XEDID_BDISP_VID_ANA_SLS_0700_0300_1000:
				xil_printf("0.700 : 0.300 : 1.000 Vp-p ");
				break;
			case XEDID_BDISP_VID_ANA_SLS_0714_0286_1000:
				xil_printf("0.714 : 0.286 : 1.000 Vp-p ");
				break;
			case XEDID_BDISP_VID_ANA_SLS_1000_0400_1400:
				xil_printf("1.000 : 0.400 : 1.400 Vp-p ");
				break;
			case XEDID_BDISP_VID_ANA_SLS_0700_0000_0700:
			default:
				xil_printf("0.700 : 0.000 : 0.700 V p-p");
				break;
		}
		xil_printf("(Video : Sync : Total)\n");

		xil_printf("\tVideo setup:\t\t");
		if (XEDID_SUPP_BDISP_VID_ANA_VID_SETUP(EdidRaw)) {
			xil_printf("Blank-to-black setup or pedestal.\n");
		}
		else {
			xil_printf("Blank level = black level.\n");
		}

		xil_printf("\tSynchronization types:\n");
		xil_printf("\t\tSeparate sync H & V signals ");
		if (XEDID_SUPP_BDISP_VID_ANA_SEP_SYNC_HV(EdidRaw)) {
			xil_printf("are supported.\n");
		}
		else {
			xil_printf("are not supported.\n");
		}
		xil_printf("\t\tComposite sync signal on horizontal ");
		if (XEDID_SUPP_BDISP_VID_ANA_COMP_SYNC_H(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}
		xil_printf("\t\tComposite sync signal on green video ");
		if (XEDID_SUPP_BDISP_VID_ANA_COMP_SYNC_G(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}

		xil_printf("\tSerrations on the vertical sync ");
		if (XEDID_SUPP_BDISP_VID_ANA_SERR_V_SYNC(EdidRaw)) {
			xil_printf("is supported.\n");
		}
		else {
			xil_printf("is not supported.\n");
		}
	}

	if (XEDID_IS_BDISP_SSAR_SS(EdidRaw)) {
		xil_printf("\tScreen size (HxV):\t%dx%d(cm)\n", XEDID_GET_BDISP_SSAR_H(EdidRaw), XEDID_GET_BDISP_SSAR_V(EdidRaw));
	}
	else if (XEDID_IS_BDISP_SSAR_AR_L(EdidRaw)) {
		xil_printf("\tAspect ratio (H:V):\t");
		switch(XEDID_GET_BDISP_SSAR_H(EdidRaw)) {
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
						(u32)XEDID_GET_BDISP_SSAR_AR_L(EdidRaw),
						FLOAT_FRAC_TO_U32(XEDID_GET_BDISP_SSAR_AR_L(EdidRaw), 1000));
				break;
		}
		xil_printf("(landscape)\n");
	}
	else if (XEDID_IS_BDISP_SSAR_AR_P(EdidRaw)) {
		xil_printf("\tAspect ratio (H:V):\t");
		switch(XEDID_GET_BDISP_SSAR_V(EdidRaw)) {
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
				xil_printf("%d.%03d:1 ", (u32)XEDID_GET_BDISP_SSAR_AR_P(EdidRaw),
						FLOAT_FRAC_TO_U32(XEDID_GET_BDISP_SSAR_AR_P(EdidRaw), 1000));
				break;
		}
		xil_printf("(portrait)\n");
	}
	else {
		xil_printf("\tScreen size and aspect ratio are undefined.\n");
	}

	if (XEDID_IS_BDISP_GAMMA_IN_EXT(EdidRaw)) {
		xil_printf("\tGamma is defined in an extension block.\n");
	}
	else {
		xil_printf("\tGamma:\t\t\t%d.%02d\n", (u32)XEDID_GET_BDISP_GAMMA(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_BDISP_GAMMA(EdidRaw), 100));
	}

	xil_printf("\tDisplay power management:\n");
	xil_printf("\t\t\t\tStandby mode ");
	if (XEDID_SUPP_BDISP_FEATURE_PM_STANDBY(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}
	xil_printf("\t\t\t\tSuspend mode ");
	if (XEDID_SUPP_BDISP_FEATURE_PM_SUSPEND(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}
	xil_printf("\t\t\t\tActive off = very low power ");
	if (XEDID_SUPP_BDISP_FEATURE_PM_OFF_VLP(EdidRaw)) {
		xil_printf("is supported.\n");
	}
	else {
		xil_printf("is not supported.\n");
	}

	if (XEDID_IS_BDISP_VID_VSI_DIGITAL(EdidRaw)) {
		/* Input is a digital video signal interface. */
		xil_printf("\tSupported color encoding format(s):\n");
		xil_printf("\t\t\t\tRGB 4:4:4\n");
		if (XEDID_SUPP_BDISP_FEATURE_DIG_COLORENC_YCRCB444(EdidRaw)) {
			xil_printf("\t\t\t\tYCrCb 4:4:4\n");
		}
		if (XEDID_SUPP_BDISP_FEATURE_DIG_COLORENC_YCRCB422(EdidRaw)) {
			xil_printf("\t\t\t\tYCrCb 4:2:2\n");
		}
	}
	else {
		/* Input is an analog video signal interface. */
		xil_printf("\tDisplay color type:\t");
		switch (XEDID_GET_BDISP_FEATURE_ANA_COLORTYPE(EdidRaw)) {
		case XEDID_BDISP_FEATURE_ANA_COLORTYPE_MCG:
			xil_printf("Monochrome or grayscale display.\n");
			break;
		case XEDID_BDISP_FEATURE_ANA_COLORTYPE_RGB:
			xil_printf("RGB color display.\n");
			break;
		case XEDID_BDISP_FEATURE_ANA_COLORTYPE_NRGB:
			xil_printf("Non-RGB color display.\n");
			break;
		case XEDID_BDISP_FEATURE_ANA_COLORTYPE_UNDEF:
		default:
			xil_printf("Display color type is undefined.\n");
			break;
		}
	}

	xil_printf("\tOther supported features:\n");
	/* sRGB standard is the default color space. */
	xil_printf("\t\tsRGB standard ");
	if (XEDID_IS_BDISP_FEATURE_SRGB_DEF(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is not ");
	}
	xil_printf("the default color space.\n");
	/* Preferred timing mode includes. */
	xil_printf("\t\tPtm ");
	if (XEDID_IS_BDISP_FEATURE_PTM_INC(EdidRaw)) {
		xil_printf("includes ");
	}
	else {
		xil_printf("does not include ");
	}
	xil_printf("the native pixel format and preferred refresh rate of the display device.\n");
	/* Continuous frequency. */
	xil_printf("\t\tDisplay ");
	if (XEDID_IS_BDISP_FEATURE_CONTFREQ(EdidRaw)) {
		xil_printf("is ");
	}
	else {
		xil_printf("is non-");
	}
	xil_printf("continuous frequency.\n");
}

static void Edid_Print_ColorChar(u8 *EdidRaw)
{
	xil_printf("Color characterisitics:\n");
	xil_printf("\tRed_x:\t\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_REDX(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_REDX(EdidRaw), 1000000000));
	xil_printf("\tRed_y:\t\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_REDY(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_REDY(EdidRaw), 1000000000));
	xil_printf("\tGreen_x:\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_GREENX(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_GREENX(EdidRaw), 1000000000));
	xil_printf("\tGreen_y:\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_GREENY(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_GREENY(EdidRaw), 1000000000));
	xil_printf("\tBlue_x:\t\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_BLUEX(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_BLUEX(EdidRaw), 1000000000));
	xil_printf("\tBlue_y:\t\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_BLUEY(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_BLUEY(EdidRaw), 1000000000));
	xil_printf("\tWhite_x:\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_WHITEX(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_WHITEX(EdidRaw), 1000000000));
	xil_printf("\tWhite_y:\t\t%d.%09d +- 0.0005\n", (u32)XEDID_GET_CC_WHITEY(EdidRaw), FLOAT_FRAC_TO_U32(XEDID_GET_CC_WHITEY(EdidRaw), 1000000000));
}

static void Edid_Print_EstTimings(u8 *EdidRaw)
{
	xil_printf("Established timings:\n");
	if (XEDID_SUPP_EST_TIMINGS_720x400_70(EdidRaw)) {
		xil_printf("\t720x400 @ 70Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_720x400_88(EdidRaw)) {
		xil_printf("\t720x400 @ 88Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_640x480_60(EdidRaw)) {
		xil_printf("\t640x480 @ 60Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_640x480_67(EdidRaw)) {
		xil_printf("\t640x480 @ 67Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_640x480_72(EdidRaw)) {
		xil_printf("\t640x480 @ 72Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_640x480_75(EdidRaw)) {
		xil_printf("\t640x480 @ 75Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_800x600_56(EdidRaw)) {
		xil_printf("\t800x600 @ 56Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_800x600_60(EdidRaw)) {
		xil_printf("\t800x600 @ 60Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_800x600_72(EdidRaw)) {
		xil_printf("\t800x600 @ 72Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_800x600_75(EdidRaw)) {
		xil_printf("\t800x600 @ 75Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_832x624_75(EdidRaw)) {
		xil_printf("\t832x624 @ 75Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1024x768_87(EdidRaw)) {
		xil_printf("\t1024x768 @ 87Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1024x768_60(EdidRaw)) {
		xil_printf("\t1024x768 @ 60Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1024x768_70(EdidRaw)) {
		xil_printf("\t1024x768 @ 70Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1024x768_75(EdidRaw)) {
		xil_printf("\t1024x768 @ 75Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1280x1024_75(EdidRaw)) {
		xil_printf("\t1280x1024 @ 75Hz supported.\n");
	}
	if (XEDID_SUPP_EST_TIMINGS_1152x870_75(EdidRaw)) {
		xil_printf("\t1152x870 @ 75Hz supported.\n");
	}
	xil_printf("\tManufacturer specified timings field: 0x%02lx.\n", XEDID_GET_TIMINGS_MAN(EdidRaw));
}

static void Edid_Print_StdTimings(u8 *EdidRaw)
{
	/* Standard timings. */
	u8 Index;

	xil_printf("Standard timings:\n");
	for (Index = 0; Index < 8; Index++) {
		xil_printf("\t%dx%d @ %dHz supported.\n",
				XEDID_GET_STD_TIMINGS_H(EdidRaw, Index + 1),
				XEDID_GET_STD_TIMINGS_V(EdidRaw, Index + 1),
				XEDID_GET_STD_TIMINGS_FRR(EdidRaw, Index + 1));
	}
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
