/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file XI3cPsx_sinit.c
* @{
*
* The implementation of the XIicPs component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------
* 1.00  sd      06/10/22 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xi3cpsx.h"

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
*		device ID was not found. See xiicps.h for the definition of
*		XI3cPsx_Config.
*
* @note		None.
*
******************************************************************************/
XI3cPsx_Config *XI3cPsx_LookupConfig(u16 DeviceId)
{
	XI3cPsx_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XI3CPSX_NUM_INSTANCES; Index++) {
		if (XI3cPsx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XI3cPsx_ConfigTable[Index];
			break;
		}
	}

	return (XI3cPsx_Config *)CfgPtr;
}
/** @} */
