/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
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
#include "xhdcp.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#endif
#include "xvidc_edid_ext.h"

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
#ifdef USE_HDMI_AUDGEN
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#ifdef USE_HDCP
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#if (HDCP_DEBUG_MENU_EN == 1)
static XHdmi_MenuType XHdmi_HdcpDebugMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
static XHdmi_MenuType XHdmi_DebugMainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
static XHdmi_MenuType XHdmi_Hdmiphy1DebugMenu(XHdmi_Menu *InstancePtr,
					u8 Input);
static XHdmi_MenuType XHdmi_DP159Menu(XHdmi_Menu *InstancePtr, u8 Input);
u8 XHdmi_SubMenuProcess(u8 Hex);

static void XHdmi_DisplayMainMenu(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static void XHdmi_DisplayEdidMenu(void);
static void XHdmi_DisplayResolutionMenu(void);
static void XHdmi_DisplayFrameRateMenu(void);
static void XHdmi_DisplayColorDepthMenu(void);
static void XHdmi_DisplayColorSpaceMenu(void);
#ifdef USE_HDMI_AUDGEN
static void XHdmi_DisplayAudioMenu(void);
static void XHdmi_DisplayAudioChannelMenu(void);
#endif
static void XHdmi_DisplayVideoMenu(void);
#endif
#ifdef USE_HDCP
static void XHdmi_DisplayHdcpMainMenu(void);
#if (HDCP_DEBUG_MENU_EN == 1)
static void XHdmi_DisplayHdcpDebugMenu(void);
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
static void XHdmi_DisplayDebugMainMenu(void);
#endif
static void XHdmi_DisplayHdmiphy1DebugMenu(void);
static void XHdmi_DisplayDP159Menu(void);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
extern u8 PLLBondedCheck (void);
#endif
extern void Info(void);
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern void EnableColorBar(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiTxSs *HdmiTxSsPtr,
						   XVidC_VideoMode VideoMode,
						   XVidC_ColorFormat ColorFormat,
						   XVidC_ColorDepth Bpc);;
extern void UpdateColorFormat(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorFormat ColorFormat);
extern void UpdateColorDepth(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiTxSs *pHdmiTxSs, XVidC_ColorDepth ColorDepth);
extern void UpdateFrameRate(XHdmiphy1 *Hdmiphy1Ptr, XV_HdmiTxSs *pHdmiTxSs, XVidC_FrameRate FrameRate);
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
#ifdef USE_HDMI_AUDGEN
	XHdmi_AudioMenu,
	XHdmi_AudioChannelMenu,
#endif
	XHdmi_VideoMenu,
#endif
#ifdef USE_HDCP
	XHdmi_HdcpMainMenu,
#if (HDCP_DEBUG_MENU_EN == 1)
	XHdmi_HdcpDebugMenu,
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
	XHdmi_DebugMainMenu,
#endif
    XHdmi_Hdmiphy1DebugMenu,
	XHdmi_DP159Menu,
};

extern XHdmiphy1 Hdmiphy1;               /* VPhy structure */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
extern XV_HdmiTxSs HdmiTxSs;       /* HDMI TX SS structure */
extern XhdmiAudioGen_t AudioGen;

extern u8 TxCableConnect;
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
#if(HDMI_DEBUG_TOOLS == 1)
		xil_printf("x - Debug Tools\r\n");
		xil_printf("       => Goto Debug menu.\r\n");
#endif
	}
#endif
//		xil_printf("y - HDMI PHY Debug Menu\r\n");
//		xil_printf("w - DP159 & TMDS181 Menu\r\n");

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
	u8 drpaddr = 0;
	u16 DrpRdVal = 0;
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
			// Check if a source is connected
			if (HdmiRxSs.IsStreamConnected == (TRUE)) {
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
				xil_printf("Force pass-through\r\n");
#elif defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
				xil_printf("Toggle HDMI RX HPD\r\n");
#endif
				ToggleHdmiRxHpd(&Hdmiphy1, &HdmiRxSs);
			}

			// No source
			else {
				xil_printf(ANSI_COLOR_YELLOW "No source device detected.\r\n"
							ANSI_COLOR_RESET);
			}
			Menu = XHDMI_MAIN_MENU;
			break;
#endif

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
				xil_printf("The example design is in pass-through mode.\r\n");
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
				xil_printf("The example design is in pass-through mode.\r\n");
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
				XHdmi_DisplayColorSpaceMenu();
				Menu = XHDMI_COLORSPACE_MENU;
			}

			// Pass-through
			else {
				xil_printf("The example design is in pass-through mode.\r\n");
				xil_printf("In this mode the video parameters can't be changed.\r\n");
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_LogDisplay(&HdmiRxSs);
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

#ifdef USE_HDMI_AUDGEN
			// Audio
		case ('a') :
		case ('A') :
			XHdmi_DisplayAudioMenu();
			Menu = XHDMI_AUDIO_MENU;
			break;
#endif

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
#if(HDMI_DEBUG_TOOLS == 1)
		case ('x') :
		case ('X') :
			XHdmi_DisplayDebugMainMenu();
			Menu = XHDMI_DEBUG_MAIN_MENU;
			break;
#endif
		case ('w') :
		case ('W') :
			XHdmi_DisplayDP159Menu();
			Menu = XHDMI_DP159_MENU;
			break;

		case ('y') :
		case ('Y') :
		    XHdmi_DisplayHdmiphy1DebugMenu();
		    Menu = XHDMI_VPHY_DEBUG_MENU;
		    break;

		case ('q') :
		case ('Q') :
			xil_printf("0xf70e2318: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2318, 0));
			xil_printf("0xf70e231C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e231C, 0));
			xil_printf("0xf70e2320: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2320, 0));
			xil_printf("0xf70e3448: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3448, 0));
			xil_printf("0xf70e300C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e300C, 0));
			xil_printf("0xf70e3840: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3840, 0));
			xil_printf("0xf70e2014: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2014, 0));
			xil_printf("0xf70e200C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e200C, 0));
			xil_printf("0xf70e25C4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e25C4, 0));
			xil_printf("0xf70e231C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e231C, 0));
			xil_printf("0xf70e3848: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3848, 0));
			xil_printf("0xf70e3290: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3290, 0));
			xil_printf("0xf70e32A4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e32A4, 0));
			xil_printf("0xf70e309C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e309C, 0));
			xil_printf("0xf70e349C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e349C, 0));
			xil_printf("0xf70e389C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e389C, 0));
			xil_printf("0xf70e3C9C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3C9C, 0));
			xil_printf("0xf70e31BC: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e31BC, 0));
			xil_printf("0xf70e35BC: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e35BC, 0));
			xil_printf("0xf70e39BC: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e39BC, 0));
			xil_printf("0xf70e3DBC: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3DBC, 0));
			xil_printf("0xf70e2008: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2008, 0));
			xil_printf("0xf70e2408: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2408, 0));
			xil_printf("0xf70e2808: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2808, 0));
			xil_printf("0xf70e2C08: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2C08, 0));
			xil_printf("0xf70e21C4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e21C4, 0));
			xil_printf("0xf70e25C4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e25C4, 0));
			xil_printf("0xf70e29C4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e29C4, 0));
			xil_printf("0xf70e2DC4: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2DC4, 0));
			xil_printf("0xf70e21C8: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e21C8, 0));
			xil_printf("0xf70e25C8: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e25C8, 0));
			xil_printf("0xf70e29C8: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e29C8, 0));
			xil_printf("0xf70e2DC8: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e2DC8, 0));

			xil_printf("\r\n");
			xil_printf("RPLL0  0xf70e304C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e304C, 0));
			xil_printf("RPLL1  0xf70e3C4C: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3C4C, 0));
			xil_printf("LCPLL0 0xf70e3048: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3048, 0));
			xil_printf("LCPLL1 0xf70e3C48: 0x%08x \r\n", XHdmiphy1_ReadReg(0xf70e3C48, 0));
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
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input) {
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
		case 22 :
			EnableColorBar(&Hdmiphy1, &HdmiTxSs, XVIDC_VM_1920x1080_60_P, XVIDC_CSF_RGB, XVIDC_BPC_12);
			InstancePtr->WaitForColorbar = (TRUE);
			return Menu;

		case 23 :
			EnableColorBar(&Hdmiphy1, &HdmiTxSs, XVIDC_VM_3840x2160_30_P, XVIDC_CSF_RGB, XVIDC_BPC_10);
			InstancePtr->WaitForColorbar = (TRUE);
			return Menu;

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
		EnableColorBar(&Hdmiphy1, &HdmiTxSs, VideoMode, XVIDC_CSF_RGB, XVIDC_BPC_8);
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
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input) {
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

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			// Clone edid
		case 2 :
			CloneTxEdid();
			break;

			// Load edid
		case 3 :
			XV_HdmiRxSs_LoadDefaultEdid(&HdmiRxSs);
			/* Toggle HPD after loading new HPD */
			ToggleHdmiRxHpd(&Hdmiphy1, &HdmiRxSs);
			xil_printf("HPD is toggled.\r\n");
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
void XHdmi_DisplayVideoMenu(void) {
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
#if (VIDEO_MASKING_MENU_EN == 1)
	xil_printf(" 13 - Black (Video Mask)\r\n");
	xil_printf(" 14 - White (Video Mask)\r\n");
	xil_printf(" 15 - Red (Video Mask)\r\n");
	xil_printf(" 16 - Green (Video Mask)\r\n");
	xil_printf(" 17 - Blue (Video Mask)\r\n");
	xil_printf(" 18 - Noise (Video Mask)\r\n");
#endif
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

#if (VIDEO_MASKING_MENU_EN == 1)
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
#endif

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
#ifdef USE_HDMI_AUDGEN
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
void XHdmi_DisplayAudioMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   AUDIO MENU   ---\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("  1 - Mute audio.\r\n");
	xil_printf("  2 - Unmute audio.\r\n");
	xil_printf("  3 - Configure audio channels.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input) {
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

			// Audio channels
		case 3 :
			xil_printf("Display Audio Channels menu.\r\n");
			XHdmi_DisplayAudioChannelMenu();
			Menu = XHDMI_AUDIO_CHANNEL_MENU;
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

/*****************************************************************************/
/**
*
* This function displays the audio channel menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayAudioChannelMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("- AUDIO CHANNEL MENU -\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("  1 - 2 Audio Channels.\r\n");
	xil_printf("  2 - 8 Audio Channels.\r\n");
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio channel menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XHdmiC_AudioInfoFrame *AudioInfoframePtr;

	AudioInfoframePtr = XV_HdmiTxSs_GetAudioInfoframe(&HdmiTxSs);

	// Default
	Menu = XHDMI_AUDIO_CHANNEL_MENU;

	switch (Input) {
			// 2 Audio Channels
		case 1 :
			print("2 Audio Channels.\r\n");
			XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
			XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
			XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 2);
			// Refer to CEA-861-D for Audio InfoFrame Channel Allocation
			// - - - - - - FR FL
			AudioInfoframePtr->ChannelAllocation = 0x0;
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// 8 Audio Channels
		case 2 :
			print("8 Audio Channels.\r\n");
			XhdmiAudGen_SetEnabChannels(&AudioGen, 8);
			XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
			XhdmiAudGen_SetPattern(&AudioGen, 3, XAUD_PAT_RAMP);
			XhdmiAudGen_SetPattern(&AudioGen, 4, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 5, XAUD_PAT_PING);
			XhdmiAudGen_SetPattern(&AudioGen, 6, XAUD_PAT_RAMP);
			XhdmiAudGen_SetPattern(&AudioGen, 7, XAUD_PAT_SINE);
			XhdmiAudGen_SetPattern(&AudioGen, 8, XAUD_PAT_PING);
			XV_HdmiTxSs_SetAudioChannels(&HdmiTxSs, 8);
			// Refer to CEA-861-D for Audio InfoFrame Channel Allocation
			// RRC RLC RR RL FC LFE FR FL
			AudioInfoframePtr->ChannelAllocation = 0x13;
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

			// Exit
		case 99 :
			xil_printf("Returning to audio menu.\r\n");
			Menu = XHDMI_AUDIO_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayAudioChannelMenu();
			break;
	}

	return Menu;
}
#endif
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
void XHdmi_DisplayHdcpMainMenu(void) {
	xil_printf("\r\n");
	xil_printf("--------------------------\r\n");
	xil_printf("---   HDCP Main Menu   ---\r\n");
	xil_printf("--------------------------\r\n");
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if ENABLE_HDCP_REPEATER
	xil_printf(" 1 - Enable repeater\r\n");
	xil_printf(" 2 - Disable repeater\r\n");
	xil_printf(" 3 - Enable detailed logging\r\n");
	xil_printf(" 4 - Disable detailed logging\r\n");
	xil_printf(" 5 - Display log\r\n");
	xil_printf(" 6 - Display repeater info\r\n");
#else
	xil_printf(" 1 - Enable detailed logging\r\n");
	xil_printf(" 2 - Disable detailed logging\r\n");
	xil_printf(" 3 - Display log\r\n");
	xil_printf(" 4 - Display info\r\n");
#endif
#if (HDCP_DEBUG_MENU_EN == 1)
#if ENABLE_HDCP_REPEATER
	xil_printf(" 7 - Display HDCP Debug menu\r\n");
#else
	xil_printf(" 5 - Display HDCP Debug menu\r\n");
#endif
#endif
#else
	xil_printf(" 1 - Enable detailed logging\r\n");
	xil_printf(" 2 - Disable detailed logging\r\n");
	xil_printf(" 3 - Display log\r\n");
	xil_printf(" 4 - Display info\r\n");
#if (HDCP_DEBUG_MENU_EN == 1)
	xil_printf(" 5 - Display HDCP Debug menu\r\n");
#endif
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
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_HDCP_MAIN_MENU;

	// Insert carriage return
	xil_printf("\r\n");

	switch (Input) {

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if ENABLE_HDCP_REPEATER
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
#endif

#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if ENABLE_HDCP_REPEATER
			/* 3 - Enable detailed logging */
		case 3 :
#else
			/* 1 - Enable detailed logging [no repeater] */
		case 1 :
#endif
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
#if ENABLE_HDCP_REPEATER
			/* 4 - Disable detailed logging */
		case 4 :
#else
			/* 2 - Disabled detailed logging [no repeater] */
		case 2 :
#endif
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
#if ENABLE_HDCP_REPEATER
			/* 5 - Display log */
		case 5 :
#else
			/* 3 - Display log [no repeater] */
		case 3 :
#endif
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
#if ENABLE_HDCP_REPEATER
			/* 6 - Display repeater info */
		case 6 :
			xil_printf("Display repeater info.\r\n");
#else
			/* 4 - Display Info [no repeater] */
		case 4 :
			xil_printf("Display Info [no repeater].\r\n");
#endif
#else
			/* 4 - Display repeater info */
		case 4 :
			xil_printf("Display info.\r\n");
#endif
			XHdcp_DisplayInfo(&HdcpRepeater, TRUE);
			break;

#if (HDCP_DEBUG_MENU_EN == 1)
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES) && defined (XPAR_XV_HDMIRXSS_NUM_INSTANCES)
#if ENABLE_HDCP_REPEATER
			/* 7 - HDCP Debug Menu */
		case 7 :
#else
			/* 5 - HDCP Debug Menu [no repeater] */
		case 5 :
#endif
#else
			/* 5 - HDCP Debug Menu */
		case 5 :
#endif
			xil_printf("Display HDCP Debug menu.\r\n");
			XHdmi_DisplayHdcpDebugMenu();
			Menu = XHDMI_HDCP_DEBUG_MENU;
			break;
#endif

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

#if (HDCP_DEBUG_MENU_EN == 1)
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
void XHdmi_DisplayHdcpDebugMenu(void) {
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
#endif

#if (HDCP_DEBUG_MENU_EN == 1)
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
static XHdmi_MenuType XHdmi_HdcpDebugMenu(XHdmi_Menu *InstancePtr, u8 Input) {
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
#endif

#if(HDMI_DEBUG_TOOLS == 1)
/*****************************************************************************/
/**
*
* This function displays the debug menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
static void XHdmi_DisplayDebugMainMenu(void) {
	xil_printf("\r\n");
	xil_printf("----------------------\r\n");
	xil_printf("---   DEBUG MENU   ---\r\n");
	xil_printf("----------------------\r\n");
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	xil_printf("  1 - Enable Scrambler.\r\n");
	xil_printf("  2 - Disable Scrambler.\r\n");
#endif
	xil_printf(" 99 - Exit\r\n");
	xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI debug menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_DebugMainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_DEBUG_MAIN_MENU;

	switch (Input) {
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
		case 1 :
			XV_HdmiTxSs_SetScrambler(&HdmiTxSs, TRUE);
			// Display the prompt for the next input
			xil_printf("Enter Selection -> ");
			break;

		case 2 :
			XV_HdmiTxSs_SetScrambler(&HdmiTxSs, FALSE);
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
			XHdmi_DisplayDebugMainMenu();
			break;
	}

	return Menu;
}
#endif

/*****************************************************************************/
/**
*
* This function displays the debug menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
static void XHdmi_DisplayHdmiphy1DebugMenu(void) {
    xil_printf("\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("---   HDMI PHY DEBUG MENU   ---\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX POLARITY\r\n");
    xil_printf("  1 - Non-Inverted\r\n");
    xil_printf("  2 - Inverted\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX PRBS\r\n");
    xil_printf("  3 - Standard operation\r\n");
    xil_printf("  4 - PRBS-7\r\n");
    xil_printf("  5 - PRBS-9\r\n");
    xil_printf("  6 - PRBS-15\r\n");
    xil_printf("  7 - PRBS-23\r\n");
    xil_printf("  8 - PRBS-31\r\n");
    xil_printf("  9 - Toggle PRBS Force Error\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX VSWING (TXDIFFCTRL)\r\n");
    xil_printf("  14- Increment\r\n");
    xil_printf("  15- Decrement\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX POST CURSOR (32 steps)\r\n");
    xil_printf("  16- Increment\r\n");
    xil_printf("  17- Decrement\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX PRE CURSOR (21 steps)\r\n");
    xil_printf("  18- Increment\r\n");
    xil_printf("  19- Decrement\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" RX POLARITY\r\n");
    xil_printf("  20- Non-Inverted\r\n");
    xil_printf("  21- Inverted\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" RX PRBS\r\n");
    xil_printf("  22- Standard operation\r\n");
    xil_printf("  23- PRBS-7\r\n");
    xil_printf("  24- PRBS-9\r\n");
    xil_printf("  25- PRBS-15\r\n");
    xil_printf("  26- PRBS-23\r\n");
    xil_printf("  27- PRBS-31\r\n");
    xil_printf("  28- Reset PRBS Error Status\r\n");
    xil_printf("  29- Display PRBS Error Status\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" RX EQUALIZATION\r\n");
    xil_printf("  30- LPM\r\n");
    xil_printf("  31- DFE (default)\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" LOOPBACK\r\n");
    xil_printf("  32- Enable\r\n");
    xil_printf("  33- Disable (default)\r\n");
    xil_printf("---------------------------\r\n");
#if defined XPAR_XV_HDMITXSS1_NUM_INSTANCES || \
	defined XPAR_XV_HDMIRXSS1_NUM_INSTANCES
    xil_printf(" FORCE TXRX LINERATE\r\n");
    xil_printf("  34- 3  Gbps\r\n");
    xil_printf("  35- 6  Gbps\r\n");
    xil_printf("  36- 8  Gbps\r\n");
    xil_printf("  37- 10 Gbps\r\n");
    xil_printf("  38- 12 Gbps\r\n");
    xil_printf("---------------------------\r\n");
#endif
    xil_printf("  88- Display Register & DRP Dump\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("  99- Exit\r\n");
    xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI debug menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_Hdmiphy1DebugMenu(XHdmi_Menu *InstancePtr,
			u8 Input) {
	/* Variables */
	XHdmi_MenuType 	Menu;
	u32 RegVal;

	/* Default */
	Menu = XHDMI_VPHY_DEBUG_MENU;

	switch (Input) {
	case 1 :
		xil_printf("TX Polarity Non-Inverted\r\n");
		XHdmiphy1_SetPolarity(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, 0);
		break;

	case 2 :
		xil_printf("TX Polarity Inverted\r\n");
		XHdmiphy1_SetPolarity(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, 1);
		break;

	case 3 :
	    xil_printf("TX PRBS Standard\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_STD_MODE);
		break;

	case 4 :
	    xil_printf("TX PRBS-7\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_PRBS7);
		break;

	case 5 :
	    xil_printf("TX PRBS-9\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_PRBS9);
		break;

	case 6 :
	    xil_printf("TX PRBS-15\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_PRBS15);
		break;

	case 7 :
	    xil_printf("TX PRBS-23\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_PRBS23);
		break;

	case 8 :
	    xil_printf("TX PRBS-31\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_PRBSSEL_PRBS31);
		break;

	case 9 :
	    xil_printf("TX PRBS Force Error Toggle\r\n");
		XHdmiphy1_TxPrbsForceError(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CHA, 1);
		XHdmiphy1_TxPrbsForceError(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CHA, 0);
		break;

	case 14:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) & 0xF);
		xil_printf("TX Diff Swing Increment old: 0x%01x new: 0x%02x \r\n",
				RegVal, RegVal+1);
		RegVal = RegVal+1;
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, RegVal);
		break;

	case 15:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) & 0xF);
		xil_printf("TX Diff Swing Decrement old: 0x%01x new: 0x%02x \r\n",
				    RegVal, RegVal-1);
		RegVal = RegVal-1;
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, RegVal);
		break;

	case 16:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) &
				  XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_MASK(
						XHDMIPHY1_CHANNEL_ID_CH1))
					>> 6;
		if (RegVal == 0x1F) {
			RegVal = 0;
		} else {
			RegVal++;
		}
	    xil_printf("TX Post Cursor Increment (%d of 31)\r\n", RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, (u8)RegVal);
		break;

	case 17:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) &
				  XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_MASK(
						XHDMIPHY1_CHANNEL_ID_CH1))
					>> 6;
		if (RegVal == 0) {
			RegVal = 0x1F;
		} else {
			RegVal--;
		}
	    xil_printf("TX Post Cursor Decrement (%d of 31)\r\n", RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, (u8)RegVal);
		XHdmiphy1_SetTxPostCursor(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, (u8)RegVal);
		break;

	case 18:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) &
				  XHDMIPHY1_TX_DRIVER_TXPRECURSOR_MASK(
						XHDMIPHY1_CHANNEL_ID_CH1))
					>> 11;
		if (RegVal == 20) {
			RegVal = 0;
		} else {
			RegVal++;
		}
	    xil_printf("TX Pre Cursor Increment (%d of 20)\r\n", RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH1, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH2, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH3, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH4, (u8)RegVal);
		break;

	case 19:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) &
				  XHDMIPHY1_TX_DRIVER_TXPRECURSOR_MASK(
					XHDMIPHY1_CHANNEL_ID_CH1))
					>> 11;
		if (RegVal == 0) {
			RegVal = 20;
		} else {
			RegVal--;
		}
	    xil_printf("TX Pre Cursor Decrement (%d of 20)\r\n", RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH1, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH2, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH3, (u8)RegVal);
	    XHdmiphy1_SetTxPreEmphasis(&Hdmiphy1, 0,
			XHDMIPHY1_CHANNEL_ID_CH4, (u8)RegVal);
		break;

	case 20:
		xil_printf("RX Polarity Non-Inverted\r\n");
		XHdmiphy1_SetPolarity(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, 0);
		break;

	case 21:
		xil_printf("RX Polarity Inverted\r\n");
		XHdmiphy1_SetPolarity(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, 1);
		break;

	case 22:
	    xil_printf("RX PRBS Standard\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_STD_MODE);
		break;

	case 23:
	    xil_printf("RX PRBS-7\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_PRBS7);
		break;

	case 24:
	    xil_printf("RX PRBS-9\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_PRBS9);
		break;

	case 25:
	    xil_printf("RX PRBS-15\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_PRBS15);
		break;

	case 26:
	    xil_printf("RX PRBS-23\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_PRBS23);
		break;

	case 27:
	    xil_printf("RX PRBS-31\r\n");
		XHdmiphy1_SetPrbsSel(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_PRBSSEL_PRBS31);
		break;

	case 28:
		xil_printf("RX Reset PRBS Error Status\r\n");
		RegVal = XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
						XHDMIPHY1_RX_CONTROL_REG);
		XHdmiphy1_WriteReg(Hdmiphy1.Config.BaseAddr,
				XHDMIPHY1_RX_CONTROL_REG, RegVal | 0x08080808);
		XHdmiphy1_WriteReg(Hdmiphy1.Config.BaseAddr,
				XHDMIPHY1_RX_CONTROL_REG, RegVal & ~0x08080808);
		break;

	case 29:
		xil_printf("RX Display PRBS Error Status\r\n");
		RegVal = XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
						XHDMIPHY1_RX_STATUS_REG);
		xil_printf(" CH0->%s\r\n",((RegVal>>4 ) & 0x1)?"Errors Found!":"None");
		xil_printf(" CH1->%s\r\n",((RegVal>>12) & 0x1)?"Errors Found!":"None");
		xil_printf(" CH2->%s\r\n",((RegVal>>20) & 0x1)?"Errors Found!":"None");
		xil_printf(" CH3->%s\r\n",((RegVal>>28) & 0x1)?"Errors Found!":"None");
		break;

	case 30:
		xil_printf("RX EQ: LPM\r\n");
		XHdmiphy1_SetRxLpm(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, 1);
		break;

	case 31:
		xil_printf("RX EQ: DFE\r\n");
		XHdmiphy1_SetRxLpm(&Hdmiphy1, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, 0);
		break;

	case 32:
		xil_printf("LOOPBACK Enabled\r\n");
		XHdmiphy1_WriteReg(Hdmiphy1.Config.BaseAddr, 0x38, 0x02020202);
		break;

	case 33:
		xil_printf("LOOPBACK Cleared\r\n");
		XHdmiphy1_WriteReg(Hdmiphy1.Config.BaseAddr, 0x38, 0);
		break;
#if defined XPAR_XV_HDMITXSS1_NUM_INSTANCES || \
	defined XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	case 34:
		xil_printf("HDMI PHY 3 Gbps\r\n");
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (FALSE));
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX, 3e9, 4);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, 3e9, 4);
		break;

	case 35:
		xil_printf("HDMI PHY 6 Gbps\r\n");
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (FALSE));
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX, 6e9, 4);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, 6e9, 4);
		break;

	case 36:
		xil_printf("HDMI PHY 8 Gbps\r\n");
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (FALSE));
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX, 8e9, 4);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, 8e9, 4);
		break;

	case 37:
		xil_printf("HDMI PHY 10 Gbps\r\n");
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (FALSE));
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX, 10e9, 4);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, 10e9, 4);
		break;

	case 38:
		xil_printf("HDMI PHY 12 Gbps\r\n");
		XHdmiphy1_IBufDsEnable(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, (FALSE));
		Vfmc_Gpio_Ch4_DataClock_Sel(&Vfmc[0], VFMC_GPIO_RX_CH4_As_Data);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_TX, 12e9, 4);
		XHdmiphy1_Hdmi21Config(&Hdmiphy1, 0, XHDMIPHY1_DIR_RX, 12e9, 4);
		break;
#endif

	case 88:
		XHdmiphy1_RegisterDebug(&Hdmiphy1);
		break;

	case 99 :
		xil_printf("Returning to main menu.\r\n");
		Menu = XHDMI_MAIN_MENU;
		break;

	default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayHdmiphy1DebugMenu();
			break;
	}

	return Menu;
}

/*****************************************************************************/
/**
*
* This function displays the debug menu.
*
* @param None
*
* @return None
*
*
******************************************************************************/
static void XHdmi_DisplayDP159Menu(void) {
    xil_printf("\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("---  PHY DEV DEBUG MENU ---\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("  DP159 Menu\r\n");
    xil_printf("  1 - DP159 REG READ\r\n");
    xil_printf("  2 - DP159 REG WRITE\r\n");
    xil_printf("  3 - DP159 REG Dump\r\n");
    xil_printf("  TMDS181 Menu\r\n");
    xil_printf("  4 - TMDS181 REG READ\r\n");
    xil_printf("  5 - TMDS181 REG WRITE\r\n");
    xil_printf("  6 - TMDS181 REG Dump\r\n");
    xil_printf("  99- Exit\r\n");
    xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI debug menu state.
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_DP159Menu(XHdmi_Menu *InstancePtr, u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;
    u8 Addr, Data;

    /* Default */
    Menu = XHDMI_DP159_MENU;

    switch (Input) {
    case 1 :
	xil_printf("Enter Read Addr: 0x");
	Addr = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");

	// Reset I2C controller before issuing new transaction. This is required to
	// recover the IIC controller in case a previous transaction is pending.
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
			XIIC_RESET_MASK);

	xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr),
			i2c_dp159_read(1, Addr));
	break;

    case 2 :
	xil_printf("Enter Write Addr: 0x");
	Addr = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter Write Data: 0x");
	Data = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");

	// Reset I2C controller before issuing new transaction. This is required to
	// recover the IIC controller in case a previous transaction is pending.
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
			XIIC_RESET_MASK);

	i2c_dp159_write(1, Addr, Data);

	break;

    case 3 :
    i2c_dp159_dump();
	break;

    case 4 :
	xil_printf("Enter Read Addr: 0x");
	Addr = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");

	// Reset I2C controller before issuing new transaction. This is required to
	// recover the IIC controller in case a previous transaction is pending.
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
			XIIC_RESET_MASK);

	xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr),
			i2c_tmds181_read(1, Addr));
	break;

    case 5 :
	xil_printf("Enter Write Addr: 0x");
	Addr = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter Write Data: 0x");
	Data = XHdmi_SubMenuProcess(1);
	xil_printf("\r\n");

	// Reset I2C controller before issuing new transaction. This is required to
	// recover the IIC controller in case a previous transaction is pending.
	XIic_WriteReg(XPAR_IIC_0_BASEADDR, XIIC_RESETR_OFFSET,
			XIIC_RESET_MASK);

	i2c_tmds181_write(1, Addr, Data);

	break;

    case 6 :
    i2c_tmds181_dump();
	break;

	// Exit
    case 99 :
	xil_printf("Returning to main menu.\r\n");
	Menu = XHDMI_MAIN_MENU;
	break;

default :
	    xil_printf("Unknown option\r\n");
	    XHdmi_DisplayDP159Menu();
	    break;
    }

    return Menu;
}
extern XHdmi_Menu         HdmiMenu;
u8 XHdmi_SubMenuProcess(u8 Hex) {
    u8 Data = 0;
    u8 Value = 0;
    u8 IsValid = 0;
    u8 Mult = Hex ? 16 : 10;
    u8 n_char = 0;

    do {
	IsValid = 0;

    /* Check if the uart has any data */
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
    if (!XUartLite_IsReceiveEmpty(HdmiMenu.UartBaseAddress)) {
#else
    if (!XUartLite_IsReceiveEmpty(HdmiMenu.UartBaseAddress)) {
#endif

	/* Read data from uart */
	Data = XUartLite_RecvByte(HdmiMenu.UartBaseAddress);
#elif defined versal
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
	if (XUartPsv_IsReceiveData(HdmiMenu.UartBaseAddress)) {
#else
	if (XUartPsv_IsReceiveData(HdmiMenu.UartBaseAddress)) {
#endif
		// Read data from uart
		Data = XUartPsv_RecvByte(HdmiMenu.UartBaseAddress);
#else
#if defined (XPAR_XV_HDMITXSS_NUM_INSTANCES)
    if (XUartPs_IsReceiveData(HdmiMenu.UartBaseAddress)) {
#else
    if (XUartPs_IsReceiveData(HdmiMenu.UartBaseAddress)) {
#endif
	/* Read data from uart */
	Data = XUartPs_RecvByte(HdmiMenu.UartBaseAddress);
#endif


	/* Numeric data */
	if ((Data >= '0') && (Data <= '9')) {
	    Value = Value * Mult + (Data-'0');
	    IsValid = 1;
	}

	/* Capital Letter data */
	else if ((Data >= 'A') && (Data <= 'F')) {
	    Value = Value * Mult + (Data-55);
	    IsValid = 1;
	}

	/* Small Letter data */
	else if ((Data >= 'a') && (Data <= 'f')) {
	    Value = Value * Mult + (Data-87);
	    IsValid = 1;
	}

	/* Backspace */
	else if (Data == '\b' || Data == 127) {
			/* discard previous input */
	    Value = Value / Mult;
	    IsValid = 1;
	}
	else if ((Data == '\n') || (Data == '\r')) {
	    IsValid = 1;
	}

	if (IsValid == 1) {
	/* Send response to user */
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
	XUartLite_SendByte(HdmiMenu.UartBaseAddress, Data);
	n_char++;
#elif defined versal
	XUartPsv_SendByte(HdmiMenu.UartBaseAddress, Data);
	n_char++;
#else
	XUartPs_SendByte(HdmiMenu.UartBaseAddress, Data);
	n_char++;
#endif
	}
    }

    } while ((!((Data == '\n') || (Data == '\r')) || (n_char < 2)));

    return Value;
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
			InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
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
				xil_printf("Invalid input. Valid entry is only digits 0-9. Try again\r\n\r\n");
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
				xil_printf("\r\n");
				InstancePtr->CurrentMenu = XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
				InstancePtr->Value = 0;
			}
		}
	}
}
