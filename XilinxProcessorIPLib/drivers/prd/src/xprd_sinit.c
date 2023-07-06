/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd_sinit.c
* @addtogroup prd Overview
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
* 2.2   Nava  06/22/23      Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprd.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifndef SDT
extern XPrd_Config XPrd_ConfigTable[XPAR_XPRD_NUM_INSTANCES];
#else
extern XPrd_Config XPrd_ConfigTable[];
#endif

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
#ifndef SDT
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
#else
XPrd_Config *XPrd_LookupConfig(UINTPTR BaseAddress)
{
	XPrd_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XPrd_ConfigTable[Index].Name != NULL;
	     Index++) {
		if ((XPrd_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XPrd_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
