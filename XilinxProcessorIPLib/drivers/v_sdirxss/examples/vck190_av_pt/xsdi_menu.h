/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/
/**
*
* @file xsdi_menu.h
*
* This is the main header file for the Xilinx Menu implementation as used
* in the SDI example design.
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
#ifndef XSDI_MENU_H_
#define XSDI_MENU_H_  /**< Prevent circular inclusions
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
#include "xuartpsv_hw.h"
#include "xuartpsv.h"
#include "xv_sdirxss.h"
#include "xv_sditxss.h"
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#include "xsdiaud.h"
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/
/**
* The SDI menu types.
*/
typedef enum
{
	XSDI_MAIN_MENU,
	XSDI_SUBCORE_MENU,
	XSDI_NUM_MENUS
} XSdi_MenuType;

/**
* The SDI menu instance data.
*/
typedef struct
{
	XSdi_MenuType		CurrentMenu;			/**< Current menu */
	u32			UartBaseAddress;		/* Uart base address */
	u8			Value;				/* Sub menu value */
	u8			WaitForColorbar;
} XSdi_Menu;

typedef enum
{
	XSDI_SDI_PASS_THROUGH,
	XSDI_AES_CAPTURE_PLAYBACK
} XSdi_AudioMOde;

extern u8 XSDIAudioMode;
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
extern XSdiAud SdiExtract;		/* Instance of the SDI Extract device */
extern XSdiAud SdiEmbed;		/* Instance of the SDI Embed device */
#endif
/************************** Function Prototypes ******************************/
void XSdi_MenuInitialize(XSdi_Menu *InstancePtr, u32 UartBaseAddress);
void XSdi_MenuProcess(XSdi_Menu *InstancePtr);
void XSdi_MenuReset(XSdi_Menu *InstancePtr);
void XSdiDisableSDIAudioPassThrough(void);
void XSdiDisableAESAudioPlayback(void);
void XSdiDisableAESAudioCapture(void);
void XSdiEnableAESAudioCapture(void);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
