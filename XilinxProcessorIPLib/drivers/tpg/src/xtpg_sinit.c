/******************************************************************************
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xtpg_sinit.c
* @addtogroup tpg_v3_2
* @{
*
* This file contains static initialization methods for Xilinx TPG core
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 3.0   adk    02/19/14 First release.
*                       Implemented the following functions:
*                       XTpg_LookupConfig.
* 3.1   ms     05/22/17 Updated the parameter naming from
*                       XPAR_TPG_NUM_INSTANCES to XPAR_XTPG_NUM_INSTANCES
*                       to avoid  compilation failure as the tools are
*                       generating XPAR_XTPG_NUM_INSTANCES in the xtpg_g.c
*                       for fixing MISRA-C files.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xtpg.h"
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
* This function returns a reference to an XTpg_Config structure based on
* the unique device id, <i>DeviceId</i>. The return value will refer to an
* entry in the device configuration table defined in the xtpg_g.c file.
*
* @param	DeviceId is the unique Device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xtpg_g.c) corresponding to <i>DeviceId</i>, or NULL
*		if no match is found.
*
* @note		None.
*
******************************************************************************/
XTpg_Config *XTpg_LookupConfig(u16 DeviceId)
{
	extern XTpg_Config XTpg_ConfigTable[XPAR_XTPG_NUM_INSTANCES];
	XTpg_Config *CfgPtr = NULL;
	u32 Index;

	/* To get the reference pointer to XTpg_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XTPG_NUM_INSTANCES);
					 Index++) {

		/* Compare device Id with configTable's device Id*/
		if (XTpg_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XTpg_ConfigTable[Index];
			break;
		}
	}

	return (XTpg_Config *)CfgPtr;
}
/** @} */
