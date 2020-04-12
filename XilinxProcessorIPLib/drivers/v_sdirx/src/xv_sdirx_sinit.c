/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sdirx_sinit.c
*
* This file contains static initialization method for Xilinx SDI RX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.0   jsr    07/17/17 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sdirx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_SdiRx_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_sdirx_g.c file.
*
* @param    DeviceId is the unique core ID of the SDI RX core for the
*       lookup operation.
*
* @return   XV_SdiRx_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_sdirx_g.c) corresponding
*       to <i>DeviceId</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_SdiRx_Config *XV_SdiRx_LookupConfig(u16 DeviceId)
{
	extern XV_SdiRx_Config
	XV_SdiRx_ConfigTable[XPAR_XV_SDIRX_NUM_INSTANCES];
	XV_SdiRx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XV_SDIRX_NUM_INSTANCES);
	Index++) {

		/* Assigning address of config table if both device ids
		* are matched
		*/
		if (XV_SdiRx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XV_SdiRx_ConfigTable[Index];
			break;
		}
	}

	return (XV_SdiRx_Config *)CfgPtr;
}
