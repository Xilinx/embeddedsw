/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_sditx_sinit.c
*
* This file contains static initialization method for Xilinx SDI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  jsr    07/17/17 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_sditx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
extern XV_SdiTx_Config XV_SdiTx_ConfigTable[];
/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XV_SdiTx_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_sditx_g.c file.
*
* @param    DeviceId is the unique core ID of the SDI TX core for the
*       lookup operation.
*
* @return   XV_SdiTx_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_sditx_g.c) corresponding
*       to <i>DeviceId</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_SdiTx_Config *XV_SdiTx_LookupConfig(UINTPTR BaseAddress)
{
	XV_SdiTx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0U; XV_SdiTx_ConfigTable[Index].Name;
	Index++) {
		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XV_SdiTx_ConfigTable[Index].BaseAddress == BaseAddress ||
		!BaseAddress) {
			CfgPtr = &XV_SdiTx_ConfigTable[Index];
			break;
		}
	}

	return (XV_SdiTx_Config *)CfgPtr;
}
