/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
