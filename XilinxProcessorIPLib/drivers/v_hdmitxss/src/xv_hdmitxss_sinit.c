/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#ifndef SDT
#include "xparameters.h"
#endif
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
#ifndef SDT
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
#else
XV_HdmiTxSs_Config* XV_HdmiTxSs_LookupConfig(UINTPTR BaseAddress)
{
	XV_HdmiTxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XV_HdmiTxSs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_HdmiTxSs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XV_HdmiTxSs_ConfigTable[Index];
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

u32 XV_HdmiTxSs_GetDrvIndex(UINTPTR BaseAddress)
{
	u32 Index = 0;

	for (Index = 0U; XV_HdmiTxSs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_HdmiTxSs_ConfigTable[Index].BaseAddress == BaseAddress)) {
			break;
		}
	}
	return Index;
}
#endif
/** @} */
