/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsrio_sinit.c
* @addtogroup srio Overview
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
* 1.6   adk  06/02/24 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"
#endif
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
#ifndef SDT
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
#else
XSrio_Config *XSrio_LookupConfig(UINTPTR BaseAddress)
{
	extern XSrio_Config XSrio_ConfigTable[];
	XSrio_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0U; XSrio_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSrio_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {

			CfgPtr = &XSrio_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
