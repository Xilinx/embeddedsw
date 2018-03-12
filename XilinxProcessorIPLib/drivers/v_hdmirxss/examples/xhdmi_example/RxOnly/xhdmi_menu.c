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
* 1.11  mmo  29-12-2017 Added EDID Parsing Capability
*       EB   16-01-2018 Added Audio Channel Menu
*       EB   23-01-2018 Reset the counter tagged to the events logged whenever
*                               log is displayed
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_menu.h"
#include "xhdcp.h"
#include "xvidc_edid_ext.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XHdmi_MenuType XHdmi_MenuFuncType(XHdmi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input);
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

static void XHdmi_DisplayMainMenu(void);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
static void XHdmi_DisplayGtPllLayoutMenu(void);
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
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
extern u8 PLLBondedCheck (void);
#endif
extern void Info(void);
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
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	XHdmi_GtPllLayoutMenu,
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
};

extern XVphy Vphy;               /* VPhy structure */
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
extern XV_HdmiRxSs HdmiRxSs;       /* HDMI RX SS structure */
#endif
extern u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;                // TX busy flag. This flag is set while the TX is initialized
extern XHdcp_Repeater HdcpRepeater;

/*HDMI EDID*/
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
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

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
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("i - Info\r\n");
	xil_printf("       => Shows information about the HDMI RX stream, HDMI TX stream, \r\n");
	xil_printf("          GT transceivers and PLL settings.\r\n");
	xil_printf("p - Toggle HPD\r\n");
	xil_printf("       => Toggles the HPD of HDMI RX.\r\n");
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	xil_printf("l - GT PLL layout\r\n");
	xil_printf("       => Select GT transceiver PLL layout.\r\n");
#endif
	xil_printf("z - GT & HDMI TX/RX log\r\n");
	xil_printf("       => Shows log information for GT & HDMI TX/RX.\r\n");

#if defined(USE_HDCP)
	/* Show HDCP menu option when HDCP is ready */
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
		xil_printf("h - HDCP\r\n");
		xil_printf("       => Goto HDCP menu.\r\n");
#if(HDMI_DEBUG_TOOLS == 1)
		xil_printf("x - Debug Tools\r\n");
		xil_printf("       => Goto Debug menu.\r\n");
#endif
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
static XHdmi_MenuType XHdmi_MainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
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
				xil_printf("\r\nToggle HDMI RX HPD\r\n");
				XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
				XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
				XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
				XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
				XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
			}

			// No source
			else {
				xil_printf(ANSI_COLOR_YELLOW "No source device detected.\r\n"
							ANSI_COLOR_RESET);
			}
			Menu = XHDMI_MAIN_MENU;
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_LogDisplay(&HdmiRxSs);
#endif
			Menu = XHDMI_MAIN_MENU;
			break;



#if defined(USE_HDCP)
			// HDCP
		case ('h') :
		case ('H') :
			/* Enable HDCP menu option when HDCP is ready */
			if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
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

		default :
			XHdmi_DisplayMainMenu();
			Menu = XHDMI_MAIN_MENU;
			break;
	}

	return Menu;
}



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
void XHdmi_DisplayGtPllLayoutMenu(void) {
	xil_printf("\r\n");
	xil_printf("------------------------------\r\n");
	xil_printf("---   GT PLL LAYOUT MENU   ---\r\n");
	xil_printf("------------------------------\r\n");
	xil_printf("\r\n");

	xil_printf("In this menu the GT PLL clocking layout can be selected.\r\n");

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
static XHdmi_MenuType XHdmi_GtPllLayoutMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;
	XVphy_SysClkDataSelType TxSysPllSelect;
	XVphy_SysClkDataSelType RxSysPllSelect;
	u8 IsValid = FALSE;

	// Default
	Menu = XHDMI_GTPLLLAYOUT_MENU;

	switch (Input) {

			// RX => QPLL / TX => CPLL
		case 1 :
			xil_printf("Setting RX => QPLL\r\n\r\n");
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			IsValid = TRUE;
			break;

			// RX => CPLL / TX => QPLL
		case 2 :
			xil_printf("Setting RX => CPLL\r\n\r\n");
			TxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK;
			RxSysPllSelect = XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK;
			IsValid = TRUE;
			break;


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

		/* Update VPHY Clocking */
		XVphy_HdmiUpdateClockSelection(&Vphy, 0, TxSysPllSelect, RxSysPllSelect);

		// Is the reference design RX Only
		if ((Input == 1) || (Input == 2)) {
			xil_printf("Issue RX HPD\r\n");
			XVphy_MmcmPowerDown(&Vphy, 0, XVPHY_DIR_RX, FALSE);
			XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (FALSE));
			XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));
			XV_HdmiRxSs_ToggleHpd(&HdmiRxSs);
			XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));
		}

		// Return to main menu
		xil_printf("Return to main menu.\r\n");
		Menu = XHDMI_MAIN_MENU;
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
void XHdmi_DisplayHdcpMainMenu(void) {
	xil_printf("\r\n");
	xil_printf("--------------------------\r\n");
	xil_printf("---   HDCP Main Menu   ---\r\n");
	xil_printf("--------------------------\r\n");
	xil_printf(" 1 - Enable detailed logging\r\n");
	xil_printf(" 2 - Disable detailed logging\r\n");
	xil_printf(" 3 - Display log\r\n");
	xil_printf(" 4 - Display info\r\n");
#if (HDCP_DEBUG_MENU_EN == 1)
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
static XHdmi_MenuType XHdmi_HdcpMainMenu(XHdmi_Menu *InstancePtr, u8 Input) {
	// Variables
	XHdmi_MenuType 	Menu;

	// Default
	Menu = XHDMI_HDCP_MAIN_MENU;

	// Insert carriage return
	xil_printf("\r\n");

	switch (Input) {


			/* 1 - Enable detailed logging */
		case 1 :
			xil_printf("Enable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, TRUE);
#endif
			break;

			/* 2 - Disable detailed logging */
		case 2 :
			xil_printf("Disable detailed logging.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpSetInfoDetail(&HdmiRxSs, FALSE);
#endif
			break;

			/* 3 - Display log */
		case 3 :
			xil_printf("Display log.\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
			XV_HdmiRxSs_HdcpInfo(&HdmiRxSs);
#endif
			break;

			/* 4 - Display repeater info */
		case 4 :
			xil_printf("Display info.\r\n");
			XHdcp_DisplayInfo(&HdcpRepeater, TRUE);
			break;

#if (HDCP_DEBUG_MENU_EN == 1)
			/* 5 - HDCP Debug Menu */
		case 5 :
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
	xil_printf(" 1 - Set upstream capability to none\r\n");
	xil_printf(" 2 - Set upstream capability to both\r\n");
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

			// Exit
		case 99 :
			xil_printf("\r\nReturning to main menu.\r\n");
			Menu = XHDMI_MAIN_MENU;
			break;

		default :
			xil_printf("\r\nUnknown option\r\n");
			XHdmi_DisplayDebugMainMenu();
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
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr) {
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);


	// Check if the uart has any data
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
	if (!XUartLite_IsReceiveEmpty(InstancePtr->UartBaseAddress)) {

		// Read data from uart
		Data = XUartLite_RecvByte(InstancePtr->UartBaseAddress);
#else
	if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {
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
