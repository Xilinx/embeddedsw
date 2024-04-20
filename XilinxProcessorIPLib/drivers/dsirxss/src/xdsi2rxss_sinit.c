/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss_sinit.c
* @addtogroup dsirxss Overview
* @{
*
* This file contains the implementation of the MIPI DSI Rx Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date    Changes
* --- --- ------- -------------------------------------------------------
* 1.0 Kunal 12/2/24 Initial Release for MIPI DSI RX subsystem
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdsi2rxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XDsi2RxSs_Config XDsi2RxSs_ConfigTable[];

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XDsi2RxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param	BaseAddress is the unique address of the device being looked up
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found
*
* @note		None.
*
******************************************************************************/
XDsi2RxSs_Config* XDsi2RxSs_LookupConfig(UINTPTR BaseAddress)
{
	XDsi2RxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XDsi2RxSs_ConfigTable[Index].Name != NULL; Index++) {

		if ((XDsi2RxSs_ConfigTable[Index].BaseAddr == BaseAddress) || !BaseAddress) {
			CfgPtr = &XDsi2RxSs_ConfigTable[Index];
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

u32 XDsi2RxSs_GetDrvIndex(XDsi2RxSs *InstancePtr, UINTPTR BaseAddress)
{
 u32 Index = 0;

 for (Index = 0U; XDsi2RxSs_ConfigTable[Index].Name != NULL; Index++) {
   if ((XDsi2RxSs_ConfigTable[Index].BaseAddr == BaseAddress)) {
	break;
   }
 }
 return Index;
}
/** @} */
