/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_sinit.c
 *
 * This file contains static initialization methods for the XHdmiphy1 driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"

/*************************** Variable Declarations ****************************/

#ifndef XPAR_XHDMIPHY1_NUM_INSTANCES
#define XPAR_XHDMIPHY1_NUM_INSTANCES 0
#endif

/**
 * A table of configuration structures containing the configuration information
 * for each Video PHY core in the system.
 */
extern XHdmiphy1_Config XHdmiphy1_ConfigTable[XPAR_XHDMIPHY1_NUM_INSTANCES];

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XHdmiphy1_ConfigTable[] contains the configuration information
 * for each device in the system.
 *
 * @param	DeviceId is the unique device ID of the device being looked up.
 *
 * @return	A pointer to the configuration table entry corresponding to the
 *		given device ID, or NULL if no match is found.
 *
 * @note	None.
 *
*******************************************************************************/
XHdmiphy1_Config *XHdmiphy1_LookupConfig(u16 DeviceId)
{
	XHdmiphy1_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XHDMIPHY1_NUM_INSTANCES; Index++) {
		if (XHdmiphy1_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdmiphy1_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
