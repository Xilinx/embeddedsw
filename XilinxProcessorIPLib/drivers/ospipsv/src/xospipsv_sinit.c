/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 1.14  vlt 12/15/25 Update Doxygen comments to include SDT flow details.
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
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XOspiPsv_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xospipsv.h for the
*               definition of XOspiPsv_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
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
