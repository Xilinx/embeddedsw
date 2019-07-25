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
* @file xv_hdmitxss1_coreinit.h
* @addtogroup v_hdmitxss1_v4_0
* @{
* @details
*
* This header file contains the hdmi tx subsystem sub-cores
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
#ifndef XV_HDMITXSS1_COREINIT_H__  /* prevent circular inclusions */
#define XV_HDMITXSS1_COREINIT_H__  /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xv_hdmitxss1.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
int XV_HdmiTxSs1_SubcoreInitHdmiTx1(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
int XV_HdmiTxSs1_SubcoreInitVtc(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
#ifdef XPAR_XHDCP_NUM_INSTANCES
int XV_HdmiTxSs1_SubcoreInitHdcpTimer(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
int XV_HdmiTxSs1_SubcoreInitHdcp14(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
int XV_HdmiTxSs1_SubcoreInitHdcp22(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
