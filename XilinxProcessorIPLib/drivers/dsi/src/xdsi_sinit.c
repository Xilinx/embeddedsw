/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi_sinit.c
* @addtogroup dsi_v1_3
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xdsi_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0 ram 02/11/16 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xdsi.h"

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return	The reference to the configuration record in the configuration
 * 		table (in xdsi_g.c) corresponding to the Device ID or if
 *		not found, a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
XDsi_Config *XDsi_LookupConfig(u32 DeviceId)
{
	extern XDsi_Config XDsi_ConfigTable[];
	XDsi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XDSI_NUM_INSTANCES; Index++) {
		if (XDsi_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDsi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
