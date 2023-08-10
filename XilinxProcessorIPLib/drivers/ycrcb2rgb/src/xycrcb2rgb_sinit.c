/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xycrcb2rgb_sinit.c
* @addtogroup ycrcb2rgb Overview
* @{
*
* This file contains static initialization methods for Xilinx YCRCB2RGB core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 7.0   adk    02/10/14 First release.
*                       Implemented following function:
*                       XYCrCb2Rgb_Config.
* 7.1   ms     01/31/17 Updated the parameter naming from
*                       XPAR_YCRCB2RGB_NUM_INSTANCES to
*                       XPAR_XYCRCB2RGB_NUM_INSTANCES to avoid compilation
*                       failure for XPAR_YCRCB2RGB_NUM_INSTANCES
*                       as the tools are generating
*                       XPAR_XYCRCB2RGB_NUM_INSTANCES in the generated
*                       xycrcb2rgb_g.c for fixing MISRA-C files. This is a
*                       fix for CR-967548 based on the update in the tools.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xycrcb2rgb.h"
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
* This function returns a reference to an XYCrCb2Rgb_Config structure
* based on the unique core id, <i>DeviceId</i>. The return value will refer
* to an entry in the core configuration table defined in the xycrcb2rgb_g.c
* file.
*
* @param	DeviceId is the unique core ID of the core for the lookup
*		operation.
*
* @return	XYCrCb2Rgb_LookupConfig returns a reference to a config record
*		in the configuration table (in xycrcb2rgb_g.c) corresponding
*		to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XYCrCb2Rgb_Config *XYCrCb2Rgb_LookupConfig(u16 DeviceId)
{
	u32 Index;
	extern XYCrCb2Rgb_Config
			XYCrCb2Rgb_ConfigTable[XPAR_XYCRCB2RGB_NUM_INSTANCES];
	XYCrCb2Rgb_Config *CfgPtr = NULL;

	/* Get the reference pointer to XYCrCb2Rgb_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XYCRCB2RGB_NUM_INSTANCES);
								Index++) {

		/* Compare device Id with configTable's device Id */
		if (XYCrCb2Rgb_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XYCrCb2Rgb_ConfigTable[Index];
			break;
		}
	}

	return (XYCrCb2Rgb_Config *)CfgPtr;
}
/** @} */
