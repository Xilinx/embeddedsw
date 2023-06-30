/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcframe_sinit.c
* @addtogroup cframe_v1_3
* @{
*
* This file contains static initialization methods for Xilinx CFRAME core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0   kc   22/10/17 First release
* 1.1   ng   06/30/23 Added support for system device tree flow
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcframe.h"
#ifndef SDT
#include "xparameters.h"
#endif


/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* XCframe_LookupConfig returns a reference to an XCframe_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcframe_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xcframe_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
#ifndef SDT
XCframe_Config *XCframe_LookupConfig(u16 DeviceId)
{
	extern XCframe_Config XCframe_ConfigTable[XPAR_XCFRAME_NUM_INSTANCES];
	XCframe_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCFRAME_NUM_INSTANCES);
								Index++) {
		if (XCframe_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCframe_ConfigTable[Index];
			break;
		}
	}

	return (XCframe_Config *)CfgPtr;
}
#else
XCframe_Config *XCframe_LookupConfig(UINTPTR BaseAddress)
{
	extern XCframe_Config XCframe_ConfigTable[];
	XCframe_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; XCframe_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCframe_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XCframe_ConfigTable[Index];
			break;
		}
	}

	return (XCframe_Config *)CfgPtr;
}
#endif
/** @} */
