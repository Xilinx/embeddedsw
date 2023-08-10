/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss_sinit.c
* @addtogroup v_hdmitxss Overview
* @{
* @details
*
* This file contains the implementation of the HDMI TX Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xv_hdmitxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XV_HdmiTxSs_Config XV_HdmiTxSs_ConfigTable[];

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XV_HdmiTxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
XV_HdmiTxSs_Config* XV_HdmiTxSs_LookupConfig(u32 DeviceId)
{
  XV_HdmiTxSs_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; Index < (u32)XPAR_XV_HDMITXSS_NUM_INSTANCES; Index++) {
    if (XV_HdmiTxSs_ConfigTable[Index].DeviceId == DeviceId) {
      CfgPtr = &XV_HdmiTxSs_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}
/** @} */
