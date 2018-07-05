/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2stx_sinit.c
 * @addtogroup i2stx_v2_2
 * @{
 *
 * This file contains static initialization methods for the i2stx drivers.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * </pre>
 *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "xi2stx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XI2stx_Config XI2stx_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function returns a reference to an XI2stx_Config structure
 * based on the core id, <i>DeviceId</i>. The return value will refer to an
 * entry in the device configuration table defined in the
 * xi2stx_g.c file.
 *
 * @param  DeviceId is the unique core ID of the I2S Transmitter core for the
 *         lookup operation.
 *
 * @return returns a reference to a config record in the configuration table
 *         corresponding to <i>DeviceId</i>, or NULL if no match is found.
 *
 * @note   None.
 *
 *****************************************************************************/
XI2stx_Config *XI2s_Tx_LookupConfig(u16 DeviceId)
{
	XI2stx_Config *CfgPtr = NULL;
	u32 Index;
	/* Checking for device id for which instance it is matching */
	for (Index = 0;
			Index < (u32)(XPAR_XI2STX_NUM_INSTANCES);
			Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XI2stx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XI2stx_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
/** @} */
