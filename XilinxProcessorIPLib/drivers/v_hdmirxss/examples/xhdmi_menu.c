/******************************************************************************
*
* Copyright (C) 2014 - 2017 Xilinx, Inc.  All rights reserved.
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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"
#include "xhdcp.h"

/************************** Constant Definitions *****************************/
#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
/* If HDCP 1.4 or HDCP 2.2 is in the system then use the HDCP abstraction layer */
#define USE_HDCP
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);

static void XHdmi_DisplayMainMenu(void);
static void XHdmi_DisplayResolutionMenu(void);
static void XHdmi_DisplayFrameRateMenu(void);
static void XHdmi_DisplayColorDepthMenu(void);
static void XHdmi_DisplayColorSpaceMenu(void);
static void XHdmi_DisplayGtPllLayoutMenu(void);
static void XHdmi_DisplayEdidMenu(void);
static void XHdmi_DisplayAudioMenu(void);
static void XHdmi_DisplayVideoMenu(void);
#ifdef USE_HDCP
static void XHdmi_DisplayHdcpMainMenu(void);
#endif

extern void Info(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern void EnableColorBar(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
		XVidC_VideoMode VideoMode,
		XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc);;
extern void UpdateColorFormat(XVphy *VphyPtr, XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorFormat ColorFormat);
extern void UpdateColorDepth(XVphy *VphyPtr, XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorDepth ColorDepth);
extern void UpdateFrameRate(XVphy *VphyPtr, XV_HdmiTxSs *pHdmiTxSs, XVidC_FrameRate FrameRate);
extern void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr);
extern void CloneTxEdid(void);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern void XV_HdmiRxSs_LoadEdid(XV_HdmiRxSs *InstancePtr, u8 *EdidData, u16 Length);
extern void XV_HdmiRxSs_ToggleRxHpd(XV_HdmiRxSs *InstancePtr);
extern void HDCPXILCMD_ProcessKey(char theCmdKey);
#endif
extern void XV_ConfigTpg(XV_tpg *InstancePtr);

/************************* Variable Definitions *****************************/
extern u8 Edid[];

/**
* This table contains the function pointers for all possible states.
* The order of elements must match the XHdmi_MenuType enumerator definitions.
*/
static XHdmi_MenuFuncType* const XHdmi_MenuTable[XHDMI_NUM_MENUS] =
{
	XHdmi_MainMenu,
	XHdmi_ResolutionMenu,
	XHdmi_FrameRateMenu,
	XHdmi_ColorDepthMenu,
	XHdmi_ColorSpaceMenu,
	XHdmi_GtPllLayoutMenu,
	XHdmi_EdidMenu,
	XHdmi_AudioMenu,
	XHdmi_VideoMenu,
	XHdmi_HdcpMainMenu
};

extern XVphy Vphy;               /* VPhy structure */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern XV_HdmiTxSs HdmiTxSs;       /* HDMI TX SS structure */
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern XV_HdmiRxSs HdmiRxSs;       /* HDMI RX SS structure */
#endif
extern u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;                // TX busy flag. This flag is set while the TX is initialized
extern XV_tpg Tpg;				/* TPG structure */
extern XTpg_PatternId Pattern;
extern XHdcp_Repeater HdcpRepeater;

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
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);
	//Xil_AssertVoid(MenuConfig != NULL);

	/* copy configuration settings */
	//memcpy(&(InstancePtr->Config), MenuConfig, sizeof(XHdmi_MenuConfig));

	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
	InstancePtr->UartBaseAddress = UartBaseAddress;
    InstancePtr->Value = 0;
    InstancePtr->WaitForColorbar = (FALSE);

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
	print("\r\n");
	print("---------------------\r\n");
	print("---   MAIN MENU   ---\r\n");
	print("---------------------\r\n");
	print("i - Info\n\r");
	print("       => Shows information about the HDMI RX stream, HDMI TX stream, \n\r");
	print("          GT transceivers and PLL settings.\n\r");
	print("c - Colorbar\n\r");
	print("       => Displays the colorbar on the source output.\n\r");
	print("r - Resolution\n\r");
	print("       => Change the video resolution of the colorbar.\n\r");
	print("f - Frame rate\n\r");
	print("       => Change the frame rate of the colorbar.\n\r");
	print("d - Color depth\n\r");
	print("       => Change the color depth of the colorbar.\n\r");
	print("s - Color space\n\r");
	print("       => Change the color space of the colorbar.\n\r");
	print("p - Pass-through\n\r");
	print("       => Passes the sink input to source output.\n\r");
	print("l - GT PLL layout\n\r");
	print("       => Select GT tranceiver PLL layout.\n\r");
	print("z - GT & HDMI TX/RX log\n\r");
	print("       => Shows log information for GT & HDMI TX/RX.\n\r");
	print("e - Edid\n\r");
	print("       => Display and set edid.\n\r");
	print("a - Audio\n\r");
	print("       => Audio options.\n\r");
	print("v - Video\n\r");
	print("       => Video pattern options.\n\r");
	print("m - Set HDMI Mode\n\r");
	print("n - Set DVI Mode\n\r");

#if defined(USE_HDCP)
	/* Show HDCP menu option when HDCP is ready */
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) && XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {

		print("h - HDCP\n\r");
		print("       => Goto HDCP menu.\n\r");
	}
#endif

	print("\n\r\n\r");
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
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
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

		// Pass-through
		case ('p') :
		case ('P') :
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			// Check if a sink is connected
		    if (HdmiRxSs.IsStreamConnected == (TRUE)) {
			    print("\n\rForce pass-through\n\r");
			    XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
			    XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
				XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
		        XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
		        XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (TRUE));
				XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
		    }

		    // No sink
		    else {
			print("No sink device detected.\n\r");
			print("Connect a sink device to activate pass-through.\n\r");
		    }
#else
			print("\r\nPass-Through not possible without HDMI RX SS.\n\r");
#endif
		    Menu = XHDMI_MAIN_MENU;
		break;

		// Colorbar
		case ('c') :
		case ('C') :

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			// The colorbar can only be displayed when the GT is not bonded.
			if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
				TxBusy = (FALSE);
				EnableColorBar(&Vphy,
					 &HdmiTxSs,
					 HdmiTxSsVidStreamPtr->VmId,
					 HdmiTxSsVidStreamPtr->ColorFormatId,
					 HdmiTxSsVidStreamPtr->ColorDepth);
				Menu = XHDMI_MAIN_MENU;
			}

			else {
				print("\n\rThe GT TX and RX are bonded and clocked by the RX clock.\n\r");
				print("Please select independent PLL layout to enable TX only mode.\n\r");
				Menu = XHDMI_MAIN_MENU;
			}
#else
				print("\n\rCan't Change Format. No HDMI TX SS present in the design.\n\r");
				Menu = XHDMI_MAIN_MENU;
#endif
		break;

		// Resolution
		case ('r') :
		case ('R') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {

				// The colorbar resolution can only selected when the GT is not bonded.
				if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
					Menu = XHDMI_RESOLUTION_MENU;
					XHdmi_DisplayResolutionMenu();
				}

				else {
					print("\n\rThe GT TX and RX are bonded and clocked by the RX clock.\n\r");
					print("Please select independent PLL layout to enable TX only mode.\n\r");
				}
			}

			// Pass-through
			else {
					print("\n\rThe example design is in pass-through mode.\n\r");
					print("In this mode the video parameters can't be changed.\n\r");
			}
		break;

		// Frame rate
		case ('f') :
		case ('F') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				// The frame rate can only be changed when the GT is not bonded.
				if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
					XHdmi_DisplayFrameRateMenu();
					Menu = XHDMI_FRAMERATE_MENU;
				}

				else {
					print("\n\rThe GT TX and RX are bonded and clocked by the RX clock.\n\r");
					print("The frame rate can only be changed when the colorbar is selected.\n\r");
					print("Please select independent PLL layout to enable TX only mode.\n\r");
				}
			}

			// Pass-through
			else {
					print("\n\rThe example design is in pass-through mode.\n\r");
					print("In this mode the video parameters can't be changed.\n\r");
			}
		break;

		// Color depth
		case ('d') :
		case ('D') :

			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {
				// The color depth can only be changed when the GT is not bonded.
				if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
					XHdmi_DisplayColorDepthMenu();
					Menu = XHDMI_COLORDEPTH_MENU;
				}

				else {
					print("\n\rThe GT TX and RX are bonded and clocked by the RX clock.\n\r");
					print("The color depth can only be changed when the colorbar is selected.\n\r");
					print("Please select independent PLL layout to enable TX only mode.\n\r");
				}
			}

			// Pass-through
			else {
					print("\n\rThe example design is in pass-through mode.\n\r");
					print("In this mode the video parameters can't be changed.\n\r");
			}
		break;

		// Color space
		case ('s') :
		case ('S') :
			// Default
			Menu = XHDMI_MAIN_MENU;

			// Check if the TX only is active
			if (!IsPassThrough) {

				// The color depth can only be changed when the GT is not bonded.
				if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
					XHdmi_DisplayColorSpaceMenu();
					Menu = XHDMI_COLORSPACE_MENU;
				}

				else {
					print("\n\rThe GT TX and RX are bonded and clocked by the RX clock.\n\r");
					print("The color space can only be changed when the colorbar is selected.\n\r");
					print("Please select independent PLL layout to enable TX only mode.\n\r");
				}
			}

			// Pass-through
			else {
					print("\n\rThe example design is in pass-through mode.\n\r");
					print("In this mode the video parameters can't be changed.\n\r");
			}
		break;

		// GT PLL layout
		case ('l') :
		case ('L') :
			XHdmi_DisplayGtPllLayoutMenu();
			Menu = XHDMI_GTPLLLAYOUT_MENU;
		break;

		// GT & HDMI TX/RX log
		case ('z') :
		case ('Z') :
			XVphy_LogDisplay(&Vphy);
			XV_HdmiTxSs_LogDisplay(&HdmiTxSs);
			XV_HdmiRxSs_LogDisplay(&HdmiRxSs);
			Menu = XHDMI_MAIN_MENU;
		break;

		// HDMI Mode
		case ('m') :
		case ('M') :
		    print("\n\rEnable HDMI Mode.\n\r");
		    XV_HdmiTxSS_SetHdmiMode(&HdmiTxSs);
		    XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			Menu = XHDMI_MAIN_MENU;
		break;

		// DVI Mode
		case ('n') :
		case ('N') :
	    print("\n\rEnable DVI Mode.\n\r");
	        XV_HdmiTxSS_SetDviMode(&HdmiTxSs);
	        XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			Menu = XHDMI_MAIN_MENU;
		break;

		// Edid
		case ('e') :
		case ('E') :
			XHdmi_DisplayEdidMenu();
			Menu = XHDMI_EDID_MENU;
		break;

		// Audio
		case ('a') :
		case ('A') :
			XHdmi_DisplayAudioMenu();
			Menu = XHDMI_AUDIO_MENU;
		break;

		// Video
		case ('v') :
		case ('V') :
			XHdmi_DisplayVideoMenu();
			Menu = XHDMI_VIDEO_MENU;
		break;

#if defined(USE_HDCP)
		// HDCP
		case ('h') :
		case ('H') :
			/* Enable HDCP menu option when HDCP is ready */
			if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) && XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
				XHdmi_DisplayHdcpMainMenu();
				Menu = XHDMI_HDCP_MAIN_MENU;
			}
		break;
#endif

		default :
			XHdmi_DisplayMainMenu();
			Menu = XHDMI_MAIN_MENU;
		break;
	}

	return Menu;
}


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
void XHdmi_DisplayResolutionMenu(void)
{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	print("\r\n");
	print("---------------------------\r\n");
	print("---   RESOLUTION MENU   ---\r\n");
	print("---------------------------\r\n");
	print(" 1 -  720 x 480p\r\n");
	print(" 2 -  720 x 576p\r\n");
	print(" 3 - 1280 x 720p\r\n");
	print(" 4 - 1680 x 720p\r\n");
	print(" 5 - 1920 x 1080p\r\n");
	print(" 6 - 2560 x 1080p\r\n");
	print(" 7 - 3840 x 2160p\r\n");
	print(" 8 - 4096 x 2160p\r\n");
	print(" 9 - 1920 x 1080i\r\n");
	print("10 -  640 x 480p (VGA / DMT0659)\r\n");
	print("11 -  800 x 600p (SVGA / DMT0860)\r\n");
	print("12 - 1024 x 768p (XGA / DMT1060)\r\n");
	print("13 - 1280 x 768p (WXGA / CVT1260E)\r\n");
	print("14 - 1366 x 768p (WXGA+ / DMT1360)\r\n");
	print("15 - 1280 x 1024p (SXGA / DMT1260G)\r\n");
	print("16 - 1680 x 1050p (WSXGA+ / CVT1660D)\r\n");
	print("17 - 1600 x 1200p (UXGA / DMT1660)\r\n");
	print("18 - 1920 x 1200p (WUXGA / CVT1960D)\r\n");
    print("19 -  720 x 480i (NTSC)\r\n");
    print("20 -  720 x 576i (PAL)\r\n");
	print("99 - Exit\n\r");
	print("Enter Selection -> ");
#else
    xil_printf("No HDMI_TX_SS in design\r\n");
#endif
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
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
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
#if (XPAR_XV_HDMITXSS_0_INPUT_PIXELS_PER_CLOCK != 1)
            VideoMode = XVIDC_VM_3840x2160_60_P;
#else
            VideoMode = XVIDC_VM_3840x2160_30_P;
#endif
            break;

        // 4096 x 2160p
        case 8 :
#if (XPAR_XV_HDMITXSS_0_INPUT_PIXELS_PER_CLOCK != 1)
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

        // 720 x 480i (NTSC)
        case 19 :
            VideoMode = XVIDC_VM_1440x480_60_I;
            break;

        // 720 x 576i (PAL)
        case 20 :
            VideoMode = XVIDC_VM_1440x576_50_I;
            break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayResolutionMenu();
			break;
	}

	if (VideoMode != XVIDC_VM_NO_INPUT) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		EnableColorBar(&Vphy, &HdmiTxSs, VideoMode, XVIDC_CSF_RGB, XVIDC_BPC_8);
#endif
		InstancePtr->WaitForColorbar = (TRUE);
	}
#else
        xil_printf("Returning to main menu.\r\n");
        Menu = XHDMI_MAIN_MENU;
#endif
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
void XHdmi_DisplayFrameRateMenu(void)
{
	print("\r\n");
	print("---------------------------\r\n");
	print("---   FRAME RATE MENU   ---\r\n");
	print("---------------------------\r\n");
	print("  1 -  24 Hz\r\n");
	print("  2 -  25 Hz\r\n");
	print("  3 -  30 Hz\r\n");
	print("  4 -  50 Hz\r\n");
	print("  5 -  60 Hz\r\n");
	print("  6 - 100 Hz\r\n");
	print("  7 - 120 Hz\r\n");
	print(" 99 - Exit\n\r");
	print("Enter Selection -> ");
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
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
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
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayFrameRateMenu();
			break;
	}

	if (FrameRate != XVIDC_FR_UNKNOWN) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		UpdateFrameRate(&Vphy, &HdmiTxSs, FrameRate);
#endif
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
void XHdmi_DisplayColorDepthMenu(void)
{
	print("\r\n");
	print("----------------------------\r\n");
	print("---   COLOR DEPTH MENU   ---\r\n");
	print("----------------------------\r\n");
	print("  1 - 24 bpp\r\n");
	print("  2 - 30 bpp\r\n");
	print("  3 - 36 bpp\r\n");
	print("  4 - 48 bpp\r\n");
	print(" 99 - Exit\n\r");
	print("Enter Selection -> ");
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
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_ColorDepth ColorDepth;

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
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayColorDepthMenu();
			break;
	}

	if (ColorDepth != XVIDC_BPC_UNKNOWN) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		UpdateColorDepth(&Vphy, &HdmiTxSs, ColorDepth);
#endif
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
void XHdmi_DisplayColorSpaceMenu(void)
{
	print("\r\n");
	print("----------------------------\r\n");
	print("---   COLOR SPACE MENU   ---\r\n");
	print("----------------------------\r\n");
	print("  1 - RGB\r\n");
	print("  2 - YUV444\r\n");
	print("  3 - YUV422\r\n");
	print("  4 - YUV420\r\n");
	print(" 99 - Exit\n\r");
	print("Enter Selection -> ");
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
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
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

		// YUV420
		case 4 :
			ColorFormat = XVIDC_CSF_YCRCB_420;
			break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayColorSpaceMenu();
			break;
	}

	if (ColorFormat != XVIDC_CSF_UNKNOWN) {
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	UpdateColorFormat(&Vphy, &HdmiTxSs, ColorFormat);
#endif
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the HDMI GT PLL layout menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayGtPllLayoutMenu(void)
{
	print("\r\n");
	print("------------------------------\r\n");
	print("---   GT PLL LAYOUT MENU   ---\r\n");
	print("------------------------------\r\n");
	print("\n\r");

	if (Vphy.Config.XcvrType != XVPHY_GT_TYPE_GTXE2) {
		print("GT PLL Layout is fixed for this reference design\n\r");
		print("\n\r");
	}
	else {
		print("In this menu the GT PLL clocking layout can be selected.\n\r");

		print("RX => QPLL / TX => CPLL\n\r");
		print("\n\r");
		print("This is mode the GT RX is clocked by the QPLL and the CPLL\n\r");
		print("is driving the GT TX. \n\r");
		print("\n\r");
		print("               --------     -----------     --------               \n\r");
		print("               |      |     | GT | GT |     |      |               \n\r");
		print(" RX clock -----| QPLL |-----| RX | TX |-----| CPLL |----- TX clock \n\r");
		print("               |      |     |    |    |     |      |               \n\r");
		print("               --------     -----------     --------               \n\r");
		print("\n\r\n\r");

		print("RX => CPLL / TX => QPLL\n\r");
		print("\n\r");
		print("This is mode the GT RX is clocked by the CPLL and the QPLL\n\r");
		print("is driving the GT TX. \n\r");
		print("\n\r");
		print("               --------     -----------     --------               \n\r");
		print("               |      |     | GT | GT |     |      |               \n\r");
		print(" RX clock -----| CPLL |-----| RX | TX |-----| QPLL |----- TX clock \n\r");
		print("               |      |     |    |    |     |      |               \n\r");
		print("               --------     -----------     --------               \n\r");
		print("\n\r\n\r");

		print("RX => CPLL / TX => CPLL\n\r");
		print("\n\r");
		print("In this mode the GT RX and GT TX are bonded and clocked by the same PLL.\n\r");
		print("When this mode is selected only pass-through video can be supported.\n\r");
		print("Also NI-DRU operation and TX oversampling are not supported in this mode.\n\r");
		print("\n\r");
		print("               --------     -----------                            \n\r");
		print("               | CPLL |     | GT | GT |                            \n\r");
		print(" RX clock -----|  /   |-----| RX | TX |-------- TX clock           \n\r");
		print("               | QPLL |  |  |    |    |  |                         \n\r");
		print("               --------  |  -----------  |                         \n\r");
		print("                         |               |                         \n\r");
		print("                         -----------------                         \n\r");
		print("\n\r\n\r");
		print("  1 - RX => QPLL / TX => CPLL\r\n");
		print("  2 - RX => CPLL / TX => QPLL\r\n");
		print("  3 - RX => CPLL / TX => CPLL\r\n");
		print("  4 - RX => QPLL / TX => QPLL\r\n");
		print(" 99 - Exit\n\r");
		print("Enter Selection -> ");
	}
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
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
	u8 IsPassThroughCopy;
	XVphy_SysClkDataSelType TxSysPllSelect;
	XVphy_SysClkDataSelType RxSysPllSelect;
	u8 IsValid = FALSE;

    if (Vphy.Config.XcvrType != XVPHY_GT_TYPE_GTXE2) {
		xil_printf("Returning to main menu.\n\r");
		Menu = XHDMI_MAIN_MENU;
	}
	else{ //GTXe2
		// Default
		Menu = XHDMI_GTPLLLAYOUT_MENU;

		switch (Input) {

			// RX => QPLL / TX => CPLL
			case 1 :
				xil_printf("Setting RX => QPLL / TX => CPLL\n\r\n\r");
				TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
				RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
				IsValid = TRUE;
				break;

			// RX => CPLL / TX => QPLL
			case 2 :
				xil_printf("Setting RX => CPLL / TX => QPLL\n\r\n\r");
				TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
				RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
				IsValid = TRUE;
				break;

			// RX => CPLL / TX => CPLL
			case 3 :
				xil_printf("Setting RX => CPLL / TX => CPLL\n\r\n\r");
				xil_printf("Please connect a HDMI source to start video in pass-through mode.\n\r");
				TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
				RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
				IsValid = TRUE;
				break;

			// RX => QPLL / TX => QPLL
			case 4 :
				xil_printf("Setting RX => QPLL / TX => QPLL\n\r\n\r");
				xil_printf("Please connect a HDMI source to start video in pass-through mode.\n\r");
				TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
				RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
				IsValid = TRUE;
				break;

			// Exit
			case 99 :
				xil_printf("Returning to main menu.\n\r");
				Menu = XHDMI_MAIN_MENU;
				IsValid = FALSE;
				break;

			default :
				xil_printf("Unknown option\n\r");
				XHdmi_DisplayGtPllLayoutMenu();
				IsValid = FALSE;
				break;
		}

		if (IsValid) {
			// The IsPassThrough variable will be cleared when the PLL layout is set.
			// Therefore we copy the variable first, so we know what to do after the PLL layout has changed.
			IsPassThroughCopy = IsPassThrough;

			/* Update VPHY Clocking */
			XVphy_HdmiUpdateClockSelection(&Vphy, 0, TxSysPllSelect, RxSysPllSelect);

			// Is the reference design in pass-through
			// Then re-start pass-through
			if ((IsPassThroughCopy) || (Input == 3) || (Input == 4)) {
				print("Restart pass-through\n\r");

				// Reset RX frequency detector
				XVphy_ClkDetFreqReset(&Vphy, 0,	XVPHY_DIR_RX);
				//ToggleRxHpd();
			}

			// Re-start colorbar
			else
			{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			  EnableColorBar(&Vphy,
							 &HdmiTxSs,
							 XVIDC_VM_1920x1080_60_P,
							 XVIDC_CSF_RGB,
							 XVIDC_BPC_8);
				InstancePtr->WaitForColorbar = (TRUE);
#endif
			}

			// Return to main menu
			print("Return to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
		}
	}


	return Menu;
}

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
void XHdmi_DisplayEdidMenu(void)
{
	print("\r\n");
	print("---------------------\r\n");
	print("---   EDID MENU   ---\r\n");
	print("---------------------\r\n");
	print("  1 - Display the edid of the connected sink device.\r\n");
	print("  2 - Clone the edid of the connected sink edid to HDMI Rx.\r\n");
	print("  3 - Load default edid to HDMI Rx.\r\n");
	print(" 99 - Exit\n\r");
	print("Enter Selection -> ");
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
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_EDID_MENU;

	switch (Input) {

		// Show source edid
		case 1 :
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_ShowEdid(&HdmiTxSs);
			// Display the prompt for the next input
			print("Enter Selection -> ");
#else
			print("\r\nNo HDMI TX SS present in design.\r\n");
#endif
			break;

		// Clone edid
		case 2 :
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			CloneTxEdid();
			// Display the prompt for the next input
			print("Enter Selection -> ");
#else
			print("\r\nNo HDMI TX SS present in design.\r\n");
#endif
			break;

		// Load edid
		case 3 :
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_LoadDefaultEdid(&HdmiRxSs);
			// Display the prompt for the next input
			print("Enter Selection -> ");
#else
			print("\r\nNo HDMI RX SS present in design.\r\n");
#endif
			break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
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
void XHdmi_DisplayVideoMenu(void)
{
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   VIDEO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
#if (XPAR_XV_TPG_0_COLOR_BAR == 1)
	xil_printf("  1 - Color bars\r\n");
#endif
#if (XPAR_XV_TPG_0_SOLID_COLOR == 1)
	xil_printf("  2 - Solid red\r\n");
	xil_printf("  3 - Solid green\r\n");
	xil_printf("  4 - Solid blue\r\n");
	xil_printf("  5 - Solid black\r\n");
	xil_printf("  6 - Solid white\r\n");
#endif
#if (XPAR_XV_TPG_0_RAMP == 1)
	xil_printf("  7 - Horizontal ramp\r\n");
	xil_printf("  8 - Vertical ramp\r\n");
#endif
#if (XPAR_XV_TPG_0_COLOR_SWEEP == 1)
	xil_printf("  9 - Rainbow color\r\n");
#endif
	xil_printf(" 10 - Checker board\r\n");
	xil_printf(" 11 - Cross hatch\r\n");
	xil_printf(" 12 - Noise\r\n");
	xil_printf(" 99 - Exit\n\r");
	xil_printf("Enter Selection -> ");
#else
    xil_printf("No HDMI TX SS in design\r\n");
#endif
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
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
	XTpg_PatternId NewPattern;

	// Default
	Menu = XHDMI_VIDEO_MENU;
	NewPattern = XTPG_BKGND_LAST;

	// Insert carriage return
	print("\n\r");

	switch (Input) {
		case 1 :
		  NewPattern = XTPG_BKGND_COLOR_BARS;
		  print("Colorbars\n\r");
		  break;

		case 2 :
		  NewPattern = XTPG_BKGND_SOLID_RED;
		  print("Solid red\n\r");
		  break;

		case 3 :
		  NewPattern = XTPG_BKGND_SOLID_GREEN;
		  print("Solid green\n\r");
		  break;

		case 4 :
		  NewPattern = XTPG_BKGND_SOLID_BLUE;
		  print("Solid blue\n\r");
		  break;

		case 5 :
		  NewPattern = XTPG_BKGND_SOLID_BLACK;
		  print("Solid black\n\r");
		  break;

		case 6 :
		  NewPattern = XTPG_BKGND_SOLID_WHITE;
		  print("Solid white\n\r");
		  break;

		case 7 :
		  NewPattern = XTPG_BKGND_H_RAMP;
		  print("Horizontal ramp\n\r");
		  break;

		case 8 :
		  NewPattern = XTPG_BKGND_V_RAMP;
		  print("Vertical ramp\n\r");
		  break;

		case 9 :
		  NewPattern = XTPG_BKGND_RAINBOW_COLOR;
		  print("Rainbow colors\n\r");
		  break;

		case 10 :
		  NewPattern = XTPG_BKGND_CHECKER_BOARD;
		  print("Checker board\n\r");
		  break;

		case 11 :
		  NewPattern = XTPG_BKGND_CROSS_HATCH;
		  print("Cross hatch\n\r");
		  break;

		case 12 :
		  NewPattern = XTPG_BKGND_PBRS;
		  print("Noise\n\r");
		  break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayVideoMenu();
			break;
	}


	if (NewPattern != XTPG_BKGND_LAST) {
		 /* Set video pattern */
		 Pattern = NewPattern;
		  /* Start TPG */
		 xil_printf("new pattern\n\r");
		  XV_ConfigTpg(&Tpg);
		  print("Enter Selection -> ");
	 }

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the audio menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayAudioMenu(void)
{
	print("\r\n");
	print("----------------------\r\n");
	print("---   AUDIO MENU   ---\r\n");
	print("----------------------\r\n");
	print("  1 - Mute audio.\r\n");
	print("  2 - Unmute audio.\r\n");
	print(" 99 - Exit\n\r");
	print("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio enu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_AUDIO_MENU;

	switch (Input) {

		// Mute
		case 1 :
			print("Mute audio.\n\r");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			// Display the prompt for the next input
			print("Enter Selection -> ");
#else
			print("\r\nNo HDMI TX SS present in design.\r\n");
#endif
			break;

		// Unmute
		case 2 :
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			print("Unmute audio.\n\r");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			// Display the prompt for the next input
			print("Enter Selection -> ");
#else
			print("\r\nNo HDMI TX SS present in design.\r\n");
#endif
			break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown option\n\r");
			XHdmi_DisplayAudioMenu();
			break;
	}

	return Menu;
}

#if defined(USE_HDCP)
/*****************************************************************************/
/**
*
* This function displays the HDCP main menu.
*
* @param None
*
* @return None
*
******************************************************************************/
void XHdmi_DisplayHdcpMainMenu(void)
{
	print("\r\n");
	print("--------------------------\n\r");
	print("---   HDCP Main Menu   ---\n\r");
	print("--------------------------\n\r");
	print(" 1 - Enable repeater\n\r");
	print(" 2 - Disable repeater\n\r");
	print(" 3 - Enable detailed logging\n\r");
	print(" 4 - Disable detailed logging\n\r");
	print(" 5 - Display log\n\r");
	print(" 6 - Display repeater info\n\r");
	print("99 - Exit\n\r");
	print("Enter Selection -> ");
}
#endif

/*****************************************************************************/
/**
*
* This function implements the HDCP main menu state.
*
* @param Input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_HDCP_MAIN_MENU;

	// Insert carriage return
	print("\n\r");

#if defined(USE_HDCP)
	switch (Input) {

		/* 1 - Enable repeater*/
		case 1:
			print("Enable repeater.\n\r");
			XHdcp_SetRepeater(&HdcpRepeater, TRUE);
			break;

		/* 2 - Disable repeater*/
		case 2:
			print("Disable repeater.\n\r");
			XHdcp_SetRepeater(&HdcpRepeater, FALSE);
			break;

		/* 3 - Enable detailed logging */
		case 3 :
			print("Enable detailed logging.\n\r");
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, TRUE);
			XV_HdmiTxSs_HdcpSetInfoDetail(&HdmiTxSs, TRUE);
			break;

		/* 4 - Disable detailed logging */
		case 4 :
			print("Disable detailed logging.\n\r");
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, FALSE);
			XV_HdmiTxSs_HdcpSetInfoDetail(&HdmiTxSs, FALSE);
			break;

		/* 5 - Display log */
		case 5 :
			print("Display log.\n\r");
			XV_HdmiRxSs_HdcpInfo(&HdmiRxSs);
			XV_HdmiTxSs_HdcpInfo(&HdmiTxSs);
			break;

		/* 6 - Display repeater info */
		case 6 :
			print("Display repeater info.\n\r");
			XHdcp_DisplayInfo(&HdcpRepeater, TRUE);
			break;

		// Exit
		case 99 :
			print("Returning to main menu.\n\r");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			print("Unknown command\n\r");
			XHdmi_DisplayHdcpMainMenu();
			break;
	}
#endif
	return Menu;
}

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
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if ((InstancePtr->WaitForColorbar) && (!TxBusy)) {
		InstancePtr->WaitForColorbar = (FALSE);
	print("Enter Selection -> ");
    }

	// Check if the uart has any data
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
	else if (!XUartLite_IsReceiveEmpty(InstancePtr->UartBaseAddress)) {

		// Read data from uart
	Data = XUartLite_RecvByte(InstancePtr->UartBaseAddress);
#else
        else if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {

		// Read data from uart
		Data = XUartPs_RecvByte(InstancePtr->UartBaseAddress);
#endif
	// Main menu
		if (InstancePtr->CurrentMenu == XHDMI_MAIN_MENU) {
			InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
		    InstancePtr->Value = 0;
		}

		// Sub menu
		else {

		// Send response to user
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
		XUartLite_SendByte(InstancePtr->UartBaseAddress, Data);
#else
		XUartPs_SendByte(InstancePtr->UartBaseAddress, Data);
#endif
			// Alpha numeric data
		    if (isalpha(Data)) {
		      print("\r\nInvalid input. Valid entry is only digits 0-9. Try again\r\n\r\n");
		      print("Enter Selection -> ");
		      InstancePtr->Value = 0;
		    }

		    // Numeric data
		    else if ((Data >= '0') && (Data <= '9')) {
		      InstancePtr->Value = InstancePtr->Value * 10 + (Data-'0');
		    }

		    // Backspace
		    else if (Data == '\b') {
		      InstancePtr->Value = InstancePtr->Value / 10; //discard previous input
		    }

		    // Execute
			else if ((Data == '\n') || (Data == '\r')) {
				InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
			    InstancePtr->Value = 0;
			}
		}
	}
}
