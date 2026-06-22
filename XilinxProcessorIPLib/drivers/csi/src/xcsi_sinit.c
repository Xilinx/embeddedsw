/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi_sinit.c
* @addtogroup csi Overview
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xcsi_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 06/18/15 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcsi.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	BaseAddress is the base address of the device to lookup for
 *
 * @return	The reference to the configuration record in the configuration
 * 		table (in xcsi_g.c) corresponding to the Base Address or if
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
XCsi_Config *XCsi_LookupConfig(UINTPTR BaseAddress)
{
	extern XCsi_Config XCsi_ConfigTable[];
	XCsi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XCsi_ConfigTable[Index].Name != NULL; Index++) {
		if (XCsi_ConfigTable[Index].BaseAddr == BaseAddress) {
			CfgPtr = &XCsi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
