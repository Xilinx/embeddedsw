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
* @file xv_hdmirxss1_coreinit.h
* @addtogroup v_hdmirxss1_v4_0
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
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XV_HDMIRXSS1_COREINIT_H__  /* prevent circular inclusions */
#define XV_HDMIRXSS1_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_hdmirxss1.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_HdmiRxSs1_SubcoreInitHdmiRx1(XV_HdmiRxSs1 *HdmiRxSs1Ptr);
#ifdef XPAR_XHDCP_NUM_INSTANCES
int XV_HdmiRxSs1_SubcoreInitHdcpTimer(XV_HdmiRxSs1 *HdmiRxSs1Ptr);
int XV_HdmiRxSs1_SubcoreInitHdcp14(XV_HdmiRxSs1 *HdmiRxSs1Ptr);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
int XV_HdmiRxSs1_SubcoreInitHdcp22(XV_HdmiRxSs1 *HdmiRxSs1Ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
