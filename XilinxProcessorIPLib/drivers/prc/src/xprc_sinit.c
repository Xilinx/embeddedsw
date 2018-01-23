/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprc_sinit.c
* @addtogroup prc_v2_0
* @{
*
* This file contains the implementation of the XPrc driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ---- ---- ------------ --------------------------------------------------
* 1.0   ms   07/18/2016    First release
* 1.2   Nava 29/03/19      Updated the tcl logic to generated the
*                          XPrc_ConfigTable properly.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XPrc_Config XPrc_ConfigTable[];

/****************************** Functions  ***********************************/

/*****************************************************************************/
/**
*
* This function Looks for the device configuration based on the unique device
* ID. The table XPrc_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match was found.
*
* @note		None.
*
******************************************************************************/
XPrc_Config *XPrc_LookupConfig(u16 DeviceId)
{
	XPrc_Config *ConfigPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XPRC_NUM_INSTANCES; Index++) {
		if (XPrc_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XPrc_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}
/** @} */
