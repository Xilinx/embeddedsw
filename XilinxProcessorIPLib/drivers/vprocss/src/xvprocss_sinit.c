/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_sinit.c
* @addtogroup vprocss_v2_10
* @{
* @details
*
* This file contains the implementation of the Video Processing Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xvprocss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XVprocSs_Config XVprocSs_ConfigTable[];

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XVprocSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
XVprocSs_Config* XVprocSs_LookupConfig(u32 DeviceId)
{
  XVprocSs_Config *CfgPtr = NULL;
  u32 index;

  for (index = 0U; index < (u32)XPAR_XVPROCSS_NUM_INSTANCES; index++)
  {
    if (XVprocSs_ConfigTable[index].DeviceId == DeviceId)
    {
      CfgPtr = &XVprocSs_ConfigTable[index];
      break;
    }
  }
  return (CfgPtr);
}
/** @} */
