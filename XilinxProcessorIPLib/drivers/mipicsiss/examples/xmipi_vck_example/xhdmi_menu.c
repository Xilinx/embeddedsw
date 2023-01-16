/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_menu.c
*
* This file contains the Xilinx Menu implementation as used
* in the HDMI example design. Please see xhdmi_menu.h for more details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* X.X   ..   DD-MM-YYYY ..
* 1.0   RHe  10-07-2015 Initial version
* 1.1   MG   16-07-2015 Expanded menu options
* 1.2   MG   23-07-2015 Added EDID options
* 1.3   MG   05-08-2015 Added Audio menu
* 1.4   MG   03-02-2016 Added HDCP menu
* 1.5   MH   06-24-2016 Added HDCP repeater support.
* 1.6   YH   03-01-2017 Remove Video Pattern from UART menu when it is not
*                             enabled in TPG XGUI (CR-961051)
*                       Remove XV_HdmiRxSs_LoadDefaultEdid from UART menu
*                       Added 480i and 576i Support in the UART
* 1.7   GM   01-02-2017 Change PLL Layout menu access to GTX only in
*                              XHdmi_GtPllLayoutMenu API
* 1.8   mmo  02-03-2017 Added Support to Set HDMI TX to be on HDMI or
*                               DVI Mode.
* 1.9   mmo  26-04-2017 Change PLL Layout menu access to GTX only in
*                              XHdmi_DisplayGtPllLayoutMenu API
* 1.10  YH   08-06-2017 Set default 4K resolution for GTPE2 to 4KP30
*                              for RGB and YUV444
*                       Disabled deep color settings at max resolution
*                       Changed printf usage to xil_printf
*                       Changed "\n\r" in xil_printf calls to "\r\n"
*       MH   09-08-2017 Added HDCP Debug menu
*       GM   18-08-2017 Clean up the flow when pressing "p" (Force
*                               Passthrough)
*       mmo  18-08-2017 Added Support to Custom Resolution in the Resolution
*                               menu
*       GM   05-09-2017 Changed PLL Layout routine to toggle HPD to improve
*                               stability
* 1.11  mmo  29-12-2017 Added EDID Parsing Capability
*       EB   16-01-2018 Added Audio Channel Menu
*       EB   23-01-2018 Reset the counter tagged to the events logged whenever
*                               log is displayed
* 1.12  EB   09-04-2018 Fixed messages printing issue
* 3.03  YB   08-14-2018 Updating the Hdcp Menu to remove Repeater options if
*                       'ENABLE_HDCP_REPEATER' macro is not selected.
* 3.04  mmo  08-03-2019 Updating the global variable "EdidHdmi20_t" during
*                               EDID parsing through menu.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif
#include "xvidc_edid_ext.h"
#include "xhdmi_example.h"

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
/* Create entry for each mode in the custom table */
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode
	[(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] = {
	/* Custom Modes . */
	{
		XVIDC_VM_1152x864_60_P, "1152x864@60Hz", XVIDC_FR_60HZ,
		{
			1152, 64, 120, 184, 1520, 0,
			864, 3, 4, 26, 897, 0, 0, 0, 0, 1
		}
	}
};
#endif
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif

static void XHdmi_DisplayMainMenu(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static void XHdmi_DisplayEdidMenu(void);
static void XHdmi_DisplayResolutionMenu(void);
static void XHdmi_DisplayFrameRateMenu(void);
static void XHdmi_DisplayColorDepthMenu(void);
static void XHdmi_DisplayColorSpaceMenu(void);
static void XHdmi_DisplayVideoMenu(void);
#endif
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
extern u8 PLLBondedCheck (void);
#endif
extern void Info(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern void EnableColorBar(XHdmiphy1 *Hdmiphy1Ptr,
                                   XV_HdmiTxSs *HdmiTxSsPtr,
				   XVidC_VideoMode VideoMode,
				   XVidC_ColorFormat ColorFormat,
					   XVidC_ColorDepth Bpc);;
extern void UpdateColorFormat(XHdmiphy1 *Hdmiphy1Ptr,
               XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorFormat ColorFormat);
extern void UpdateColorDepth(XHdmiphy1 *Hdmiphy1Ptr,
                 XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorDepth ColorDepth);
extern void UpdateFrameRate(XHdmiphy1 *Hdmiphy1Ptr,
                   XV_HdmiTxSs *pHdmiTxSs, XVidC_FrameRate FrameRate);
extern void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr);
extern void CloneTxEdid(void);
#endif

/************************* Variable Definitions *****************************/

/**
* This table contains the function pointers for all possible states.
* The order of elements must match the XHdmi_MenuType enumerator definitions.
*/
static XHdmi_MenuFuncType* const XHdmi_MenuTable[XHDMI_NUM_MENUS] = {
	XHdmi_MainMenu,
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XHdmi_EdidMenu,
	XHdmi_ResolutionMenu,
	XHdmi_FrameRateMenu,
	XHdmi_ColorDepthMenu,
	XHdmi_ColorSpaceMenu,
	XHdmi_VideoMenu,
#endif
};

extern XHdmiphy1 Hdmiphy1;               /* VPhy structure */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern XV_HdmiTxSs HdmiTxSs;       /* HDMI TX SS structure */

extern u8 TxCableConnect;
#endif
extern u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;// TX busy flag. This flag is set while the TX is initialized

/*HDMI EDID*/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern EdidHdmi20 EdidHdmi20_t;
#endif
extern u8 Buffer[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function takes care of the HDMI menu initialization.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @return None
*
*
******************************************************************************/
void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress)
{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	u32 Status;
#endif
#endif
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
	InstancePtr->UartBaseAddress = UartBaseAddress;
	InstancePtr->Value = 0;
	InstancePtr->WaitForColorbar = (FALSE);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	//Initialize and Add Custom Resolution in to the Video Table
	//Added for the resolution menu
	/* Example : User registers custom timing table */
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
			 (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
           xil_printf("ERR: Unable to register custom timing table\r\n\r\n");
	}
#endif
#endif

	// Show main menu
	XHdmi_DisplayMainMenu();
}


/*****************************************************************************/
/**
*
* This function resets the menu to the main menu.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @return None
*
*
******************************************************************************/
void XHdmi_MenuReset(XHdmi_Menu *InstancePtr)
{
	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI main menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayMainMenu(void)
{
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("i - Info\r\n");
	xil_printf("       => Shows information about the HDMI \
                                           RX stream, HDMI TX stream, \r\n");
	xil_printf("          GT transceivers and PLL settings.\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("c - Colorbar\r\n");
	xil_printf("    => Displays the colorbar on the source output.\r\n");
	xil_printf("r - Resolution\r\n");
	xil_printf("   => Change the video resolution of the colorbar.\r\n");
	xil_printf("f - Frame rate\r\n");
	xil_printf("       => Change the frame rate of the colorbar.\r\n");
	xil_printf("d - Color depth\r\n");
	xil_printf("       => Change the color depth of the colorbar.\r\n");
	xil_printf("s - Color space\r\n");
	xil_printf("       => Change the color space of the colorbar.\r\n");
#endif
	xil_printf("z - GT & HDMI TX/RX log\r\n");
	xil_printf("     => Shows log information for GT & HDMI TX/RX.\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("e - Edid\r\n");
	xil_printf("       => Display and set edid.\r\n");
	xil_printf("a - Audio\r\n");
	xil_printf("       => Audio options.\r\n");
	xil_printf("v - Video\r\n");
	xil_printf("       => Video pattern options.\r\n");
	xil_printf("m - Set HDMI Mode\r\n");
	xil_printf("n - Set DVI Mode\r\n");
#endif

	xil_printf("\r\n\r\n");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI main menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
#endif
	// Default
	Menu = XHDMI_MAIN_MENU;

	switch (Input) {
			// Info
		case ('i') :
		case ('I') :
			Info();
			Menu = XHDMI_MAIN_MENU;
			break;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// Colorbar
		case ('c') :
		case ('C') :
			if  (TxCableConnect) {
				TxBusy = (FALSE);
			}
			EnableColorBar(&Hdmiphy1,
						   &HdmiTxSs,
						   HdmiTxSsVidStreamPtr->VmId,
						   HdmiTxSsVidStreamPtr->ColorFormatId,
						   HdmiTxSsVidStreamPtr->ColorDepth);
			Menu = XHDMI_MAIN_MENU;
			break;

			// Resolution
		case ('r') :
		case ('R') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				Menu = XHDMI_RESOLUTION_MENU;
				XHdmi_DisplayResolutionMenu();
			}

			// Pass-through
			else {
			xil_printf("The example design is in \
                                             pass-through mode.\r\n");
			xil_printf("In this mode the video parameters \
                                                  can't be changed.\r\n");
			}
			break;

			// Frame rate
		case ('f') :
		case ('F') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				XHdmi_DisplayFrameRateMenu();
				Menu = XHDMI_FRAMERATE_MENU;
			}

			// Pass-through
			else {
			xil_printf("The example design is in pass-through mode.\r\n");
			xil_printf("In this mode the video parameters can't be changed.\r\n");
			}
			break;

			// Color depth
		case ('d') :
		case ('D') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				XHdmi_DisplayColorDepthMenu();
				Menu = XHDMI_COLORDEPTH_MENU;
			}

			// Pass-through
			else {
			 xil_printf("The example design is in \
                                                pass-through mode.\r\n");
		         xil_printf("In this mode the video \
                                      parameters can't be changed.\r\n");
			}
			break;

			// Color space
		case ('s') :
		case ('S') :
			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				XHdmi_DisplayColorSpaceMenu();
				Menu = XHDMI_COLORSPACE_MENU;
			}

			// Pass-through
			else {
			xil_printf("The example design is in \
                                                 pass-through mode.\r\n");
			xil_printf("In this mode the video \
                                       parameters can't be changed.\r\n");
			}
			break;
#endif

			// GT & HDMI TX/RX log
		case ('z') :
		case ('Z') :
			XHdmiphy1_LogDisplay(&Hdmiphy1);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_LogDisplay(&HdmiTxSs);
#endif
			Menu = XHDMI_MAIN_MENU;
			break;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// HDMI Mode
		case ('m') :
		case ('M') :
			xil_printf("Set TX Mode To HDMI.\r\n");
			XV_HdmiTxSS_SetHdmiMode(&HdmiTxSs);
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			Menu = XHDMI_MAIN_MENU;
			break;

			// DVI Mode
		case ('n') :
		case ('N') :
			xil_printf("Set TX Mode To DVI .\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			XV_HdmiTxSS_SetDviMode(&HdmiTxSs);
			Menu = XHDMI_MAIN_MENU;
			break;
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// Edid
		case ('e') :
		case ('E') :
			XHdmi_DisplayEdidMenu();
			Menu = XHDMI_EDID_MENU;
			break;

			// Video
		case ('v') :
		case ('V') :
			XHdmi_DisplayVideoMenu();
			Menu = XHDMI_VIDEO_MENU;
			break;
#endif

		case ('o') :
		case ('O') :
			/* Toggle GTWiz_RESET_ALL */
		XHdmiphy1_WriteReg(XPAR_HDMIPHY1_0_BASEADDR, 0x14,
		(XHdmiphy1_ReadReg(XPAR_HDMIPHY1_0_BASEADDR, 0x14) | 0x1));
		XHdmiphy1_WriteReg(XPAR_HDMIPHY1_0_BASEADDR, 0x14,
		(XHdmiphy1_ReadReg(XPAR_HDMIPHY1_0_BASEADDR, 0x14) & ~0x1));
			Menu = XHDMI_MAIN_MENU;
			break;

		case ('k') :
		case ('K') :
			I2cClk(0,148500000);
			Menu = XHDMI_MAIN_MENU;
			break;

		case ('y') :
		case ('Y') :
			XHdmiphy1_RegisterDebug(&Hdmiphy1);
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			XHdmi_DisplayMainMenu();
			Menu = XHDMI_MAIN_MENU;
			break;
	}

	return Menu;
}


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function displays the HDMI resolution menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayResolutionMenu(void) {
	xil_printf("\r\n");
	xil_printf("---------------------------\r\n");
	xil_printf("---   RESOLUTION MENU   ---\r\n");
	xil_printf("---------------------------\r\n");
	xil_printf(" 1 -  720 x 480p\r\n");
	xil_printf(" 2 -  720 x 576p\r\n");
	xil_printf(" 3 - 1280 x 720p\r\n");
	xil_printf(" 4 - 1680 x 720p\r\n");
	xil_printf(" 5 - 1920 x 1080p\r\n");
	xil_printf(" 6 - 2560 x 1080p\r\n");
	xil_printf(" 7 - 3840 x 2160p\r\n");
	xil_printf(" 8 - 4096 x 2160p\r\n");
	xil_printf(" 9 - 1920 x 1080i\r\n");
	xil_printf("10 -  640 x 480p (VGA / DMT0659)\r\n");
	xil_printf("11 -  800 x 600p (SVGA / DMT0860)\r\n");
	xil_printf("12 - 1024 x 768p (XGA / DMT1060)\r\n");
	xil_printf("13 - 1280 x 768p (WXGA / CVT1260E)\r\n");
	xil_printf("14 - 1366 x 768p (WXGA+ / DMT1360)\r\n");
	xil_printf("15 - 1280 x 1024p (SXGA / DMT1260G)\r\n");
	xil_printf("16 - 1680 x 1050p (WSXGA+ / CVT1660D)\r\n");
	xil_printf("17 - 1600 x 1200p (UXGA / DMT1660)\r\n");
	xil_printf("18 - 1920 x 1200p (WUXGA / CVT1960D)\r\n");
#if (XPAR_XV_HDMITXSS_0_INCLUDE_LOW_RESO_VID == 1)
	xil_printf("19 -  720 x 480i (NTSC)\r\n");
	xil_printf("20 -  720 x 576i (PAL)\r\n");
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	xil_printf("21 - 1152 x 864p (Custom)\r\n");
#endif
#else
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	xil_printf("19 - 1152 x 864p (Custom)\r\n");
#endif
#endif
	xil_printf("99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI resolution menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr,u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_VideoMode	VideoMode;

	// Default
	Menu = XHDMI_RESOLUTION_MENU;
	VideoMode = XVIDC_VM_NO_INPUT;

	switch (Input) {
			// 720 x 480p
		case 1 :
			VideoMode = XVIDC_VM_720x480_60_P;
			break;

			// 720  x 576p
		case 2 :
			VideoMode = XVIDC_VM_720x576_50_P;
			break;

			// 1280 x 720p
		case 3 :
			VideoMode = XVIDC_VM_1280x720_60_P;
			break;

			// 1680 x 720p
		case 4 :
			VideoMode = XVIDC_VM_1680x720_60_P;
			break;

			// 1920 x 1080p
		case 5 :
			VideoMode = XVIDC_VM_1920x1080_60_P;
			break;

			// 2560 x 1080p
		case 6 :
			VideoMode = XVIDC_VM_2560x1080_60_P;
			break;

			// 3840 x 2160p
		case 7 :
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XVPHY_GTPE2)
			VideoMode = XVIDC_VM_3840x2160_60_P;
#else
			VideoMode = XVIDC_VM_3840x2160_30_P;
#endif
			break;

			// 4096 x 2160p
		case 8 :
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XVPHY_GTPE2)
			VideoMode = XVIDC_VM_4096x2160_60_P;
#else
			VideoMode = XVIDC_VM_4096x2160_30_P;
#endif
			break;

			// 1920 x 1080i
		case 9 :
			VideoMode = XVIDC_VM_1920x1080_60_I;
			break;

			// 640 x 480p (VGA)
		case 10 :
			VideoMode = XVIDC_VM_640x480_60_P;
			break;

			// 800 x 600p (SVGA)
		case 11 :
			VideoMode = XVIDC_VM_800x600_60_P;
			break;

			//  1024 x 768p (XGA)
		case 12 :
			VideoMode = XVIDC_VM_1024x768_60_P;
			break;

			// 1280 x 768p (WXGA)
		case 13 :
			VideoMode = XVIDC_VM_1280x768_60_P;
			break;

			// 1366 x 768p (WXGA+)
		case 14 :
			VideoMode = XVIDC_VM_1366x768_60_P;
			break;

			// 1280 x 1024p (SXGA)
		case 15 :
			VideoMode = XVIDC_VM_1280x1024_60_P;
			break;

			// 1680 x 1050p (WSXGA+)
		case 16 :
			VideoMode = XVIDC_VM_1680x1050_60_P;
			break;

			// 1600 x 1200p (UXGA)
		case 17 :
			VideoMode = XVIDC_VM_1600x1200_60_P;
			break;

			// 1920 x 1200p (WUXGA)
		case 18 :
			VideoMode = XVIDC_VM_1920x1200_60_P;
			break;
#if (XPAR_XV_HDMITXSS_0_INCLUDE_LOW_RESO_VID == 1)
			// 720 x 480i (NTSC)
		case 19 :
			VideoMode = XVIDC_VM_1440x480_60_I;
			break;

			// 720 x 576i (PAL)
		case 20 :
			VideoMode = XVIDC_VM_1440x576_50_I;
			break;

#if(CUSTOM_RESOLUTION_ENABLE == 1)
			//3840 x 2160p (SB) (Custom)
		case 21 :
			VideoMode = XVIDC_VM_1152x864_60_P;
			break;
#endif
#else
#if(CUSTOM_RESOLUTION_ENABLE == 1)
		case 19 :
			VideoMode = XVIDC_VM_1152x864_60_P;
			break;
#endif
#endif
			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayResolutionMenu();
			break;
	}

	if (VideoMode != XVIDC_VM_NO_INPUT) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		EnableColorBar(&Hdmiphy1, &HdmiTxSs,
                                VideoMode, XVIDC_CSF_RGB, XVIDC_BPC_8);
#endif
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI frame rate menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayFrameRateMenu(void) {
	xil_printf("\r\n");
	xil_printf("---------------------------\r\n");
	xil_printf("---   FRAME RATE MENU   ---\r\n");
	xil_printf("---------------------------\r\n");
	xil_printf("  1 -  24 Hz\r\n");
	xil_printf("  2 -  25 Hz\r\n");
	xil_printf("  3 -  30 Hz\r\n");
	xil_printf("  4 -  50 Hz\r\n");
	xil_printf("  5 -  60 Hz\r\n");
	xil_printf("  6 - 100 Hz\r\n");
	xil_printf("  7 - 120 Hz\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI frame rate menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_FrameRate FrameRate;

	// Default
	Menu = XHDMI_FRAMERATE_MENU;
	FrameRate = XVIDC_FR_UNKNOWN;

	switch (Input) {

			// 24 Hz
		case 1 :
			FrameRate = XVIDC_FR_24HZ;
			break;

			// 25 Hz
		case 2 :
			FrameRate = XVIDC_FR_25HZ;
			break;

			// 30 Hz
		case 3 :
			FrameRate = XVIDC_FR_30HZ;
			break;

			// 50 Hz
		case 4 :
			FrameRate = XVIDC_FR_50HZ;
			break;

			// 60 Hz
		case 5 :
			FrameRate = XVIDC_FR_60HZ;
			break;

			// 100 Hz
		case 6 :
			FrameRate = XVIDC_FR_100HZ;
			break;

			// 120 Hz
		case 7 :
			FrameRate = XVIDC_FR_120HZ;
			break;

			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayFrameRateMenu();
			break;
	}

	if (FrameRate != XVIDC_FR_UNKNOWN) {
		UpdateFrameRate(&Hdmiphy1, &HdmiTxSs, FrameRate);
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI color depth menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayColorDepthMenu(void) {
	XVidC_VideoStream *VidStrPtr;

	/* Check if TX is running at max resolution */
	VidStrPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XVPHY_GTPE2)
	if (((VidStrPtr->VmId == XVIDC_VM_3840x2160_60_P) ||
			(VidStrPtr->VmId == XVIDC_VM_4096x2160_60_P)) &&
#else
	if (((VidStrPtr->VmId == XVIDC_VM_3840x2160_30_P) ||
			(VidStrPtr->VmId == XVIDC_VM_4096x2160_30_P)) &&
#endif
			((VidStrPtr->ColorFormatId == XVIDC_CSF_RGB) ||
		 (VidStrPtr->ColorFormatId == XVIDC_CSF_YCRCB_444))) {
	xil_printf("Only 8 BPC is supported for the current resolution ");
	 xil_printf("and colorspace.\r\n");
		return;
	}

	xil_printf("\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("---   COLOR DEPTH MENU   ---\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("  1 - 24 bpp\r\n");
	xil_printf("  2 - 30 bpp\r\n");
	xil_printf("  3 - 36 bpp\r\n");
	xil_printf("  4 - 48 bpp\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI color depth menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr,u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_ColorDepth ColorDepth;
	XVidC_VideoStream *VidStrPtr;

	/* Check if TX is running at max resolution */
	VidStrPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XVPHY_GTPE2)
	if(((VidStrPtr->VmId == XVIDC_VM_3840x2160_60_P) ||
			(VidStrPtr->VmId == XVIDC_VM_4096x2160_60_P)) &&
#else
	if(((VidStrPtr->VmId == XVIDC_VM_3840x2160_30_P) ||
			(VidStrPtr->VmId == XVIDC_VM_4096x2160_30_P)) &&
#endif
			((VidStrPtr->ColorFormatId == XVIDC_CSF_RGB) ||
			 (VidStrPtr->ColorFormatId == XVIDC_CSF_YCRCB_444))) {
		xil_printf("Returning to main menu.\r\n");
		return XHDMI_MAIN_MENU;
	}


	// Default
	ColorDepth = XVIDC_BPC_UNKNOWN;
	Menu = XHDMI_COLORDEPTH_MENU;

	switch (Input) {
			// 24 bpp
		case 1 :
			ColorDepth = XVIDC_BPC_8;
			break;

			// 30 bpp
		case 2 :
			ColorDepth = XVIDC_BPC_10;
			break;

			// 36 bpp
		case 3 :
			ColorDepth = XVIDC_BPC_12;
			break;

			// 48 bpp
		case 4 :
			ColorDepth = XVIDC_BPC_16;
			break;

			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayColorDepthMenu();
			break;
	}

	if (ColorDepth != XVIDC_BPC_UNKNOWN) {
		UpdateColorDepth(&Hdmiphy1, &HdmiTxSs, ColorDepth);
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI color space menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayColorSpaceMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("---   COLOR SPACE MENU   ---\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("  1 - RGB\r\n");
	xil_printf("  2 - YUV444\r\n");
	xil_printf("  3 - YUV422\r\n");
#if (XPAR_XV_HDMITXSS_0_INCLUDE_YUV420_SUP == 1)
	xil_printf("  4 - YUV420\r\n");
#endif
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI color space menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_ColorFormat ColorFormat;

	// Default
	ColorFormat = XVIDC_CSF_UNKNOWN;
	Menu = XHDMI_COLORSPACE_MENU;

	switch (Input) {
			// RGB
		case 1 :
			ColorFormat = XVIDC_CSF_RGB;
			break;

			// YUV444
		case 2 :
			ColorFormat = XVIDC_CSF_YCRCB_444;
			break;

			// YUV422
		case 3 :
			ColorFormat = XVIDC_CSF_YCRCB_422;
			break;
#if (XPAR_XV_HDMITXSS_0_INCLUDE_YUV420_SUP == 1)
			// YUV420
		case 4 :
			ColorFormat = XVIDC_CSF_YCRCB_420;
			break;
#endif
			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayColorSpaceMenu();
			break;
	}

	if (ColorFormat != XVIDC_CSF_UNKNOWN) {
		UpdateColorFormat(&Hdmiphy1, &HdmiTxSs, ColorFormat);
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function displays the HDMI edid menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayEdidMenu(void) {
	xil_printf("\r\n");
	SinkCapabilityCheck(&EdidHdmi20_t);
	SinkCapWarningMsg(&EdidHdmi20_t);
	xil_printf("---------------------\r\n");
	xil_printf("---   EDID MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("1 - Display the edid of the connected sink device.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI edid menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	u8 Buffer[256];
	int Status = XST_FAILURE;

	// Default
	Menu = XHDMI_EDID_MENU;
	switch (Input) {

			// Show source edid
		case 1 :
			XV_HdmiTxSs_ShowEdid(&HdmiTxSs);
			// Read TX edid
			xil_printf("\r\n");

			Status = XV_HdmiTxSs_ReadEdid(&HdmiTxSs, (u8*)&Buffer);
			/* Only Parse the EDID when the Read EDID success */
			if (Status == XST_SUCCESS) {
				XV_VidC_parse_edid((u8*)&Buffer,
									&EdidHdmi20_t.EdidCtrlParam,
									XVIDC_VERBOSE_ENABLE);
			} else {
				xil_printf(ANSI_COLOR_YELLOW "EDID parsing has failed.\r\n"
							ANSI_COLOR_RESET);
			}
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayEdidMenu();
			break;
	}


	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the video menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayVideoMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   VIDEO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the video menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_VIDEO_MENU;


	// Insert carriage return
	xil_printf("\r\n");



	return Menu;
}

#endif

/*****************************************************************************/
/**
*
* This function is called to trigger the HDMI menu statemachine.
*
* @param InstancePtr is a pointer to the XHdmi_Menu instance.
*
* @param input is the value used for the next menu state decoder.
*
* @return None
*
******************************************************************************/
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr) {
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	if ((InstancePtr->WaitForColorbar) && (!TxBusy)) {
		InstancePtr->WaitForColorbar = (FALSE);
		xil_printf("Enter Selection -> ");
	}
#endif

	// Check if the uart has any data
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	else if (!XUartLite_IsReceiveEmpty(InstancePtr->UartBaseAddress)) {
#else
	if (!XUartLite_IsReceiveEmpty(InstancePtr->UartBaseAddress)) {
#endif

		// Read data from uart
		Data = XUartLite_RecvByte(InstancePtr->UartBaseAddress);
#elif defined versal
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	else if (XUartPsv_IsReceiveData(InstancePtr->UartBaseAddress)) {
#else
	if (XUartPsv_IsReceiveData(InstancePtr->UartBaseAddress)) {
#endif
		// Read data from uart
		Data = XUartPsv_RecvByte(InstancePtr->UartBaseAddress);
#else
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	else if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {
#else
	if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {
#endif
		// Read data from uart
		Data = XUartPs_RecvByte(InstancePtr->UartBaseAddress);
#endif
		// Main menu
		if (InstancePtr->CurrentMenu == XHDMI_MAIN_MENU) {
			InstancePtr->CurrentMenu =
          XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
			InstancePtr->Value = 0;
		}

		// Sub menu
		else {

			// Send response to user
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
			XUartLite_SendByte(InstancePtr->UartBaseAddress, Data);
#elif defined versal
			XUartPsv_SendByte(InstancePtr->UartBaseAddress, Data);
#else
			XUartPs_SendByte(InstancePtr->UartBaseAddress, Data);
#endif
			// Alpha numeric data
			if (isalpha(Data)) {
				xil_printf("Invalid input. \
                          Valid entry is only digits 0-9. Try again\r\n\r\n");
				xil_printf("Enter Selection -> ");
				InstancePtr->Value = 0;
			}

			// Numeric data
			else if ((Data >= '0') && (Data <= '9')) {
		         InstancePtr->Value = InstancePtr->Value * 10 + (Data-'0');
			}

			// Backspace
			else if (Data == '\b') {
//discard previous input
				InstancePtr->Value = InstancePtr->Value / 10;
			}

			// Execute
			else if ((Data == '\n') || (Data == '\r')) {
				xil_printf("\r\n");
				InstancePtr->CurrentMenu =
     XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
				InstancePtr->Value = 0;
			}
		}
	}
}
