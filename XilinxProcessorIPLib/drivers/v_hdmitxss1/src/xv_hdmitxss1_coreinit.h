/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1_coreinit.h
* @addtogroup v_hdmitxss1_v2_0
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
