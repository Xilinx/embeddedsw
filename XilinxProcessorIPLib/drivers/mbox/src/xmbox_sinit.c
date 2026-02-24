/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmbox_sinit.c
* @addtogroup mbox Overview
* @{
*
* Implements static initialization
* See xmbox.h for more information about the component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 4.2   ms   08/07/17 Fixed compilation warnings.
* 4.6   ht   07/06/23 Added support for system device-tree flow.
* 4.10  vlt  12/14/25 Update Doxygen comments to include SDT flow details.
* 4.10  ht   02/24/26 Fix doxygen warnings
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmbox.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XMbox_Config XMbox_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XMbox_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xmbox.h for the
*               definition of XMbox_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XMbox_Config *XMbox_LookupConfig(u16 DeviceId)
{
	XMbox_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XMBOX_NUM_INSTANCES; Index++) {
		if (XMbox_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMbox_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XMbox_Config *XMbox_LookupConfig(UINTPTR BaseAddress)
{
	XMbox_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XMbox_ConfigTable[Index].Name != NULL; Index++) {
		if ((XMbox_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XMbox_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
