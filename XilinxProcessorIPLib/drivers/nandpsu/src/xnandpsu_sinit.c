/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandpsu_sinit.c
* @addtogroup Overview
* @{
*
* The implementation of the XNandPsu driver's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* 1.12  akm    06/27/23    Update the driver to support for system device-tree flow.
* 1.16  vlt    12/15/25    Update Doxygen comments to include SDT flow details.
* </pre>
*
******************************************************************************/

/***************************** Include Files ********************************/
#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xnandpsu.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

extern XNandPsu_Config XNandPsu_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XNandPsu_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the
*               specified device ID/BaseAddress was not found. See
*               xnandpsu.h for the definition of XNandPsu_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XNandPsu_Config *XNandPsu_LookupConfig(u16 DevID)
{
	XNandPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XNANDPSU_NUM_INSTANCES; Index++) {
		if (XNandPsu_ConfigTable[Index].DeviceId == DevID) {
			CfgPtr = &XNandPsu_ConfigTable[Index];
			break;
		}
	}

	return (XNandPsu_Config *)CfgPtr;
}
#else
XNandPsu_Config *XNandPsu_LookupConfig(UINTPTR BaseAddress)
{
	XNandPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XNandPsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XNandPsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XNandPsu_ConfigTable[Index];
			break;
		}
	}

	return (XNandPsu_Config *)CfgPtr;
}
#endif
/** @} */
