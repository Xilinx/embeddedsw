/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 *****************************************************************************/
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
 * 1.0	ssh  07/05/2018 Initial release
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
};

extern XGpio Gpio_AxisFifo_resetn;
extern XV_SdiTxSs SdiTxSs;       /* SDI TX SS structure */
extern XV_SdiRxSs SdiRxSs;       /* SDI RX SS structure */
extern u8 IsPassThrough;         /* Demo mode 0-colorbar 1-pass through */
extern u8 TxBusy;                /* TX busy set while the TX is initialized */

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
		case ('i') :
		case ('I') :
			/* Info */
			Info();
			Menu = XSDI_MAIN_MENU;
			break;
		case ('z') :
		case ('Z') :
			/* GT & SDI TX/RX log */
			XV_SdiTxSs_LogDisplay(&SdiTxSs);
			XV_SdiRxSs_LogDisplay(&SdiRxSs);
			Menu = XSDI_MAIN_MENU;
			break;
		case ('d') :
		case ('D') :
			/* Debug */
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

	/* Main Menu */
	InstancePtr->CurrentMenu = XSdi_MenuTable[InstancePtr->CurrentMenu]
				   (InstancePtr, Data);
	InstancePtr->Value = 0;
	}
}
