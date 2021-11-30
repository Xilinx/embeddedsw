/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_sinit.c
* @addtogroup xresetps_v1_5
* @{
*
* This file contains method for static initialization (compile-time) of the
* driver.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- ----------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.1   Nava   04/20/18 Fixed compilation warnings.
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xresetps.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Variable Definitions ****************************/
extern XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XResetPs_Config *XResetPs_LookupConfig(u16 DeviceId)
{
	XResetPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XRESETPS_NUM_INSTANCES; Index++) {
		if (XResetPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XResetPs_ConfigTable[Index];
			break;
		}
	}
	return (XResetPs_Config *)CfgPtr;
}
/** @} */
