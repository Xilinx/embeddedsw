/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditxss_sinit.c
*
* This file contains static initialization method for Xilinx SDI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  jsr    07/17/17 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sditxss.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
extern XV_SdiTxSs_Config XV_SdiTxSs_ConfigTable[];
/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XV_SdiTxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param  DeviceId is the unique device ID of the device being looked up
*
* @return A pointer to the configuration table entry corresponding to the
*         given device ID, or NULL if no match is found
*
*******************************************************************************/
#ifndef SDT
XV_SdiTxSs_Config *XV_SdiTxSs_LookupConfig(u32 DeviceId)
{
	XV_SdiTxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XV_SDITXSS_NUM_INSTANCES; Index++) {
		if (XV_SdiTxSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XV_SdiTxSs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
#else
XV_SdiTxSs_Config *XV_SdiTxSs_LookupConfig(UINTPTR BaseAddress)
{
	XV_SdiTxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XV_SdiTxSs_ConfigTable[Index].Name; Index++) {
		if (XV_SdiTxSs_ConfigTable[Index].BaseAddress == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XV_SdiTxSs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}

/*****************************************************************************/
/**
 * This function returns the Index number of config table using BaseAddress
 *
 * @param A pointer to the instance structure
 *
 * @param Base address of the instance
 *
 * @return Index number of the config table
 *
 *****************************************************************************/
u32 XV_SdiTxSs_GetDrvIndex(XV_SdiTxSs *InstancePtr, UINTPTR BaseAddress)
{
	u32 Index = 0;

	for (Index = 0U; XV_SdiTxSs_ConfigTable[Index].Name; Index++) {
		if (XV_SdiTxSs_ConfigTable[Index].BaseAddress == BaseAddress)
			break;
	}
	return Index;
}
#endif
/** @} */
