/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtrafgen_sinit.c
* @addtogroup trafgen Overview
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

#include "xtrafgen.h"
#ifndef SDT
#include "xparameters.h"
#endif

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
#ifndef SDT
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
#else
XTrafGen_Config *XTrafGen_LookupConfig(UINTPTR BaseAddress)
{
	extern XTrafGen_Config XTrafGen_ConfigTable[];
	XTrafGen_Config *CfgPtr;
	u32 Index;

	CfgPtr = NULL;

	for (Index = 0; XTrafGen_ConfigTable[Index].Name != NULL; Index++) {
		if ((XTrafGen_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XTrafGen_ConfigTable[Index];
			break;
		}
	}

	return (XTrafGen_Config *) CfgPtr;
}
#endif
/** @} */
