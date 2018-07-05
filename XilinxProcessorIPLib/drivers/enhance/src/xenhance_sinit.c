/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xenhance_sinit.c
* @addtogroup enhance_v7_1
* @{
*
* This file contains static initialization methods for Xilinx Enhance core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------
* 7.0   adk  02/19/14 First release.
*                     Implemented the following function
*                     XEnhance_LookupConfig
* 7.1   ms   01/31/17 Updated the parameter naming from
*                     XPAR_ENHANCE_NUM_INSTANCES to
*                     XPAR_XENHANCE_NUM_INSTANCES to avoid compilation
*                     failure for XPAR_ENHANCE_NUM_INSTANCES
*                     as the tools are generating
*                     XPAR_XENHANCE_NUM_INSTANCES in the generated
*                     xenhance_g.c for fixing MISRA-C files. This is a
*                     fix for CR-967548 based on the update in the tools.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xenhance.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Declaration *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function returns a reference to an XEnhance_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xenhance_g.c.
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	XEnhance_LookupConfig returns a reference to a config record in
*		the configuration table (in xenhance_g.c) corresponding to
*		<i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XEnhance_Config *XEnhance_LookupConfig(u16 DeviceId)
{
	extern XEnhance_Config
			XEnhance_ConfigTable[XPAR_XENHANCE_NUM_INSTANCES];
	XEnhance_Config *CfgPtr = NULL;
	u32 Index;

	/* To get the reference pointer to XEnhance_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XENHANCE_NUM_INSTANCES);
								Index++) {

		/* Compare device Id with configTable's device Id */
		if (XEnhance_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XEnhance_ConfigTable[Index];
			break;
		}
	}

	return (XEnhance_Config *)CfgPtr;
}
/** @} */
