/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xresetps_sinit.c
* @addtogroup resetps Overview
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
* 1.5   sd     07/07/23 Added SDT support.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xresetps.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Variable Definitions ****************************/
#ifndef SDT
extern XResetPs_Config XResetPs_ConfigTable[XPAR_XRESETPS_NUM_INSTANCES];
#else
extern XResetPs_Config XResetPs_ConfigTable[];
#endif

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
#ifndef SDT
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
#else
XResetPs_Config *XResetPs_LookupConfig(u32 BaseAddress)
{
	XResetPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XResetPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XResetPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XResetPs_ConfigTable[Index];
			break;
		}
	}
	return (XResetPs_Config *)CfgPtr;
}
#endif
/** @} */
