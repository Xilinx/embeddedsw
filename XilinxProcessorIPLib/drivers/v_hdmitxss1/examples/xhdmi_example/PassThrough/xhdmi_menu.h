/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
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
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106)
#include "xiicps.h"
#else
#include "xiic.h"
#endif
#include "xvidc_edid_ext.h"
#if defined (XPAR_XV_FRMBUFRD_NUM_INSTANCES) && \
		      (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"
#endif

#include "xhdmiphy1.h"
#include "xhdmi_edid.h"
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
#include "xhdmi_exdes_sm_rx.h"
#endif
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
#include "xhdmi_exdes_sm_tx.h"
#ifdef XPAR_XV_TPG_NUM_INSTANCES
#include "xv_tpg.h"
#endif
#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
#include "audiogen_drv.h"
#endif
#endif
#include "video_fmc.h"
/************************** Variable Definitions *****************************/
extern u8 Edid[];
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU106)
extern XIicPs Ps_Iic0, Iic;
#else
extern XIic Iic;
#endif

/************************** Constant Definitions *****************************/
/***************************** MENU OPTIONS **********************************/
/* Enabling this will enable a debug UART menu */
#define HDMI_DEBUG_TOOLS 1

/* Enabling this will register a custom resolution to the video timing table
 */
#define CUSTOM_RESOLUTION_ENABLE 1

/* Enabling this will enable HDCP Debug menu */
#define HDCP_DEBUG_MENU_EN 1

/* Enabling this will enable Video Masking menu */
#define VIDEO_MASKING_MENU_EN 1

#ifdef CUSTOM_RESOLUTION_ENABLE
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    /* Assign Mode ID Enumeration. First entry Must be > XVIDC_VM_CUSTOM */
    typedef enum {
	XVIDC_VM_1152x864_60_P = (XVIDC_VM_CUSTOM + 1),
	XVIDC_VM_2560x1440_144_P = (XVIDC_VM_CUSTOM + 2),
	XVIDC_CM_NUM_SUPPORTED
    }
    XVIDC_CUSTOM_MODES;
#endif
#endif

#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
/* This is only required for the audio over HDMI */
#define USE_HDMI_AUDGEN
#endif

/**************************** Type Definitions *******************************/
/**
* The HDMI menu types.
*/
typedef enum {
	XHDMI_MAIN_MENU,
	XHDMI_EDID_MENU,
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
	XHDMI_RESOLUTION_MENU,
	XHDMI_FRAMERATE_MENU,
	XHDMI_COLORDEPTH_MENU,
	XHDMI_COLORSPACE_MENU,
#ifdef USE_HDMI_AUDGEN
	XHDMI_AUDIO_MENU,
	XHDMI_AUDIO_CHANNEL_MENU,
	XHDMI_AUDIO_SAMPFREQ_MENU,
#endif
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
#if defined (XPAR_XV_FRMBUFWR_NUM_INSTANCES) && \
               (XPAR_XV_FRMBUFWR_NUM_INSTANCES)
	XHDMI_8KQUAD_MENU,
#endif
	XHDMI_NUM_MENUS
    } XHdmi_MenuType;

    /**
    * The HDMI menu configuration.
    */
    typedef struct {
	u8 HdcpIsSupported; /**< Indicates if HDCP is supported */

    } XHdmi_MenuConfig;

    /**
    * The HDMI ExDes Settings.
    */
#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    typedef void (*ChangeColorbarOutput)(XVidC_VideoMode VideoMode,
			XVidC_ColorFormat ColorFormat, XVidC_ColorDepth  Bpc);
#endif

#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
    typedef void (*Toggle_HdmiRxHpd)(XHdmiphy1 *Hdmiphy1Ptr,
		XV_HdmiRxSs1 *HdmiRxSs1Ptr);
#endif

#ifdef XPAR_XV_HDMITXSS1_NUM_INSTANCES
    typedef void (*ConfigureTpgEnableInput)(u32 EnableExtSrcInput);
#endif
    typedef struct {
	u8 *ForceIndependentPtr;
	u8 *SystemEventPtr;
	u8 *IsTxPresentPtr;
	u8 *IsRxPresentPtr;
#if defined(XPAR_XV_HDMITXSS1_NUM_INSTANCES)
	ChangeColorbarOutput ChangeColorbarOutputCB;
	ConfigureTpgEnableInput ConfigureTpgEnableInputCB;
#endif
#ifdef XPAR_XV_HDMIRXSS1_NUM_INSTANCES
	Toggle_HdmiRxHpd ToggleHdmiRxHpdCB;
#endif
    } XHdmi_Menu_ExDes_Ctrlr;

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
	XHdmi_Menu_ExDes_Ctrlr ExDesCtrlr;

	XVidC_ColorDepth    AdvColorDepth;
	XVidC_ColorFormat   AdvColorSpace;
    } XHdmi_Menu;

/************************** Function Prototypes ******************************/
void XHdmi_MenuInitialize(XHdmi_Menu *InstancePtr, u32 UartBaseAddress,
		u8 *ForceIndependentPtr, u8 *SystemEventPtr, u8 *IsTxPresentPtr,
		u8 *IsRxPresentPtr, void *ChangeColorbarOutputCB,
		void *ConfigureTpgEnableInputCB, void *ToggleHdmiRxHpdCB);
    void XHdmi_MenuProcess(XHdmi_Menu *InstancePtr, u8 TxBusy);
    void XHdmi_MenuReset(XHdmi_Menu *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
