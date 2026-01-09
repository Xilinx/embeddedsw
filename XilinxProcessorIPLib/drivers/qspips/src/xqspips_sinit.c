/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspips_sinit.c
* @addtogroup qspips Overview
* @{
*
* The implementation of the XQspiPs component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  sdm 11/25/10 First release
* 3.11	akm 07/10/23 Update the driver to support for system device-tree flow.
* 3.15  vlt 12/16/25 Update Doxygen comments to include SDT flow details.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xqspips.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern XQspiPs_Config XQspiPs_ConfigTable[];

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XQspiPs_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xqspips.h for the
*               definition of XQspiPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XQspiPs_Config *XQspiPs_LookupConfig(u16 DeviceId)
{
	XQspiPs_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0; Index < XPAR_XQSPIPS_NUM_INSTANCES; Index++) {
		if (XQspiPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XQspiPs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
#else
XQspiPs_Config *XQspiPs_LookupConfig(UINTPTR BaseAddress)
{
	XQspiPs_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0U; XQspiPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XQspiPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XQspiPs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
#endif
/** @} */
