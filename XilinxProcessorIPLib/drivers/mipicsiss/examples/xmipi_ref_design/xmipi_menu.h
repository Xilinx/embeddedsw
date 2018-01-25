/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xmipi_menu.h
 *
 * This is the main header file for the Xilinx Menu implementation as used
 * in the MIPI example design.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.00  pg    12/07/17 Initial release.
 * </pre>
 *
 ******************************************************************************/
#ifndef XMIPI_MENU_H_
#define XMIPI_MENU_H_  /**< Prevent circular inclusions
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
#include "xuartps.h"
#include "xvphy.h"
#include "xv_hdmitxss.h"
#include "xv_tpg.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * The MIPI menu types.
 */
typedef enum
{
	XMIPI_MAIN_MENU,
	XMIPI_RESOLUTION_MENU,
	XMIPI_CSI_LANES_MENU,
	XMIPI_NUM_MENUS
} XMipi_MenuType;

/**
 * The MIPI menu configuration.
 */
typedef struct
{
	u8 HdcpIsSupported; /**< Indicates if HDCP is supported */

} XMipi_MenuConfig;

/**
 * The MIPI menu instance data.
 */
typedef struct
{
	XMipi_MenuConfig Config;    	/* HDMI menu configuration data */
	XMipi_MenuType CurrentMenu;	/* Current menu */
	u32 UartBaseAddress; 		/* Uart base address */
	u8 Value;			/* Sub menu value */
	u8 WaitForColorbar;
} XMipi_Menu;

/************************** Function Prototypes ******************************/
void XMipi_MenuInitialize(XMipi_Menu *InstancePtr, u32 UartBaseAddress);
void XMipi_MenuProcess(XMipi_Menu *InstancePtr);
void XMipi_MenuReset(XMipi_Menu *InstancePtr);
extern void InitImageProcessingPipe(void);
extern void DisableImageProcessingPipe(void);
extern void CamReset(void);
extern void Reset_IP_Pipe(void);
#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
