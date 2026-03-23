/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsdps_sinit.c
* @addtogroup sdps_api SDPS APIs
* @{
*
* The file contains the implementation of the static initialization
* functionality of the XSdPs component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.00a hk/sg  10/17/13 Initial release
*       kvn    07/15/15 Modified the code according to MISRAC-2012.
* 3.7   aru    03/12/19 Modified the code according to MISRAC-2012.
* 4.2   ro     06/12/23 Added support for system device-tree flow.
* 4.6   vlt    12/18/25 Update Doxygen comments to include SDT flow details.
* 4.6   vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xsdps.h"
#include "xparameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XSdPs_ConfigTable[] contains the configuration info for each device in
* the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return       A pointer to the configuration found or NULL if the specified
*               device ID/BaseAddress was not found. See xsdps.h for the
*               definition of XSdPs_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XSdPs_Config *XSdPs_LookupConfig(u16 DeviceId)
{
	XSdPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XSDPS_NUM_INSTANCES; Index++) {
		if (XSdPs_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSdPs_ConfigTable[Index];
			break;
		}
	}
	return (XSdPs_Config *)CfgPtr;
}
#else
XSdPs_Config *XSdPs_LookupConfig(UINTPTR BaseAddress)
{
	XSdPs_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XSdPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSdPs_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XSdPs_ConfigTable[Index];
			break;
		}
	}
	return (XSdPs_Config *)CfgPtr;
}
#endif

/** @} */
