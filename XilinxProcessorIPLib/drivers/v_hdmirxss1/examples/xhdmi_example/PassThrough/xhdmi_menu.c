/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.0   mmo  30-04-2019 Initial version
* 1.1   ku   14-07-2020 Menu option for 4K Quad selection
*                       Menu option for 16 BPC
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
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
    },
    {
	XVIDC_VM_2560x1440_144_P, "2560x1440@144Hz", XVIDC_FR_144HZ,
	{
	    2560, 48, 32, 80, 2720, 0,
	    1440, 3, 5, 7, 1455, 0, 0, 0, 0, 1
	}
    }
};
#endif
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#if defined (XPAR_XUARTPSV_NUM_INSTANCES )
#define XV_UART_SEND_BYTE XUartPsv_SendByte
#define XV_UART_RECV_BYTE XUartPsv_RecvByte
#define XV_UART_ISRECEIVEDATA XUartPsv_IsReceiveData
#elif defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define XV_UART_SEND_BYTE XUartLite_SendByte
#define XV_UART_RECV_BYTE XUartLite_RecvByte
#define XV_UART_ISRECEIVEDATA !XUartLite_IsReceiveEmpty
#else
#define XV_UART_SEND_BYTE XUartPs_SendByte
#define XV_UART_RECV_BYTE XUartPs_RecvByte
#define XV_UART_ISRECEIVEDATA XUartPs_IsReceiveData
#endif

/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_EdidMenu(XHdmi_Menu *InstancePtr, u8 Input);
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
static XHdmi_MenuType XHdmi_ResolutionMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_FrameRateMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorDepthMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input);
#ifdef USE_HDMI_AUDGEN
static XHdmi_MenuType XHdmi_AudioMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr, u8 Input);
static XHdmi_MenuType
	XHdmi_AudioSampFreqMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#ifdef XPAR_XV_TPG_NUM_INSTANCES
static XHdmi_MenuType XHdmi_VideoMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
#endif
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
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
static XHdmi_MenuType XHdmi_OnSemiDebugMenu(XHdmi_Menu *InstancePtr, u8 Input);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
static XHdmi_MenuType XHdmi_8kQuadMenu(XHdmi_Menu *InstancePtr, u8 Input);
#endif
static void XHdmi_DisplayMainMenu(void);
static void XHdmi_DisplayEdidMenu(void);
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
static void XHdmi_DisplayResolutionMenu(void);
static void XHdmi_DisplayFrameRateMenu(void);
static void XHdmi_DisplayColorDepthMenu(void);
static void XHdmi_DisplayColorSpaceMenu(void);
#ifdef USE_HDMI_AUDGEN
static void XHdmi_DisplayAudioMenu(void);
static void XHdmi_DisplayAudioChannelMenu(void);
static void XHdmi_DisplayAudioSampFreqMenu(void);
#endif
static void XHdmi_DisplayVideoMenu(void);
#endif
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
static void XHdmi_DisplayHdcpMainMenu(void);
#if (HDCP_DEBUG_MENU_EN == 1)
static void XHdmi_DisplayHdcpDebugMenu(void);
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
static void XHdmi_DisplayDebugMainMenu(void);
#endif
static void XHdmi_DisplayHdmiphy1DebugMenu(void);
static void XHdmi_DisplayOnSemiDebugMenu(void);
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
static void XHdmi_Display8kQuadMenu(void);
#endif
static u8 XHdmi_OneSemiMenuProcess(u8 Hex);

extern void Info(void);
extern void DetailedInfo(void);
extern void TxFrlStartDebug(void);
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
extern void UpdateColorFormat(XHdmiphy1 *Hdmiphy1Ptr,
				XV_HdmiTxSs1 *pHdmiTxSs,
				XVidC_ColorFormat ColorFormat);
extern void UpdateColorDepth(XHdmiphy1 *Hdmiphy1Ptr,
				XV_HdmiTxSs1 *pHdmiTxSs,
				XVidC_ColorDepth ColorDepth);
extern void UpdateFrameRate(XHdmiphy1 *Hdmiphy1Ptr,
				XV_HdmiTxSs1 *pHdmiTxSs,
				XVidC_FrameRate FrameRate);
extern void XV_HdmiTxSs1_ShowEdid_extension(XV_HdmiTxSs1 *InstancePtr, XV_VidC_EdidCntrlParam *EdidCtrlParam);
extern void CloneTxEdid(void);
extern EdidHdmi EdidHdmi_t;
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
extern void XV_HdmiRxSs1_LoadEdid(XV_HdmiRxSs1 *InstancePtr,
					u8 *EdidData,
					u16 Length);
extern void XV_HdmiRxSs1_ToggleRxHpd(XV_HdmiRxSs1 *InstancePtr);
extern void HDCPXILCMD_ProcessKey(char theCmdKey);
#endif

/************************* Variable Definitions *****************************/
/*HDMI Menu Variable */
XHdmi_Menu       HdmiMenu;

/**
* This table contains the function pointers for all possible states.
* The order of elements must match the XHdmi_MenuType enumerator definitions.
*/
static XHdmi_MenuFuncType* const XHdmi_MenuTable[XHDMI_NUM_MENUS] = {
    XHdmi_MainMenu,
    XHdmi_EdidMenu,
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    XHdmi_ResolutionMenu,
    XHdmi_FrameRateMenu,
    XHdmi_ColorDepthMenu,
    XHdmi_ColorSpaceMenu,
#ifdef USE_HDMI_AUDGEN
    XHdmi_AudioMenu,
    XHdmi_AudioChannelMenu,
    XHdmi_AudioSampFreqMenu,
#endif
#ifdef XPAR_XV_TPG_NUM_INSTANCES
    XHdmi_VideoMenu,
#endif
#endif
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
    XHdmi_HdcpMainMenu,
#if (HDCP_DEBUG_MENU_EN == 1)
    XHdmi_HdcpDebugMenu,
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
    XHdmi_DebugMainMenu,
#endif
    XHdmi_Hdmiphy1DebugMenu,
    XHdmi_OnSemiDebugMenu,
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
               (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	XHdmi_8kQuadMenu,
#endif
};

extern XVfmc Vfmc[1];
extern XHdmiphy1 Hdmiphy1;               /* VPhy structure */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
extern XV_HdmiTxSs1 HdmiTxSs;       /* HDMI TX SS structure */
#ifdef USE_HDMI_AUDGEN
extern XhdmiAudioGen_t AudioGen;
#endif
#endif

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
extern XV_Rx xhdmi_example_rx_controller;
extern XV_HdmiRxSs1 HdmiRxSs;       /* HDMI RX SS structure */
#endif
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
extern XV_Tx xhdmi_example_tx_controller;
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
extern XV_tpg Tpg;              /* TPG structure */
extern XTpg_PatternId Pattern;
#endif
#endif

/*HDMI EDID*/
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
extern EdidHdmi EdidHdmi_t;
#endif
extern u8 Buffer[];

#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
		      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
extern void ResetVidFrameBufWr(void);
extern void ResetVidFrameBufRd(void);
extern void XV_ConfigVidFrameBuf(XV_FrmbufWr_l2 *FrmBufWrPtr,
			  XV_FrmbufRd_l2 *FrmBufRdPtr);
extern XV_FrmbufWr_l2       FrameBufWr;
extern XV_FrmbufRd_l2        FrameBufRd;
extern u32 offset;
#endif

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
void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress,
		u8 *ForceIndependentPtr, u8 *SystemEventPtr, u8 *IsTxPresentPtr,
		u8 *IsRxPresentPtr, void *ChangeColorbarOutputCB,
		void *ConfigureTpgEnableInputCB, void *ToggleHdmiRxHpdCB)
{
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
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
    InstancePtr->AdvColorDepth = XVIDC_BPC_8;
    InstancePtr->AdvColorSpace = XVIDC_CSF_RGB;

    InstancePtr->ExDesCtrlr.ForceIndependentPtr = ForceIndependentPtr;
    InstancePtr->ExDesCtrlr.SystemEventPtr = SystemEventPtr;
    InstancePtr->ExDesCtrlr.IsTxPresentPtr = IsTxPresentPtr;
    InstancePtr->ExDesCtrlr.IsRxPresentPtr = IsRxPresentPtr;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    InstancePtr->ExDesCtrlr.ChangeColorbarOutputCB =
		ChangeColorbarOutputCB;
    InstancePtr->ExDesCtrlr.ConfigureTpgEnableInputCB=
		ConfigureTpgEnableInputCB;
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
    InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB =
		ToggleHdmiRxHpdCB;
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
    /* Initialize and Add Custom Resolution in to the Video Table
     * Added for the resolution menu
     * Example : User registers custom timing table
     */
    Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
	     (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
    if (Status != XST_SUCCESS) {
	xil_printf("ERR: Unable to register custom timing table\r\n\r\n");
    }
#endif
#endif

    /* Show main menu*/
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
    xil_printf("       => Shows information about the HDMI RX stream, \r\n"
	  "          HDMI TX stream, GT transceivers and PLL settings.\r\n");
    xil_printf("l - Detailed Info\r\n");
    xil_printf("       => Additional/Detail Info of the system\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    xil_printf("c - Change Mode\r\n");
    xil_printf("       => Change the mode of the application between \r\n"
	       "          independent and non-independent modes.\r\n");
    xil_printf("r - Resolution\r\n");
    xil_printf("       => Change the video resolution of the colorbar.\r\n");
    xil_printf("f - Frame rate\r\n");
    xil_printf("       => Change the frame rate of the colorbar.\r\n");
    xil_printf("d - Color depth\r\n");
    xil_printf("       => Change the color depth of the colorbar.\r\n");
    xil_printf("s - Color space\r\n");
    xil_printf("       => Change the color space of the colorbar.\r\n");
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
    xil_printf("q - View 4K Quad Video\r\n");
    xil_printf("       => Select to display a part 4K Video on TX (TMDS) when \r\n"
		"              RX (FRL) receives 8K Video\r\n");
#endif
#endif
#if defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    xil_printf("p - Toggle HPD\r\n");
    xil_printf("       => Toggles the HPD of HDMI RX.\r\n");
#endif
    xil_printf("z - GT & HDMI TX/RX log\r\n");
    xil_printf("       => Shows log information for GT & HDMI TX/RX.\r\n");
    xil_printf("e - Edid\r\n");
    xil_printf("       => Display and set edid.\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef USE_HDMI_AUDGEN
    xil_printf("a - Audio\r\n");
    xil_printf("       => Audio options.\r\n");
#endif
    xil_printf("v - Video\r\n");
    xil_printf("       => Video pattern options.\r\n");
#endif

#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
    /* Show HDCP menu option when HDCP is ready */
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
	defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs) &&
		XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
    if (XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs)) {
#endif
	xil_printf("h - HDCP\r\n");
	xil_printf("       => Goto HDCP menu.\r\n");
    }
#endif
#if(HDMI_DEBUG_TOOLS == 1)
    xil_printf("x - Debug Tools\r\n");
    xil_printf("       => Goto Debug menu.\r\n");
#endif
    xil_printf("y - HDMI PHY Debug Menu\r\n");
    xil_printf("o - OnSemi NB7NQ621M/ TI TMDS1204 Debug\r\n");



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
    /* Variables */
    XHdmi_MenuType  Menu;

    /* Default */
    Menu = XHDMI_MAIN_MENU;

    switch (Input) {
	    /* Info */
	case ('i') :
	case ('I') :
	    Info();
	    Menu = XHDMI_MAIN_MENU;
	    break;

	    /* Detailed/Additional Info */
	case ('l') :
	case ('L') :
		DetailedInfo();
	    Menu = XHDMI_MAIN_MENU;
	    break;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* Sending HPD */
	case ('p') :
	case ('P') :
	    if (HdmiRxSs.IsStreamConnected == (TRUE)) {
		XV_Rx_SetHpd(&xhdmi_example_rx_controller, FALSE);
		/* Wait 500 ms */
		usleep(500000);
		XV_Rx_SetHpd(&xhdmi_example_rx_controller, TRUE);
	    }
	    Menu = XHDMI_MAIN_MENU;
	    break;
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    /* Changing Operating Mode */
	case ('c') :
	case ('C') :
	    if (*InstancePtr->ExDesCtrlr.ForceIndependentPtr == TRUE) {
		*InstancePtr->ExDesCtrlr.ForceIndependentPtr = FALSE;
	    } else if (*InstancePtr->ExDesCtrlr.ForceIndependentPtr == FALSE) {
		*InstancePtr->ExDesCtrlr.ForceIndependentPtr = TRUE;
	    }
		*InstancePtr->ExDesCtrlr.SystemEventPtr = TRUE;
	    Menu = XHDMI_MAIN_MENU;
	    break;

	    /* Resolution */
	case ('r') :
	case ('R') :

	    /* Default */
	    Menu = XHDMI_MAIN_MENU;

	    /* Check if the TX only is active or tx and rx are independent */
	    if ((!(*InstancePtr->ExDesCtrlr.IsRxPresentPtr) &&
			*InstancePtr->ExDesCtrlr.IsTxPresentPtr) ||
		(*InstancePtr->ExDesCtrlr.ForceIndependentPtr == TRUE)) {
		Menu = XHDMI_RESOLUTION_MENU;
		xil_printf("---------------------------\r\n");
		xil_printf("---   RESOLUTION MENU   ---\r\n");
		xil_printf("---------------------------\r\n");
		xil_printf("---------------------------\r\n");
		xil_printf("--- CS: %s CD: %d       ---\r\n",
		    XVidC_GetColorFormatStr(InstancePtr->AdvColorSpace),
		    InstancePtr->AdvColorDepth);
		xil_printf("---------------------------\r\n");
		XHdmi_DisplayResolutionMenu();

	    }

	    /* Pass-through */
	    else {
		xil_printf("The example design is in pass-through mode.\r\n");
		xil_printf("In this mode the "
				"video parameters can't be changed.\r\n");
	    }
	    break;

	    /* Frame rate */
	case ('f') :
	case ('F') :

	    /* Default */
	    Menu = XHDMI_MAIN_MENU;

	    /* Check if the TX only is active or tx and rx are independent */
	    if ((!(*InstancePtr->ExDesCtrlr.IsRxPresentPtr) &&
		*InstancePtr->ExDesCtrlr.IsTxPresentPtr) ||
		(*InstancePtr->ExDesCtrlr.ForceIndependentPtr == TRUE)) {

		XHdmi_DisplayFrameRateMenu();
		Menu = XHDMI_FRAMERATE_MENU;

	    }

	    /* Pass-through */
	    else {
		xil_printf("The example design is in pass-through mode.\r\n");
		xil_printf("In this mode the "
				"video parameters can't be changed.\r\n");
	    }
	    break;

	    /* Color depth */
	case ('d') :
	case ('D') :

	    /* Default */
	    Menu = XHDMI_MAIN_MENU;

	    /* Check if the TX only is active or tx and rx are independent */
	    if ((!(*InstancePtr->ExDesCtrlr.IsRxPresentPtr) &&
		*InstancePtr->ExDesCtrlr.IsTxPresentPtr) ||
		(*InstancePtr->ExDesCtrlr.ForceIndependentPtr == TRUE)) {

		XHdmi_DisplayColorDepthMenu();
		Menu = XHDMI_COLORDEPTH_MENU;

	    }

	    /* Pass-through */
	    else {
		xil_printf("The example design is in pass-through mode.\r\n");
		xil_printf("In this mode the "
				"video parameters can't be changed.\r\n");
	    }
	    break;

	    /* Color space */
	case ('s') :
	case ('S') :
	    /* Default */
	    Menu = XHDMI_MAIN_MENU;

	    /* Check if the TX only is active or tx and rx are independent */
		if ((!(*InstancePtr->ExDesCtrlr.IsRxPresentPtr) &&
		*InstancePtr->ExDesCtrlr.IsTxPresentPtr) ||
		(*InstancePtr->ExDesCtrlr.ForceIndependentPtr == TRUE)) {

		XHdmi_DisplayColorSpaceMenu();
		Menu = XHDMI_COLORSPACE_MENU;

	    }

	    /* Pass-through */
	    else {
		xil_printf("The example design is in pass-through mode.\r\n");
		xil_printf("In this mode the "
				"video parameters can't be changed.\r\n");
	    }
	    break;
#endif

	    /* GT & HDMI TX/RX log */
	case ('z') :
	case ('Z') :
	    XHdmiphy1_LogDisplay(&Hdmiphy1);
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    XV_HdmiTxSs1_LogDisplay(&HdmiTxSs);
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    XV_HdmiRxSs1_LogDisplay(&HdmiRxSs);
#endif
	    Menu = XHDMI_MAIN_MENU;
	    break;


	    /* Edid */
	case ('e') :
	case ('E') :
	    XHdmi_DisplayEdidMenu();
	    Menu = XHDMI_EDID_MENU;
	    break;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef USE_HDMI_AUDGEN
	    /* Audio */
	case ('a') :
	case ('A') :
	    XHdmi_DisplayAudioMenu();
	    Menu = XHDMI_AUDIO_MENU;
	    break;
#endif
	    /* Video */
	case ('v') :
	case ('V') :
	    XHdmi_DisplayVideoMenu();
	    Menu = XHDMI_VIDEO_MENU;
	    break;
#endif

#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
	    /* HDCP */
	case ('h') :
	case ('H') :
	    /* Enable HDCP menu option when HDCP is ready */
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs) &&
			XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	    if (XV_HdmiTxSs1_HdcpIsReady(&HdmiTxSs)) {
#elif defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    if (XV_HdmiRxSs1_HdcpIsReady(&HdmiRxSs)) {
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

	case ('y') :
	case ('Y') :
	    XHdmi_DisplayHdmiphy1DebugMenu();
	    Menu = XHDMI_VPHY_DEBUG_MENU;
	    break;

	case ('o') :
	case ('O') :
	    XHdmi_DisplayOnSemiDebugMenu();
	    Menu = XHDMI_ONSEMI_DEBUG_MENU;
	    break;
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	case ('q') :
	case ('Q') :
	    XHdmi_Display8kQuadMenu();
	    Menu = XHDMI_8KQUAD_MENU;
	    break;
#endif

	default :
	    XHdmi_DisplayMainMenu();
	    Menu = XHDMI_MAIN_MENU;
	    break;
    }

    return Menu;
}


#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
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
    xil_printf(" 1    - 1920  x 1080p25\r\n");
    xil_printf(" 2    - 1920  x 1080p30\r\n");
    xil_printf(" 3    - 1920  x 1080p50\r\n");
    xil_printf(" 4    - 1920  x 1080p60\r\n");
    xil_printf(" 5    - 1920  x 1080p100\r\n");
    xil_printf(" 6    - 1920  x 1080p120\r\n");

    xil_printf(" 7    - 3840  x 2160p24\r\n");
    xil_printf(" 8    - 3840  x 2160p25\r\n");
    xil_printf(" 9    - 3840  x 2160p30\r\n");
    xil_printf(" 10   - 3840  x 2160p50\r\n");
    xil_printf(" 11   - 3840  x 2160p60\r\n");
    xil_printf(" 12   - 3840  x 2160p100\r\n");
    xil_printf(" 13   - 3840  x 2160p120\r\n");

    xil_printf(" 14   - 4096  x 2160p50\r\n");
    xil_printf(" 15   - 4096  x 2160p60\r\n");
    xil_printf(" 16   - 4096  x 2160p100\r\n");
    xil_printf(" 17   - 4096  x 2160p120\r\n");

    xil_printf(" 18   - 5120  x 2160p24\r\n");
    xil_printf(" 19   - 5120  x 2160p25\r\n");
    xil_printf(" 20   - 5120  x 2160p30\r\n");
    xil_printf(" 21   - 5120  x 2160p50\r\n");
    xil_printf(" 22   - 5120  x 2160p60\r\n");
    xil_printf(" 23   - 5120  x 2160p100\r\n");
    xil_printf(" 24   - 5120  x 2160p120\r\n");

    xil_printf(" 25   - 7680  x 4320p24\r\n");
    xil_printf(" 26   - 7680  x 4320p25\r\n");
    xil_printf(" 27   - 7680  x 4320p30\r\n");

    xil_printf(" 28   - 10240 x 4320p24\r\n");
    xil_printf(" 29   - 10240 x 4320p25\r\n");
    xil_printf(" 30   - 10240 x 4320p30\r\n");

    xil_printf(" 31   - 720   x 576p50\r\n");
    xil_printf(" 32   - 1280  x 720p50\r\n");
    xil_printf(" 33   - 1680  x 720p60\r\n");
    xil_printf(" 34   - 2560  x 1080p60\r\n");
    xil_printf(" 35   - 1920  x 1080i60\r\n");

    xil_printf(" 36   - 640   x 480p60  (VGA / DMT0659)\r\n");
    xil_printf(" 37   - 800   x 600p60  (SVGA / DMT0860)\r\n");
    xil_printf(" 38   - 1024  x 768p60  (XGA / DMT1060)\r\n");
    xil_printf(" 39   - 1280  x 768p60  (WXGA / CVT1260E)\r\n");
    xil_printf(" 40   - 1280  x 1024p60 (SXGA / DMT1260G)\r\n");
    xil_printf(" 41   - 1680  x 1050p60 (WSXGA+ / CVT1660D)\r\n");
    xil_printf(" 42   - 1600  x 1200p60 (UXGA / DMT1660)\r\n");
    xil_printf(" 43   - 1920  x 1200p60 (WUXGA / CVT1960D)\r\n");
#if(CUSTOM_RESOLUTION_ENABLE == 1)
    xil_printf(" 44   - 1152  x 864p60  (Custom)\r\n");
    xil_printf(" 45   - 2560  x 1440p144(Custom)\r\n");
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
    /* Variables */
    XHdmi_MenuType  Menu;
    XVidC_VideoMode VideoMode;

    /* Default */
    Menu = XHDMI_RESOLUTION_MENU;
    VideoMode = XVIDC_VM_NO_INPUT;

    switch (Input) {
	case 1 :
	    VideoMode = XVIDC_VM_1920x1080_25_P;
	    break;
	case 2 :
	    VideoMode = XVIDC_VM_1920x1080_30_P;
	    break;
	case 3 :
	    VideoMode = XVIDC_VM_1920x1080_50_P;
	    break;
	case 4 :
	    VideoMode = XVIDC_VM_1920x1080_60_P;
	    break;
	case 5 :
	    VideoMode = XVIDC_VM_1920x1080_100_P;
	    break;
	case 6 :
	    VideoMode = XVIDC_VM_1920x1080_120_P;
	    break;

	case 7 :
	    VideoMode = XVIDC_VM_3840x2160_24_P;
	    break;
	case 8 :
	    VideoMode = XVIDC_VM_3840x2160_25_P;
	    break;
	case 9 :
	    VideoMode = XVIDC_VM_3840x2160_30_P;
	    break;
	case 10 :
	    VideoMode = XVIDC_VM_3840x2160_50_P;
	    break;
	case 11 :
	    VideoMode = XVIDC_VM_3840x2160_60_P;
	    break;
	case 12 :
	    VideoMode = XVIDC_VM_3840x2160_100_P;
	    break;
	case 13 :
	    VideoMode = XVIDC_VM_3840x2160_120_P;
	    break;

	case 14 :
	    VideoMode = XVIDC_VM_4096x2160_50_P;
	    break;
	case 15 :
	    VideoMode = XVIDC_VM_4096x2160_60_P;
	    break;
	case 16 :
	    VideoMode = XVIDC_VM_4096x2160_100_P;
	    break;
	case 17 :
	    VideoMode = XVIDC_VM_4096x2160_120_P;
	    break;

	case 18 :
	    VideoMode = XVIDC_VM_5120x2160_24_P;
	    break;
	case 19 :
	    VideoMode = XVIDC_VM_5120x2160_25_P;
	    break;
	case 20 :
	    VideoMode = XVIDC_VM_5120x2160_30_P;
	    break;
	case 21 :
	    VideoMode = XVIDC_VM_5120x2160_50_P;
	    break;
	case 22 :
	    VideoMode = XVIDC_VM_5120x2160_60_P;
	    break;
	case 23 :
	    VideoMode = XVIDC_VM_5120x2160_100_P;
	    break;
	case 24 :
	    VideoMode = XVIDC_VM_5120x2160_120_P;
	    break;

	case 25 :
	    VideoMode = XVIDC_VM_7680x4320_24_P;
	    break;
	case 26 :
	    VideoMode = XVIDC_VM_7680x4320_25_P;
	    break;
	case 27 :
	    VideoMode = XVIDC_VM_7680x4320_30_P;
	    break;

	case 28 :
	    VideoMode = XVIDC_VM_10240x4320_24_P;
	    break;
	case 29 :
	    VideoMode = XVIDC_VM_10240x4320_25_P;
	    break;
	case 30 :
	    VideoMode = XVIDC_VM_10240x4320_30_P;
	    break;

	case 31 :
	    VideoMode = XVIDC_VM_720x576_50_P;
	    break;
	case 32 :
	    VideoMode = XVIDC_VM_1280x720_50_P;
	    break;
	case 33 :
	    VideoMode = XVIDC_VM_1680x720_60_P;
	    break;
	case 34 :
	    VideoMode = XVIDC_VM_2560x1080_60_P;
	    break;
	case 35 :
	    VideoMode = XVIDC_VM_1920x1080_60_I;
	    break;

	case 36 :
	    VideoMode = XVIDC_VM_640x480_60_P;
	    break;
	case 37 :
	    VideoMode = XVIDC_VM_800x600_60_P;
	    break;
	case 38 :
	    VideoMode = XVIDC_VM_1024x768_60_P;
	    break;
	case 39 :
	    VideoMode = XVIDC_VM_1280x768_60_P;
	    break;
	case 40 :
	    VideoMode = XVIDC_VM_1280x1024_60_P;
	    break;
	case 41 :
	    VideoMode = XVIDC_VM_1680x1050_60_P;
	    break;
	case 42 :
	    VideoMode = XVIDC_VM_1600x1200_60_P;
	    break;
	case 43 :
	    VideoMode = XVIDC_VM_1920x1200_60_P;
	    break;
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	case 44 :
	    VideoMode = XVIDC_VM_1152x864_60_P;
	    break;
	case 45 :
	    VideoMode = XVIDC_VM_2560x1440_144_P;
	    break;
#endif
	/* Exit */
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
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	if (InstancePtr->AdvColorSpace == XVIDC_CSF_YCRCB_422) {
		InstancePtr->ExDesCtrlr.ChangeColorbarOutputCB(VideoMode,
					InstancePtr->AdvColorSpace, 12);
	} else {
		InstancePtr->ExDesCtrlr.ChangeColorbarOutputCB(VideoMode,
			InstancePtr->AdvColorSpace, InstancePtr->AdvColorDepth);
	}
	*InstancePtr->ExDesCtrlr.SystemEventPtr = TRUE;
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
    /* Variables */
    XHdmi_MenuType  Menu;
    XVidC_FrameRate FrameRate;

    /* Default */
    Menu = XHDMI_FRAMERATE_MENU;
    FrameRate = XVIDC_FR_UNKNOWN;

    switch (Input) {

	    /* 24 Hz */
	case 1 :
	    FrameRate = XVIDC_FR_24HZ;
	    break;

	    /* 25 Hz */
	case 2 :
	    FrameRate = XVIDC_FR_25HZ;
	    break;

	    /* 30 Hz */
	case 3 :
	    FrameRate = XVIDC_FR_30HZ;
	    break;

	    /* 50 Hz */
	case 4 :
	    FrameRate = XVIDC_FR_50HZ;
	    break;

	    /* 60 Hz */
	case 5 :
	    FrameRate = XVIDC_FR_60HZ;
	    break;

	    /* 100 Hz */
	case 6 :
	    FrameRate = XVIDC_FR_100HZ;
	    break;

	    /* 120 Hz */
	case 7 :
	    FrameRate = XVIDC_FR_120HZ;
	    break;

	    /* Exit */
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
#if 0
    /* Check if TX is running at max resolution */
    VidStrPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XHDMIPHY1_GTPE2)
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
#endif
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
    /* Variables */
    XHdmi_MenuType  Menu;
    XVidC_ColorDepth ColorDepth;
#if 0
    /* Check if TX is running at max resolution */
    VidStrPtr = XV_HdmiTxSs1_GetVideoStream(&HdmiTxSs);
#if (XPAR_VID_PHY_CONTROLLER_TRANSCEIVER != XHDMIPHY1_GTPE2)
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
#endif

    /* Default */
    ColorDepth = XVIDC_BPC_UNKNOWN;
    Menu = XHDMI_COLORDEPTH_MENU;

    switch (Input) {
	    /* 24 bpp */
	case 1 :
	    ColorDepth = XVIDC_BPC_8;
	    break;

	    /* 30 bpp */
	case 2 :
	    ColorDepth = XVIDC_BPC_10;
	    break;

	    /* 36 bpp */
	case 3 :
	    ColorDepth = XVIDC_BPC_12;
	    break;

	    /* 48 bpp */
	case 4 :
	    ColorDepth = XVIDC_BPC_16;
	    break;
	    /* Exit */
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
	InstancePtr->AdvColorDepth = ColorDepth;
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
static XHdmi_MenuType XHdmi_ColorSpaceMenu(XHdmi_Menu *InstancePtr, u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;
    XVidC_ColorFormat ColorFormat;

    /* Default */
    ColorFormat = XVIDC_CSF_UNKNOWN;
    Menu = XHDMI_COLORSPACE_MENU;

    switch (Input) {
	    /* RGB */
	case 1 :
	    ColorFormat = XVIDC_CSF_RGB;
	    break;

	    /* YUV444 */
	case 2 :
	    ColorFormat = XVIDC_CSF_YCRCB_444;
	    break;

	    /* YUV422 */
	case 3 :
	    ColorFormat = XVIDC_CSF_YCRCB_422;
	    break;
	    /* YUV420 */
	case 4 :
	    ColorFormat = XVIDC_CSF_YCRCB_420;
	    break;
	    /* Exit */
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
	InstancePtr->AdvColorSpace = ColorFormat;
	UpdateColorFormat(&Hdmiphy1, &HdmiTxSs, ColorFormat);
	InstancePtr->WaitForColorbar = (TRUE);
    }

    return Menu;
}
#endif


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
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    SinkCapabilityCheck(&EdidHdmi_t);
    SinkCapWarningMsg(&EdidHdmi_t);
#endif
    xil_printf("---------------------\r\n");
    xil_printf("---   EDID MENU   ---\r\n");
    xil_printf("---------------------\r\n");
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    xil_printf("  1 - Display the EDID of the connected sink device.\r\n");

    xil_printf("  2 - Clone the EDID of the connected sink to "
						"HDMI Rx EDID.\r\n");
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
    xil_printf("  3 - Load default EDID to HDMI Rx.\r\n");
    xil_printf("\r\n");
    xil_printf("---------------------\r\n");
    xil_printf("  Load HDMI Rx EDID :\r\n");
    xil_printf("---------------------\r\n");
    xil_printf("  10 - TMDS.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate == 6)
	xil_printf("  11 - RX FRL 4 Lanes 12G.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate >= 5)
	xil_printf("  12 - RX FRL 4 Lanes 10G.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate >= 4)
	xil_printf("  13 - RX FRL 4 Lanes 8G.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate >= 3)
	xil_printf("  14 - RX FRL 4 Lanes 6G.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate >= 2)
	xil_printf("  15 - RX FRL 3 Lanes 6G.\r\n");
    if(HdmiRxSs.Config.MaxFrlRate >= 1)
	xil_printf("  16 - RX FRL 3 Lanes 3G.\r\n");
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
    /* Variables */
    XHdmi_MenuType  Menu;
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    u8 Buffer[512];
    int Status = XST_FAILURE;
#endif
    /* Default */
    Menu = XHDMI_EDID_MENU;
    switch (Input) {
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    /* Show source edid */
	case 1 :
	    XV_HdmiTxSs1_ShowEdid_extension(&HdmiTxSs, &EdidHdmi_t.EdidCtrlParam);
	    /* Read TX edid */
	    xil_printf("\r\n");

	    Status = XV_HdmiTxSs1_ReadEdid_extension(&HdmiTxSs, &EdidHdmi_t.EdidCtrlParam);
	    /* Only Parse the EDID when the Read EDID success */
	    if (Status == XST_SUCCESS) {
		XV_VidC_parse_edid((u8*)&Buffer,
				    &EdidHdmi_t.EdidCtrlParam,
				    XVIDC_VERBOSE_ENABLE);
	    } else {
		xil_printf(ANSI_COLOR_YELLOW "EDID parsing has failed.\r\n"
			    ANSI_COLOR_RESET);
	    }
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;


	    /* Clone edid */
	case 2 :
	    CloneTxEdid();
	    break;
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* Load edid */
	case 3 :
	    XV_HdmiRxSs1_LoadDefaultEdid(&HdmiRxSs);
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 10 :
	    xhdmi_example_rx_controller.Edid[189] = 0x03;
	    xhdmi_example_rx_controller.Edid[255] = 0x2B;
	    XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			(u8*)&(xhdmi_example_rx_controller.Edid), 256);
	    InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
	    break;

	case 11 :
		if(HdmiRxSs.Config.MaxFrlRate == 6) {
	        xhdmi_example_rx_controller.Edid[189] = 0x63;
	        xhdmi_example_rx_controller.Edid[255] = 0xCB;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			     (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	        InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 12Gbps @ 4 Lanes is not supported.\r\n");
		}
	    break;

	case 12 :
		if(HdmiRxSs.Config.MaxFrlRate >= 5) {
	        xhdmi_example_rx_controller.Edid[189] = 0x53;
	        xhdmi_example_rx_controller.Edid[255] = 0xDB;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			    (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	         InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 10Gbps @ 4 Gbps is not supported.\r\n");
		}
	    break;

	case 13 :
		if(HdmiRxSs.Config.MaxFrlRate >= 4) {
	        xhdmi_example_rx_controller.Edid[189] = 0x43;
	        xhdmi_example_rx_controller.Edid[255] = 0xEB;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			   (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	        InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 8Gbps @ 4 Lanes is not supported.\r\n");
		}
	    break;

	case 14 :
		if(HdmiRxSs.Config.MaxFrlRate >= 3) {
	        xhdmi_example_rx_controller.Edid[189] = 0x33;
	        xhdmi_example_rx_controller.Edid[255] = 0xFB;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			    (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	        InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 6Gbps @ 4 Lanes is not supported.\r\n");
		}
	    break;

	case 15 :
		if(HdmiRxSs.Config.MaxFrlRate >= 2) {
	        xhdmi_example_rx_controller.Edid[189] = 0x23;
	        xhdmi_example_rx_controller.Edid[255] = 0x0B;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			   (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	        InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 6Gbps @ 3 Lanes is not supported.\r\n");
		}
	    break;

	case 16 :
		if(HdmiRxSs.Config.MaxFrlRate >= 1) {
	        xhdmi_example_rx_controller.Edid[189] = 0x13;
	        xhdmi_example_rx_controller.Edid[255] = 0x1B;
	        XV_HdmiRxSs1_LoadEdid(&HdmiRxSs,
			     (u8*)&(xhdmi_example_rx_controller.Edid), 256);
	        InstancePtr->ExDesCtrlr.ToggleHdmiRxHpdCB(&Hdmiphy1, &HdmiRxSs);
		}else {
			xil_printf("FRL Rate : 3Gbps @ 3 Lanes is not supported.\r\n");
		}
	    break;
#endif
	    /* Exit */
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
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
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
#ifdef XPAR_XV_TPG_NUM_INSTANCES
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
    xil_printf(" 13 - Tartan Color Bar\r\n");
#endif
#if (VIDEO_MASKING_MENU_EN == 1)
    xil_printf(" 20 - Black (Video Mask)\r\n");
    xil_printf(" 21 - White (Video Mask)\r\n");
    xil_printf(" 22 - Red   (Video Mask)\r\n");
    xil_printf(" 23 - Green (Video Mask)\r\n");
    xil_printf(" 24 - Blue  (Video Mask)\r\n");
    xil_printf(" 25 - Noise (Video Mask)\r\n");
#endif
    xil_printf(" 99 - Exit\r\n");
    xil_printf("Enter Selection -> ");
}
#ifdef XPAR_XV_TPG_NUM_INSTANCES
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
    /* Variables */
    XHdmi_MenuType  Menu;
    XTpg_PatternId NewPattern;

    /* Default */
    Menu = XHDMI_VIDEO_MENU;
    NewPattern = XTPG_BKGND_LAST;

    /* Insert carriage return */
    xil_printf("\r\n");

    switch (Input) {
#ifdef XPAR_XV_TPG_NUM_INSTANCES
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

	case 13 :
	    NewPattern = XTPG_BKGND_TARTAN_COLOR_BARS;
	    xil_printf("Tartan Color Bar\r\n");
	    break;
#endif
#if (VIDEO_MASKING_MENU_EN == 1)
	case 20:
	    xil_printf("Set Video to Static Black (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_BLACK);
	    break;

	case 21:
	    xil_printf("Set Video to Static White (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_WHITE);
	    break;

	case 22:
	    xil_printf("Set Video to Static Red (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_RED);
	    break;

	case 23:
	    xil_printf("Set Video to Static Green (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_GREEN);
	    break;

	case 24:
	    xil_printf("Set Video to Static Blue (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_BLUE);
	    break;

	case 25:
	    xil_printf("Set Video to Noise (Video Mask).\r\n");
	    XV_HdmiTxSS1_SetBackgroundColor(&HdmiTxSs, XV_BKGND_NOISE);
	    break;
#endif

	    /* Exit */
	case 99 :
	    xil_printf("Returning to main menu.\r\n");
	    Menu = XHDMI_MAIN_MENU;
	    break;

	default :
	    xil_printf("Unknown option\r\n");
	    XHdmi_DisplayVideoMenu();
	    break;
    }

#ifdef XPAR_XV_TPG_NUM_INSTANCES
    if (NewPattern != XTPG_BKGND_LAST) {
	/* Set video pattern */
	Pattern = NewPattern;
	/* Start TPG */
	xil_printf("new pattern\r\n");
	InstancePtr->ExDesCtrlr.ConfigureTpgEnableInputCB(FALSE);
	XV_HdmiTxSS1_MaskDisable(&HdmiTxSs);
	xil_printf("Enter Selection -> ");
    }
#endif
    return Menu;
}

#endif

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
    xil_printf("  4 - Configure audio sampling frequency.\r\n");
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
    /* Variables */
    XHdmi_MenuType  Menu;

    /* Default */
    Menu = XHDMI_AUDIO_MENU;

    switch (Input) {
	    /* Mute */
	case 1 :
	    xil_printf("Mute audio.\r\n");
	    XV_HdmiTxSs1_AudioMute(&HdmiTxSs, TRUE);
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	    /* Unmute */
	case 2 :
	    xil_printf("Unmute audio.\r\n");
	    XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	    /* Audio channels */
	case 3 :
	    xil_printf("Display Audio Channels menu.\r\n");
	    XHdmi_DisplayAudioChannelMenu();
	    Menu = XHDMI_AUDIO_CHANNEL_MENU;
	    break;

	    /* Audio Sample Frequency */
	case 4 :
	    xil_printf("Display Audio Sample Frequency menu.\r\n");
	    XHdmi_DisplayAudioSampFreqMenu();
	    Menu = XHDMI_AUDIO_SAMPFREQ_MENU;
	    break;

	    /* Exit */
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
static XHdmi_MenuType XHdmi_AudioChannelMenu(XHdmi_Menu *InstancePtr,
						u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;
    XHdmiC_AudioInfoFrame *AudioInfoframePtr;

    AudioInfoframePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);

    /* Default */
    Menu = XHDMI_AUDIO_CHANNEL_MENU;

    switch (Input) {
	    /* 2 Audio Channels */
	case 1 :
	    print("2 Audio Channels.\r\n");
	    XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
	    XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_SINE);
	    XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
	    XV_HdmiTxSs1_SetAudioChannels(&HdmiTxSs, 2);
	    /* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
	     * - - - - - - FR FL
	     */
	    AudioInfoframePtr->ChannelAllocation = 0x0;
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	    /* 8 Audio Channels */
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
	    XV_HdmiTxSs1_SetAudioChannels(&HdmiTxSs, 8);
	    /* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
	     * RRC RLC RR RL FC LFE FR FL
	     */
	    AudioInfoframePtr->ChannelAllocation = 0x13;
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	    /* Exit */
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

/*****************************************************************************/
/**
*
* This function displays the audio sampling frequency
*
* @param None
*
* @return None
*
*
******************************************************************************/
void XHdmi_DisplayAudioSampFreqMenu(void) {
    xil_printf("\r\n");
    xil_printf("-------------------------------\r\n");
    xil_printf("- AUDIO SAMPLE FREQUENCY MENU -\r\n");
    xil_printf("-   2 CHANNEL AUDIO CONFIG.   -\r\n");
    xil_printf("-------------------------------\r\n");
    xil_printf("  1 - 32kHz.\r\n");
    xil_printf("  2 - 44.1kHz.\r\n");
    xil_printf("  3 - 48kHz.\r\n");
    xil_printf("  4 - 96kHz.\r\n");
    xil_printf("  5 - 176.4kHz.\r\n");
    xil_printf("  6 - 192kHz.\r\n");
    xil_printf(" 99 - Exit\r\n");
    xil_printf("Enter Selection -> ");
}

/*****************************************************************************/
/**
*
* This function implements the HDMI audio sampling frequency
*
* @param input is the value used for the next menu state decoder.
*
* @return The next menu state.
*
******************************************************************************/
static XHdmi_MenuType XHdmi_AudioSampFreqMenu(XHdmi_Menu *InstancePtr,
						u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;
    XHdmiC_AudioInfoFrame *AudioInfoframePtr;

    /* Default */
    Menu = XHDMI_AUDIO_SAMPFREQ_MENU;
	/* Set audio format and number of audio channels */
	XV_Tx_SetAudioFormatAndChannels(&xhdmi_example_tx_controller,
					XV_HDMITX1_AUDFMT_LPCM, 2);

	/* Set the Audio Info-Frame for 2 Channel */
    AudioInfoframePtr = XV_HdmiTxSs1_GetAudioInfoframe(&HdmiTxSs);
    /* Refer to CEA-861-D for Audio InfoFrame Channel Allocation
     * - - - - - - FR FL
     */
    AudioInfoframePtr->ChannelAllocation = 0x0;

	/* Enable audio generator */
	XhdmiAudGen_Start(&AudioGen, TRUE);

	/* Select ACR from ACR Ctrl */
	XhdmiACRCtrl_Sel(&AudioGen, ACR_SEL_GEN);

	/* Enable 2-channel audio */
	XhdmiAudGen_SetEnabChannels(&AudioGen, 2);
	XhdmiAudGen_SetPattern(&AudioGen, 1, XAUD_PAT_PING);
	XhdmiAudGen_SetPattern(&AudioGen, 2, XAUD_PAT_PING);
	xil_printf("Audio Sampling Frequency: ");
    switch (Input) {
		case 1 :
			xil_printf("32kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_32K);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_32K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_32K;
			xil_printf("Enter Selection -> ");
			break;
		case 2 :
			xil_printf("44.1kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_44K1);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_44_1K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_44_1K;
			xil_printf("Enter Selection -> ");
			break;
		case 3 :
			xil_printf("48kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_48K);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_48K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_48K;
			xil_printf("Enter Selection -> ");
			break;
		case 4 :
			xil_printf("96kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_96K);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_96K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_96K;
			xil_printf("Enter Selection -> ");
			break;
		case 5 :
			xil_printf("176.4kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_176K4);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_176_4K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_176_4K;
			xil_printf("Enter Selection -> ");
			break;
		case 6 :
			xil_printf("192kHz\r\n");
			XhdmiAudGen_SetSampleRate(&AudioGen,
				   XV_HdmiTxSs1_GetTmdsClockFreqHz(&HdmiTxSs),
				   XAUD_SRATE_192K);
			/* Set to use the Internal ACR module of the HDMI TX */
			XV_Tx_SetUseInternalACR(&xhdmi_example_tx_controller);
			/* Set the Audio Sampling Frequency to 48kHz */
			XV_Tx_SetAudSamplingFreq(&xhdmi_example_tx_controller,
					XHDMIC_SAMPLING_FREQ_192K);
			/* Un-Mute the Audio */
			XV_HdmiTxSs1_AudioMute(&HdmiTxSs, FALSE);
			/* Set the Audio Sampling Frequency */
			AudioInfoframePtr->SampleFrequency =
				XHDMIC_SAMPLING_FREQUENCY_192K;
			xil_printf("Enter Selection -> ");
			break;
			/* Exit */
		case 99 :
			xil_printf("Returning to audio menu.\r\n");
			Menu = XHDMI_AUDIO_MENU;
			break;

		default :
			xil_printf("Unknown option\r\n");
			XHdmi_DisplayAudioSampFreqMenu();
			break;
    }

    return Menu;
}
#endif
#endif


#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
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
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) || \
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    xil_printf(" 1 - Enable detailed logging\r\n");
    xil_printf(" 2 - Disable detailed logging\r\n");
    xil_printf(" 3 - Display log\r\n");
#if (HDCP_DEBUG_MENU_EN == 1)
    xil_printf(" 4 - Display HDCP Debug menu\r\n");
#endif
#endif
    xil_printf("99 - Exit\r\n");
    xil_printf("Enter Selection -> ");
}
#endif

#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
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
    /* Variables */
    XHdmi_MenuType  Menu;

    /* Default */
    Menu = XHDMI_HDCP_MAIN_MENU;

    /* Insert carriage return */
    xil_printf("\r\n");

    switch (Input) {

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) || \
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    /* 1 - Enable detailed logging */
	case 1 :
#endif
	    xil_printf("Enable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    XV_HdmiRxSs1_HdcpSetInfoDetail(&HdmiRxSs, TRUE);
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    XV_HdmiTxSs1_HdcpSetInfoDetail(&HdmiTxSs, TRUE);
#endif
	    break;

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) ||\
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    /* 2 - Disable detailed logging */
	case 2 :
#endif
	    xil_printf("Disable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    XV_HdmiRxSs1_HdcpSetInfoDetail(&HdmiRxSs, FALSE);
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    XV_HdmiTxSs1_HdcpSetInfoDetail(&HdmiTxSs, FALSE);
#endif
	    break;

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) ||\
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    /* 3 - Display log */
	case 3 :
#endif
	    xil_printf("Display log.\r\n");
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    HdmiRxSs.EnableHDCPLogging = (TRUE);
	    XV_HdmiRxSs1_HdcpInfo(&HdmiRxSs);
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    HdmiTxSs.EnableHDCPLogging = (TRUE);
	    XV_HdmiTxSs1_HdcpInfo(&HdmiTxSs);
#endif
	    break;

#if (HDCP_DEBUG_MENU_EN == 1)
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) ||\
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    /* 4 - HDCP Debug Menu */
	case 4 :
#endif
	    xil_printf("Display HDCP Debug menu.\r\n");
	    XHdmi_DisplayHdcpDebugMenu();
	    Menu = XHDMI_HDCP_DEBUG_MENU;
	    break;
#endif

	    /* Exit */
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
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
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
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) && \
		defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    xil_printf(" 1 - Set upstream capability to none\r\n");
    xil_printf(" 2 - Set upstream capability to both\r\n");
    xil_printf(" 3 - Set downstream capability to none\r\n");
    xil_printf(" 4 - Set downstream capability to 1.4\r\n");
    xil_printf(" 5 - Set downstream capability to 2.2\r\n");
    xil_printf(" 6 - Set downstream capability to both\r\n");
    /*xil_printf(" 7 - Toggle downstream content blocking \r\n");*/
    xil_printf(" 7 - Force HDCP 2.2 auth & encryption\r\n");
    xil_printf(" 8 - Disable HDCP 2.2 encryption\r\n");
    xil_printf(" 9 - Enable HDCP 2.2 encryption\r\n");
    xil_printf(" 10 - Force HDCP 1.4 auth & encryption\r\n");
    xil_printf(" 11 - Disable HDCP 1.4 encryption\r\n");
    xil_printf(" 12 - Display log\r\n");
    xil_printf(" 13 - Display HDCP 2.2 cipher status\r\n");
	xil_printf(" 14 - Set user HDCP preference to NONE\r\n");
	xil_printf(" 15 - Set upstream capability to HDCP22\r\n");
	xil_printf(" 16 - Set upstream capability to HDCP14\r\n");
	xil_printf(" 17 - Set upstream capability to none\r\n");
#elif defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    xil_printf(" 1 - Set upstream capability to none\r\n");
    xil_printf(" 2 - Set upstream capability to both\r\n");
#elif defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
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
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
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
    /* Variables */
    XHdmi_MenuType  Menu;

    /* Default */
    Menu = XHDMI_HDCP_DEBUG_MENU;

    /* Insert carriage return */
    xil_printf("\r\n");

    switch (Input) {
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* 1 - Set upstream capability to none */
	case 1:
	    xil_printf("Set upstream capability to none.\r\n");
	    XV_Rx_Hdcp_SetCapability(&xhdmi_example_rx_controller,
			    XV_HDMIRXSS1_HDCP_NONE);
	    break;

	    /* 2 - Set upstream capability to both */
	case 2:
	    xil_printf("Set upstream capability to both.\r\n");
	    XV_Rx_Hdcp_SetCapability(&xhdmi_example_rx_controller,
			    XV_HDMIRXSS1_HDCP_BOTH);
	    break;
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* 3 - Set downstream capability to none */
	case 3:
#else
	    /* 1 - Set downstream capability to none */
	case 1:
#endif
	    xil_printf("Set downstream capability to none.\r\n");
	    XV_Tx_Hdcp_SetCapability(&xhdmi_example_tx_controller,
			    XV_HDMITXSS1_HDCP_NONE);
	    break;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* 4 - Set downstream capability to 1.4 */
	case 4:
#else
	    /* 2 - Set downstream capability to 1.4 */
	case 2:
#endif
	    xil_printf("Set downstream capability to 1.4\r\n");
	    XV_Tx_Hdcp_SetCapability(&xhdmi_example_tx_controller,
			    XV_HDMITXSS1_HDCP_14);
	    break;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* 4 - Set downstream capability to 2.2 */
	case 5:
#else
	    /* 3 - Set downstream capability to 2.2 */
	case 3:
#endif
	    xil_printf("Set downstream capability to 2.2\r\n");
	    XV_Tx_Hdcp_SetCapability(&xhdmi_example_tx_controller,
			    XV_HDMITXSS1_HDCP_22);
	    break;

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    /* 6 - Set downstream capability to both */
	case 6:
#else
	    /* 4 - Set downstream capability to both */
	case 4:
#endif
	    xil_printf("Set downstream capability to both\r\n");
	    XV_Tx_Hdcp_SetCapability(&xhdmi_example_tx_controller,
			    XV_HDMITXSS1_HDCP_BOTH);
	    break;

	case 7:
	    xil_printf("Forcing HDCP 2.2 authentication\r\n");
	    /* Set desired downstream capability */
	    XV_HdmiTxSs1_HdcpSetCapability(
			xhdmi_example_tx_controller.HdmiTxSs,
				XV_HDMITXSS1_HDCP_22);

	    /* Enable and authenticate */
	    XHdcp22Tx_Enable(
			xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Authenticate(
			xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_EnableEncryption(
			xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    break;

	case 8:
	    xil_printf("Disabling HDCP 2.2 cipher encryption \r\n");
	    XHdcp22Tx_DisableEncryption(
			xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    break;

	case 9:
	    xil_printf("Enabling HDCP 2.2 cipher encryption \r\n");
	    XHdcp22Tx_EnableEncryption(
			xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    XHdcp22Tx_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr);
	    break;

	case 10:
	    xil_printf("Forcing HDCP 1.4 authentication \r\n");
	    /* Set desired downstream capability */
	    XV_HdmiTxSs1_HdcpSetCapability(
			xhdmi_example_tx_controller.HdmiTxSs,
			XV_HDMITXSS1_HDCP_14);

	    /* Set physical state */
	    XHdcp1x_SetPhysicalState(
		xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr, TRUE);
	    /* This is needed to ensure that the previous command is executed.*/
	    XHdcp1x_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    /* Enable and authenticate */
	    XHdcp1x_Enable(
		xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    XHdcp1x_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    XHdcp1x_Authenticate(
		xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    XHdcp1x_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    XHdcp1x_EnableEncryption(
		xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr, 0x1);
	    XHdcp1x_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    break;

	case 11:
	    xil_printf("Disabling HDCP 1.4 cipher \r\n");
	    XHdcp1x_DisableEncryption(
		xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr, 0x1);
	    XHdcp1x_Poll(xhdmi_example_tx_controller.HdmiTxSs->Hdcp14Ptr);
	    break;

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES) ||\
    defined (XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    /* 3 - Display log */
	case 12:
#endif
	    xil_printf("Display log.\r\n");
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	    HdmiRxSs.EnableHDCPLogging = (TRUE);
	    XV_HdmiRxSs1_HdcpInfo(&HdmiRxSs);
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	    HdmiTxSs.EnableHDCPLogging = (TRUE);
	    XV_HdmiTxSs1_HdcpInfo(&HdmiTxSs);
#endif
	    break;
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	case 13:
	    /* HDCP 2.2 Cipher status */
	    xil_printf(" Offset |     Register      |"
		" MSB(31).................LSB(0) |       Value    \r\n");
	    u32 Hdcp22_TxCipherBaseAddr = xhdmi_example_tx_controller.HdmiTxSs->Hdcp22Ptr->Cipher.Config.BaseAddress;

	    xil_printf("   0    | Version ID        | .............................. | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr,XHDCP22_CIPHER_VER_ID_OFFSET));
	    xil_printf("  0x4   | Version Offset    | .............................. | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr,XHDCP22_CIPHER_VER_VERSION_OFFSET));
	    xil_printf("\r\n");
	    xil_printf("  0x40  | Base Ctrl Reg     | ............(5)(4)(3)(2)(1)(0) | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr, XHDCP22_CIPHER_REG_CTRL_OFFSET));
	    xil_printf("                                   Noise --^  ^  ^  ^  ^  ^ \r\n");
	    xil_printf("                                   Blank -----|  |  |  |  | \r\n");
	    xil_printf("                                 Encrypt --------|  |  |  | \r\n");
	    xil_printf("                                    Mode -----------|  |  | \r\n");
	    xil_printf("                                      IE --------------|  | \r\n");
	    xil_printf("                                     Run -----------------| \r\n");
	    xil_printf("  0x44  | Base Ctrl Set Reg | ............(5)(4)(3)(2)(1)(0) | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr, XHDCP22_CIPHER_REG_CTRL_SET_OFFSET));
	    xil_printf("  0x48  | Base Ctrl Clr Reg | ............(5)(4)(3)(2)(1)(0) | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr, XHDCP22_CIPHER_REG_CTRL_CLR_OFFSET));
	    xil_printf("\r\n");
	    xil_printf("  0x4C  | Base Status Reg   | .....................(2)(1)(0) | %x\r\n",
		    XHdcp22Cipher_ReadReg(Hdcp22_TxCipherBaseAddr, XHDCP22_CIPHER_REG_STA_OFFSET));
	    xil_printf("                               Encrypted -----------^  ^  ^ \r\n");
	    xil_printf("                                   Event --------------|  | \r\n");
	    xil_printf("                                     Irq -----------------| \r\n");
	    xil_printf("\r\n");
	    break;
#endif
	case 14:
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		XV_HdmiRxSs1_SetUserHdcpProtocol(&HdmiRxSs, XV_HDMIRXSS1_HDCP_NOUSERPREF);
#endif
		break;
	case 15:
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		XV_HdmiRxSs1_SetUserHdcpProtocol(&HdmiRxSs, XV_HDMIRXSS1_HDCP_22);
#endif
		break;
	case 16:
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		XV_HdmiRxSs1_SetUserHdcpProtocol(&HdmiRxSs,	XV_HDMIRXSS1_HDCP_14);
#endif
		break;
	case 17:
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
		XV_HdmiRxSs1_SetUserHdcpProtocol(&HdmiRxSs, XV_HDMIRXSS1_HDCP_NONE);
#endif
		break;
	    /* Exit */
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
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
    xil_printf("Force TX to perform :\r\n");
    xil_printf("  1  - TX TMDS.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate == 6)
       xil_printf("  2  - TX FRL 4 Lanes 12G.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate >= 5)
       xil_printf("  3  - TX FRL 4 Lanes 10G.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate >= 4)
       xil_printf("  4  - TX FRL 4 Lanes 8G.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate >= 3)
       xil_printf("  5  - TX FRL 4 Lanes 6G.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate >= 2)
       xil_printf("  6  - TX FRL 3 Lanes 6G.\r\n");
    if(HdmiTxSs.Config.MaxFrlRate >= 1)
       xil_printf("  7  - TX FRL 3 Lanes 3G.\r\n");
    xil_printf("\r\n");
#endif
    xil_printf("  10 - RX Request Rate Drop.\r\n");
    xil_printf("  11 - RX sets FltNoTimeout.\r\n");
    xil_printf("  12 - RX clears FltNoTimeout.\r\n");
    xil_printf("  13 - RX requests for FRL LT (during LTS:P).\r\n");
    xil_printf("  14 - RX PHY Reset.\r\n");
    xil_printf("\r\n");
    xil_printf("  20 - Register Dump (Debug). \r\n");
    xil_printf("  21 - SCDC Register Dump (Debug). \r\n");
    xil_printf("  99 - Exit\r\n");
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
    /* Variables */
    XHdmi_MenuType  Menu;
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
    XV_HdmiRx1_FrlLtp Temp;
#endif
    u32 RegOffset;
    /* Default */
    Menu = XHDMI_DEBUG_MAIN_MENU;

    switch (Input) {
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	case 1 :
	    XV_HdmiTxSs1_StartTmdsMode(&HdmiTxSs);
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 2 :
	    if(HdmiTxSs.Config.MaxFrlRate == 6) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_4X12GBITSPS);
		}else {
			xil_printf("FRL Rate : 12Gbps @ 4 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 3 :
	    if(HdmiTxSs.Config.MaxFrlRate >= 5) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_4X10GBITSPS);
		}else {
			xil_printf("FRL Rate : 10Gbps @ 4 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 4 :
	    if(HdmiTxSs.Config.MaxFrlRate >= 4) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_4X8GBITSPS);
		}else {
			xil_printf("FRL Rate : 8Gbps @ 4 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 5 :
	    if(HdmiTxSs.Config.MaxFrlRate >= 3) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_4X6GBITSPS);
		}else {
			xil_printf("FRL Rate : 6Gbps @ 4 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 6 :
	    if(HdmiTxSs.Config.MaxFrlRate >= 2) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_3X6GBITSPS);
		}else {
			xil_printf("FRL Rate : 6Gbps @ 3 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 7 :
	    if(HdmiTxSs.Config.MaxFrlRate >= 1) {
	       XV_HdmiTxSs1_StartFrlTraining(&HdmiTxSs,
			      XHDMIC_MAXFRLRATE_3X3GBITSPS);
		}else {
			xil_printf("FRL Rate : 3Gbps @ 3 Lanes is not supported.\r\n");
		}
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	case 10 :
	    Temp.Byte[0] = 0x5;
	    Temp.Byte[1] = 0x6;
	    Temp.Byte[2] = 0x7;
	    Temp.Byte[3] = 0x8;

	    XV_HdmiRxSs1_FrlLinkRetrain(&HdmiRxSs, 1, Temp);
	    /* Display the prompt for the next input */
	    xil_printf("Enter Selection -> ");
	    break;

	case 11:
	    XV_HdmiRxSs1_SetFrlFltNoTimeout(&HdmiRxSs);
	    break;

	case 12:
	    XV_HdmiRxSs1_ClearFrlFltNoTimeout(&HdmiRxSs);
	    break;

	case 13:
	    XV_HdmiRx1_RestartFrlLt(HdmiRxSs.HdmiRx1Ptr);
	    break;

	case 14:
	    XHdmiphy1_ResetGtTxRx(&Hdmiphy1,
			    0,
			    XHDMIPHY1_CHANNEL_ID_CHA,
			    XHDMIPHY1_DIR_RX,
			    FALSE);
	    break;
#endif
	case 20:
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	    XV_HdmiTxSs1_RegisterDebug(&HdmiTxSs);
#endif
#if defined(XPAR_XV_HDMIRXSS1_NUM_INSTANCES)
	    XV_HdmiRxSs1_RegisterDebug(&HdmiRxSs);
#endif
#if defined (XPAR_XV_TPG_NUM_INSTANCES)
	    xil_printf("-------------------------------------\r\n");
	    xil_printf("         V TPG Register Dump\r\n");
	    xil_printf("-------------------------------------\r\n");
	    if (xhdmi_example_tx_controller.XV_Tx_StreamState ==
			XV_TX_HDMITXSS_STREAM_UP) {
	    for (RegOffset = 0; RegOffset <= XV_TPG_CTRL_ADDR_FIELDID_DATA; ) {
	    xil_printf("0x%04x      0x%08x\r\n",RegOffset,
		XV_HdmiTx1_ReadReg(Tpg.Config.BaseAddress, RegOffset));
	    RegOffset += 4;
	    }
	    }
#endif
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
		      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	    xil_printf("-------------------------------------\r\n");
	    xil_printf("  V Frame Buffer Read Register Dump\r\n");
	    xil_printf("-------------------------------------\r\n");
	    for (RegOffset = 0;
		RegOffset <= XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA; ) {
		xil_printf("0x%04x      0x%08x\r\n",RegOffset,
			XV_HdmiTx1_ReadReg(
			FrameBufRd.FrmbufRd.Config.BaseAddress, RegOffset));
		RegOffset += 4;
	    }
#endif
	    break;
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	case 21:
	    XV_HdmiRxSs1_DdcRegDump(&HdmiRxSs);
	    break;
#endif
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
#if defined (XPS_BOARD_ZCU106)
    xil_printf("  10- 765 mV\r\n");
    xil_printf("  11- 822 mV\r\n");
    xil_printf("  12- 873 mV\r\n");
    xil_printf("  13- 921 mV (default)\r\n");
    xil_printf("  14- 963 mV\r\n");
    xil_printf("  15- 1000 mV\r\n");
#elif defined (XPS_BOARD_VCU118)
    xil_printf("  10- 840 mV (0x12)\r\n");
    xil_printf("  11- 870 mV (0x14)\r\n");
    xil_printf("  12- 920 mV (0x16)\r\n");
    xil_printf("  13- 950 mV (0x18)\r\n");
    xil_printf("  14- Increment by 2\r\n");
    xil_printf("  15- Decrement by 2\r\n");
#endif
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
    xil_printf(" FORCE TXRX LINERATE\r\n");
    xil_printf("  34- 3  Gbps\r\n");
    xil_printf("  35- 6  Gbps\r\n");
    xil_printf("  36- 8  Gbps\r\n");
    xil_printf("  37- 10 Gbps\r\n");
    xil_printf("  38- 12 Gbps\r\n");
    xil_printf("---------------------------\r\n");
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

#if defined (XPS_BOARD_ZCU106)
	case 10:
	    xil_printf("TX Diff Swing 765 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xA);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xA);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xA);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xA);
		break;

	case 11 :
	    xil_printf("TX Diff Swing 822 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xB);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xB);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xB);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xB);
		break;

	case 12 :
	    xil_printf("TX Diff Swing 873 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xC);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xC);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xC);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xC);
		break;

	case 13:
	    xil_printf("TX Diff Swing 921 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xD);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xD);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xD);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xD);
		break;

	case 14:
	    xil_printf("TX Diff Swing 963 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xE);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xE);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xE);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xE);
		break;

	case 15:
	    xil_printf("TX Diff Swing 1000 mV\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0xF);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0xF);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0xF);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0xF);
		break;
#elif defined (XPS_BOARD_VCU118)
	case 10:
	    xil_printf("TX Diff Swing 840 mV (0x12)\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0x12);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0x12);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0x12);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0x12);
		break;

	case 11 :
	    xil_printf("TX Diff Swing 870 mV (0x14)\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0x14);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0x14);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0x14);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0x14);
		break;

	case 12 :
	    xil_printf("TX Diff Swing 920 mV (0x16)\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0x16);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0x16);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0x16);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0x16);
		break;

	case 13:
	    xil_printf("TX Diff Swing 950 mV (0x18)\r\n");
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, 0x18);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, 0x18);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, 0x18);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, 0x18);
		break;

	case 14:
		RegVal = (XHdmiphy1_ReadReg(Hdmiphy1.Config.BaseAddr,
					XHDMIPHY1_TX_DRIVER_CH12_REG) & 0x1F);
		RegVal = RegVal+1;
		xil_printf("TX Diff Swing Increment Enc: 0x%02x RegVal: 0x%02x \r\n",
				    RegVal << 1, RegVal);
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
					XHDMIPHY1_TX_DRIVER_CH12_REG) & 0x1F);
		RegVal = RegVal-1;
		xil_printf("TX Diff Swing Decrement Enc: 0x%02x RegVal: 0x%02x \r\n",
				    RegVal << 1, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH1, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH2, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH3, RegVal);
		XHdmiphy1_SetTxVoltageSwing(&Hdmiphy1, 0,
				XHDMIPHY1_CHANNEL_ID_CH4, RegVal);
		break;
#endif

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

#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*          specified data to.
* @param MsgPtr points to the data to be sent.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
*         transmitting the data.
*
* @return   The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned ONSEMI_NB7NQ621M_I2cSend(void *IicPtr,
						u16 SlaveAddr,
						u8 *MsgPtr,
						unsigned ByteCount,
						u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
    u32 Status;

    /* Set operation to 7-bit mode */
    XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
    XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);

    /* Set Repeated Start option */
    if (Option == I2C_REPEATED_START) {
	XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    } else {
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    }

    Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount, SlaveAddr);

    /*
     * Wait until bus is idle to start another transfer.
     */
    if (!(Iic_Ptr->IsRepeatedStart)) {
	while (XIicPs_BusIsBusy(Iic_Ptr));
    }

    if (Status == XST_SUCCESS) {
	return ByteCount;
    } else {
	return 0;
    }
#else
    XIic *Iic_Ptr = IicPtr;
	/* This delay prevents IIC access from hanging */
	usleep(350);
    return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
		    ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to TI TMDS1204
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*          specified data to.
* @param MsgPtr points to the data to be sent.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
*         transmitting the data.
*
* @return   The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned TI_TMDS1204_I2cSend(void *IicPtr,
						u16 SlaveAddr,
						u8 *MsgPtr,
						unsigned ByteCount,
						u8 Option)
{
#if (defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190))
    XIicPs *Iic_Ptr = IicPtr;
    u32 Status;

    /* Set operation to 7-bit mode */
    XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
    XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);

    /* Set Repeated Start option */
    if (Option == I2C_REPEATED_START) {
	XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    } else {
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    }

    Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount, SlaveAddr);

    /*
     * Wait until bus is idle to start another transfer.
     */
    if (!(Iic_Ptr->IsRepeatedStart)) {
	while (XIicPs_BusIsBusy(Iic_Ptr));
    }

    if (Status == XST_SUCCESS) {
	return ByteCount;
    } else {
	return 0;
    }
#else
    XIic *Iic_Ptr = IicPtr;
	/* This delay prevents IIC access from hanging */
	usleep(350);
    return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
		    ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to ONSEMI_NB7NQ621M
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*          specified data to.
* @param BufPtr points to the memory to write the data.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
*         transmitting the data.
*
* @return   The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned ONSEMI_NB7NQ621M_I2cRecv(void *IicPtr,
						u16 SlaveAddr,
						u8 *BufPtr,
						unsigned ByteCount,
						u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
    u32 Status;

    XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
    XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
    if (Option == I2C_REPEATED_START) {
	XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    } else {
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    }

    Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount, SlaveAddr);

    /*
     * Wait until bus is idle to start another transfer.
     */
    if (!(Iic_Ptr->IsRepeatedStart)) {
	while (XIicPs_BusIsBusy(Iic_Ptr));
    }

    if (Status == XST_SUCCESS) {
	return ByteCount;
    } else {
	return 0;
    }
#else
    XIic *Iic_Ptr = IicPtr;
    return XIic_Recv(Iic_Ptr->BaseAddress, SlaveAddr, BufPtr,
		    ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to TI_TMDS1204
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*          specified data to.
* @param BufPtr points to the memory to write the data.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
*         transmitting the data.
*
* @return   The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned TI_TMDS1204_I2cRecv(void *IicPtr,
						u16 SlaveAddr,
						u8 *BufPtr,
						unsigned ByteCount,
						u8 Option)
{
#if (defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190))
    XIicPs *Iic_Ptr = IicPtr;
    u32 Status;

    XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
    XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
    if (Option == I2C_REPEATED_START) {
	XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    } else {
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
    }

    Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount, SlaveAddr);

    /*
     * Wait until bus is idle to start another transfer.
     */
    if (!(Iic_Ptr->IsRepeatedStart)) {
	while (XIicPs_BusIsBusy(Iic_Ptr));
    }

    if (Status == XST_SUCCESS) {
	return ByteCount;
    } else {
	return 0;
    }
#else
    XIic *Iic_Ptr = IicPtr;
    return XIic_Recv(Iic_Ptr->BaseAddress, SlaveAddr, BufPtr,
		    ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int ONSEMI_NB7NQ621M_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value)
{
    u32 ByteCount = 0;
    u8 Buffer[2];

    Buffer[0] = RegisterAddress;
    Buffer[1] = Value;
    ByteCount = ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
				2, I2C_STOP);
    if (ByteCount != 2) {
	return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI TMDS1204
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static int TI_TMDS1204_SetRegister(void *IicPtr, u8 I2CSlaveAddress,
		u8 RegisterAddress, u8 Value)
{
    u32 ByteCount = 0;
    u8 Buffer[2];

    Buffer[0] = RegisterAddress;
    Buffer[1] = Value;
    ByteCount = TI_TMDS1204_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
				2, I2C_STOP);
    if (ByteCount != 2) {
	return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the ONSEMI NB7NQ621M
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static u8 ONSEMI_NB7NQ621M_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
	    u8 RegisterAddress)
{
    u8 Buffer[2];

    Buffer[0] = RegisterAddress;
    ONSEMI_NB7NQ621M_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
				1, I2C_REPEATED_START);
    ONSEMI_NB7NQ621M_I2cRecv(IicPtr, I2CSlaveAddress,
		    (u8*)Buffer, 1, I2C_STOP);
    return Buffer[0];
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI TMDS1204
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
static u8 TI_TMDS1204_GetRegister(void *IicPtr, u8 I2CSlaveAddress,
	    u8 RegisterAddress)
{
    u8 Buffer[2];

    Buffer[0] = RegisterAddress;
    TI_TMDS1204_I2cSend(IicPtr, I2CSlaveAddress, (u8*)Buffer,
				1, I2C_REPEATED_START);
    TI_TMDS1204_I2cRecv(IicPtr, I2CSlaveAddress,
		    (u8*)Buffer, 1, I2C_STOP);
    return Buffer[0];
}


#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
                      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)

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
static void XHdmi_Display8kQuadMenu(void) {
    xil_printf("\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("---   4K Quad Selection ---\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("  1 - Quad 1\r\n");
    xil_printf("  2 - Quad 2\r\n");
    xil_printf("  3 - Quad 3\r\n");
    xil_printf("  4 - Quad 4\r\n");
    xil_printf("  99- Exit\r\n");
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
static XHdmi_MenuType XHdmi_8kQuadMenu(XHdmi_Menu *InstancePtr, u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;

    /* Default */
    Menu = XHDMI_8KQUAD_MENU;
    u32 stride = 0;
    stride = XV_frmbufwr_Get_HwReg_stride (&FrameBufWr.FrmbufWr);

    /*
     * Quad selection is done by adjusting the buffer address
     * of FrameBuffer Read
     *
     * +---------+---------+   ^
       |         |         |   |
       |  Quad 1 |  Quad 2 |   |
       |     +-------+     |   +
       |     |   |   |     |  4320
       +-------Quad 5+-----+
       |     |   |   |     |   +
       |     +-------+     |   |
       | Quad 3  |  Quad 4 |   |
       |         |         |   |
       +---------+---------+   v

       <----+ 7680   +----->
    *
    */

    switch (Input) {
		case 1 :
			offset = 0;
			break;
		case 2 :
			offset = 3840 * 3;
			break;
		case 3 :
			offset = stride * 2160;
			break;
		case 4 :
			offset = (stride *2160)+ (3840 * 3);
			break;
		case 5 :
			offset = (stride *1080)+ (1920 * 3);
			break;

		case 99 :
			xil_printf("Returning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default:
			XHdmi_Display8kQuadMenu();
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
static void XHdmi_DisplayOnSemiDebugMenu(void) {
    xil_printf("\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf("---   ONSEMI/TI DEBUG MENU   ---\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" TX \r\n");
    xil_printf("  1 - TX I2C READ\r\n");
    xil_printf("  2 - TX I2C WRITE\r\n");
    xil_printf("---------------------------\r\n");
    xil_printf(" RX \r\n");
    xil_printf("  3 - RX I2C READ\r\n");
    xil_printf("  4 - RX I2C WRITE\r\n");
    xil_printf(" INFO \r\n");
    xil_printf("  5 - TXRX Register Dump\r\n");
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
static XHdmi_MenuType XHdmi_OnSemiDebugMenu(XHdmi_Menu *InstancePtr, u8 Input) {
    /* Variables */
    XHdmi_MenuType  Menu;
    u8 Addr, Data, i;

    /* Default */
    Menu = XHDMI_ONSEMI_DEBUG_MENU;
    Vfmc_I2cMuxSelect(&Vfmc[0]);

    switch (Input) {
    case 1 :
	xil_printf("Enter TX Read Addr: 0x");
	Addr = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter Size: ");
	Data = XHdmi_OneSemiMenuProcess(0);
	xil_printf("\r\n");

	for (i=0; i < Data; i++) {
		if (Vfmc[0].isTxTi == 1) {
			xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr+i),
					TI_TMDS1204_GetRegister(Vfmc[0].IicPtr,
							0x5E,
							(Addr+i)));
		} else {
			xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr+i),
					ONSEMI_NB7NQ621M_GetRegister(Vfmc[0].IicPtr,
							0x5B,
							(Addr+i)));
		}
	}
	break;

    case 2 :
	xil_printf("Enter TX Write Addr: 0x");
	Addr = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter TX Write Data: 0x");
	Data = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");

	if (Vfmc[0].isTxTi == 1) {
		TI_TMDS1204_SetRegister(Vfmc[0].IicPtr,
			    0x5E, Addr, Data);
	} else {
		ONSEMI_NB7NQ621M_SetRegister(Vfmc[0].IicPtr,
		    0x5B, Addr, Data);
	}

	break;

    case 3 :
	xil_printf("Enter RX Read Addr: 0x");
	Addr = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter Size: ");
	Data = XHdmi_OneSemiMenuProcess(0);
	xil_printf("\r\n");

	for (i=0; i < Data; i++) {
		if (Vfmc[0].isRxTi == 1) {
			xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr+i),
			    TI_TMDS1204_GetRegister(Vfmc[0].IicPtr,
					0x5B,
					(Addr+i)));
		} else {
			xil_printf("Addr 0x%02x: 0x%02x\r\n", (Addr+i),
					ONSEMI_NB7NQ621M_GetRegister(Vfmc[0].IicPtr,
							0x5C,
							(Addr+i)));
		}
	}
	break;

    case 4 :
	xil_printf("Enter RX Write Addr: 0x");
	Addr = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");
	xil_printf("Enter RX Write Data: 0x");
	Data = XHdmi_OneSemiMenuProcess(1);
	xil_printf("\r\n");

	if (Vfmc[0].isRxTi == 1) {
		TI_TMDS1204_SetRegister(Vfmc[0].IicPtr,
		    0x5B, Addr, Data);
	} else {
		ONSEMI_NB7NQ621M_SetRegister(Vfmc[0].IicPtr,
		    0x5C, Addr, Data);
	}
	break;

    case 5 :
	xil_printf("-----TX Reg dump -------\r\n");
#if defined (XPS_BOARD_VEK280) || \
	defined (XPS_BOARD_VEK385)
	if (Vfmc[0].isTxTi == 1) {
		TI_TMDS1204_RegisterDump(&Iic,0x5E);
	} else {
#endif
		ONSEMI_NB7NQ621M_RegisterDump(&Iic,0x5B);
#if defined (XPS_BOARD_VEK280) || \
	defined (XPS_BOARD_VEK385)
	}
#endif
	xil_printf("-----RX Reg dump -------\r\n");
#if defined (XPS_BOARD_VEK280) || \
	defined (XPS_BOARD_VEK385)
	if (Vfmc[0].isRxTi == 1) {
		TI_TMDS1204_RegisterDump(&Iic,0x5B);
	} else {
#endif
		ONSEMI_NB7NQ621M_RegisterDump(&Iic,0x5C);
#if defined (XPS_BOARD_VEK280) || \
	defined (XPS_BOARD_VEK385)
	}
#endif
	break;

    case 99 :
	xil_printf("Returning to main menu.\r\n");
	Menu = XHDMI_MAIN_MENU;
	break;

    default :
	    xil_printf("Unknown option\r\n");
	    XHdmi_DisplayOnSemiDebugMenu();
	    break;
    }

    return Menu;
}

extern XHdmi_Menu         HdmiMenu;
u8 XHdmi_OneSemiMenuProcess(u8 Hex) {
    u8 Data = 0;
    u8 Value = 0;
    u8 IsValid = 0;
    u8 Mult = Hex ? 16 : 10;
    u8 n_char = 0;

    do {
	IsValid = 0;

    /* Check if the uart has any data */
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
    if (XV_UART_ISRECEIVEDATA(HdmiMenu.UartBaseAddress)) {
#else
    if (XV_UART_ISRECEIVEDATA(HdmiMenu.UartBaseAddress)) {
#endif
	/* Read data from uart */
	Data = XV_UART_RECV_BYTE(HdmiMenu.UartBaseAddress);



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
	XV_UART_SEND_BYTE(HdmiMenu.UartBaseAddress, Data);
	n_char++;

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
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr, u8 TxBusy) {
    u8 Data;

    /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
    if ((InstancePtr->WaitForColorbar) && (!TxBusy)) {
	InstancePtr->WaitForColorbar = (FALSE);
	xil_printf("Enter Selection -> ");
    }
#endif

    /* Check if the uart has any data */
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
    else if (XV_UART_ISRECEIVEDATA(InstancePtr->UartBaseAddress)) {
#else
    if (XV_UART_ISRECEIVEDATA(InstancePtr->UartBaseAddress)) {
#endif
	/* Read data from uart */
	Data = XV_UART_RECV_BYTE(InstancePtr->UartBaseAddress);

	/* Main menu */
	if (InstancePtr->CurrentMenu == XHDMI_MAIN_MENU) {
	    InstancePtr->CurrentMenu =
		XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
	    InstancePtr->Value = 0;
	}

	/* Sub menu */
	else {

	    /* Send response to user */
	    XV_UART_SEND_BYTE(InstancePtr->UartBaseAddress, Data);

	    /* Alpha numeric data */
	    if (isalpha(Data)) {
		xil_printf("Invalid input. Valid entry is only digits 0-9. "
				"Try again\r\n\r\n");
		xil_printf("Enter Selection -> ");
		InstancePtr->Value = 0;
	    }

	    /* Numeric data */
	    else if ((Data >= '0') && (Data <= '9')) {
		InstancePtr->Value = InstancePtr->Value * 10 + (Data-'0');
	    }

	    /* Backspace */
	    else if (Data == '\b' || Data == 127) {
		/* discard previous input */
		InstancePtr->Value = InstancePtr->Value / 10;
	    }

	    /* Execute */
	    else if ((Data == '\n') || (Data == '\r')) {
		xil_printf("\r\n");
		InstancePtr->CurrentMenu =
			XHdmi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
		InstancePtr->Value = 0;
	    }
	}
    }
}

