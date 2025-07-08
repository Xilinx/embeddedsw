/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss_sinit.c
* @addtogroup csiss Overview
* @{
*
* This file contains the implementation of the MIPI CSI Rx Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ----------------------------------------------------
* 1.0 vsa 07/21/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xcsiss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XCsiSs_Config XCsiSs_ConfigTable[];

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XCsiSs_ConfigTable[] contains the configuration information
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
XCsiSs_Config* XCsiSs_LookupConfig(u32 DeviceId)
{
	XCsiSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XCSISS_NUM_INSTANCES; Index++) {
		if (XCsiSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsiSs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XCsiSs_Config* XCsiSs_LookupConfig(UINTPTR BaseAddress)
{
	XCsiSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XCsiSs_ConfigTable[Index].Name != NULL; Index++) {
		if (XCsiSs_ConfigTable[Index].BaseAddr == BaseAddress) {
			CfgPtr = &XCsiSs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/*****************************************************************************/
/**
* This function returns the Index number of config table using BaseAddress.
*
* @param  BaseAddress is base address of the instance
*
* @return Index number of the config table
*
*
*******************************************************************************/

u32 XCsiSs_GetDrvIndex(UINTPTR BaseAddress)
{
 u32 Index = 0;

 for (Index = 0U; XCsiSs_ConfigTable[Index].Name != NULL; Index++) {
   if (XCsiSs_ConfigTable[Index].BaseAddr == BaseAddress) {
	break;
   }
 }
 return Index;
}

#endif
/** @} */
