/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xdfxasm_sinit.c
* @addtogroup dfxasm Overview
* @{
*
* This file contains the implementation of the Xdfxasm driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date          Changes
* ----- ----- -----------   ---------------------------------------------
* 1.0   dp    07/14/20      First release
* 1.2   Nava  06/22/23      Added support for system device-tree flow.
* 1.3   bdk   12/08/25      Updated comments to support SDT flow for Doxygen
*                           documentation.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdfxasm.h"
#ifndef SDT
#include "xparameters.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifndef SDT
extern XDfxasm_Config XDfxasm_ConfigTable[XPAR_XDFXASM_NUM_INSTANCES];
#else
extern XDfxasm_Config XDfxasm_ConfigTable[];
#endif
/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID/BaseAddress. The table XDfxasm_ConfigTable[] contains the configuration
* information for each device in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId is the unique device ID of the device being looked up.
* @endif
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID/BaseAddress, or NULL if no match is found.
*
* @note		In XSCT/classic flow, DeviceId is used to look up the device
*		configuration.
*
******************************************************************************/
#ifndef SDT
XDfxasm_Config *XDfxasm_LookupConfig(u16 DeviceId)
{
	XDfxasm_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XDFXASM_NUM_INSTANCES;
	     Index++) {
		if (XDfxasm_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDfxasm_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XDfxasm_Config *XDfxasm_LookupConfig(UINTPTR BaseAddress)
{
	XDfxasm_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XDfxasm_ConfigTable[Index].Name != NULL;
	     Index++) {
		if ((XDfxasm_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XDfxasm_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
