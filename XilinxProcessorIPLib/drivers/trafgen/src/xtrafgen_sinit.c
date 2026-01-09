/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrafgen_sinit.c
* @addtogroup trafgen Overview
* @{
*
* This file contains static initialization functionality for Axi Traffic
* Generator driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a srt  01/24/13 First release
* 4.9  vlt  12/18/25 Update Doxygen comments to include SDT flow details.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtrafgen.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XTrafGen_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return      A pointer to the configuration found or NULL if the specified
*              device ID/BaseAddress was not found. See xtrafgen.h for the
*              definition of XTrafGen_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XTrafGen_Config *XTrafGen_LookupConfig(u32 DeviceId)
{
	extern XTrafGen_Config XTrafGen_ConfigTable[];
	XTrafGen_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XTRAFGEN_NUM_INSTANCES; Index++) {
		if (XTrafGen_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XTrafGen_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XTrafGen_Config *XTrafGen_LookupConfig(UINTPTR BaseAddress)
{
	extern XTrafGen_Config XTrafGen_ConfigTable[];
	XTrafGen_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; XTrafGen_ConfigTable[Index].Name != NULL; Index++) {
		if ((XTrafGen_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XTrafGen_ConfigTable[Index];
			break;
		}
	}

	return (XTrafGen_Config *) CfgPtr;
}
#endif
/** @} */
