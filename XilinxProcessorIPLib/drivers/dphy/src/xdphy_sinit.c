/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdphy_sinit.c
*
* @addtogroup dphy Overview
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xdphy_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/09/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdphy.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param 	BaseAddress is the base address of the device to lookup for
 *
 * @return 	The reference to the configuration record in the configuration
 * 		table (in xdphy_g.c) corresponding to the Base Address or if
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
XDphy_Config * XDphy_LookupConfig(UINTPTR BaseAddress)
{
	extern XDphy_Config XDphy_ConfigTable[];
	XDphy_Config *CfgPtr = NULL;
	u32 Index;
	for (Index = 0; XDphy_ConfigTable[Index].Name != NULL; Index++) {
		if ((XDphy_ConfigTable[Index].BaseAddr == BaseAddress) || !BaseAddress) {
			CfgPtr = &XDphy_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
