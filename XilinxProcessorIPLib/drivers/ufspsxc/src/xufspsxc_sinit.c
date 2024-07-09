/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_sinit.c
* @addtogroup ufspsxc Overview
* @{
*
* The implementation of the XOspiPsv component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sk  01/16/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xufspsxc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	BaseAddress contains the BaseAddress of the device to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified device ID
* 		was not found. See XUfsPsxc.h for the definition of XUfsPsxc_Config.
*
******************************************************************************/
XUfsPsxc_Config *XUfsPsxc_LookupConfig(UINTPTR BaseAddress)
{
	XUfsPsxc_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; XUfsPsxc_ConfigTable[Index].Name != NULL; Index++) {
		if (XUfsPsxc_ConfigTable[Index].BaseAddress == BaseAddress) {
			CfgPtr = &XUfsPsxc_ConfigTable[Index];
			break;
		}
	}
	return (XUfsPsxc_Config *)CfgPtr;
}
/** @} */
