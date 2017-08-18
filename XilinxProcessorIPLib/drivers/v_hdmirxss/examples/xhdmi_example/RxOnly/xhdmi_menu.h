/******************************************************************************
*
* Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
* @file xhdmi_menu.h
*
* This is the main header file for the Xilinx Menu implementation as used
* in the HDMI example design.
*
* <b>Software Initialization & Configuration</b>
*
*
* <b>Interrupts </b>
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* X.X   ..   DD-MM-YYYY ..
* 1.0   RHe  10-07-2015 Initial version
* 1.1   MG   03-02-2016 Added HDCP support
* 1.1   YH   27-07-2016 Remove Separate HDCP menu, keep only HDCP Main Menu
* 1.2   MH   09-08-2017 Added HDCP Debug Menu
*       mmo  18-08-2017 Added Support to Custom Resolution in the Resolution
*                               menu
* </pre>
*
******************************************************************************/
#ifndef XHDMI_MENU_H_
#define XHDMI_MENU_H_  /**< Prevent circular inclusions
                         *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "xil_assert.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xvidc.h"
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif

#include "xvphy.h"
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif

/************************** Constant Definitions *****************************/
#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
/* If HDCP 1.4 or HDCP 2.2 is in the system then use the HDCP abstraction layer */
#define USE_HDCP
#endif

/**************************** Type Definitions *******************************/
/**
* The HDMI menu types.
*/
typedef enum
{
	XHDMI_MAIN_MENU,
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	XHDMI_GTPLLLAYOUT_MENU,
#endif
#ifdef USE_HDCP
	XHDMI_HDCP_MAIN_MENU,
	XHDMI_HDCP_DEBUG_MENU,
#endif
	XHDMI_NUM_MENUS
} XHdmi_MenuType;

/**
* The HDMI menu configuration.
*/
typedef struct
{
	u8 HdcpIsSupported; /**< Indicates if HDCP is supported */

} XHdmi_MenuConfig;

/**
* The HDMI menu instance data.
*/
typedef struct
{
	XHdmi_MenuConfig 	Config;    				/**< HDMI menu configuration data */
	XHdmi_MenuType 		CurrentMenu; 			/**< Current menu */
	u32			 		UartBaseAddress;		// Uart base address
	u8					Value;					// Sub menu value
	u8					WaitForColorbar;
} XHdmi_Menu;

/************************** Function Prototypes ******************************/
void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress);
void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr);
void XHdmi_MenuReset(XHdmi_Menu *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
