/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmutex_sinit.c
* @addtogroup mutex Overview
* @{
*
* Implements static initialization
* See xmutex.h for more information about the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 4.3   ms   08/07/17 Fixed compilation warnings.
* 4.7   ht   06/21/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmutex.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XMutex_Config XMutex_ConfigTable[];

/*************************** Function Prototypes *****************************/

/*****************************************************************************
*
* Looks up the device configuration based on the unique device ID. The config
* table contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique device ID to search for in the config
*		table.
*
* @return	A pointer to the configuration that matches the given device ID,
*		or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XMutex_Config *XMutex_LookupConfig(u16 DeviceId)
{
	XMutex_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XMUTEX_NUM_INSTANCES; Index++) {
		if (XMutex_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMutex_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XMutex_Config *XMutex_LookupConfig(UINTPTR BaseAddress)
{
	XMutex_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; XMutex_ConfigTable[Index].Name != NULL; Index++) {
		if ((XMutex_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XMutex_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
