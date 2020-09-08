/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxis_switch_sinit.c
* @addtogroup axis_switch_v1_4
* @{
*
* This file contains static initialization method for Xilinx AXI4-Stream
* Source Control Router core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.00  sha 07/15/15 Defined macro XPAR_XAXIS_SWITCH_NUM_INSTANCES if not
*                    defined in xparameters.h
* 1.2   ms  02/20/17 Fixed compilation warning. This is a fix for CR-969126.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxis_switch.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#ifndef XPAR_XAXIS_SWITCH_NUM_INSTANCES
#define XPAR_XAXIS_SWITCH_NUM_INSTANCES			0
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XAxis_Switch_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xaxis_switch_g.c file.
*
* @param	DeviceId is the unique core ID of the XAxis_Switch core for
*		the lookup operation.
*
* @return	XAxisScr_LookupConfig returns a reference to a config record
*		in the configuration table (in xaxis_switch_g.c)
*		corresponding to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XAxis_Switch_Config *XAxisScr_LookupConfig(u16 DeviceId)
{
	extern XAxis_Switch_Config
		XAxis_Switch_ConfigTable[XPAR_XAXIS_SWITCH_NUM_INSTANCES];
	XAxis_Switch_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; Index < XPAR_XAXIS_SWITCH_NUM_INSTANCES;
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XAxis_Switch_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxis_Switch_ConfigTable[Index];
			break;
		}
	}

	return (XAxis_Switch_Config *)CfgPtr;
}
/** @} */
