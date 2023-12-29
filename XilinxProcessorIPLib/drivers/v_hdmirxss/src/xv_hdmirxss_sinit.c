/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss_sinit.c
* @addtogroup v_hdmirxss Overview
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
* 1.00         10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xv_hdmirxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XV_HdmiRxSs_Config XV_HdmiRxSs_ConfigTable[];

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XV_HdmiRxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
#ifndef SDT
XV_HdmiRxSs_Config* XV_HdmiRxSs_LookupConfig(u32 DeviceId)
{
  XV_HdmiRxSs_Config *CfgPtr = NULL;
  u32 Index;

  for (Index = 0U; Index < (u32)XPAR_XV_HDMIRXSS_NUM_INSTANCES; Index++)
  {
    if (XV_HdmiRxSs_ConfigTable[Index].DeviceId == DeviceId)
    {
      CfgPtr = &XV_HdmiRxSs_ConfigTable[Index];
      break;
    }
  }
  return (CfgPtr);
}
#else
XV_HdmiRxSs_Config* XV_HdmiRxSs_LookupConfig(UINTPTR BaseAddress)
{
	XV_HdmiRxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XV_HdmiRxSs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_HdmiRxSs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XV_HdmiRxSs_ConfigTable[Index];
			break;
		}
	}
	return (CfgPtr);
}

/*****************************************************************************/
/**
* This function returns the Index number of config table using BaseAddress.
*
* @param  A pointer to the instance structure
*
* @param  Base address of the instance
*
* @return Index number of the config table
*
*
*******************************************************************************/

u32 XV_HdmiRxSs_GetDrvIndex(XV_HdmiRxSs *InstancePtr, UINTPTR BaseAddress)
{
	u32 Index = 0;

	for (Index = 0U; XV_HdmiRxSs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_HdmiRxSs_ConfigTable[Index].BaseAddress == BaseAddress)) {
			break;
		}
	}
	return Index;
}
#endif
/** @} */
