/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcfupmc_sinit.c
* @addtogroup cfupmc Overview
* @{
*
* This file contains static initialization methods for Xilinx CFU core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0   kc   22/10/17 First release
* 2.0   bsv  27/06/20 Code clean up
* 2.1   ng   06/30/23 Added support for system device tree flow
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcfupmc.h"
#ifndef SDT
#include "xparameters.h"
#endif


/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifndef SDT
extern XCfupmc_Config XCfupmc_ConfigTable[XPAR_XCFUPMC_NUM_INSTANCES];
#else
extern XCfupmc_Config XCfupmc_ConfigTable[];
#endif


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* XCfupmc_LookupConfig returns a reference to an XCfupmc_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcfupmc_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xcfupmc_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
******************************************************************************/
#ifndef SDT
XCfupmc_Config *XCfupmc_LookupConfig(u16 DeviceId)
{
	XCfupmc_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; Index < (u32)XPAR_XCFUPMC_NUM_INSTANCES; Index++) {
		if (XCfupmc_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCfupmc_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XCfupmc_Config *XCfupmc_LookupConfig(UINTPTR BaseAddress)
{
	extern XCfupmc_Config XCfupmc_ConfigTable[];
	XCfupmc_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; XCfupmc_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCfupmc_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XCfupmc_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/** @} */
