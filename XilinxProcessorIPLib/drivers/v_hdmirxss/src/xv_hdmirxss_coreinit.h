/******************************************************************************
*
* Copyright (C) 2016 - 2017 Xilinx, Inc. All rights reserved.
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
* @addtogroup v_hdmirxss_v4_0
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
* 1.3   MH     08/08/16 Updates to optimize out HDCP when excluded.
* 1.4   YH     14/11/16 Remove Remapper APIs as remapper feature is moved to
*                       video bridge and controlled by HDMI core
* 1.5   MMO    03/01/16 Remove repetitive inclusion of header files
* </pre>
*
******************************************************************************/
#ifndef XV_HDMIRXSS_COREINIT_H__  /* prevent circular inclusions */
#define XV_HDMIRXSS_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_hdmirxss.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_HdmiRxSs_SubcoreInitHdmiRx(XV_HdmiRxSs *HdmiRxSsPtr);
#ifdef XPAR_XHDCP_NUM_INSTANCES
int XV_HdmiRxSs_SubcoreInitHdcpTimer(XV_HdmiRxSs *HdmiRxSsPtr);
int XV_HdmiRxSs_SubcoreInitHdcp14(XV_HdmiRxSs *HdmiRxSsPtr);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
int XV_HdmiRxSs_SubcoreInitHdcp22(XV_HdmiRxSs *HdmiRxSsPtr);
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
