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
* @file xhdmi_example.c
*
* This file demonstrates how to use Xilinx HDMI TX Subsystem, HDMI RX Subsystem
* and Video PHY Controller drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         25/11/15 Initial release.
* 1.10         05/02/16 Updated function RxAuxCallback.
* 2.00  MG     02/03/15 Added upgraded with HDCP driver and overlay
* 2.10  MH     06/23/16 Added HDCP repeater support.
* 2.11  YH     04/08/16 Added two level validation routines
*                       Basic_validation will only check the received VmId
*                       PRBS_validation will check both video & audio contents
* 2.12  GM     07/10/16 Added onboard SI5324 Initialization API to enable
*                       125Mhz as NI-DRU reference clock
* 2.13  YH     03/01/16 Fixed a system hang issue by clearing TxBusy flag when a
*                            non-supportedvideo resolution is set
*                            during enable colorbar API
* 2.14  GM     23/01/17 Replace the Extraction Value of VPhy line rate with,
*                            XVphy_GetLineRateHz Rate API return value.
* 2.15  ms     04/10/17 Modified filename tag to include the file in doxygen
*                            examples.
* 2.16  mmo    05/05/17 Replace pre-processed interrupt vector ID with the
*                            pre-processed canonical interrupt vector ID for
*                            microblaze processor
* 2.17  YH     12/06/17 Removed unused PRBS validation related codes
*                       Added VPHY error processing APIs and typedef
*                       Placed Si5324 on reset on bonded mode in StartTxAfterRx
*                       Changed printf usage to xil_printf
*                       Changed "\n\r" in xil_printf calls to "\r\n"
* 2.18  mmo    21/07/17 Remove the i2c_dp159 API Call and
*                            XVphy_Clkout1OBufTdsEnable API Call from the
*                            TxStreamCallback API to avoid the race condition,
*                            and replace to be call at the global while loop.
*       MH     26/07/17 Set TMDS SCDC register after TX HPD toggle event
*       GM     18/08/17 Added SI Initialization after the SI Reset in
*                            StartTxAfterRx API
*       YH     18/08/17 Add HDCP Ready checking before set down streams
*       GM     28/08/17 Replace XVphy_HdmiInitialize API Call during
*                            Initialization with XVphy_Hdmi_CfgInitialize API
*                            Call
*       mmo    04/10/17 Updated function TxStreamUpCallback to include
*                            XhdmiACRCtrl_TMDSClkRatio API Call
*       EB     06/11/17 Updated function RxAudCallback to allow pass-through
*                            of audio format setting
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xparameters.h"
#if defined (ARMR5) || (__aarch64__)
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
#include "si5324drv.h"
#include "xvidc.h"
#include "xvidc_edid.h"
#include "dp159.h"
#include "audiogen_drv.h"
#include "sleep.h"
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

#define LOOPBACK_MODE_EN 0

#if defined (ARMR5) || (__aarch64__)
#define XPAR_CPU_CORE_CLOCK_FREQ_HZ 99990000
#elif defined (__arm__)
#define XPAR_CPU_CORE_CLOCK_FREQ_HZ 100000000
#endif

#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define UART_BASEADDR XPAR_MB_SS_0_AXI_UARTLITE_BASEADDR
#else
#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR
#endif

/************************** Constant Definitions *****************************/
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*
  EDID
*/
// Xilinx EDID
static const u8 Edid[] = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x61, 0x98, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12,
  0x1F, 0x19, 0x01, 0x03, 0x80, 0x59, 0x32, 0x78, 0x0A, 0xEE, 0x91, 0xA3, 0x54, 0x4C, 0x99, 0x26,
  0x0F, 0x50, 0x54, 0x21, 0x08, 0x00, 0x71, 0x4F, 0x81, 0xC0, 0x81, 0x00, 0x81, 0x80, 0x95, 0x00,
  0xA9, 0xC0, 0xB3, 0x00, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
  0x45, 0x00, 0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x58, 0x49, 0x4C,
  0x49, 0x4E, 0x58, 0x20, 0x48, 0x44, 0x4D, 0x49, 0x0A, 0x20, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C,
  0x02, 0x03, 0x34, 0x71, 0x57, 0x61, 0x10, 0x1F, 0x04, 0x13, 0x05, 0x14, 0x20, 0x21, 0x22, 0x5D,
  0x5E, 0x5F, 0x60, 0x65, 0x66, 0x62, 0x63, 0x64, 0x07, 0x16, 0x03, 0x12, 0x23, 0x09, 0x07, 0x07,
  0x67, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x78, 0x3C, 0xE3, 0x0F, 0x01, 0xE0, 0x67, 0xD8, 0x5D, 0xC4,
  0x01, 0x78, 0x80, 0x07, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00,
  0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x08, 0xE8, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
  0x8A, 0x00, 0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80,
  0xB0, 0x58, 0x8A, 0x00, 0x20, 0x52, 0x31, 0x00, 0x00, 0x1E, 0x66, 0x21, 0x56, 0xAA, 0x51, 0x00,
  0x1E, 0x30, 0x46, 0x8F, 0x33, 0x00, 0x50, 0x1D, 0x74, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x2E
};

//// CTS EDID
//static const u8 Edid[] = {
//0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x61, 0x98, 0x34, 0x12, 0x00, 0x53, 0x54, 0x43,
//0x1F, 0x19, 0x01, 0x03, 0x80, 0x50, 0x2D, 0x78, 0x1A, 0x0D, 0xC9, 0xA0, 0x57, 0x47, 0x98, 0x27,
//0x12, 0x48, 0x4C, 0x20, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
//0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
//0x45, 0x00, 0x20, 0xC2, 0x31, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
//0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x58,
//0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20, 0x43, 0x54, 0x53, 0x0A, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,
//0x00, 0x17, 0xF1, 0x08, 0x8C, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x04,
//
//0x02, 0x03, 0x65, 0x71, 0x5F, 0x90, 0x1F, 0x22, 0x20, 0x05, 0x14, 0x04, 0x13, 0x3E, 0x3C, 0x11,
//0x02, 0x03, 0x15, 0x06, 0x01, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x12, 0x16,
//0x17, 0x18, 0x19, 0x1A, 0x5F, 0x1B, 0x1C, 0x1D, 0x1E, 0x21, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
//0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
//0x39, 0x3A, 0x3B, 0x3D, 0x42, 0x3F, 0x40, 0x23, 0x0F, 0x7F, 0x07, 0x83, 0x4F, 0x00, 0x00, 0x6E,
//0x03, 0x0C, 0x00, 0x10, 0x00, 0x78, 0x3C, 0x20, 0x00, 0x80, 0x01, 0x02, 0x03, 0x04, 0xE2, 0x00,
//0xFF, 0xE3, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47,
//};
#endif

u8 Hdcp22Srm[] =
{
  0x91, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x8B, 0xBE, 0x2D, 0x46,
  0x05, 0x9F, 0x00, 0x78, 0x7B, 0xF2, 0x84, 0x79, 0x7F, 0xC4, 0xF5, 0xF6, 0xC4, 0x06, 0x36, 0xA1,
  0x20, 0x2E, 0x57, 0xEC, 0x8C, 0xA6, 0x5C, 0xF0, 0x3A, 0x14, 0x38, 0xF0, 0xB7, 0xE3, 0x68, 0xF8,
  0xB3, 0x64, 0x22, 0x55, 0x6B, 0x3E, 0xA9, 0xA8, 0x08, 0x24, 0x86, 0x55, 0x3E, 0x20, 0x0A, 0xDB,
  0x0E, 0x5F, 0x4F, 0xD5, 0x0F, 0x33, 0x52, 0x01, 0xF3, 0x62, 0x54, 0x40, 0xF3, 0x43, 0x0C, 0xFA,
  0xCD, 0x98, 0x1B, 0xA8, 0xB3, 0x77, 0xB7, 0xF8, 0xFA, 0xF7, 0x4D, 0x71, 0xFB, 0xB5, 0xBF, 0x98,
  0x9F, 0x1A, 0x1E, 0x2F, 0xF2, 0xBA, 0x80, 0xAD, 0x20, 0xB5, 0x08, 0xBA, 0xF6, 0xB5, 0x08, 0x08,
  0xCF, 0xBA, 0x49, 0x8D, 0xA5, 0x73, 0xD5, 0xDE, 0x2B, 0xEA, 0x07, 0x58, 0xA8, 0x08, 0x05, 0x66,
  0xB8, 0xD5, 0x2B, 0x9C, 0x0B, 0x32, 0xF6, 0x5A, 0x61, 0xE4, 0x9B, 0xC2, 0xF6, 0xD1, 0xF6, 0x2D,
  0x0C, 0x19, 0x06, 0x0E, 0x3E, 0xCE, 0x62, 0x97, 0x80, 0xFC, 0x50, 0x56, 0x15, 0xCB, 0xE1, 0xC7,
  0x23, 0x4B, 0x52, 0x34, 0xC0, 0x9F, 0x85, 0xEA, 0xA9, 0x15, 0x8C, 0xDD, 0x7C, 0x78, 0xD6, 0xAD,
  0x1B, 0xB8, 0x28, 0x1F, 0x50, 0xD4, 0xD5, 0x42, 0x29, 0xEC, 0xDC, 0xB9, 0xA1, 0xF4, 0x26, 0xFA,
  0x43, 0xCC, 0xCC, 0xE7, 0xEA, 0xA5, 0xD1, 0x76, 0x4C, 0xDD, 0x92, 0x9B, 0x1B, 0x1E, 0x07, 0x89,
  0x33, 0xFE, 0xD2, 0x35, 0x2E, 0x21, 0xDB, 0xF0, 0x31, 0x8A, 0x52, 0xC7, 0x1B, 0x81, 0x2E, 0x43,
  0xF6, 0x59, 0xE4, 0xAD, 0x9C, 0xDB, 0x1E, 0x80, 0x4C, 0x8D, 0x3D, 0x9C, 0xC8, 0x2D, 0x96, 0x23,
  0x2E, 0x7C, 0x14, 0x13, 0xEF, 0x4D, 0x57, 0xA2, 0x64, 0xDB, 0x33, 0xF8, 0xA9, 0x10, 0x56, 0xF4,
  0x59, 0x87, 0x43, 0xCA, 0xFC, 0x54, 0xEA, 0x2B, 0x46, 0x7F, 0x8A, 0x32, 0x86, 0x25, 0x9B, 0x2D,
  0x54, 0xC0, 0xF2, 0xEF, 0x8F, 0xE7, 0xCC, 0xFD, 0x5A, 0xB3, 0x3C, 0x4C, 0xBC, 0x51, 0x89, 0x4F,
  0x41, 0x20, 0x7E, 0xF3, 0x2A, 0x90, 0x49, 0x5A, 0xED, 0x3C, 0x8B, 0x3D, 0x9E, 0xF7, 0xC1, 0xA8,
  0x21, 0x99, 0xCF, 0x20, 0xCC, 0x17, 0xFC, 0xC7, 0xB6, 0x5F, 0xCE, 0xB3, 0x75, 0xB5, 0x27, 0x76,
  0xCA, 0x90, 0x99, 0x2F, 0x80, 0x98, 0x9B, 0x19, 0x21, 0x6D, 0x53, 0x7E, 0x1E, 0xB9, 0xE6, 0xF3,
  0xFD, 0xCB, 0x69, 0x0B, 0x10, 0xD6, 0x2A, 0xB0, 0x10, 0x5B, 0x43, 0x47, 0x11, 0xA4, 0x60, 0x28,
  0x77, 0x1D, 0xB4, 0xB2, 0xC8, 0x22, 0xDB, 0x74, 0x3E, 0x64, 0x9D, 0xA8, 0xD9, 0xAA, 0xEA, 0xFC,
  0xA8, 0xA5, 0xA7, 0xD0, 0x06, 0x88, 0xBB, 0xD7, 0x35, 0x4D, 0xDA, 0xC0, 0xB2, 0x11, 0x2B, 0xFA,
  0xED, 0xBF, 0x2A, 0x34, 0xED, 0xA4, 0x30, 0x7E, 0xFD, 0xC5, 0x21, 0xB6
};

u8 Hdcp22RxPrivateKey[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// LC128
u8 Hdcp22Lc128[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// HDCP 1.4 Key A
u8 Hdcp14KeyA[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// HDCP 1.4 Key B
u8 Hdcp14KeyB[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define I2C_MUX_ADDR    0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR    0x68  /**< I2C Clk Address */

#ifdef XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR
/* This is only required for the audio over HDMI */
#define USE_HDMI_AUDGEN
#endif

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
/* If HDCP 1.4 or HDCP 2.2 is in the system then use the HDCP abstraction layer */
#define USE_HDCP
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/
int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

#if defined (__arm__) && (!defined(ARMR5))
int OnBoardSi5324Init(void);
#endif



#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
void RxConnectCallback(void *CallbackRef);
void RxStreamUpCallback(void *CallbackRef);
void RxStreamDownCallback(void *CallbackRef);
void VphyHdmiRxInitCallback(void *CallbackRef);
void VphyHdmiRxReadyCallback(void *CallbackRef);
#endif
void VphyErrorCallback(void *CallbackRef);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
void VphyPllLayoutErrorCallback(void *CallbackRef);
#endif
void VphyProcessError(void);

/************************** Variable Definitions *****************************/

XVphy Vphy;               	/* VPHY structure */
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
XV_HdmiRxSs HdmiRxSs;       /* HDMI RX SS structure */
XV_HdmiRxSs_Config *XV_HdmiRxSs_ConfigPtr;
#endif
#if defined (ARMR5) || (__aarch64__) || (__arm__)
static XScuGic Intc;
#else
static XIntc Intc;        	/* INTC structure */
#endif
#ifdef XPAR_XGPIO_NUM_INSTANCES
XGpio Gpio_Tpg_resetn;
XGpio_Config *Gpio_Tpg_resetn_ConfigPtr;
#endif
XhdmiAudioGen_t AudioGen; /* Audio Generator structure */
#ifdef VIDEO_FRAME_CRC_EN
Video_CRC_Config VidFrameCRC;
#endif


u8 IsPassThrough;         /**< Demo mode 0-colorbar 1-pass through */
u8 StartTxAfterRxFlag;
u8 VphyErrorFlag;
u8 VphyPllLayoutErrorFlag;
u8 MuteAudio;
u8 TxBusy;                // TX busy flag. This flag is set while the TX is initialized
u8 TxRestartColorbar;     // TX restart colorbar. This flag is set when the TX cable has been reconnected and the TX colorbar was showing.

//New Global Variable for CR-979053 Fix
u8 IsStreamUp;
u64 TxLineRate;

u32 Index;
XHdmi_Menu HdmiMenu;      // Menu structure
#ifdef USE_HDCP
XHdcp_Repeater HdcpRepeater;
#endif

#if defined (ARMR5) || (__aarch64__)
XIicPs Ps_Iic0, Ps_Iic1;
#define PS_IIC_CLK 100000
#endif

// Needed for ZCU106 RevB
#if defined (ARMR5) || (__aarch64__)
void Disable_TMDS181_HPD_passthrough();
#define TMDS181_ADDR    0x5c
#endif

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator over IIC.
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int I2cMux(void)
{
  u8 Buffer;
  int Status;

  xil_printf("Set i2c mux... \r\n");

  /* Select SI5324 clock generator */
#if defined (ARMR5) || (__aarch64__)
  Buffer = 0x10;
  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1, 0);
#else
  Buffer = 0x80;
  Status = XIic_Send((XPAR_IIC_0_BASEADDR), (I2C_MUX_ADDR),
								  (u8 *)&Buffer, 1, (XIIC_STOP));
#endif
  xil_printf("Set i2c mux done\r\n");

  return Status;
}

/*****************************************************************************/
/**
*
* This function setup SI5324 clock generator either in free or locked mode.
*
* @param  Index specifies an index for selecting mode frequency.
* @param  Mode specifies either free or locked mode.
*
* @return
*   - Zero if error in programming external clock.
*   - One if programmed external clock.
*
* @note   None.
*
******************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq)
{
  int Status;

  /* Free running mode */
  if (InFreq == 0) {

	  Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
						   (SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), OutFreq);
	if (Status != (SI5324_SUCCESS)) {
		xil_printf("Error programming SI5324\r\n");
		return 0;
	}
  }

  /* Locked mode */
  else {
	  Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR), (I2C_CLK_ADDR),
						   (SI5324_CLKSRC_CLK1), InFreq, OutFreq);
	if (Status != (SI5324_SUCCESS)) {
		xil_printf("Error programming SI5324\r\n");
		return 0;
	}
  }

  return 1;
}

#if defined (ARMR5) || (__aarch64__)

int I2cMux_Ps(void)
{
  u8 Buffer;
  int Status;

  xil_printf("Set i2c mux... \r\n");

  /* Select SI5324 clock generator */
  Buffer = 0x10;
  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1, I2C_MUX_ADDR);
  xil_printf("Set i2c mux done\r\n");

  return Status;
}

int I2cClk_Ps(u32 InFreq, u32 OutFreq)
{
  int Status;

  /* Free running mode */
  if (InFreq == 0) {

	  Status = Si5324_SetClock_Ps(&Ps_Iic1, (0x69),
						   (SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), OutFreq);

	if (Status != (SI5324_SUCCESS)) {
		print("Error programming SI5324\r\n");
		return 0;
	}
  }

  /* Locked mode */
  else {
	Status = Si5324_SetClock_Ps(&Ps_Iic1, (0x69),
						   (SI5324_CLKSRC_CLK1), InFreq, OutFreq);

	if (Status != (SI5324_SUCCESS)) {
		print("Error programming SI5324\r\n");
		return 0;
	}
  }

  return 1;
}

void Disable_TMDS181_HPD_passthrough() {
	u8 Buffer[2];

	/* Disable TMDS181 HPD pass through */
	Buffer[1] = 0xF1;
	Buffer[0] = 0x0A;
    XIic_Send((XPAR_IIC_0_BASEADDR), (TMDS181_ADDR),
								  (u8 *)&Buffer, 2, (XIIC_STOP));

}
#endif

#if defined (__arm__) && (!defined(ARMR5))
/*****************************************************************************/
/**
*
* This function initializes the ZC706 on-board SI5324 clock generator over IIC.
* CLKOUT1 is set to 125 MHz
*
* @param  None.
*
* @return The number of bytes sent.
*
* @note   None.
*
******************************************************************************/
int OnBoardSi5324Init(void)
{
  u8 Buffer;
  int Status;

  /* Select SI5324 clock generator */
  Buffer = 0x10;
  XIic_Send((XPAR_IIC_1_BASEADDR), (I2C_MUX_ADDR),
								  (u8 *)&Buffer, 1, (XIIC_STOP));

  /* Initialize Si5324 */
  Si5324_Init(XPAR_IIC_1_BASEADDR, I2C_CLK_ADDR);

  /* Program Output Frequency Si5324 */
  Status = Si5324_SetClock((XPAR_IIC_1_BASEADDR), (I2C_CLK_ADDR),
			   (SI5324_CLKSRC_XTAL), (SI5324_XTAL_FREQ), (u32)125000000);

if (Status != (SI5324_SUCCESS)) {
	xil_printf("Error programming On-Board SI5324\r\n");
	return 0;
}

  return Status;
}
#endif


/*****************************************************************************/
/**
*
* This function outputs the video timing , Audio, Link Status, HDMI RX state of
* HDMI RX core. In addition, it also prints information about HDMI TX, and
* HDMI GT cores.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Info(void)
{
  u32 Data;
  xil_printf("\r\n-----\r\n");
  xil_printf("Info\r\n");
  xil_printf("-----\r\n\r\n");

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  XV_HdmiRxSs_ReportInfo(&HdmiRxSs);
#endif
#ifdef VIDEO_FRAME_CRC_EN
  XVidFrameCrc_Report();
#endif

  // GT
  xil_printf("------------\r\n");
  xil_printf("HDMI PHY\r\n");
  xil_printf("------------\r\n");
  Data = XVphy_GetVersion(&Vphy);
  xil_printf("  VPhy version : %02d.%02d (%04x)\r\n",
     ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  xil_printf("\r\n");
  xil_printf("GT status\r\n");
  xil_printf("---------\r\n");
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  xil_printf("RX reference clock frequency: %0d Hz\r\n",
				  XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX));
  if(Vphy.Config.DruIsPresent == (TRUE)) {
    xil_printf("DRU reference clock frequency: %0d Hz\r\n",
				XVphy_DruGetRefClkFreqHz(&Vphy));
  }
#endif
  XVphy_HdmiDebugInfo(&Vphy, 0, XVPHY_CHANNEL_ID_CH1);
}


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a RX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxConnectCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	// RX cable is disconnected
	if(HdmiRxSsPtr->IsStreamConnected == (FALSE))
	{
		Vphy.HdmiRxTmdsClockRatio = 0; // Clear GT RX TMDS clock ratio
		IsPassThrough = (FALSE);   // Clear pass-through flag
#if(LOOPBACK_MODE_EN != 1)
		TxRestartColorbar = (TRUE);// Start colorbar with same video parameters
		TxBusy = (FALSE);
#endif
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (FALSE));

#ifdef USE_HDCP
		/* Call HDCP disconnect callback */
		XHdcp_StreamDisconnectCallback(&HdcpRepeater);
#endif
	}
	else
	{
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_RX, (TRUE));

#ifdef USE_HDCP
		/* Call HDCP connect callback */
		XHdcp_StreamConnectCallback(&HdcpRepeater);
#endif
	}

}

/*****************************************************************************/
/**
*
* This function is called when the GT RX reference input clock has changed.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxInitCallback(void *CallbackRef)
{
	XVphy *VphyPtr = (XVphy *)CallbackRef;

	XV_HdmiRxSs_RefClockChangeInit(&HdmiRxSs);
	VphyPtr->HdmiRxTmdsClockRatio = HdmiRxSs.TMDSClockRatio;
}

/*****************************************************************************/
/**
*
* This function is called when the GT RX has been initialized.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyHdmiRxReadyCallback(void *CallbackRef)
{
	XVphy *VphyPtr = (XVphy *)CallbackRef;
	XVphy_PllType RxPllType;

	/* Reset the menu to main */
	XHdmi_MenuReset(&HdmiMenu);

	RxPllType = XVphy_GetPllType(VphyPtr, 0, XVPHY_DIR_RX,
											XVPHY_CHANNEL_ID_CH1);
	if (!(RxPllType == XVPHY_PLL_TYPE_CPLL)) {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
				(XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CMN0)/1000000));

	}
	else {
		XV_HdmiRxSs_SetStream(&HdmiRxSs, VphyPtr->HdmiRxRefClkHz,
				(XVphy_GetLineRateHz(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)/1000000));
	}
}
#endif

/*****************************************************************************/
/**
*
* This function is called whenever an error condition in VPHY occurs.
* This will fill the FIFO of VPHY error events which will be processed outside
* the ISR.
*
* @param  CallbackRef is the VPHY instance pointer
* @param  ErrIrqType is the VPHY error type
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyErrorCallback(void *CallbackRef)
{
	VphyErrorFlag = TRUE;
}

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
/*****************************************************************************/
/**
*
* This function is called whenever a GTXE2 PLL layout error condition in VPHY
* occurs. This function can be used automatically switch the PLL layout.
* This will set the VPHY PLL Layout error flag to TRUE.
*
* @param  CallbackRef is the VPHY instance pointer
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void VphyPllLayoutErrorCallback(void *CallbackRef)
{
	VphyPllLayoutErrorFlag = TRUE;
}
#endif

/*****************************************************************************/
/**
*
* This function is called in the application to process the pending
* VPHY errors
*
* @param  None.
*
* @return None.
*
* @note   This function can be expanded to perform necessary actions depending
* 		  on the error type. For example, XVPHY_ERR_PLL_LAYOUT can be used to
* 		  automatically switch in and out of bonded mode for GTXE2 devices
*
******************************************************************************/
void VphyProcessError(void)
{

	if (VphyErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED "VPHY Error: See log for details"
				ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	VphyErrorFlag = FALSE;

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	if (VphyPllLayoutErrorFlag == TRUE) {
		xil_printf(ANSI_COLOR_RED "VPHY Error: Try changing to "
				"another PLL Layout" ANSI_COLOR_RESET "\r\n");
	}
	/* Clear Flag */
	VphyPllLayoutErrorFlag = FALSE;
#endif
}


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when a RX aux irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAuxCallback(void *CallbackRef)
{
}

/*****************************************************************************/
/**
*
* This function is called when a RX audio irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxAudCallback(void *CallbackRef)
{
}

/*****************************************************************************/
/**
*
* This function is called when a RX link status irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxLnkStaCallback(void *CallbackRef)
{
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

	if (IsPassThrough) {
		/* Reset RX when the link error has reached its maximum */
		if ((HdmiRxSsPtr->IsLinkStatusErrMax) &&
			(Vphy.Quads[0].Plls[0].RxState == XVPHY_GT_STATE_READY)) {

			/* Pulse TX PLL reset */
			XVphy_ResetGtPll(&Vphy, 0, XVPHY_CHANNEL_ID_CHA,
							XVPHY_DIR_TX, (TRUE));
		}
	}
}

/*****************************************************************************/
/**
*
* This function is called when the RX DDC irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxDdcCallback(void *CallbackRef)
{
}

/*****************************************************************************/
/**
*
* This function is called when the RX HDCP irq has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxHdcpCallback(void *CallbackRef)
{
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamDownCallback(void *CallbackRef)
{
  IsPassThrough = (FALSE);

#ifdef USE_HDCP
  /* Call HDCP stream-down callback */
  XHdcp_StreamDownCallback(&HdcpRepeater);
#endif

}

/*****************************************************************************/
/**
*
* This function is called when the RX stream init
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamInitCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
  XVidC_VideoStream *HdmiRxSsVidStreamPtr;
  u32 Status;
//xil_printf("RxStreamInitCallback\r\n");
	// Calculate RX MMCM parameters
	// In the application the YUV422 colordepth is 12 bits
	// However the HDMI transports YUV422 in 8 bits.
	// Therefore force the colordepth to 8 bits when the colorspace is YUV422

    HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(HdmiRxSsPtr);

    if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_422) {
	Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DIR_RX,
				HdmiRxSsVidStreamPtr->PixPerClk,
				XVIDC_BPC_8);
	}

	// Other colorspaces
	else {
		Status = XVphy_HdmiCfgCalcMmcmParam(&Vphy, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DIR_RX,
				HdmiRxSsVidStreamPtr->PixPerClk,
				HdmiRxSsVidStreamPtr->ColorDepth);
	}

    if (Status == XST_FAILURE) {
	return;
    }

	// Enable and configure RX MMCM
	XVphy_MmcmStart(&Vphy, 0, XVPHY_DIR_RX);

	usleep(10000);
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void RxStreamUpCallback(void *CallbackRef)
{
	xil_printf("RX stream is up\r\n");
	XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
	XVidC_ReportStreamInfo(&HdmiRxSsPtr->HdmiRxPtr->Stream.Video);


#ifdef USE_HDCP
	/* Call HDCP stream-up callback */
	XHdcp_StreamUpCallback(&HdcpRepeater);
#endif

#ifdef VIDEO_FRAME_CRC_EN
	/* Reset Video Frame CRC */
	XVidFrameCrc_Reset();
#endif

}
#endif

/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* HDMI cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void)
{
  int Status;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
  XScuGic *IntcInstPtr = &Intc;
#else
  XIntc *IntcInstPtr = &Intc;
#endif

  /*
   * Initialize the interrupt controller driver so that it's ready to
   * use, specify the device ID that was generated in xparameters.h
   */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
  XScuGic_Config *IntcCfgPtr;
  IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
  if(IntcCfgPtr == NULL)
  {
	  xil_printf("ERR:: Interrupt Controller not found");
	  return (XST_DEVICE_NOT_FOUND);
  }
  Status = XScuGic_CfgInitialize(IntcInstPtr,
								 IntcCfgPtr,
								 IntcCfgPtr->CpuBaseAddress);
#else
  Status = XIntc_Initialize(IntcInstPtr, XPAR_INTC_0_DEVICE_ID);
#endif
  if (Status != XST_SUCCESS) {
    xil_printf("Intc initialization failed!\r\n");
    return XST_FAILURE;
  }


  /*
   * Start the interrupt controller such that interrupts are recognized
   * and handled by the processor
   */
#if defined (__MICROBLAZE__)
  Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
//  Status = XIntc_Start(IntcInstPtr, XIN_SIMULATION_MODE);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }
#endif

  Xil_ExceptionInit();

  /*
   * Register the interrupt controller handler with the exception table.
   */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					   (Xil_ExceptionHandler)XScuGic_InterruptHandler,
					   (XScuGic *)IntcInstPtr);
#else
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					   (Xil_ExceptionHandler)XIntc_InterruptHandler,
					   (XIntc *)IntcInstPtr);
#endif

  return (XST_SUCCESS);
}


void Xil_AssertCallbackRoutine(u8 *File, s32 Line)
{
	  xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}

/*****************************************************************************/
/**
*
* Main function to call example with HDMI TX, HDMI RX and HDMI GT drivers.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if HDMI example was successfully.
*   - XST_FAILURE if HDMI example failed.
*
* @note   None.
*
******************************************************************************/
int main()
{
  u32 Status;
  XVphy_Config *XVphyCfgPtr;
#if defined (ARMR5) || (__aarch64__)
  XIicPs_Config *XIic0Ps_ConfigPtr;
  XIicPs_Config *XIic1Ps_ConfigPtr;
#endif

  xil_printf("\r\n\r\n");
  xil_printf("--------------------------------------\r\n");
  xil_printf("---  HDMI SS + VPhy Example v2.0   ---\r\n");
  xil_printf("---  (c) 2017 by Xilinx, Inc.      ---\r\n");
  xil_printf("--------------------------------------\r\n");
  xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
  xil_printf("--------------------------------------\r\n");


  MuteAudio = 0;
  StartTxAfterRxFlag = (FALSE);
  TxBusy = (FALSE);
  TxRestartColorbar = (FALSE);
  VphyErrorFlag = FALSE;
  VphyPllLayoutErrorFlag = FALSE;

  /* Start in color bar */
  IsPassThrough = 0;

  /* Initialize platform */
  init_platform();

  /* Initialize IIC */
#if defined (ARMR5) || (__aarch64__)
  /* Initialize PS IIC0 */
  XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
  if (NULL == XIic0Ps_ConfigPtr) {
	  return XST_FAILURE;
  }

  Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr, XIic0Ps_ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
	  return XST_FAILURE;
  }

  XIicPs_Reset(&Ps_Iic0);
  /*
   * Set the IIC serial clock rate.
   */
  XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);

  /* Initialize PS IIC1 */
  XIic1Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
  if (NULL == XIic1Ps_ConfigPtr) {
	  return XST_FAILURE;
  }

  Status = XIicPs_CfgInitialize(&Ps_Iic1, XIic1Ps_ConfigPtr, XIic1Ps_ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
	  return XST_FAILURE;
  }

  XIicPs_Reset(&Ps_Iic1);
  /*
   * Set the IIC serial clock rate.
   */
  XIicPs_SetSClk(&Ps_Iic1, PS_IIC_CLK);

  // On Board SI5328 chip for DRU reference clock
  I2cMux_Ps();
  /* Initialize external clock generator */
  Si5324_Init_Ps(&Ps_Iic1, 0x69);

  I2cClk_Ps(0, 156250000);

  // Delay 15ms to allow SI chip to lock
  usleep (15000);

#endif

  /* Initialize external clock generator */
  Si5324_Init(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR);

  /* Disable TMDS181 HPD passthrough for ZCU106 Rev B and below */
  //Disable_TMDS181_HPD_passthrough();

  /* Load HDCP keys from EEPROM */
#if defined (XPAR_XHDCP_NUM_INSTANCES) || defined (XPAR_XHDCP22_RX_NUM_INSTANCES) || defined (XPAR_XHDCP22_TX_NUM_INSTANCES)
	if (XHdcp_LoadKeys(Hdcp22Lc128, sizeof(Hdcp22Lc128),
	                  Hdcp22RxPrivateKey, sizeof(Hdcp22RxPrivateKey),
	                  Hdcp14KeyA, sizeof(Hdcp14KeyA),
	                  Hdcp14KeyB, sizeof(Hdcp14KeyB)) == XST_SUCCESS) {

    /* Set pointers to HDCP 2.2 Keys */
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
#if XPAR_XHDCP22_RX_NUM_INSTANCES
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_LC128, Hdcp22Lc128);
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_PRIVATE, Hdcp22RxPrivateKey);
#endif
#endif

    /* Set pointers to HDCP 1.4 keys */
#if XPAR_XHDCP_NUM_INSTANCES
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP14, Hdcp14KeyB);
#endif

    /* Initialize key manager */

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    Status = XHdcp_KeyManagerInit(XPAR_HDCP_KEYMNGMT_BLK_1_BASEADDR, HdmiRxSs.Hdcp14KeyPtr);
    if (Status != XST_SUCCESS) {
      xil_printf("HDCP 1.4 RX Key Manager Initialization error\r\n");
      return XST_FAILURE;
    }
#endif
#endif

  }

  /* Clear pointers */
  else {

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    /* Set pointer to NULL */
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_LC128, (NULL));

    /* Set pointer to NULL */
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP22_PRIVATE, (NULL));

    /* Set pointer to NULL */
    XV_HdmiRxSs_HdcpSetKey(&HdmiRxSs, XV_HDMIRXSS_KEY_HDCP14, (NULL));
#endif


  }
#endif

#if defined(USE_HDMI_AUDGEN)
  /* Initialize the Audio Generator */
  XhdmiAudGen_Init(&AudioGen,
                   XPAR_AUDIO_SS_0_AUD_PAT_GEN_BASEADDR,
                   XPAR_AUDIO_SS_0_HDMI_ACR_CTRL_BASEADDR,
                   XPAR_AUDIO_SS_0_CLK_WIZ_BASEADDR);
#endif

#if defined (__arm__) && (!defined(ARMR5))
  /* Initialize on-board clock generator */
  OnBoardSi5324Init();

  // Delay 15ms to allow SI chip to lock
  usleep (15000);
#endif

 /* Initialize IRQ */
  Status = SetupInterruptSystem();
  if (Status == XST_FAILURE) {
    xil_printf("IRQ init failed.\r\n\r");
    return XST_FAILURE;
  }

#ifdef VIDEO_FRAME_CRC_EN
  XVidFrameCrc_Initialize(&VidFrameCRC);
#endif


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  /////
  // Initialize HDMI RX Subsystem
  /////
  /* Get User Edid Info */
  XV_HdmiRxSs_SetEdidParam(&HdmiRxSs, (u8*)&Edid, sizeof(Edid));
  XV_HdmiRxSs_ConfigPtr =
		  XV_HdmiRxSs_LookupConfig(XPAR_XV_HDMIRX_0_DEVICE_ID);

  if(XV_HdmiRxSs_ConfigPtr == NULL)
  {
	  HdmiRxSs.IsReady = 0;
      return (XST_DEVICE_NOT_FOUND);
  }

  //Initialize top level and all included sub-cores
  Status = XV_HdmiRxSs_CfgInitialize(&HdmiRxSs, XV_HdmiRxSs_ConfigPtr,
									  XV_HdmiRxSs_ConfigPtr->BaseAddress);
  if(Status != XST_SUCCESS)
  {
    xil_printf("ERR:: HDMI RX Subsystem Initialization failed %d\r\n", Status);
    return(XST_FAILURE);
  }

  //Register HDMI RX SS Interrupt Handler with Interrupt Controller
#if defined(__arm__) || (__aarch64__)
  Status |= XScuGic_Connect(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_IRQ_INTR,
			  (XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
			  (void *)&HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4 Cipher interrupt
  Status |= XScuGic_Connect(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_IRQ_INTR,
			  (XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
			  (void *)&HdmiRxSs);

  Status |= XScuGic_Connect(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_TIMER_IRQ_INTR,
			  (XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
			  (void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
  //HDCP 2.2 Timer interrupt */
  Status |= XScuGic_Connect(&Intc,
               XPAR_FABRIC_V_HDMI_RX_SS_HDCP22_TIMER_IRQ_INTR,
               (XInterruptHandler)XV_HdmiRxSS_Hdcp22TimerIntrHandler,
               (void *)&HdmiRxSs);
#endif

#else
  Status |= XIntc_Connect(&Intc,
#if defined(USE_HDCP)
                          XPAR_INTC_0_V_HDMIRXSS_0_IRQ_VEC_ID,
#else
						  XPAR_INTC_0_V_HDMIRXSS_0_VEC_ID,
#endif
                          (XInterruptHandler)XV_HdmiRxSS_HdmiRxIntrHandler,
                          (void *)&HdmiRxSs);

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4 Cipher interrupt
  Status |= XIntc_Connect(&Intc,
		                  XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID,
                          (XInterruptHandler)XV_HdmiRxSS_HdcpIntrHandler,
                          (void *)&HdmiRxSs);

  // HDCP 1.4 Timer interrupt
  Status |= XIntc_Connect(&Intc,
		                  XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID,
                          (XInterruptHandler)XV_HdmiRxSS_HdcpTimerIntrHandler,
                          (void *)&HdmiRxSs);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
  // HDCP 2.2 Timer interrupt
  Status |= XIntc_Connect(&Intc,
                          XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID,
                          (XInterruptHandler)XV_HdmiRxSS_Hdcp22TimerIntrHandler,
                          (void *)&HdmiRxSs);
#endif

#endif

  if (Status == XST_SUCCESS){
#if defined(__arm__) || (__aarch64__)
	  XScuGic_Enable(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_IRQ_INTR);
#ifdef XPAR_XHDCP_NUM_INSTANCES
	  XScuGic_Enable(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_IRQ_INTR);
	  XScuGic_Enable(&Intc,
			  XPAR_FABRIC_V_HDMI_RX_SS_HDCP14_TIMER_IRQ_INTR);
#endif
#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
      XScuGic_Enable(&Intc,
              XPAR_FABRIC_V_HDMI_RX_SS_HDCP22_TIMER_IRQ_INTR);
#endif

#else
	  XIntc_Enable(&Intc,
#if defined(USE_HDCP)
                   XPAR_INTC_0_V_HDMIRXSS_0_IRQ_VEC_ID
#else
			       XPAR_INTC_0_V_HDMIRXSS_0_VEC_ID
#endif
				   );

#ifdef XPAR_XHDCP_NUM_INSTANCES
	  // HDCP 1.4 Cipher interrupt
    XIntc_Enable(&Intc,
                 XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_IRQ_VEC_ID);

    // HDCP 1.4 Timer interrupt
    XIntc_Enable(&Intc,
		     XPAR_INTC_0_V_HDMIRXSS_0_HDCP14_TIMER_IRQ_VEC_ID);
#endif

#if (XPAR_XHDCP22_RX_NUM_INSTANCES)
    // HDCP 2.2 Timer interrupt
    XIntc_Enable(&Intc,
                 XPAR_INTC_0_V_HDMIRXSS_0_HDCP22_TIMER_IRQ_VEC_ID);
#endif

#endif
  }
  else {
	  xil_printf("ERR:: Unable to register HDMI RX interrupt handler");
      xil_printf("HDMI RX SS initialization error\r\n");
	  return XST_FAILURE;
  }

  /* RX callback setup */
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_CONNECT,
							  (void *)RxConnectCallback,
							  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_AUX,
							  (void *)RxAuxCallback,
							  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_AUD,
							  (void *)RxAudCallback,
							  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_LNKSTA,
							  (void *)RxLnkStaCallback,
							  (void *)&HdmiRxSs);
  //XV_HdmiRxSs_SetCallback(&HdmiRxSs,
  //	  	  	  	  	  	  	  XV_HDMIRXSS_HANDLER_DDC,
  //	  	  	  	  	  	  	  RxDdcCallback,
  //	  	  	  	  	  	  	  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_STREAM_DOWN,
							  (void *)RxStreamDownCallback,
							  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_STREAM_INIT,
							  (void *)RxStreamInitCallback,
							  (void *)&HdmiRxSs);
  XV_HdmiRxSs_SetCallback(&HdmiRxSs,
							  XV_HDMIRXSS_HANDLER_STREAM_UP,
							  (void *)RxStreamUpCallback,
							  (void *)&HdmiRxSs);

#ifdef USE_HDCP
  /* Set HDCP upstream interface */
  XHdcp_SetUpstream(&HdcpRepeater, &HdmiRxSs);
#endif
#endif

    /////
    // Initialize Video PHY
    // The GT needs to be initialized after the HDMI RX and TX.
    // The reason for this is the GtRxInitStartCallback
    // calls the RX stream down callback.
    /////

    XVphyCfgPtr = XVphy_LookupConfig(XPAR_VPHY_0_DEVICE_ID);
    if (XVphyCfgPtr == NULL) {
      xil_printf("Video PHY device not found\r\n\r");
      return XST_FAILURE;
    }

    /* Register VPHY Interrupt Handler */
#if defined(__arm__) || (__aarch64__)
    Status = XScuGic_Connect(&Intc,
			XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR,
			(XInterruptHandler)XVphy_InterruptHandler,
			(void *)&Vphy);
#else
  Status = XIntc_Connect(&Intc,
		                 XPAR_INTC_0_VPHY_0_VEC_ID,
			             (XInterruptHandler)XVphy_InterruptHandler,
			             (void *)&Vphy);
#endif
    if (Status != XST_SUCCESS) {
      xil_printf("HDMI VPHY Interrupt Vec ID not found!\r\n");
      return XST_FAILURE;
    }

    /* Initialize HDMI VPHY */
    Status = XVphy_Hdmi_CfgInitialize(&Vphy, 0, XVphyCfgPtr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMI VPHY initialization error\r\n");
      return XST_FAILURE;
    }

    /* Enable VPHY Interrupt */
#if defined(__arm__) || (__aarch64__)
  XScuGic_Enable(&Intc,
		XPAR_FABRIC_VID_PHY_CONTROLLER_IRQ_INTR);
#else
    XIntc_Enable(&Intc,
                 XPAR_INTC_0_VPHY_0_VEC_ID);
#endif

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    XVphy_SetHdmiCallback(&Vphy,
						XVPHY_HDMI_HANDLER_RXINIT,
						(void *)VphyHdmiRxInitCallback,
						(void *)&Vphy);
    XVphy_SetHdmiCallback(&Vphy,
						XVPHY_HDMI_HANDLER_RXREADY,
						(void *)VphyHdmiRxReadyCallback,
						(void *)&Vphy);
#endif

    XVphy_SetErrorCallback(&Vphy,
						(void *)VphyErrorCallback,
						(void *)&Vphy);

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
    XVphy_SetPllLayoutErrorCallback(&Vphy,
						(void *)VphyPllLayoutErrorCallback,
						(void *)&Vphy);
#endif


  xil_printf("---------------------------------\r\n");

  /* Enable exceptions. */
  Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
  Xil_ExceptionEnable();

  // Initialize menu
  XHdmi_MenuInitialize(&HdmiMenu, UART_BASEADDR);



  /* Main loop */
  do {

#ifdef USE_HDCP
	if (XV_HdmiRxSs_HdcpIsReady(&HdmiRxSs)) {
		/* Poll HDCP */
		XHdcp_Poll(&HdcpRepeater);
	}
#endif


    // HDMI menu
    XHdmi_MenuProcess(&HdmiMenu);

    /* VPHY error */
    VphyProcessError();

  } while (1);

  return 0;
}
