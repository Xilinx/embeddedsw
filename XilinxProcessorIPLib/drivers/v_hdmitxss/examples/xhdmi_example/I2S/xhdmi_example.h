/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_example.h
*
* This file contains set of definition for the main application
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         12/02/18 Initial release.
* 3.03  YB     08/14/18 Adding macro 'ENABLE_HDCP_REPEATER' to allow application
*                       to select/deselect the Repeater specific code.
*       EB     09/21/18 Added new API ToggleHdmiRxHpd
* </pre>
*
******************************************************************************/
#ifndef _XHDMI_EXAMPLE_H_
/**  prevent circular inclusions by using protection macros */
#define _XHDMI_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xparameters.h"
#if defined (ARMR5) || ((__aarch64__) && (!defined XPS_BOARD_ZCU104))
#include "xiicps.h"
#endif
#include "xiic.h"

#include "xil_io.h"
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#include "xuartlite_l.h"
#else
#include "xuartps.h"
#endif
#include "xil_types.h"
#include "xil_exception.h"
#include "string.h"
#if (defined XPS_BOARD_ZCU104)
#include "idt_8t49n24x.h"
#else
#include "si5324drv.h"
#endif
#include "xvidc.h"
#include "xv_hdmic.h"
#include "xv_hdmic_vsif.h"
#include "dp159.h"
#include "sleep.h"
#include "xhdmi_edid.h"
#include "xhdmi_menu.h"
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#include "xv_hdmirxss.h"
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#include "xv_hdmitxss.h"
#include "audiogen_drv.h"
#ifdef XPAR_XI2SRX_NUM_INSTANCES
/* This is only required for the I2S audio over HDMI */
#define USE_HDMI_AUDGEN
#endif
#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
/* This is only required for the audio over HDMI */
#define USE_HDMI_AUDGEN
#endif
#endif
#include "xvphy.h"
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
#ifdef XPAR_XV_TPG_NUM_INSTANCES
#include "xv_tpg.h"
#endif
#endif
#ifdef XPAR_XGPIO_NUM_INSTANCES
#include "xgpio.h"
#endif
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif
#include "xhdmi_hdcp_keys.h"
#include "xhdcp.h"
#include "xvidframe_crc.h"

/* AUXFIFOSIZE: Must be set to 3 or higher*/
#define AUXFIFOSIZE 10

#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define UART_BASEADDR XPAR_MB_SS_0_AXI_UARTLITE_BASEADDR
#else
#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR
#endif

/************************** Constant Definitions *****************************/
#define I2C_MUX_ADDR    0x74  /**< I2C Mux Address */
#define SI5328_I2C_ADDR 0x69
#if (defined XPS_BOARD_ZCU104)
#define I2C_CLK_ADDR    0x6C  /**< I2C Clk Address IDT_8T49N241*/
#else
#define I2C_CLK_ADDR    0x68  /**< I2C Clk Address */
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"


/************************** Constant Definitions *****************************/

/******************************** OPTIONS ************************************/
/* Enabling this will disable Pass-through mode and TX and RX will operate
 * separately
 */
#define LOOPBACK_MODE_EN 0

/* Enabling this will enable a debug UART menu */
#define HDMI_DEBUG_TOOLS 0

/* Enabling this will register a custom resolution to the video timing table
 */
#define CUSTOM_RESOLUTION_ENABLE 1

#if defined (XPAR_XHDCP_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || \
	defined (XPAR_XHDCP22_TX_NUM_INSTANCES)

/* If HDCP 1.4 or HDCP 2.2 is in the system
 * then use the HDCP abstraction layer */
#define USE_HDCP

#if defined XPAR_XV_HDMITXSS_NUM_INSTANCES && \
	defined XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* Option to enable or disable HDCP Repeater , if
 * HDCP 1.4 or HDCP 2.2 is in the system */
#define ENABLE_HDCP_REPEATER		0
#endif

#endif

/* Enabling this will enable HDCP Debug menu */
#define HDCP_DEBUG_MENU_EN 0

/* Enabling this will enable Video Masking menu */
#define VIDEO_MASKING_MENU_EN 0

/************************** Variable Definitions *****************************/
/* VPhy structure */
extern XVphy     Vphy;

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/* HDMI TX SS structure */
extern XV_HdmiTxSs HdmiTxSs;

#ifdef USE_HDMI_AUDGEN
extern XhdmiAudioGen_t AudioGen;
#endif

#ifdef XPAR_XV_TPG_NUM_INSTANCES
/* TPG structure */
extern XV_tpg Tpg;
extern XTpg_PatternId Pattern;
#endif

extern u8 TxCableConnect;
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/* HDMI RX SS structure */
extern XV_HdmiRxSs HdmiRxSs;
#endif

#ifdef USE_HDCP
extern XHdcp_Repeater HdcpRepeater;
#endif

/* TX busy flag. This flag is set while the TX is initialized*/
extern u8 TxBusy;
extern u8 IsPassThrough;

#if defined (ARMR5) || ((__aarch64__) && (!defined XPS_BOARD_ZCU104))
extern XIicPs Ps_Iic0, Ps_Iic1;
#define PS_IIC_CLK 100000
#endif

/************************** Function Prototypes ******************************/
int I2cClk_Ps(u32 InFreq, u32 OutFreq);
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void ToggleHdmiRxHpd (XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr);
void SetHdmiRxHpd(XVphy *VphyPtr, XV_HdmiRxSs *HdmiRxSsPtr, u8 Hpd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EXAMPLE_H_ */
