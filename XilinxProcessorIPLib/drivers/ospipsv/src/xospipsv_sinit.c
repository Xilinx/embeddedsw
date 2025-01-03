/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_sinit.c
* @addtogroup ospipsv_api OSPIPSV APIs
* @{
*
* The xospipsv_sinit.c file contains implementation of the XOspiPsv component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   hk  02/19/18 First release
* 1.6   sk  02/07/22 Replaced driver version in addtogroup with Overview.
* 1.9   sb  06/06/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif

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
* @param	DeviceId Contains the ID of the device to look up the
*		configuration for.
*
* @return
*		A pointer to the configuration found or NULL if the specified device ID
* 		was not found. See XOspiPsv.h for the definition of XOspiPsv_Config.
*
******************************************************************************/
#ifndef SDT
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId)
{
	XOspiPsv_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; Index < XPAR_XOSPIPSV_NUM_INSTANCES; Index++) {
		if (XOspiPsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XOspiPsv_ConfigTable[Index];
			break;
		}
	}
	return (XOspiPsv_Config *)CfgPtr;
}
#else
XOspiPsv_Config *XOspiPsv_LookupConfig(UINTPTR BaseAddress)
{
	XOspiPsv_Config *CfgPtr = NULL;
	s32 Index;

	for (Index = 0; XOspiPsv_ConfigTable[Index].Name != NULL; Index++) {
		if (XOspiPsv_ConfigTable[Index].BaseAddress == BaseAddress) {
			CfgPtr = &XOspiPsv_ConfigTable[Index];
			break;
		}
	}
	return (XOspiPsv_Config *)CfgPtr;
}
#endif

/** @} */
