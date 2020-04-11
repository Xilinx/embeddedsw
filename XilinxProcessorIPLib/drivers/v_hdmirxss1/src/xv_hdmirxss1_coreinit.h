/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
