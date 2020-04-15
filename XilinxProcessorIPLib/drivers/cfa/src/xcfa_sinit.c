/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcfa_sinit.c
* @addtogroup cfa_v7_1
* @{
*
* This file contains static initialization methods for Xilinx CFA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 7.0   adk    01/07/14 First release.
*                       Implemented XCfa_LookupConfig function.
* 7.1   ms     01/16/17 Updated the parameter naming from
*                       XPAR_CFA_NUM_INSTANCES to XPAR_XCFA_NUM_INSTANCES
*                       to avoid  compilation failure for
*                       XPAR_CFA_NUM_INSTANCES as the tools are generating
*                       XPAR_XCFA_NUM_INSTANCES in the generated xcfa_g.c
*                       for fixing MISRA-C files. This is a fix for
*                       CR-966099 based on the update in the tools.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcfa.h"
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
* This function returns a reference to an XCfa_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xcfa_g.c file.
*
* @param	DeviceId is the unique device ID of the core for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the
*		configuration table (in xcfa_g.c) corresponding to
*		<i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCfa_Config *XCfa_LookupConfig(u16 DeviceId)
{
	extern XCfa_Config XCfa_ConfigTable[XPAR_XCFA_NUM_INSTANCES];
	u32 Index;
	XCfa_Config *CfgPtr = NULL;

	/* To get the reference pointer to XCfa_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCFA_NUM_INSTANCES);
								Index++) {

		/* Compare device Id with configTable's device Id */
		if (XCfa_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCfa_ConfigTable[Index];
			break;
		}
	}

	return (XCfa_Config *)CfgPtr;
}
/** @} */
