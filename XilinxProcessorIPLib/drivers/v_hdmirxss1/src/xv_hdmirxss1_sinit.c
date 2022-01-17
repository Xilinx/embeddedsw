/******************************************************************************
# Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1_sinit.c
* @addtogroup v_hdmirxss1_v3_1
* @{
* @details
*
* This file contains the implementation of the HDMI RX Subsystem
* driver's static initialization functionality.
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

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xv_hdmirxss1.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XV_HdmiRxSs1_Config XV_HdmiRxSs1_ConfigTable[];

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XV_HdmiRxSs1_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
XV_HdmiRxSs1_Config* XV_HdmiRxSs1_LookupConfig(u32 DeviceId)
{
  XV_HdmiRxSs1_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; Index < (u32)XPAR_XV_HDMIRXSS1_NUM_INSTANCES; Index++)
  {
    if (XV_HdmiRxSs1_ConfigTable[Index].DeviceId == DeviceId)
    {
      CfgPtr = &XV_HdmiRxSs1_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}
/** @} */
