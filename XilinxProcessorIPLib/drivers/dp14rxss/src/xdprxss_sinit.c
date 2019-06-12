/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_sinit.c
* @addtogroup dprxss_v6_0
* @{
*
* This file contains static initialization method for Xilinx DisplayPort
* Receiver Subsystem core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XDpRxSs_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xdprxss_g.c file.
*
* @param	DeviceId is the unique core ID of the XDpRxSs core for
*		the lookup operation.
*
* @return	XDpRxSs_LookupConfig returns a reference to a config record
*		in the configuration table (in xdprxss_g.c) corresponding
*		to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XDpRxSs_Config *XDpRxSs_LookupConfig(u16 DeviceId)
{
	extern XDpRxSs_Config XDpRxSs_ConfigTable[XPAR_XDPRXSS_NUM_INSTANCES];
	XDpRxSs_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XDPRXSS_NUM_INSTANCES);
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XDpRxSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpRxSs_ConfigTable[Index];
			break;
		}
	}

	return (XDpRxSs_Config *)CfgPtr;
}
/** @} */
