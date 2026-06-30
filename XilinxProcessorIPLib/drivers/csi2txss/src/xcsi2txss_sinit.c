/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_sinit.c
* @addtogroup csi2txss Overview
* @{
*
* This file contains the implementation of the MIPI CSI Rx Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who   Date     Changes
* --- --- -------- ----------------------------------------------------------
* 1.0 sss 07/21/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XCsi2TxSs_Config XCsi2TxSs_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function looks for the device configuration based on the base
* address. The table XCsi2TxSs_ConfigTable[] contains the configuration
* information for each instance of the device in the system.
*
* @param	BaseAddress is the base address of the device being looked up
*
* @return	A pointer to the configuration table entry corresponding to the
*		given base address, or NULL if no match is found
*
* @note		None.
*
******************************************************************************/
XCsi2TxSs_Config* XCsi2TxSs_LookupConfig(UINTPTR BaseAddress)
{
	XCsi2TxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XCsi2TxSs_ConfigTable[Index].Name != NULL; Index++) {
		if (XCsi2TxSs_ConfigTable[Index].BaseAddr == BaseAddress) {
			CfgPtr = &XCsi2TxSs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
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

u32 XCsi2TxSs_GetDrvIndex(XCsi2TxSs *InstancePtr, UINTPTR BaseAddress)
{
 u32 Index = 0;

 (void)InstancePtr;

 for (Index = 0U; XCsi2TxSs_ConfigTable[Index].Name != NULL; Index++) {
   if ((XCsi2TxSs_ConfigTable[Index].BaseAddr == BaseAddress)) {
	break;
   }
 }
 return Index;
}

/** @} */
