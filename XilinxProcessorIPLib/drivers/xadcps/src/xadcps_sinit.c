/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xadcps_sinit.c
* @addtogroup Overview
* @{
*
* This file contains the implementation of the XAdcPs driver's static
* initialization functionality.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a ssb    12/22/11 First release based on the XPS/AXI XADC driver
* 2.6   aad    11/02/20 Fix MISRAC Mandatory and Advisory errors.
* 2.7   cog    07/24/23 Added support for SDT flow
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xadcps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XAdcPs_Config XAdcPs_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks up the device configuration based on the unique device ID.
* The table XAdcPs_ConfigTable contains the configuration info for each device
* in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XAdcPs_Config *XAdcPs_LookupConfig(u16 DeviceId)
{
	XAdcPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < 1U; Index++) {
		if (XAdcPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAdcPs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XAdcPs_Config *XAdcPs_LookupConfig(u32 BaseAddress)
{
	XAdcPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; XAdcPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XAdcPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			CfgPtr = &XAdcPs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
