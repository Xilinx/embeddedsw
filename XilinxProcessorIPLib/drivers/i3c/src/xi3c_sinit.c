/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file XI3c_sinit.c
* @{
*
* The implementation of the XI3c component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------
* 1.00  gm      01/25/24 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xi3c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return	A pointer to the configuration found or NULL if the specified
*		device ID was not found. See xi3c.h for the definition of
*		XI3c_Config.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XI3c_Config *XI3c_LookupConfig(u16 DeviceId)
{
	XI3c_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XI3C_NUM_INSTANCES; Index++) {
		if (XI3c_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XI3c_ConfigTable[Index];
			break;
		}
	}

	return (XI3c_Config *)CfgPtr;
}
#else
XI3c_Config *XI3c_LookupConfig(u32 BaseAddress)
{
	XI3c_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; XI3c_ConfigTable[Index].Name != NULL; Index++) {
		if (XI3c_ConfigTable[Index].BaseAddress == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XI3c_ConfigTable[Index];
			break;
		}
	}

	return (XI3c_Config *)CfgPtr;
}
#endif
/** @} */
