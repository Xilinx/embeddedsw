/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrgb2ycrcb_sinit.c
* @addtogroup rgb2ycrcb Overview
* @{
*
* This file contains static initialization function for RGB2YCRCB core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 7.0   adk    01/28/14 First release.
*                       Implemented following function:
*                       XRgb2YCrCb_LookupConfig.
* 7.1   ms     01/16/17 Updated the parameter naming from
*                       XPAR_RGB2YCRCB_NUM_INSTANCES  to
*                       XPAR_XRGB2YCRCB_NUM_INSTANCES to avoid compilation
*                       failure for XPAR_RGB2YCRCB_NUM_INSTANCES as the
*                       tools are generating XPAR_XRGB2YCRCB_NUM_INSTANCES
*                       in the generated xrgb2ycrcb_g.c for fixing MISRA-C
*                       files. This is a fix for CR-966099 based on the
*                       update in the tools.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrgb2ycrcb.h"
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
* This function returns a reference to an XRgb2YCrCb_Config structure
* based on the unique core id, <i>DeviceId</i>. The return value will refer
* to an entry in the core configuration table defined in the xrgb2ycrcb_g.c
* file.
*
* @param	DeviceId is the unique core ID of the core for the lookup
*		operation.
*
* @return	XRgb2YCrCb_LookupConfig returns a reference to a config record
*		in the configuration table (in xrgb2ycrcb_g.c) corresponding to
*		<i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XRgb2YCrCb_Config *XRgb2YCrCb_LookupConfig(u16 DeviceId)
{
	u32 Index;
	XRgb2YCrCb_Config *CfgPtr = NULL;
	extern XRgb2YCrCb_Config
		XRgb2YCrCb_ConfigTable[XPAR_XRGB2YCRCB_NUM_INSTANCES];

	/* Get the reference pointer to XRgb2YCrCb_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XRGB2YCRCB_NUM_INSTANCES);
								Index++) {

		/* Compare device Id with configTable's device Id */
		if (XRgb2YCrCb_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XRgb2YCrCb_ConfigTable[Index];
			break;
		}
	}

	return (XRgb2YCrCb_Config *)CfgPtr;
}
/** @} */
