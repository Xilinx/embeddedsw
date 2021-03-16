/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xipipsu_sinit.c
* @addtogroup ipipsu_v2_9
* @{
*
* The implementation of the XIpiPsu component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   mjr  03/15/15 First release
* 2.1   kvn  05/05/16 Modified code for MISRA-C:2012 Compliance
* 2.9   sdd  03/11/21 Moved declaration to header files
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xparameters.h"
#include "xipipsu.h"

/*****************************************************************************/

/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*			configuration for.
*
* @return	A pointer to the configuration found or NULL if the specified
*			device ID was not found. See xipipsu.h for the definition of
*			XIpiPsu_Config.
*
* @note		None.
*
******************************************************************************/
XIpiPsu_Config *XIpiPsu_LookupConfig(u32 DeviceId)
{
	XIpiPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XIPIPSU_NUM_INSTANCES; Index++) {
		if (XIpiPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XIpiPsu_ConfigTable[Index];
			break;
		}
	}

	return (XIpiPsu_Config *) CfgPtr;
}
/** @} */
