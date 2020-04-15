/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xccm_sinit.c
* @addtogroup ccm_v6_1
* @{
*
* This file contains static initialization methods for Xilinx CCM core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------- -------- --------------------------------------------------
* 6.0   adk     03/06/14 First release.
*                        Implemented XCcm_LookupConfig function.
* 6.1   ms     01/16/17  Updated the parameter naming from
*                        XPAR_CCM_NUM_INSTANCES to XPAR_XCCM_NUM_INSTANCES
*                        to avoid  compilation failure for
*                        XPAR_CCM_NUM_INSTANCES as the tools are generating
*                        XPAR_XCCM_NUM_INSTANCES in the generated xccm_g.c
*                        for fixing MISRA-C files. This is a fix for
*                        CR-966099 based on the update in the tools.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xccm.h"
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
* XCcm_LookupConfig returns a reference to an XCcm_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xccm_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xccm_g.c) corresponding to <i>DeviceId</i>, or NULL
*		if no match is found.
*
* @note		None.
******************************************************************************/
XCcm_Config *XCcm_LookupConfig(u16 DeviceId)
{
	extern XCcm_Config XCcm_ConfigTable[XPAR_XCCM_NUM_INSTANCES];
	XCcm_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCCM_NUM_INSTANCES);
								Index++) {
		if (XCcm_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCcm_ConfigTable[Index];
			break;
		}
	}

	return (XCcm_Config *)CfgPtr;
}
/** @} */
