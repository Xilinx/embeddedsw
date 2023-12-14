/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss_sinit.c
* @addtogroup dsitxss Overview
* @{
*
* This file contains the implementation of the MIPI DSI Tx Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/2/16 Initial Release for MIPI DSI TX subsystem
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsitxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XDsiTxSs_Config XDsiTxSs_ConfigTable[];

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XDsiTxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XDsiTxSs_Config* XDsiTxSs_LookupConfig(u32 DeviceId)
{
	XDsiTxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XDSITXSS_NUM_INSTANCES; Index++) {
		if (XDsiTxSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDsiTxSs_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else
XDsiTxSs_Config* XDsiTxSs_LookupConfig(UINTPTR BaseAddress)
{
	XDsiTxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XDsiTxSs_ConfigTable[Index].Name != NULL; Index++) {

		if ((XDsiTxSs_ConfigTable[Index].BaseAddr == BaseAddress) || !BaseAddress) {
			CfgPtr = &XDsiTxSs_ConfigTable[Index];
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

u32 XDsiTxSs_GetDrvIndex(XDsiTxSs *InstancePtr, UINTPTR BaseAddress)
{
 u32 Index = 0;

 for (Index = 0U; XDsiTxSs_ConfigTable[Index].Name != NULL; Index++) {
   if ((XDsiTxSs_ConfigTable[Index].BaseAddr == BaseAddress)) {
	break;
   }
 }
 return Index;
}
#endif
/** @} */
