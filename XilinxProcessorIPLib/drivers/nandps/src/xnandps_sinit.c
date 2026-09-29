/******************************************************************************
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps_sinit.c
* @brief Implements the static initialization functions of the XNandPs driver.
*
* This file contains the implementation of the XNand driver's static
* initialization functionality.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date    	   Changes
* ----- ---- ----------  -----------------------------------------------
* 1.00a nm   12/10/2010  First release
* 2.8  akm   07/06/23    Update the driver to support for system device-tree flow.
* 2.10 akm   10/04/24    Retrieve the 'reg' property value from smcc node 'ranges'
* 2.11 vlt   12/14/25    Update Doxygen comments to include SDT flow details.
* 2.12 vn    09/29/26    Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup nandps Overview
 * @{
 */

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xnandps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @brief Configuration table of all NAND flash device instances.
 */
extern XNandPs_Config XNandPs_ConfigTable[];

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the unique device ID or
*	 base address.
*
* The XNandPs_ConfigTable[] contains the configuration information for each
* device in the system.
*
* @if SDT
* @param[in]   BaseAddress Base address of the device.
* @else
* @param[in]   DeviceId Unique ID of the device.
* @endif
*
* @return      A pointer to the configuration found or NULL if the specified
*              device ID/BaseAddress was not found. See xnandps.h for the
*              definition of XNandPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XNandPs_Config *XNandPs_LookupConfig(u16 DeviceId)
{
	XNandPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XNANDPS_NUM_INSTANCES; Index++) {
		if (XNandPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XNandPs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
#else
XNandPs_Config *XNandPs_LookupConfig(UINTPTR BaseAddress)
{
	XNandPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XNandPs_ConfigTable[Index].Name != NULL; Index++) {
		if (XNandPs_ConfigTable[Index].FlashBase == BaseAddress ||
		    !BaseAddress) {
			CfgPtr = &XNandPs_ConfigTable[Index];
			break;
		}
	}
	return CfgPtr;
}
#endif
/** @} */
