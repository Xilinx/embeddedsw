/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_sinit.c
* @addtogroup xwdttb_v5_0
* @{
*
* This file contains static initialization method for Xilinx AXI Timebase
* Window Watchdog Timer core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------------
* 1.00 sha 12/17/15 First release.
* 4.0  sha 01/29/16 Updated version to 4.0 as it is newly added file in driver
*                   version 4.0.
* 4.3  srm 01/30/18 Added doxygen tags
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xwdttb.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief
*
* This function returns a reference to an XWdtTb_Config structure based on the
* core id, <i>DeviceId</i>. The return value will refer to an entry in the
* device configuration table defined in the xwdttb_g.c file.
*
* @param	DeviceId is the unique core ID of the XWdtTb core for
*		the lookup operation.
*
* @return	XWdtTb_LookupConfig returns a reference to a config record
*		in the configuration table (in xwdttb_g.c) corresponding
*		to <i>DeviceId</i>, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XWdtTb_Config *XWdtTb_LookupConfig(u16 DeviceId)
{
	extern XWdtTb_Config XWdtTb_ConfigTable[XPAR_XWDTTB_NUM_INSTANCES];
	XWdtTb_Config *CfgPtr = NULL;
	u32 Index;

	/* Checking for device id for which instance it is matching */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XWDTTB_NUM_INSTANCES);
								Index++) {

		/* Assigning address of config table if both device ids
		 * are matched
		 */
		if (XWdtTb_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XWdtTb_ConfigTable[Index];
			break;
		}
	}

	return (XWdtTb_Config *)CfgPtr;
}
/** @} */
