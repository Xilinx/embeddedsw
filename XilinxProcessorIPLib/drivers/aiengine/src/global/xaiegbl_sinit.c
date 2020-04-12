/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl_sinit.c
* @{
*
* This file contains the global look up configuration function for the
* AIE device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  06/18/2018  Updated code to be inline with standalone driver
* 1.2  Naresh  07/11/2018  Updated copyright info
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_Config XAieGbl_ConfigTable[XPAR_AIE_NUM_INSTANCES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique identifier for a device.
*
* @return	A pointer to the XAieGbl configuration structure for the
*		specified device, or NULL if the device was not found.
*
* @note		None.
*
******************************************************************************/
XAieGbl_Config *XAieGbl_LookupConfig(u16 DeviceId)
{
	XAieGbl_Config *CfgPtr = XAIE_NULL;
	u32 Index;

	for (Index=0U; Index < (u32)XPAR_AIE_NUM_INSTANCES; Index++) {
		if (XAieGbl_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAieGbl_ConfigTable[Index];
			break;
		}
	}

	return (XAieGbl_Config *)CfgPtr;
}
/** @} */
