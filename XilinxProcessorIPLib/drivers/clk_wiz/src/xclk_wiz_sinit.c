/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return	The reference to the configuration record in the configuration
 * 		table (in xclk_wiz_g.c) corresponding to the Device ID or if
 *		not found, a NULL pointer is returned.
 *
 * @note	None
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
