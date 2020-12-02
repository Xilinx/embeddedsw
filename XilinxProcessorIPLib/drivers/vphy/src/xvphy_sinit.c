/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvphy_sinit.c
 *
 * This file contains static initialization methods for the XVphy driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xvphy.h"
#include "xvphy_i.h"

/*************************** Variable Declarations ****************************/

#ifndef XPAR_XVPHY_NUM_INSTANCES
#define XPAR_XVPHY_NUM_INSTANCES 0
#endif

/**
 * A table of configuration structures containing the configuration information
 * for each Video PHY core in the system.
 */
extern XVphy_Config XVphy_ConfigTable[XPAR_XVPHY_NUM_INSTANCES];

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XVphy_ConfigTable[] contains the configuration information for
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
XVphy_Config *XVphy_LookupConfig(u16 DeviceId)
{
	XVphy_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XVPHY_NUM_INSTANCES; Index++) {
		if (XVphy_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XVphy_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
