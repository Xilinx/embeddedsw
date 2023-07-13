/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
* Lookup the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
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
XClockPs_Config *XClock_LookupConfig(u32 BaseAddress)
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
