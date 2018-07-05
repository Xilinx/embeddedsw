/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_sinit.c
* @addtogroup csudma_v1_6
* @{
*
* This file contains static initialization methods for Xilinx CSU_DMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   vnsld   22/10/14 First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
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
* XCsuDma_LookupConfig returns a reference to an XCsuDma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcsudma_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xcsudma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XCsuDma_Config *XCsuDma_LookupConfig(u16 DeviceId)
{
	extern XCsuDma_Config XCsuDma_ConfigTable[XPAR_XCSUDMA_NUM_INSTANCES];
	XCsuDma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCSUDMA_NUM_INSTANCES);
								Index++) {
		if (XCsuDma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsuDma_ConfigTable[Index];
			break;
		}
	}

	return (XCsuDma_Config *)CfgPtr;
}
/** @} */
