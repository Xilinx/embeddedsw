/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xv_hdmirxss_coreinit.h
* @addtogroup v_hdmirxss
* @{
* @details
*
* This header file contains the hdmi rx subsystem sub-cores
* initialization routines and helper functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.
* 1.1   yh     20/01/16 Added remapper support
* 1.2   MG     03/02/16 Added HDCP support
* </pre>
*
******************************************************************************/
#ifndef XV_HDMIRXSS_COREINIT_H__  /* prevent circular inclusions */
#define XV_HDMIRXSS_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include "xv_hdmirxss.h"
#include "xv_hdmirx.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "xv_axi4s_remap.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_HdmiRxSs_SubcoreInitHdmiRx(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitHdcpTimer(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitHdcp14(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitHdcp22(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitRemapperReset(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitRemapper(XV_HdmiRxSs *HdmiRxSsPtr);

#ifdef __cplusplus
}
#endif

#endif
/** @} */
