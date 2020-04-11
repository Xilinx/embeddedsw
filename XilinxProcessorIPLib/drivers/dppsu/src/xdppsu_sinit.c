/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_sinit.c
 *
 * This file contains static initialization methods for the XDpPsu driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  05/17/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdppsu.h"
#include "xparameters.h"

/*************************** Variable Declarations ****************************/
/*************************** Constant Declarations ****************************/
#define XDPPSU_NUM_INSTANCES	1
/**
 * A table of configuration structures containing the configuration information
 * for each DisplayPort TX core in the system.
 */
extern XDpPsu_Config XDpPsu_ConfigTable[XDPPSU_NUM_INSTANCES];

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDpPsu_ConfigTable[] contains the configuration information for
 * each device in the system.
 *
 * @param	DeviceId is the unique device ID of the device being looked up.
 *
 * @return	A pointer to the configuration table entry corresponding to the
 *		given device ID, or NULL if no match is found.
 *
 * @note	None.
 *
*******************************************************************************/
XDpPsu_Config *XDpPsu_LookupConfig(u16 DeviceId)
{
	XDpPsu_Config *CfgPtr;
	u32 Index;

	for (Index = 0; Index < XDPPSU_NUM_INSTANCES; Index++) {
		if (XDpPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
