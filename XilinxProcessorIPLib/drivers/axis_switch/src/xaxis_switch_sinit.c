/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch_sinit.c
* @addtogroup axis_switch Overview
* @{
*
* This file contains static initialization method for Xilinx AXI4-Stream
* Source Control Router core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.00  sha 07/15/15 Defined macro XPAR_XAXIS_SWITCH_NUM_INSTANCES if not
*                    defined in xparameters.h
* 1.2   ms  02/20/17 Fixed compilation warning. This is a fix for CR-969126.
* 1.8  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxis_switch.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

#ifndef XPAR_XAXIS_SWITCH_NUM_INSTANCES
#define XPAR_XAXIS_SWITCH_NUM_INSTANCES			0
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XAxis_Switch_ConfigTable[] contains the configuration info for each
* device in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the
*               specified device ID/BaseAddress was not found. See
*               xaxis_switch.h for the definition of XAxis_Switch_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XAxis_Switch_Config *XAxisScr_LookupConfig(u16 DeviceId)
{
	extern XAxis_Switch_Config
		XAxis_Switch_ConfigTable[XPAR_XAXIS_SWITCH_NUM_INSTANCES];
	XAxis_Switch_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; Index < XPAR_XAXIS_SWITCH_NUM_INSTANCES;
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XAxis_Switch_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxis_Switch_ConfigTable[Index];
			break;
		}
	}

	return (XAxis_Switch_Config *)CfgPtr;
}
#else
XAxis_Switch_Config *XAxisScr_LookupConfig(UINTPTR BaseAddress)
{
	extern XAxis_Switch_Config
		XAxis_Switch_ConfigTable[];
	XAxis_Switch_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; XAxis_Switch_ConfigTable[Index].Name;
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if ((XAxis_Switch_ConfigTable[Index].BaseAddress == BaseAddress) ||
			!BaseAddress) {
			CfgPtr = &XAxis_Switch_ConfigTable[Index];
			break;
		}
	}

	return (XAxis_Switch_Config *)CfgPtr;
}
#endif
/** @} */
