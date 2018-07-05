/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint_sinit.c
* @addtogroup deinterlacer_v3_3
* @{
*
* This file contains static initialization methods for Xilinx Video
* Deinterlacer (DEINT) core driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who   Date     Changes
* ----- ----- -------- ------------------------------------------------------
* 1.00a rjh   07/10/11 First release.
* 2.00a rjh   18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk   02/13/14 Added Doxygen support, adherence to Xilinx
*                      coding standards.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdeint.h"
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
* This function returns a reference to an XDeint_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xdeint_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	XDeint_LookupConfig returns a reference to a config record
*		in the configuration table (in xDEINT_g.c) corresponding to
*		<i>DeviceId</i> or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XDeint_Config *XDeint_LookupConfig(u16 DeviceId)
{
	extern XDeint_Config XDeint_ConfigTable[XPAR_XDEINT_NUM_INSTANCES];
	XDeint_Config *CfgPtr = NULL;
	u32 Index;

	/* To get the reference pointer to XDeint_Config structure */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XDEINT_NUM_INSTANCES);
						Index++) {

		/* Compare device Id with configTable's device Id */
		if (XDeint_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDeint_ConfigTable[Index];
			break;
		}
	}

	return (XDeint_Config *)(CfgPtr);
}
/** @} */
