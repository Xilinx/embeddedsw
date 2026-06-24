/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_sinit.c
*
* This file contains static initialization method for Xilinx HDMI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx1.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XV_HdmiTx1_Config XV_HdmiTx1_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_HdmiTx1_Config structure based
* on the base address, <i>BaseAddress</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_hdmitx1_g.c file.
*
* @param    BaseAddress is the unique base address of the HDMI TX core for the
*       lookup operation.
*
* @return   XV_HdmiTx1_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_hdmitx1_g.c) corresponding
*       to <i>BaseAddress</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx1_Config* XV_HdmiTx1_LookupConfig(UINTPTR BaseAddress)
{
  XV_HdmiTx1_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; XV_HdmiTx1_ConfigTable[Index].Name != NULL; Index++) {
    if ((XV_HdmiTx1_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
      CfgPtr = &XV_HdmiTx1_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}
