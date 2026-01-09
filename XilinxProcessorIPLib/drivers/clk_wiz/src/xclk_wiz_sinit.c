/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclk_wiz_sinit.c
* @addtogroup Overview
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xclk_wiz_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 02/12/16 Initial version for Clock Wizard
* 1.6 sd  7/07/23 Added SDT support.
* 1.11 vlt 12/12/25 Update Doxygen comments to include SDT flow details.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xclk_wiz.h"

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XClk_Wiz_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xclk_wiz.h for the
*               definition of XClk_Wiz_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
*****************************************************************************/
#ifndef SDT
XClk_Wiz_Config *XClk_Wiz_LookupConfig(u32 DeviceId)
{
	extern XClk_Wiz_Config XClk_Wiz_ConfigTable[];
	XClk_Wiz_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XCLK_WIZ_NUM_INSTANCES; Index++) {
		if (XClk_Wiz_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XClk_Wiz_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XClk_Wiz_Config *XClk_Wiz_LookupConfig(UINTPTR BaseAddress)
{
	extern XClk_Wiz_Config XClk_Wiz_ConfigTable[];
	XClk_Wiz_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XClk_Wiz_ConfigTable[Index].Name != NULL; Index++) {
		if (XClk_Wiz_ConfigTable[Index].BaseAddr == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XClk_Wiz_ConfigTable[Index];
			break;
		}
	}

	return (XClk_Wiz_Config *)CfgPtr;
}
#endif
/** @} */
