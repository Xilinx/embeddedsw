/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtmrctr_sinit.c
* @addtogroup tmrctr_v4_6
* @{
*
* This file contains static initialization methods for the XTmrCtr driver.
*
* @note	None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 4.0   als  09/30/15 Creation of this file. Moved LookupConfig from xtmrctr.c.
* 4.4	mn   07/31/17 Resolve Compilation warning
* </pre>
*
******************************************************************************/

/******************************* Include Files *******************************/

#include "xparameters.h"
#include "xtmrctr.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XTMRCTR_NUM_INSTANCES
#define XPAR_XTMRCTR_NUM_INSTANCES	0
#endif

/*************************** Variable Declarations ***************************/

/* A table of configuration structures containing the configuration information
 * for each timer core in the system. */
extern XTmrCtr_Config XTmrCtr_ConfigTable[XPAR_XTMRCTR_NUM_INSTANCES];

/**************************** Function Definitions ***************************/

/*****************************************************************************/
/**
* Looks up the device configuration based on the unique device ID. The table
* TmrCtr_ConfigTable contains the configuration info for each device in the
* system.
*
* @param	DeviceId is the unique device ID to search for in the config
*		table.
*
* @return	A pointer to the configuration that matches the given device
* 		ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XTmrCtr_Config *XTmrCtr_LookupConfig(u16 DeviceId)
{
	XTmrCtr_Config *CfgPtr = NULL;
	u16 Index;

	for (Index = 0; Index < XPAR_XTMRCTR_NUM_INSTANCES; Index++) {
		if (XTmrCtr_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTmrCtr_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/** @} */
