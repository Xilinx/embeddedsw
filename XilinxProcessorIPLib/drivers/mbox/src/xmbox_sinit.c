/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmbox_sinit.c
* @addtogroup mbox_v4_4
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
*
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xmbox.h"
#include "xparameters.h"

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
/** @} */
