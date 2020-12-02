/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsrio_sinit.c
* @addtogroup srio_v1_4
* @{
* 
* This file contains static Initialization functionality for Xilinx SRIO Gen2
* Core driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   adk  16/04/14 Initial release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xsrio.h"

/*****************************************************************************/
/**
 * Looks up the device configuration based on the unique device ID. The table
 * XSrio_ConfigTable contains the configuration info for each device in the 
 * system.
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return
 *		The configuration structure for the device. If the device ID is
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 ******************************************************************************/
XSrio_Config *XSrio_LookupConfig(u32 DeviceId)
{
	extern XSrio_Config XSrio_ConfigTable[];
	XSrio_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; Index < XPAR_XSRIO_NUM_INSTANCES; Index++) {
		if (XSrio_ConfigTable[Index].DeviceId == DeviceId) {

			CfgPtr = &XSrio_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
