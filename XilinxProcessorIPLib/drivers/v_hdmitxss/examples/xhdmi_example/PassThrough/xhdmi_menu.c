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
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"
#include "xhdcp.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif

/************************** Constant Definitions *****************************/
#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
/* If HDCP 1.4 or HDCP 2.2 is in the system then use the HDCP abstraction layer */
#define USE_HDCP
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/* Create entry for each mode in the custom table */
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
	/* Custom Modes . */
	{ XVIDC_VM_3840x2160_30_P_SB, "3840x2160@30Hz (SB)", XVIDC_FR_30HZ,
		{3840, 48, 32, 80, 4000, 1,
		2160, 3, 5, 23, 2191, 0, 0, 0, 0, 1} }
};
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#ifdef USE_HDCP
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_HdcpDebugMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif

static void XHdmi_DisplayMainMenu(void);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
static void XHdmi_DisplayGtPllLayoutMenu(void);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static void XHdmi_DisplayEdidMenu(void);
static void XHdmi_DisplayResolutionMenu(void);
static void XHdmi_DisplayFrameRateMenu(void);
static void XHdmi_DisplayColorDepthMenu(void);
static void XHdmi_DisplayColorSpaceMenu(void);
static void XHdmi_DisplayAudioMenu(void);
static void XHdmi_DisplayVideoMenu(void);
#endif
#ifdef USE_HDCP
static void XHdmi_DisplayHdcpMainMenu(void);
static void XHdmi_DisplayHdcpDebugMenu(void);
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
extern void XV_ConfigTpg(XV_tpg *InstancePtr);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern void XV_HdmiRxSs_LoadEdid(XV_HdmiRxSs *InstancePtr, u8 *EdidData, u16 Length);
extern void XV_HdmiRxSs_ToggleRxHpd(XV_HdmiRxSs *InstancePtr);
extern void HDCPXILCMD_ProcessKey(char theCmdKey);
#endif

/************************* Variable Definitions *****************************/
extern u8 Edid[];


/**
* This table contains the function pointers for all possible states.
* The order of elements must match the XHdmi_MenuType enumerator definitions.
*/
static XHdmi_MenuFuncType* const XHdmi_MenuTable[XHDMI_NUM_MENUS] =
{
	XHdmi_MainMenu,
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	XHdmi_GtPllLayoutMenu,
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	XHdmi_EdidMenu,
	XHdmi_ResolutionMenu,
	XHdmi_FrameRateMenu,
	XHdmi_ColorDepthMenu,
	XHdmi_ColorSpaceMenu,
	XHdmi_AudioMenu,
	XHdmi_VideoMenu,
#endif
#ifdef USE_HDCP
	XHdmi_HdcpMainMenu,
	XHdmi_HdcpDebugMenu
#endif
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
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern XV_tpg Tpg;				/* TPG structure */
extern XTpg_PatternId Pattern;
#endif
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
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	u32 Status;
#endif
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->CurrentMenu = XHDMI_MAIN_MENU;
	InstancePtr->UartBaseAddress = UartBaseAddress;
    InstancePtr->Value = 0;
    InstancePtr->WaitForColorbar = (FALSE);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
    //Initialize and Add Custom Resolution in to the Video Table
    //Added for the resolution menu
    /* Example : User registers custom timing table */
    Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
		(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
    if (Status != XST_SUCCESS) {
	xil_printf("ERR: Unable to register custom timing table\r\n\r\n");
    }
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
	xil_printf("       => Shows information about the HDMI RX stream, HDMI TX stream, \r\n");
	xil_printf("          GT transceivers and PLL settings.\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
	xil_printf("c - Colorbar\r\n");
	xil_printf("       => Displays the colorbar on the source output.\r\n");
	xil_printf("r - Resolution\r\n");
	xil_printf("       => Change the video resolution of the colorbar.\r\n");
	xil_printf("f - Frame rate\r\n");
	xil_printf("       => Change the frame rate of the colorbar.\r\n");
	xil_printf("d - Color depth\r\n");
	xil_printf("       => Change the color depth of the colorbar.\r\n");
	xil_printf("s - Color space\r\n");
	xil_printf("       => Change the color space of the colorbar.\r\n");
#endif
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("p - Pass-through\r\n");
	xil_printf("       => Passes the sink input to source output.\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("p - Toggle HPD\r\n");
	xil_printf("       => Toggles the HPD of HDMI RX.\r\n");
#endif
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	xil_printf("l - GT PLL layout\r\n");
	xil_printf("       => Select GT transceiver PLL layout.\r\n");
#endif
	xil_printf("z - GT & HDMI TX/RX log\r\n");
	xil_printf("       => Shows log information for GT & HDMI TX/RX.\r\n");
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

#if defined(USE_HDCP)
	/* Show HDCP menu option when HDCP is ready */
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) && XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	if (XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
#endif
		xil_printf("h - HDCP\r\n");
		xil_printf("       => Goto HDCP menu.\r\n");
	}
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

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		// Pass-through
		case ('p') :
		case ('P') :
			// Check if a sink is connected
		    if (HdmiRxSs.IsStreamConnected == (TRUE)) {
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			    xil_printf("\r\nForce pass-through\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			    xil_printf("\r\nToggle HDMI RX HPD\r\n");
#endif
                XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
                XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
                XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
                XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
                XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
		    }

		    // No sink
		    else {
			xil_printf("No sink device detected.\r\n");
			xil_printf("Connect a sink device to activate pass-through.\r\n");
		    }
		    Menu = XHDMI_MAIN_MENU;
		break;
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		// Colorbar
		case ('c') :
		case ('C') :

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
				xil_printf("\r\nThe GT TX and RX are bonded and clocked by the RX clock.\r\n");
				xil_printf("Please select independent PLL layout to enable TX only mode.\r\n");
				Menu = XHDMI_MAIN_MENU;
			}
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
					xil_printf("\r\nThe GT TX and RX are bonded and clocked by the RX clock.\r\n");
					xil_printf("Please select independent PLL layout to enable TX only mode.\r\n");
				}
			}

			// Pass-through
			else {
					xil_printf("\r\nThe example design is in pass-through mode.\r\n");
					xil_printf("In this mode the video parameters can't be changed.\r\n");
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
					xil_printf("\r\nThe GT TX and RX are bonded and clocked by the RX clock.\r\n");
					xil_printf("The frame rate can only be changed when the colorbar is selected.\r\n");
					xil_printf("Please select independent PLL layout to enable TX only mode.\r\n");
				}
			}

			// Pass-through
			else {
					xil_printf("\r\nThe example design is in pass-through mode.\r\n");
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
				// The color depth can only be changed when the GT is not bonded.
				if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)){
					XHdmi_DisplayColorDepthMenu();
					Menu = XHDMI_COLORDEPTH_MENU;
				}

				else {
					xil_printf("\r\nThe GT TX and RX are bonded and clocked by the RX clock.\r\n");
					xil_printf("The color depth can only be changed when the colorbar is selected.\r\n");
					xil_printf("Please select independent PLL layout to enable TX only mode.\r\n");
				}
			}

			// Pass-through
			else {
					xil_printf("\r\nThe example design is in pass-through mode.\r\n");
					xil_printf("In this mode the video parameters can't be changed.\r\n");
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
					xil_printf("\r\nThe GT TX and RX are bonded and clocked by the RX clock.\r\n");
					xil_printf("The color space can only be changed when the colorbar is selected.\r\n");
					xil_printf("Please select independent PLL layout to enable TX only mode.\r\n");
				}
			}

			// Pass-through
			else {
					xil_printf("\r\nThe example design is in pass-through mode.\r\n");
					xil_printf("In this mode the video parameters can't be changed.\r\n");
			}
		break;
#endif

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
		// GT PLL layout
		case ('l') :
		case ('L') :
			XHdmi_DisplayGtPllLayoutMenu();
			Menu = XHDMI_GTPLLLAYOUT_MENU;
		break;
#endif

		// GT & HDMI TX/RX log
		case ('z') :
		case ('Z') :
			XVphy_LogDisplay(&Vphy);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_LogDisplay(&HdmiTxSs);
#endif
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_LogDisplay(&HdmiRxSs);
#endif
			Menu = XHDMI_MAIN_MENU;
		break;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		// HDMI Mode
		case ('m') :
		case ('M') :
		    xil_printf("\r\nSet TX Mode To HDMI.\r\n");
		    XV_HdmiTxSS_SetHdmiMode(&HdmiTxSs);
		    XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
			Menu = XHDMI_MAIN_MENU;
		break;

		// DVI Mode
		case ('n') :
		case ('N') :
			xil_printf("\r\nSet TX Mode To DVI .\r\n");
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
#endif

#if defined(USE_HDCP)
		// HDCP
		case ('h') :
		case ('H') :
			/* Enable HDCP menu option when HDCP is ready */
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs) && XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
			if (XV_HdmiTxSs_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
#endif
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
void XHdmi_DisplayResolutionMenu(void)
{
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
    xil_printf("19 -  720 x 480i (NTSC)\r\n");
    xil_printf("20 -  720 x 576i (PAL)\r\n");
    xil_printf("21 - 3840 x 2160p (SB) (Custom)\r\n");
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
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
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
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XVPHY_GTPE2)
            VideoMode = XVIDC_VM_3840x2160_60_P;
#else
            VideoMode = XVIDC_VM_3840x2160_30_P;
#endif
            break;

        // 4096 x 2160p
        case 8 :
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XVPHY_GTPE2)
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

		//3840 x 2160p (SB) (Custom)
		case 21 :
			VideoMode = XVIDC_VM_3840x2160_30_P_SB;
			break;

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
		EnableColorBar(&Vphy, &HdmiTxSs, VideoMode, XVIDC_CSF_RGB, XVIDC_BPC_8);
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
void XHdmi_DisplayFrameRateMenu(void)
{
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
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayFrameRateMenu();
			break;
	}

	if (FrameRate != XVIDC_FR_UNKNOWN) {
		UpdateFrameRate(&Vphy, &HdmiTxSs, FrameRate);
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
	XVidC_VideoStream *VidStrPtr;

	/* Check if TX is running at max resolution */
	VidStrPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XVPHY_GTPE2)
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
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
	XVidC_ColorDepth ColorDepth;
	XVidC_VideoStream *VidStrPtr;

	/* Check if TX is running at max resolution */
	VidStrPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XVPHY_GTPE2)
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
		UpdateColorDepth(&Vphy, &HdmiTxSs, ColorDepth);
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
	xil_printf("\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("---   COLOR SPACE MENU   ---\r\n");
	xil_printf("----------------------------\r\n");
	xil_printf("  1 - RGB\r\n");
	xil_printf("  2 - YUV444\r\n");
	xil_printf("  3 - YUV422\r\n");
	xil_printf("  4 - YUV420\r\n");
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
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayColorSpaceMenu();
			break;
	}

	if (ColorFormat != XVIDC_CSF_UNKNOWN) {
	UpdateColorFormat(&Vphy, &HdmiTxSs, ColorFormat);
		InstancePtr->WaitForColorbar = (TRUE);
	}

	return Menu;
}
#endif

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
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
	xil_printf("\r\n");
	xil_printf("------------------------------\r\n");
	xil_printf("---   GT PLL LAYOUT MENU   ---\r\n");
	xil_printf("------------------------------\r\n");
	xil_printf("\r\n");

	xil_printf("In this menu the GT PLL clocking layout can be selected.\r\n");

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("RX => QPLL / TX => CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT RX is clocked by the QPLL and the CPLL\r\n");
	xil_printf("is driving the GT TX. \r\n");
	xil_printf("\r\n");
	xil_printf("               --------     -----------     --------               \r\n");
	xil_printf("               |      |     | GT | GT |     |      |               \r\n");
	xil_printf(" RX clock -----| QPLL |-----| RX | TX |-----| CPLL |----- TX clock \r\n");
	xil_printf("               |      |     |    |    |     |      |               \r\n");
	xil_printf("               --------     -----------     --------               \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("RX => CPLL / TX => QPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT RX is clocked by the CPLL and the QPLL\r\n");
	xil_printf("is driving the GT TX. \r\n");
	xil_printf("\r\n");
	xil_printf("               --------     -----------     --------               \r\n");
	xil_printf("               |      |     | GT | GT |     |      |               \r\n");
	xil_printf(" RX clock -----| CPLL |-----| RX | TX |-----| QPLL |----- TX clock \r\n");
	xil_printf("               |      |     |    |    |     |      |               \r\n");
	xil_printf("               --------     -----------     --------               \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("RX => CPLL / TX => CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("In this mode the GT RX and GT TX are bonded and clocked by the same PLL.\r\n");
	xil_printf("When this mode is selected only pass-through video can be supported.\r\n");
	xil_printf("Also NI-DRU operation and TX oversampling are not supported in this mode.\r\n");
	xil_printf("\r\n");
	xil_printf("               --------     -----------                            \r\n");
	xil_printf("               | CPLL |     | GT | GT |                            \r\n");
	xil_printf(" RX clock -----|  /   |-----| RX | TX |-------- TX clock           \r\n");
	xil_printf("               | QPLL |  |  |    |    |  |                         \r\n");
	xil_printf("               --------  |  -----------  |                         \r\n");
	xil_printf("                         |               |                         \r\n");
	xil_printf("                         -----------------                         \r\n");
	xil_printf("\r\n\r\n");
	xil_printf("  1 - RX => QPLL / TX => CPLL\r\n");
	xil_printf("  2 - RX => CPLL / TX => QPLL\r\n");
	xil_printf("  3 - RX => CPLL / TX => CPLL\r\n");
	xil_printf("  4 - RX => QPLL / TX => QPLL\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf("RX => QPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT RX is clocked by the QPLL\r\n");
	xil_printf("\r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("               |      |     | GT |                                 \r\n");
	xil_printf(" RX clock -----| QPLL |-----| RX |                                 \r\n");
	xil_printf("               |      |     |    |                                 \r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("RX => CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT RX is clocked by the CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("               |      |     | GT |                                 \r\n");
	xil_printf(" RX clock -----| CPLL |-----| RX |                                 \r\n");
	xil_printf("               |      |     |    |                                 \r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("  1 - RX => QPLL             \r\n");
	xil_printf("  2 - RX => CPLL             \r\n");
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	xil_printf("TX => CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT TX is clocked by the CPLL\r\n");
	xil_printf("\r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("               |      |     | GT |                                 \r\n");
	xil_printf(" TX clock -----| CPLL |-----| TX |                                 \r\n");
	xil_printf("               |      |     |    |                                 \r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("TX => QPLL\r\n");
	xil_printf("\r\n");
	xil_printf("This is mode the GT TX is clocked by the QPLL\r\n");
	xil_printf("\r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("               |      |     | GT |                                 \r\n");
	xil_printf(" TX clock -----| QPLL |-----| TX |                                 \r\n");
	xil_printf("               |      |     |    |                                 \r\n");
	xil_printf("               --------     ------                                 \r\n");
	xil_printf("\r\n\r\n");

	xil_printf("  1 - TX => CPLL             \r\n");
	xil_printf("  2 - TX => QPLL             \r\n");
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
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	u8 IsPassThroughCopy;
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
#endif
	XVphy_SysClkDataSelType TxSysPllSelect;
	XVphy_SysClkDataSelType RxSysPllSelect;
	u8 IsValid = FALSE;

	// Default
	Menu = XHDMI_GTPLLLAYOUT_MENU;

	switch (Input) {

		// RX => QPLL / TX => CPLL
		case 1 :
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			xil_printf("Setting RX => QPLL / TX => CPLL\r\n\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			xil_printf("Setting RX => QPLL\r\n\r\n");
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
			xil_printf("Setting TX => CPLL\r\n\r\n");
#endif
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			IsValid = TRUE;
			break;

		// RX => CPLL / TX => QPLL
		case 2 :
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			xil_printf("Setting RX => CPLL / TX => QPLL\r\n\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
			xil_printf("Setting RX => CPLL\r\n\r\n");
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
			xil_printf("Setting TX => QPLL\r\n\r\n");
#endif
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			IsValid = TRUE;
			break;

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		// RX => CPLL / TX => CPLL
		case 3 :
			xil_printf("Setting RX => CPLL / TX => CPLL\r\n\r\n");
			xil_printf("Please connect a HDMI source to start video in pass-through mode.\r\n");
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			IsValid = TRUE;
			break;

		// RX => QPLL / TX => QPLL
		case 4 :
			xil_printf("Setting RX => QPLL / TX => QPLL\r\n\r\n");
			xil_printf("Please connect a HDMI source to start video in pass-through mode.\r\n");
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			IsValid = TRUE;
			break;
#endif

		// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			IsValid = FALSE;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayGtPllLayoutMenu();
			IsValid = FALSE;
			break;
	}

	if (IsValid) {
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		// The IsPassThrough variable will be cleared when the PLL layout is set.
		// Therefore we copy the variable first, so we know what to do after the PLL layout has changed.
		IsPassThroughCopy = IsPassThrough;
#endif

		/* Update VPHY Clocking */
		XVphy_HdmiUpdateClockSelection(&Vphy, 0, TxSysPllSelect, RxSysPllSelect);

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		// Is the reference design in pass-through
		// Then re-start pass-through
		if ((IsPassThroughCopy) || (Input == 3) || (Input == 4)) {
			xil_printf("Restart pass-through\r\n");
            XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
            XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
            XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
            XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
            XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
		}
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		// Is the reference design RX Only
		if ((Input == 1) || (Input == 2)) {
			xil_printf("Issue RX HPD\r\n");
            XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
            XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
            XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
            XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
            XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
		}
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
		// Is the reference design TX Only
		if ((Input == 1) || (Input == 2)) {
			// Get current video stream
			HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
			EnableColorBar(&Vphy,
				 &HdmiTxSs,
				 HdmiTxSsVidStreamPtr->VmId,
				 HdmiTxSsVidStreamPtr->ColorFormatId,
				 HdmiTxSsVidStreamPtr->ColorDepth);
			// Reset TX frequency detector
			XVphy_ClkDetFreqReset(&Vphy, 0,	XVPHY_DIR_TX);
		}
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES

		// Re-start colorbar
		else
		{
		  EnableColorBar(&Vphy,
						 &HdmiTxSs,
						 XVIDC_VM_1920x1080_60_P,
						 XVIDC_CSF_RGB,
						 XVIDC_BPC_8);
			InstancePtr->WaitForColorbar = (TRUE);
		}
#endif

		// Return to main menu
		xil_printf("Return to main menu.\r\n");
		Menu = XHDMI_MAIN_MENU;
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
void XHdmi_DisplayEdidMenu(void)
{
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   EDID MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("  1 - Display the edid of the connected sink device.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
	xil_printf("  2 - Clone the edid of the connected sink edid to HDMI Rx.\r\n");
	xil_printf("  3 - Load default edid to HDMI Rx.\r\n");
#endif
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
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_EDID_MENU;

	switch (Input) {

		// Show source edid
		case 1 :
			XV_HdmiTxSs_ShowEdid(&HdmiTxSs);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		// Clone edid
		case 2 :
			CloneTxEdid();
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

		// Load edid
		case 3 :
			XV_HdmiRxSs_LoadDefaultEdid(&HdmiRxSs);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;
#endif

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
void XHdmi_DisplayVideoMenu(void)
{
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
	xil_printf(" 13 - Black (Video Mask)\r\n");
	xil_printf(" 14 - White (Video Mask)\r\n");
	xil_printf(" 15 - Red (Video Mask)\r\n");
	xil_printf(" 16 - Green (Video Mask)\r\n");
	xil_printf(" 17 - Blue (Video Mask)\r\n");
	xil_printf(" 18 - Noise (Video Mask)\r\n");
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
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;
	XTpg_PatternId NewPattern;

	// Default
	Menu = XHDMI_VIDEO_MENU;
	NewPattern = XTPG_BKGND_LAST;

	// Insert carriage return
	xil_printf("\r\n");

	switch (Input) {
		case 1 :
		  NewPattern = XTPG_BKGND_COLOR_BARS;
		  xil_printf("Colorbars\r\n");
		  break;

		case 2 :
		  NewPattern = XTPG_BKGND_SOLID_RED;
		  xil_printf("Solid red\r\n");
		  break;

		case 3 :
		  NewPattern = XTPG_BKGND_SOLID_GREEN;
		  xil_printf("Solid green\r\n");
		  break;

		case 4 :
		  NewPattern = XTPG_BKGND_SOLID_BLUE;
		  xil_printf("Solid blue\r\n");
		  break;

		case 5 :
		  NewPattern = XTPG_BKGND_SOLID_BLACK;
		  xil_printf("Solid black\r\n");
		  break;

		case 6 :
		  NewPattern = XTPG_BKGND_SOLID_WHITE;
		  xil_printf("Solid white\r\n");
		  break;

		case 7 :
		  NewPattern = XTPG_BKGND_H_RAMP;
		  xil_printf("Horizontal ramp\r\n");
		  break;

		case 8 :
		  NewPattern = XTPG_BKGND_V_RAMP;
		  xil_printf("Vertical ramp\r\n");
		  break;

		case 9 :
		  NewPattern = XTPG_BKGND_RAINBOW_COLOR;
		  xil_printf("Rainbow colors\r\n");
		  break;

		case 10 :
		  NewPattern = XTPG_BKGND_CHECKER_BOARD;
		  xil_printf("Checker board\r\n");
		  break;

		case 11 :
		  NewPattern = XTPG_BKGND_CROSS_HATCH;
		  xil_printf("Cross hatch\r\n");
		  break;

		case 12 :
		  NewPattern = XTPG_BKGND_PBRS;
		  xil_printf("Noise\r\n");
		  break;

		case 13:
			xil_printf("Set Video to Static Black (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_BLACK);
			break;

		case 14:
			xil_printf("Set Video to Static White (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_WHITE);
			break;

		case 15:
			xil_printf("Set Video to Static Red (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_RED);
			break;

		case 16:
			xil_printf("Set Video to Static Green (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_GREEN);
			break;

		case 17:
			xil_printf("Set Video to Static Blue (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_BLUE);
			break;

		case 18:
			xil_printf("Set Video to Noise (Video Mask).\r\n");
			XV_HdmiTxSS_SetBackgroundColor(&HdmiTxSs, XV_BKGND_NOISE);
			break;

		// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayVideoMenu();
			break;
	}


	if (NewPattern != XTPG_BKGND_LAST) {
		 /* Set video pattern */
		 Pattern = NewPattern;
		  /* Start TPG */
		 xil_printf("new pattern\r\n");
		  XV_ConfigTpg(&Tpg);
		  XV_HdmiTxSS_MaskDisable(&HdmiTxSs);
		  xil_printf("Enter Selection -> ");
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
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   AUDIO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("  1 - Mute audio.\r\n");
	xil_printf("  2 - Unmute audio.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
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
			xil_printf("Mute audio.\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, TRUE);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

		// Unmute
		case 2 :
			xil_printf("Unmute audio.\r\n");
			XV_HdmiTxSs_AudioMute(&HdmiTxSs, FALSE);
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
			XHdmi_DisplayAudioMenu();
			break;
	}

	return Menu;
}
#endif


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
	xil_printf("\r\n");
	xil_printf("--------------------------\r\n");
	xil_printf("---   HDCP Main Menu   ---\r\n");
	xil_printf("--------------------------\r\n");
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf(" 1 - Enable repeater\r\n");
	xil_printf(" 2 - Disable repeater\r\n");
	xil_printf(" 3 - Enable detailed logging\r\n");
	xil_printf(" 4 - Disable detailed logging\r\n");
	xil_printf(" 5 - Display log\r\n");
	xil_printf(" 6 - Display repeater info\r\n");
	xil_printf(" 7 - Display HDCP Debug menu\r\n");
#else
	xil_printf(" 1 - Enable detailed logging\r\n");
	xil_printf(" 2 - Disable detailed logging\r\n");
	xil_printf(" 3 - Display log\r\n");
	xil_printf(" 4 - Display info\r\n");
	xil_printf(" 5 - Display HDCP Debug menu\r\n");
#endif
	xil_printf("99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}
#endif

#if defined(USE_HDCP)
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
	xil_printf("\r\n");

	switch (Input) {

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 1 - Enable repeater*/
		case 1:
			xil_printf("Enable repeater.\r\n");
			XHdcp_SetRepeater(&HdcpRepeater, TRUE);
			break;

		/* 2 - Disable repeater*/
		case 2:
			xil_printf("Disable repeater.\r\n");
			XHdcp_SetRepeater(&HdcpRepeater, FALSE);
			break;
#endif

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 3 - Enable detailed logging */
		case 3 :
#else
		/* 1 - Enable detailed logging */
		case 1 :
#endif
			xil_printf("Enable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, TRUE);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_HdcpSetInfoDetail(&HdmiTxSs, TRUE);
#endif
			break;

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 4 - Disable detailed logging */
		case 4 :
#else
		/* 2 - Disable detailed logging */
		case 2 :
#endif
			xil_printf("Disable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, FALSE);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_HdcpSetInfoDetail(&HdmiTxSs, FALSE);
#endif
			break;

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 5 - Display log */
		case 5 :
#else
		/* 3 - Display log */
		case 3 :
#endif
			xil_printf("Display log.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpInfo(&HdmiRxSs);
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
			XV_HdmiTxSs_HdcpInfo(&HdmiTxSs);
#endif
			break;

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 6 - Display repeater info */
		case 6 :
			xil_printf("Display repeater info.\r\n");
#else
		/* 4 - Display repeater info */
		case 4 :
			xil_printf("Display info.\r\n");
#endif
			XHdcp_DisplayInfo(&HdcpRepeater, TRUE);
			break;

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
		/* 7 - HDCP Debug Menu */
		case 7 :
#else
		/* 5 - HDCP Debug Menu */
		case 5 :
#endif
			xil_printf("Display HDCP Debug menu.\r\n");
			XHdmi_DisplayHdcpDebugMenu();
			Menu = XHDMI_HDCP_DEBUG_MENU;
			break;

		// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown command\r\n");
			XHdmi_DisplayHdcpMainMenu();
			break;
	}
	return Menu;
}
#endif

#if defined(USE_HDCP)
/*****************************************************************************/
/**
*
* This function displays the HDCP Debug menu.
*
* @param None
*
* @return None
*
******************************************************************************/
void XHdmi_DisplayHdcpDebugMenu(void)
{
	xil_printf("\r\n");
	xil_printf("--------------------------\r\n");
	xil_printf("---   HDCP Debug Menu   ---\r\n");
	xil_printf("--------------------------\r\n");
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf(" 1 - Set upstream capability to none\r\n");
	xil_printf(" 2 - Set upstream capability to both\r\n");
	xil_printf(" 3 - Set downstream capability to none\r\n");
	xil_printf(" 4 - Set downstream capability to 1.4\r\n");
	xil_printf(" 5 - Set downstream capability to 2.2\r\n");
	xil_printf(" 6 - Set downstream capability to both\r\n");
	/*xil_printf(" 7 - Toggle downstream content blocking \r\n");*/
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
	xil_printf(" 1 - Set upstream capability to none\r\n");
	xil_printf(" 2 - Set upstream capability to both\r\n");
#elif defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	xil_printf(" 1 - Set downstream capability to none\r\n");
	xil_printf(" 2 - Set downstream capability to 1.4\r\n");
	xil_printf(" 3 - Set downstream capability to 2.2\r\n");
	xil_printf(" 4 - Set downstream capability to both\r\n");
#endif
	xil_printf("99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}
#endif

#if defined(USE_HDCP)
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
static XHdmi_MenuType XHdmi_HdcpDebugMenu(XHdmi_Menu *InstancePtr, u8 Input)
{
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_HDCP_DEBUG_MENU;

	// Insert carriage return
	xil_printf("\r\n");

	switch (Input) {
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 1 - Set upstream capability to none */
		case 1:
			xil_printf("Set upstream capability to none.\r\n");
			XHdcp_SetUpstreamCapability(&HdcpRepeater, XV_HDMIRXSS_HDCP_NONE);
			break;

		/* 2 - Set upstream capability to both */
		case 2:
			xil_printf("Set upstream capability to both.\r\n");
			XHdcp_SetUpstreamCapability(&HdcpRepeater, XV_HDMIRXSS_HDCP_BOTH);
			break;
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 3 - Set downstream capability to none */
		case 3:
#else
		/* 1 - Set downstream capability to none */
		case 1:
#endif
			xil_printf("Set downstream capability to none.\r\n");
			XHdcp_SetDownstreamCapability(&HdcpRepeater, XV_HDMITXSS_HDCP_NONE);
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 4 - Set downstream capability to 1.4 */
		case 4:
#else
		/* 2 - Set downstream capability to 1.4 */
		case 2:
#endif
			xil_printf("Set downstream capability to 1.4\r\n");
			XHdcp_SetDownstreamCapability(&HdcpRepeater, XV_HDMITXSS_HDCP_14);
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 4 - Set downstream capability to 2.2 */
		case 5:
#else
		/* 3 - Set downstream capability to 2.2 */
		case 3:
#endif
			xil_printf("Set downstream capability to 2.2\r\n");
			XHdcp_SetDownstreamCapability(&HdcpRepeater, XV_HDMITXSS_HDCP_22);
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 6 - Set downstream capability to both */
		case 6:
#else
		/* 4 - Set downstream capability to both */
		case 4:
#endif
			xil_printf("Set downstream capability to both\r\n");
			XHdcp_SetDownstreamCapability(&HdcpRepeater, XV_HDMITXSS_HDCP_BOTH);
			break;

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
		/* 7 - Toggle downstream content blocking */
		case 7:
#else
		/* 5 - Toggle downstream content blocking */
		case 5:
#endif
			HdcpRepeater.EnforceBlocking = !(HdcpRepeater.EnforceBlocking);
               if (HdcpRepeater.EnforceBlocking) {
			  xil_printf("Enable downstream content blocking\r\n");
               } else {
			  xil_printf("Disable downstream content blocking\r\n");
               }
			break;
#endif

		// Exit
		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_HDCP_MAIN_MENU;
			break;

		default :
			xil_printf("Unknown command\r\n");
			XHdmi_DisplayHdcpDebugMenu();
			break;
	}
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
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if ((InstancePtr->WaitForColorbar) && (!TxBusy)) {
		InstancePtr->WaitForColorbar = (FALSE);
	xil_printf("Enter Selection -> ");
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
		      xil_printf("\r\nInvalid input. Valid entry is only digits 0-9. Try again\r\n\r\n");
		      xil_printf("Enter Selection -> ");
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
