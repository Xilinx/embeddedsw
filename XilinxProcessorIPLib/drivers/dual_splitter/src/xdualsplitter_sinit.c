/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdualsplitter_sinit.c
* @addtogroup dual_splitter Overview
* @{
*
* This file contains static initialization function for Xilinx Dual Splitter
* core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 1.00  sha 07/21/14 Initial release.
* 1.00  sha 07/08/15 Defined macro XPAR_XDUALSPLITTER_NUM_INSTANCES if not
*                    defined in xparameters.h.
* 1.1   tu  09/18/17 Removed compilation warning in function
*                    XDualSplitter_LookupConfig()
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdualsplitter.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/
#ifdef SDT
#ifndef XPAR_XDUAL_SPLITTER_NUM_INSTANCES
#define XPAR_XDUAL_SPLITTER_NUM_INSTANCES	0
#endif
#else
#ifndef XPAR_XDUALSPLITTER_NUM_INSTANCES
#define XPAR_XDUALSPLITTER_NUM_INSTANCES	0
#endif
#endif
/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to the XDualSplitter_Config structure
* based on the core id, <i>DeviceId</i>. The return value will refer to an
* entry in the device configuration table defined in the xdualsplitter_g.c
* file.
*
* @param	DeviceId is the unique core ID of the Dual Splitter core for
*		the lookup operation.
*
* @return	XDualSplitter_LookupConfig returns a reference to a config
*		record in the configuration table (in xdaulsplitter_g.c)
*		corresponding to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XDualSplitter_Config *XDualSplitter_LookupConfig(u16 DeviceId)
{
	extern XDualSplitter_Config
		XDualSplitter_ConfigTable[XPAR_XDUALSPLITTER_NUM_INSTANCES];
	XDualSplitter_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; Index < XPAR_XDUALSPLITTER_NUM_INSTANCES; Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XDualSplitter_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDualSplitter_ConfigTable[Index];
			break;
		}
	}

	return (XDualSplitter_Config *)CfgPtr;
}
#else
XDualSplitter_Config *XDualSplitter_LookupConfig(UINTPTR BaseAddress)
{
	extern XDualSplitter_Config
		XDualSplitter_ConfigTable[XPAR_XDUALSPLITTER_NUM_INSTANCES];
	XDualSplitter_Config *CfgPtr = NULL;
	int Index;

	/* Checking for device id for which instance it is matching */
	for (Index = 0x0; XDualSplitter_ConfigTable[Index].Name != NULL; Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if ( (XDualSplitter_ConfigTable[Index].BaseAddress == BaseAddress)
            || (!BaseAddress)) {
			CfgPtr = &XDualSplitter_ConfigTable[Index];
			break;
		}
	}

	return (XDualSplitter_Config *)CfgPtr;
}
#endif
/** @} */
