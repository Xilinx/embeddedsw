/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsi2txss_sinit.c
* @addtogroup csi2txss_v1_3
* @{
*
* This file contains the implementation of the MIPI CSI Rx Subsystem
* driver's static initialization functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who   Date     Changes
* --- --- -------- ----------------------------------------------------------
* 1.0 sss 07/21/16 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XCsi2TxSs_Config XCsi2TxSs_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function looks for the device configuration based on the unique device
* ID. The table XCsi2TxSs_ConfigTable[] contains the configuration information
* for each instance of the device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found
*
* @note		None.
*
******************************************************************************/
XCsi2TxSs_Config* XCsi2TxSs_LookupConfig(u32 DeviceId)
{
	XCsi2TxSs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < (u32)XPAR_XCSI2TXSS_NUM_INSTANCES; Index++) {
		if (XCsi2TxSs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsi2TxSs_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
