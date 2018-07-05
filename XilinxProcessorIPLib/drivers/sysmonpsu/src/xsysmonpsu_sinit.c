/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsu_sinit.c
* @addtogroup sysmonpsu_v2_7
*
* This file contains the implementation of the XSysMonPsu driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	    Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   kvn    12/15/15 First release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmonpsu.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern XSysMonPsu_Config XSysMonPsu_ConfigTable[XPAR_XSYSMONPSU_NUM_INSTANCES];

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XSysmonPsu_ConfigTable[] contains the configuration information
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
XSysMonPsu_Config *XSysMonPsu_LookupConfig(u16 DeviceId)
{
	XSysMonPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XSYSMONPSU_NUM_INSTANCES; Index++) {
		if (XSysMonPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSysMonPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
