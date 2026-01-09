/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
* 5.12 vlt 12/18/25 Update Doxygen comments to include SDT flow details.
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
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XWdtTb_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xwdttb.h for the
*               definition of XWdtTb_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
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
