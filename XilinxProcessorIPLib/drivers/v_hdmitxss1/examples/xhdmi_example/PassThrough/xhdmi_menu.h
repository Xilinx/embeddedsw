/******************************************************************************
*
* Copyright (C) 2018 – 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
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
* Ver	Who  Date	Changes
* ----- ---- ---------- --------------------------------------------------
* X.X	..   DD-MM-YYYY ..
* 1.0	mmo  24-04-2019 Initial version
*
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
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif
#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
		      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"
#endif

#include "xhdmiphy1.h"
#include "xhdmi_edid.h"
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
#include "xv_hdmirxss1.h"
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#include "xv_hdmitxss1.h"
#include "xv_tpg.h"
#include "audiogen_drv.h"
#endif
#include "video_fmc.h"
#include "xhdmi_example.h"
/************************** Variable Definitions *****************************/
extern u8 Counter;
extern u8 Edid[];

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#if(CUSTOM_RESOLUTION_ENABLE == 1)
    /* Assign Mode ID Enumeration. First entry Must be > XVIDC_VM_CUSTOM */
    typedef enum {
	XVIDC_VM_1152x864_60_P = (XVIDC_VM_CUSTOM + 1),
	XVIDC_VM_2560x1440_144_P = (XVIDC_VM_CUSTOM + 2),
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
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	XHDMI_EDID_MENU,
	XHDMI_RESOLUTION_MENU,
	XHDMI_FRAMERATE_MENU,
	XHDMI_COLORDEPTH_MENU,
	XHDMI_COLORSPACE_MENU,
	XHDMI_AUDIO_MENU,
	XHDMI_AUDIO_CHANNEL_MENU,
	XHDMI_AUDIO_SAMPFREQ_MENU,
	XHDMI_VIDEO_MENU,
#endif
#if defined(USE_HDCP_HDMI_RX) || defined(USE_HDCP_HDMI_TX)
	XHDMI_HDCP_MAIN_MENU,
#if (HDCP_DEBUG_MENU_EN == 1)
	XHDMI_HDCP_DEBUG_MENU,
#endif
#endif
#if(HDMI_DEBUG_TOOLS == 1)
	XHDMI_DEBUG_MAIN_MENU,
#endif
	XHDMI_VPHY_DEBUG_MENU,
	XHDMI_ONSEMI_DEBUG_MENU,
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
	XHdmi_MenuConfig    Config;		    /**< HDMI menu
							 configuration data */
	XHdmi_MenuType	    CurrentMenu;	    /**< Current menu */
	u32		    UartBaseAddress;	    /**< Uart base address */
	u8		    Value;		    /**< Sub menu value */
	u8		    WaitForColorbar;

	XVidC_ColorDepth    AdvColorDepth;
	XVidC_ColorFormat   AdvColorSpace;
    } XHdmi_Menu;

/************************** Function Prototypes ******************************/
    void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress);
    void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr);
    void XHdmi_MenuReset(XHdmi_Menu *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
