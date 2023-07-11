/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmbox_sinit.c
* @addtogroup mbox Overview
* @{
*
* Implements static initialization
* See xmbox.h for more information about the component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 4.2   ms   08/07/17 Fixed compilation warnings.
* 4.6   ht   07/06/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmbox.h"
#ifndef SDT
#include "xparameters.h"
#endif

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

extern XMbox_Config XMbox_ConfigTable[];

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
XMbox_Config *XMbox_LookupConfig(u16 DeviceId)
{
	XMbox_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XMBOX_NUM_INSTANCES; Index++) {
		if (XMbox_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMbox_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XMbox_Config *XMbox_LookupConfig(UINTPTR BaseAddress)
{
	XMbox_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XMbox_ConfigTable[Index].Name != NULL; Index++) {
		if ((XMbox_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XMbox_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
