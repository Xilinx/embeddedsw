/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmonpsv_sint.c
* @addtogroup pmonpsv_v2_1
* @{
*
* This file contains the implementation of the XPmonPsv driver's static
* initialization functionality.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd    01/20/19 First release
* 2.0 sd    04/22/20  Rename the APIs
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xpmonpsv.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XPmonPsv_Config XPmonPsv_ConfigTable[XPAR_XPMONPSV_NUM_INSTANCES];

/*****************************************************************************/
/**
*
* This function looks up the device configuration based on the unique device ID.
* The table XPmonPsv_ConfigTable contains the configuration info for each device
* in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
* @note		None.
*
******************************************************************************/
XPmonPsv_Config *XPmonPsv_LookupConfig(u16 DeviceId)
{
	XPmonPsv_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0U; Index < (u32)XPAR_XPMONPSV_NUM_INSTANCES; Index++) {
		if (XPmonPsv_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XPmonPsv_ConfigTable[Index];
			break;
		}
	}

	return (XPmonPsv_Config *)CfgPtr;
}
/** @} */
