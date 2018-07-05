/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_rng_sinit.c
* @addtogroup hdcp22_rng_v1_4
* @{
* @details
*
* This file contains the static initialization methods for the Xilinx HDCP
* 2.2 RNG core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     10/01/15 Initial release.
* </pre>
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xhdcp22_rng.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifndef XPAR_XHDCP22_RNG_NUM_INSTANCES
#define XPAR_XHDCP22_RNG_NUM_INSTANCES 0
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XHdcp22_Rng_Config XHdcp22_Rng_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XHdcp22_Rng_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xhdcp22_rng_g.c file.
*
* @param  DeviceId is the unique core ID of the XHDCP22 Rng core for the
*         lookup operation.
*
* @return XHdcp22Rng_LookupConfig returns a reference to a config record
*         in the configuration table (in xhdcp22_rng_g.c) corresponding
*         to <i>DeviceId</i>, or NULL if no match is found.
*
* @note   None.
*
******************************************************************************/

/* Definitions for driver XHDCP22 RNG */
XHdcp22_Rng_Config *XHdcp22Rng_LookupConfig(u16 DeviceId)
{
	XHdcp22_Rng_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XHDCP22_RNG_NUM_INSTANCES);
		Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XHdcp22_Rng_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdcp22_Rng_ConfigTable[Index];
			break;
		}
	}

	return (XHdcp22_Rng_Config *)CfgPtr;
}

/** @} */
