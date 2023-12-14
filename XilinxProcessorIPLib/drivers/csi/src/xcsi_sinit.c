/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
#ifndef SDT
#include "xparameters.h"
#endif
#include "xcsi.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return	The reference to the configuration record in the configuration
 * 		table (in xcsi_g.c) corresponding to the Device ID or if
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
#ifndef SDT
XCsi_Config *XCsi_LookupConfig(u32 DeviceId)
{
	extern XCsi_Config XCsi_ConfigTable[];
	XCsi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XCSI_NUM_INSTANCES; Index++) {
		if (XCsi_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
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
#endif
/** @} */
