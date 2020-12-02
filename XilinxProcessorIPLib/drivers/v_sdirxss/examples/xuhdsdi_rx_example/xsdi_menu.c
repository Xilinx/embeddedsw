/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsdi_menu.c
 *
 * This file contains the Xilinx Menu implementation as used
 * in the SDI example design. Please see xsdi_menu.h for more details.
 *
 * <pre>
 * MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  PG   05/09/2017 Initial version
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdi_menu.h"
#include "xgpio.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/

/* Pointer to the menu handling functions */
typedef XSdi_MenuType XSdi_MenuFuncType(XSdi_Menu *InstancePtr, u8 Input);

/************************** Function Prototypes ******************************/
static XSdi_MenuType XSdi_MainMenu(XSdi_Menu *InstancePtr, u8 Input);
static XSdi_MenuType XSdi_SubcoreMenu(XSdi_Menu *InstancePtr, u8 Input);

static void XSdi_DisplayMainMenu(void);

extern void Info(void);
extern void DebugInfo(void);

/************************* Variable Definitions *****************************/

/**
 * This table contains the function pointers for all possible states.
 * The order of elements must match the XSdi_MenuType enumerator definitions.
 */
static XSdi_MenuFuncType* const XSdi_MenuTable[XSDI_NUM_MENUS] =
{
	XSdi_MainMenu,
	XSdi_SubcoreMenu
};

extern XGpio Gpio_AxisFifo_resetn;

extern XV_SdiRxSs SdiRxSs;       /* SDI RX SS structure */
extern u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;                /* TX busy is set while the TX is initialized */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function takes care of the SDI menu initialization.
 *
 * @param	InstancePtr is a pointer to the XSdi_Menu instance.
 * @param	UartBaseAddress points to the base address of PS uart.
 *
 * @return	None
 *
 *
 ******************************************************************************/
void XSdi_MenuInitialize(XSdi_Menu *InstancePtr, u32 UartBaseAddress)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* copy configuration settings */
	InstancePtr->CurrentMenu = XSDI_MAIN_MENU;
	InstancePtr->UartBaseAddress = UartBaseAddress;
	InstancePtr->Value = 0;
	InstancePtr->WaitForColorbar = (FALSE);

	/* Show main menu */
	XSdi_DisplayMainMenu();
}

/*****************************************************************************/
/**
 *
 * This function resets the menu to the main menu.
 *
 * @param	InstancePtr is a pointer to the XSdi_Menu instance.
 *
 * @return	None
 *
 * @note	None
 *
 ******************************************************************************/
void XSdi_MenuReset(XSdi_Menu *InstancePtr)
{
	InstancePtr->CurrentMenu = XSDI_MAIN_MENU;
}

/*****************************************************************************/
/**
 *
 * This function displays the SDI main menu.
 *
 * @param	None
 *
 * @return	None
 *
 *
 ******************************************************************************/
void XSdi_DisplayMainMenu(void)
{
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("i - Info\n\r");
	xil_printf("       => Shows information about the SDI RX stream, SDI TX stream.\n\r");
	xil_printf("z - SDI TX & RX log\n\r");
	xil_printf("       => Shows log information for SDI TX & RX.\n\r");
	xil_printf("d - Debug Info\n\r");
	xil_printf("       => Registers Dump.\n\r");
	xil_printf("\n\r\n\r");
}

/*****************************************************************************/
/**
 *
 * This function implements the SDI main menu state.
 *
 * @param	input is the value used for the next menu state decoder.
 *
 * @return	The next menu state.
 *
 ******************************************************************************/
static XSdi_MenuType XSdi_MainMenu(XSdi_Menu *InstancePtr, u8 Input)
{
	/* Variables */
	XSdi_MenuType Menu;
	/* Default */
	Menu = XSDI_MAIN_MENU;

	switch (Input) {
		/* Info */
		case ('i') :
		case ('I') :
			Info();
			Menu = XSDI_MAIN_MENU;
			break;

			/* GT & SDI TX/RX log */
		case ('z') :
		case ('Z') :

			XV_SdiRxSs_LogDisplay(&SdiRxSs);
			Menu = XSDI_MAIN_MENU;
			break;

			/* Debug */
		case ('d') :
		case ('D') :
			DebugInfo();
			Menu = XSDI_MAIN_MENU;
			break;

		default :
			XSdi_DisplayMainMenu();
			Menu = XSDI_MAIN_MENU;
			break;
	}

	return Menu;
}

/*****************************************************************************/
/**
 *
 * This function implements the SDI subcores menu state.
 *
 * @param	input is the value used for the next menu state decoder.
 *
 * @return	The next menu state.
 *
 ******************************************************************************/
static XSdi_MenuType XSdi_SubcoreMenu(XSdi_Menu *InstancePtr, u8 Input)
{
	/* Variables */
	XSdi_MenuType Menu;

	/* Default */
	Menu = XSDI_SUBCORE_MENU;

	xil_printf("\r\n");

	switch (Input) {
		/* Enable RX Video Bridge */
		case 1:
			xil_printf("Enable RX Video Bridge\r\n");
			XV_SdiRx_VidBridgeEnable(SdiRxSs.SdiRxPtr);
			Menu = XSDI_MAIN_MENU;
			break;

			/* Enable RX AXIS Bridge */
		case 2 :
			xil_printf("Enable RX AXIS Bridge\r\n");
			XV_SdiRx_Axi4sBridgeEnable(SdiRxSs.SdiRxPtr);
			Menu = XSDI_MAIN_MENU;
			break;

		case 99 :
			Menu = XSDI_MAIN_MENU;
			break;
	}

	return Menu;
}

/*****************************************************************************/
/**
 *
 * This function is called to trigger the SDI menu statemachine.
 *
 * @param	InstancePtr is a pointer to the XSdi_Menu instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XSdi_MenuProcess(XSdi_Menu *InstancePtr)
{
	u8 Data;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Check if the uart has any data */
	if (XUartPs_IsReceiveData(InstancePtr->UartBaseAddress)) {

		/* Read data from uart */
		Data = XUartPs_RecvByte(InstancePtr->UartBaseAddress);
		/* Main menu */
		if (InstancePtr->CurrentMenu == XSDI_MAIN_MENU) {
			InstancePtr->CurrentMenu = XSdi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, Data);
			InstancePtr->Value = 0;
		}

		/* Sub menu */
		else {
			/* Send response to user */
			XUartPs_SendByte(InstancePtr->UartBaseAddress, Data);
			/* Alpha numeric data */
			if (isalpha(Data)) {
				xil_printf("\r\nInvalid input. Valid entry is only digits 0-2. Try again\r\n\r\n");
				xil_printf("Enter Selection -> ");
				InstancePtr->Value = 0;
			}

			/* Numeric data */
			else if ((Data >= '0') && (Data <= '2')) {
				InstancePtr->Value = InstancePtr->Value * 10 + (Data-'0');
			}

			/* Backspace */
			else if (Data == '\b') {
				InstancePtr->Value = InstancePtr->Value / 10; /* discard previous input */
			}

			/* Execute */
			else if ((Data == '\n') || (Data == '\r')) {
				InstancePtr->CurrentMenu = XSdi_MenuTable[InstancePtr->CurrentMenu](InstancePtr, InstancePtr->Value);
				InstancePtr->Value = 0;
			}
		}
	}
}
