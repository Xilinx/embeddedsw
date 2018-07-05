/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcresample_sinit.c
* @addtogroup cresample_v4_1
* @{
*
* This file contains initialization methods for Xilinx Chroma Resampler core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- --------------------------------------------------
* 4.0   adk     03/12/14 First release.
*                        Implemented XCresample_LookupConfig function.
* 4.1   ms      01/16/17 Updated the parameter naming from
*                        XPAR_CRESAMPLE_NUM_INSTANCES to
*                        XPAR_XCRESAMPLE_NUM_INSTANCES to avoid compilation
*                        failure for XPAR_CRESAMPLE_NUM_INSTANCES as the
*                        tools are generating XPAR_XCRESAMPLE_NUM_INSTANCES
*                        in the generated xcresample_g.c for fixing MISRA-C
*                        files. This is a fix for CR-966099 based on the
*                        update in the tools.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xcresample.h"


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Declaration *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XCresample_Config structure based on
* the unique device id, <i>DeviceId</i>. The return value will refer to an
* entry in the device configuration table defined in the xcresample_g.c file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
* 		operation.
*
* @return	XCresample_LookupConfig returns a reference to a config record
*		in the configuration table (in xcresample_g.c) corresponding to
*		<i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCresample_Config *XCresample_LookupConfig(u16 DeviceId)
{
	extern XCresample_Config
			XCresample_ConfigTable[XPAR_XCRESAMPLE_NUM_INSTANCES];
	u32 Index;
	XCresample_Config *CfgPtr = NULL;

	/*
	 * Checking for which instance's device id is
	 * matching with our device id
	 */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCRESAMPLE_NUM_INSTANCES);
		Index++) {

		/* If device ids are equal assign address to CfgPtr */
		if (XCresample_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCresample_ConfigTable[Index];
			break;
		}
	}

	return (XCresample_Config *)CfgPtr;
}
/** @} */
