/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_sinit.c
* @addtogroup prd_v2_1
* @{
*
* This file contains the implementation of the XPrd driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date          Changes
* ----- ----- -----------   ---------------------------------------------
* 1.0   ms    07/14/16      First release
* 1.1   ms    01/16/17      Updated the parameter naming from
*                           XPAR_PR_DECOUPLER_NUM_INSTANCES to
*                           XPAR_XPRD_NUM_INSTANCES to avoid compilation
*                           failure for XPAR_PR_DECOUPLER_NUM_INSTANCES as
*                           the tools are generating XPAR_XPRD_NUM_INSTANCES
*                           in the generated xprd_g.c for fixing MISRA-C
*                           files. This is a fix for CR-966099 based on the
*                           update in the tools.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XPrd_Config XPrd_ConfigTable[XPAR_XPRD_NUM_INSTANCES];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XPrd_ConfigTable[] contains the configuration information
* for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XPrd_Config *XPrd_LookupConfig(u16 DeviceId)
{
	XPrd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XPRD_NUM_INSTANCES;
		Index++) {
		if (XPrd_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XPrd_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
