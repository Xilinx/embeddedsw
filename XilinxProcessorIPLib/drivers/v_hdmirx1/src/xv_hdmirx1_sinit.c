/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirx1_sinit.c
*
* This file contains static initialization method for Xilinx HDMI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     02/05/19 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmirx1.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XV_HdmiRx1_Config XV_HdmiRx1_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_HdmiRx1_Config structure based
* on the base address, <i>BaseAddress</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_hdmirx1_g.c file.
*
* @param    BaseAddress is the unique base address of the HDMI RX core for the
*       lookup operation.
*
* @return   XV_HdmiRx1_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_hdmirx1_g.c) corresponding
*       to <i>BaseAddress</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_HdmiRx1_Config* XV_HdmiRx1_LookupConfig(UINTPTR BaseAddress)
{
  XV_HdmiRx1_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; XV_HdmiRx1_ConfigTable[Index].Name != NULL; Index++) {
    if ((XV_HdmiRx1_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
      CfgPtr = &XV_HdmiRx1_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}
