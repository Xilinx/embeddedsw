/******************************************************************************
*
* Copyright (C) 2014 - 2017 Xilinx, Inc.  All rights reserved.
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
#include "xvphy.h"
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
/* If HDCP 1.4 or HDCP 2.2 is in the system then use the HDCP abstraction layer */
#define USE_HDCP
#endif

/* Enabling this will enable HDCP Debug menu */
#define HDCP_DEBUG_MENU_EN 0

/* Enabling this will enable Video Masking menu */
#define VIDEO_MASKING_MENU_EN 0

/************************** Variable Definitions *****************************/
/* VPhy structure */
extern XVphy     Vphy;


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
XIicPs Ps_Iic0, Ps_Iic1;
#define PS_IIC_CLK 100000
#endif

#ifdef __cplusplus
}
#endif

#endif /* _XHDMI_EXAMPLE_H_ */
