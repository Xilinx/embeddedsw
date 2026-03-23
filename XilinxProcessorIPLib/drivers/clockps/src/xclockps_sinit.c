/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xclockps_sinit.c
* @addtogroup clockps Overview
* @{
*
* This file contains method for static initialization (compile-time) of the
* driver.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  cjp    02/09/18 First release
* 1.7   vlt    12/12/25 Update Doxygen comments to include SDT flow details.
* 1.7   vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xclockps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Variable Definitions ****************************/
/**
 * configuration table defined in xclockps_g.c
 */
#ifndef SDT
extern XClockPs_Config XClockPs_ConfigTable[XPAR_XCLOCKPS_NUM_INSTANCES];
#else
extern XClockPs_Config XClockPs_ConfigTable[];
#endif

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XClockPs_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the
*               specified device ID/BaseAddress was not found. See
*               xclockps.h for the definition of XClockPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XClockPs_Config *XClock_LookupConfig(u16 DeviceId)
{
	XClockPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XCLOCKPS_NUM_INSTANCES; Index++) {
		if (XClockPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XClockPs_ConfigTable[Index];
			break;
		}
	}

	return (XClockPs_Config *)CfgPtr;
}
#else
XClockPs_Config *XClock_LookupConfig(UINTPTR BaseAddress)
{
	XClockPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XClockPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XClockPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XClockPs_ConfigTable[Index];
			break;
		}
	}
	return (XClockPs_Config *)CfgPtr;
}
#endif

/** @} */
