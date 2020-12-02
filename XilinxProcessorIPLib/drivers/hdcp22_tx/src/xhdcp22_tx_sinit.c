/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp22_tx_sinit.c
* @addtogroup hdcp22_tx_v3_0
* @{
* @details
*
* This file contains static initialization method for Xilinx HDCP 2.2
* Transmitter driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  JO     16/06/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xhdcp22_tx.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifndef XPAR_XHDCP22_TX_NUM_INSTANCES
#define XPAR_XHDCP22_TX_NUM_INSTANCES  0
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XHdcp22_Tx_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xhdcp22_tx_g.c file.
*
* @param  DeviceId is the unique core ID of the HDCP2.2 TX core for the
*         lookup operation.
*
* @return XHdcp22Tx_LookupConfig returns a reference to a config record
*         in the configuration table (in xhdcp22_tx_g.c) corresponding
*         to <i>DeviceId</i>, or NULL if no match is found.
*
* @note   None.
*
******************************************************************************/
XHdcp22_Tx_Config *XHdcp22Tx_LookupConfig(u16 DeviceId)
{
	extern XHdcp22_Tx_Config XHdcp22_Tx_ConfigTable[XPAR_XHDCP22_TX_NUM_INSTANCES];
	XHdcp22_Tx_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XHDCP22_TX_NUM_INSTANCES); Index++) {
		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XHdcp22_Tx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdcp22_Tx_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/** @} */
