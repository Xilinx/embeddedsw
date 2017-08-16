/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmutex_sinit.c
* @addtogroup mutex_v4_5
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
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmutex.h"
#include "xparameters.h"

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
/** @} */
