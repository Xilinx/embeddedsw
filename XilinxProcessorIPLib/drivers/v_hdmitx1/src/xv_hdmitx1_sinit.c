/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitx1_sinit.c
*
* This file contains static initialization method for Xilinx HDMI TX core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  EB     22/05/18 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xv_hdmitx1.h"
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
* This function returns a reference to an XV_HdmiTx1_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xv_hdmitx1_g.c file.
*
* @param    DeviceId is the unique core ID of the HDMI TX core for the
*       lookup operation.
*
* @return   XV_HdmiTx1_LookupConfig returns a reference to a config record
*       in the configuration table (in xv_hdmitx1_g.c) corresponding
*       to <i>DeviceId</i>, or NULL if no match is found.
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx1_Config *XV_HdmiTx1_LookupConfig(u16 DeviceId)
{
	extern XV_HdmiTx1_Config
	XV_HdmiTx1_ConfigTable[XPAR_XV_HDMITX1_NUM_INSTANCES];
	XV_HdmiTx1_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XV_HDMITX1_NUM_INSTANCES);
	     Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XV_HdmiTx1_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XV_HdmiTx1_ConfigTable[Index];
			break;
		}
	}

	return (XV_HdmiTx1_Config *)CfgPtr;
}
