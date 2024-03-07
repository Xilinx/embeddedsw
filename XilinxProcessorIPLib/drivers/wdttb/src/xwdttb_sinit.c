/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_sinit.c
* @addtogroup wdttb_api WDTTB APIs
* @{
*
* The xwdttb_sinit.c file contains static initialization method for AXI
* Timebase Window Watchdog Timer core.
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
* 5.7  sb  07/12/23 Added support for system device-tree flow.
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
* @param	DeviceId Unique core ID of the XWdtTb core for
*		the lookup operation.
*
* @return	XWdtTb_LookupConfig returns a reference to a config record
*		in the configuration table (in xwdttb_g.c) corresponding
*		to <i>DeviceId</i>, or NULL if no match is found.
*
*
******************************************************************************/
#ifndef SDT
XWdtTb_Config *XWdtTb_LookupConfig(u16 DeviceId)
{
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
#else
XWdtTb_Config *XWdtTb_LookupConfig(UINTPTR BaseAddress)
{
	XWdtTb_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XWdtTb_ConfigTable[Index].Name != NULL; Index++) {
		if ((XWdtTb_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XWdtTb_ConfigTable[Index];
			break;
		}
	}

	return (XWdtTb_Config *)CfgPtr;
}
/** @} */

#endif
/** @} */
