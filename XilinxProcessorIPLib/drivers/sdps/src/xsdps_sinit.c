/******************************************************************************
* Copyright (C) 2013 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
* @brief
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId Contains the ID of the device to look up the
*		configuration for.
*
* @return
*
* A pointer to the configuration found or NULL if the specified device ID was
* not found. See xsdps.h for the definition of XSdPs_Config.
*
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
XSdPs_Config *XSdPs_LookupConfig(u32 BaseAddress)
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
