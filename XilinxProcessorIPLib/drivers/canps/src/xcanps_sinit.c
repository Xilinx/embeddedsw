/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanps_sinit.c
* @addtogroup canps Overview
* @{
*
* This file contains the implementation of the XCanPs driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.5	sne    07/01/20 Fixed MISRAC warnings.
* 3.7	ht     06/28/23 Added support for system device-tree flow.
* 3.12  vlt    12/12/25 Update Doxygen comments to include SDT flow details.
* 3.12  vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR to
* 			support 64-bit addressing.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcanps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XCanPs_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xcanps.h for the
*               definition of XCanPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XCanPs_Config *XCanPs_LookupConfig(u16 DeviceId)
{
	XCanPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XCANPS_NUM_INSTANCES; Index++) {
		if (XCanPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCanPs_ConfigTable[Index];
			break;
		}
	}

	return (XCanPs_Config *)CfgPtr;
}
#else
XCanPs_Config *XCanPs_LookupConfig(UINTPTR BaseAddress)
{
	XCanPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XCanPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCanPs_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XCanPs_ConfigTable[Index];
			break;
		}
	}

	return (XCanPs_Config *)CfgPtr;
}

#endif
/** @} */
