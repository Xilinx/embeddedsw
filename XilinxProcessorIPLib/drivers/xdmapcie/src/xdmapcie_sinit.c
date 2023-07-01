/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_sinit.c
*
* This file contains the implementation of XDMA PCIe driver's static
* initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/

#include "xparameters.h"
#include "xdmapcie.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XDmaPcie_Config XDmaPcie_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param 	DeviceId is the device identifier to lookup.
*
* @return
* 		- XDmaPcie configuration structure pointer if DeviceID is
*		found.
* 		- NULL if DeviceID is not found.
*
* @note		None
*
******************************************************************************/
#ifndef SDT
XDmaPcie_Config *XDmaPcie_LookupConfig(u16 DeviceId)
{
	XDmaPcie_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XDMAPCIE_NUM_INSTANCES; Index++) {
		if (XDmaPcie_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDmaPcie_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else
XDmaPcie_Config *XDmaPcie_LookupConfig(UINTPTR BaseAddress)
{
	XDmaPcie_Config *CfgPtr = NULL;

	int Index;

	for (Index = 0; XDmaPcie_ConfigTable[Index].Name != NULL; Index++) {
		if (XDmaPcie_ConfigTable[Index].BaseAddress == BaseAddress) {
			CfgPtr = &XDmaPcie_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#endif
