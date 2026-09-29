/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmon_sinit.c
*
* @brief Implements the static initialization functionality for the driver.
*
* This file contains the implementation of the XSysMon driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 7.8   cog    07/20/23 Added support for SDT flow
* 8.1   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xsysmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @brief Configuration table for each System Monitor/ADC device in the system.
 */
extern XSysMon_Config XSysMon_ConfigTable[];

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the unique device ID.
*
* The table XSysMon_ConfigTable contains the configuration info for each device
* in the system.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId ID of the device for which the
*              device configuration pointer is to be returned.
* @endif
*
* @return
*              - A pointer to the configuration found.
*              - NULL if the specified device ID was not found.
*
******************************************************************************/
#ifndef SDT
XSysMon_Config *XSysMon_LookupConfig(u16 DeviceId)
{
	XSysMon_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XSYSMON_NUM_INSTANCES; Index++) {
		if (XSysMon_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSysMon_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XSysMon_Config *XSysMon_LookupConfig(u32 BaseAddress)
{
	XSysMon_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; XSysMon_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSysMon_ConfigTable[Index].BaseAddress == BaseAddress)  ||
			!BaseAddress) {
			CfgPtr = &XSysMon_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
