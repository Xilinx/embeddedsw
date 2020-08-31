/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2tx_sinit.c
* @addtogroup csi2tx_v1_3
* @{
*
* Look up the hardware settings using device ID. The hardware setting is inside
* the configuration table in xcsi2tx_g.c, generated automatically by XPS or
* manually by the user.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/28/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsi2tx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/*************************** Macros Definitions ******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * Look up the hardware configuration for a device instance
 *
 * @param	DeviceId is the unique device ID of the device to lookup for
 *
 * @return	The reference to the configuration record in the configuration
 * 		table (in xcsi2tx_g.c) corresponding to the Device ID or if
 *		not found,a NULL pointer is returned.
 *
 * @note	None
 *
 *****************************************************************************/
XCsi2Tx_Config *XCsi2Tx_LookupConfig(u32 DeviceId)
{
	extern XCsi2Tx_Config XCsi2Tx_ConfigTable[];
	XCsi2Tx_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XCSI2TX_NUM_INSTANCES; Index++) {
		if (XCsi2Tx_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsi2Tx_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
