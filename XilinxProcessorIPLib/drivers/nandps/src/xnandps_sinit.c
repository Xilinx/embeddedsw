/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_sinit.c
* @addtogroup nandps_v2_7
* @{
*
* This file contains the implementation of the XNand driver's static
* initialization functionality.
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ---- ----------  -----------------------------------------------
* 1.00a nm   12/10/2010  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xnandps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XNandPs_Config XNandPs_ConfigTable[];

/*****************************************************************************/
/**
*
* This function looks up the device configuration based on the unique device ID.
* The table XNandPs_ConfigTable contains the configuration info for each device
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
XNandPs_Config *XNandPs_LookupConfig(u16 DeviceId)
{
	XNandPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index=0; Index < XPAR_XNANDPS_NUM_INSTANCES; Index++) {
		if (XNandPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XNandPs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
/** @} */
