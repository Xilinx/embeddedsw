/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xhdcp22_rx_dp_sinit.c
* @addtogroup hdcp22_rx_dp_v3_0
* @{
* @details
*
* This file contains static initialization method for Xilinx HDCP 2.2 Receiver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  JB   02/19/19 First Release.
* 3.00  JB   12/24/21 File name changed from xhdcp22_rx_sinit.c to
*                     xhdcp22_rx_dp_sinit.c, Also all the APIs and
*                     sructure names are added with suffix _dp.
*</pre>
*
*****************************************************************************/


/***************************** Include Files *********************************/

#include "xhdcp22_rx_dp.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifndef XPAR_XHDCP22_RX_DP_NUM_INSTANCES
#define XPAR_XHDCP22_RX_DP_NUM_INSTANCES  0
#endif
/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XHdcp22_Rx_Dp_Config structure based
* on the core id, <i>DeviceId</i>. The return value will refer to an entry in
* the device configuration table defined in the xhdcp22_rx_g.c file.
*
* @param	DeviceId is the unique core ID of the HDCP RX core for the
*			lookup operation.
*
* @return	XHdcp22Rx_Dp_LookupConfig returns a reference to a config record
*			in the configuration table (in xhdmi_tx_g.c) corresponding
*			to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XHdcp22_Rx_Dp_Config *XHdcp22Rx_Dp_LookupConfig(u16 DeviceId)
{
	extern XHdcp22_Rx_Dp_Config XHdcp22_Rx_Dp_ConfigTable[XPAR_XHDCP22_RX_DP_NUM_INSTANCES];
	XHdcp22_Rx_Dp_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XHDCP22_RX_DP_NUM_INSTANCES); Index++) {
		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XHdcp22_Rx_Dp_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHdcp22_Rx_Dp_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/** @} */
