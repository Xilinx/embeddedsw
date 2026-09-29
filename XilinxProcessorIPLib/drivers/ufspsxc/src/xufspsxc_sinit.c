/******************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xufspsxc_sinit.c
* @brief Implements UFSPSXC static initialization.
*
* Implements static initialization for the UFSPSXC driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sk  01/16/24 First release
* 1.3   tb  09/16/26 Align Doxygen comments with AMD standards.
* </pre>
*
******************************************************************************/

/**
* @addtogroup ufspsxc_api UFSPSXC APIs
* @{
*/

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
* @brief Looks up UFSPSXC configuration by base address.
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param[in] BaseAddress Contains the BaseAddress of the device to look up the
*   configuration for.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
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
