/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1_frl.h
*
* This is main header file of the Xilinx HDMI TX Subsystem driver
*
* <b>HDMI Transmitter Subsystem Overview</b>
*
* HDMI TX Subsystem is a collection of IP cores bounded together by software
* to provide an abstract view of the processing pipe. It hides all the
* complexities of programming the underlying cores from end user.
*
* <b>Subsystem Driver Features</b>
*
* HDMI Subsystem supports following features
*   - AXI Stream Input/Output interface
*   - 1, 2 or 4 pixel-wide video interface
*   - 8/10/12/16 bits per component
*   - RGB & YCbCr color space
*   - Up to 4k2k 60Hz resolution at both Input and Output interface
*   - Interlaced input support (1080i 50Hz/60Hz)

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef HDMITXSS1_FRL_H /**< prevent circular inclusions by using protection macros*/
#define HDMITXSS1_FRL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

#ifdef __cplusplus
}
#endif

void XV_HdmiTxSs1_FrlConfigCallback(void *CallbackRef);
void XV_HdmiTxSs1_FrlFfeCallback(void *CallbackRef);
void XV_HdmiTxSs1_FrlStartCallback(void *CallbackRef);
void XV_HdmiTxSs1_FrlStopCallback(void *CallbackRef);
void XV_HdmiTxSs1_TmdsConfigCallback(void *CallbackRef);

#endif /* end of protection macro */
