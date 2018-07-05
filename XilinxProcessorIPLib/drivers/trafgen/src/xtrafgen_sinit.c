/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrafgen_sinit.c
* @addtogroup trafgen_v4_3
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
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xtrafgen.h"

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
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
/** @} */
