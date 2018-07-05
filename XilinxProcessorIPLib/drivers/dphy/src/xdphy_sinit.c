/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdphy_sinit.c
*
* @addtogroup dphy_v1_5
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

#include "xparameters.h"
#include "xdphy.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param 	DeviceId is the unique device ID of the device to lookup for
 *
 * @return 	The reference to the configuration record in the configuration
 * 		table (in xdphy_g.c) corresponding to the Device ID or if
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
XDphy_Config * XDphy_LookupConfig(u32 DeviceId)
{
	extern XDphy_Config XDphy_ConfigTable[];
	XDphy_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XDPHY_NUM_INSTANCES; Index++) {
		if (XDphy_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDphy_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
