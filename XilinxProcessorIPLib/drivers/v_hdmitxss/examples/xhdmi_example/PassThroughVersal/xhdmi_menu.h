/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#include "xparameters.h"
#if defined (XPAR_XUARTLITE_NUM_INSTANCES) && (!defined (versal))
#include "xuartlite_l.h"
#elif defined versal
#include "xuartpsv.h"
#else
#include "xuartps.h"
#endif

#include "xhdmiphy1.h"
#include "xhdmi_edid.h"
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#include "xv_tpg.h"
#include "audiogen_drv.h"
#endif
#include "dp159.h"
#include "tmds181.h"
#include "xhdmi_example.h"
/************************** Variable Definitions *****************************/

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
	/* Assign Mode ID Enumeration. First entry Must be > XVIDC_VM_CUSTOM */
	typedef enum {
		XVIDC_VM_1152x864_60_P = (XVIDC_VM_CUSTOM + 1),
		XVIDC_CM_NUM_SUPPORTED
	}
	XVIDC_CUSTOM_MODES;
#endif
#endif
/**************************** Type Definitions *******************************/
/**
* The HDMI menu types.
*/
typedef enum {
		XHDMI_MAIN_MENU,
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
		XHDMI_EDID_MENU,
		XHDMI_RESOLUTION_MENU,
		XHDMI_FRAMERATE_MENU,
		XHDMI_COLORDEPTH_MENU,
		XHDMI_COLORSPACE_MENU,
#ifdef USE_HDMI_AUDGEN
		XHDMI_AUDIO_MENU,
		XHDMI_AUDIO_CHANNEL_MENU,
#endif
		XHDMI_VIDEO_MENU,
#endif
#ifdef USE_HDCP
		XHDMI_HDCP_MAIN_MENU,
#if (HDCP_DEBUG_MENU_EN == 1)
		XHDMI_HDCP_DEBUG_MENU,
#endif
#endif
#if (HDMI_DEBUG_TOOLS == 1)
		XHDMI_DEBUG_MAIN_MENU,
#endif
		XHDMI_VPHY_DEBUG_MENU,
		XHDMI_DP159_MENU,
		XHDMI_NUM_MENUS
	} XHdmi_MenuType;

	/**
	* The HDMI menu configuration.
	*/
	typedef struct {
		u8 HdcpIsSupported; /**< Indicates if HDCP is supported */

	} XHdmi_MenuConfig;

	/**
	* The HDMI menu instance data.
	*/
	typedef struct {
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
