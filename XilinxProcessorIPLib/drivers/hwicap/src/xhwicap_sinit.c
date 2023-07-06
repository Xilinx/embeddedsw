/******************************************************************************
* Copyright (C) 2007 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhwicap_sinit.c
* @addtogroup hwicap Overview
* @{
*
* This file contains the implementation of the XHwicap driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 2.00a  sv  09/28/07 First release
* 11.5  Nava 09/30/22 Added new IDCODE's as mentioned in the ug570 Doc.
* 11.6  Nava 06/28/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifndef SDT
#include "xparameters.h"
#endif
#include "xhwicap.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern XHwIcap_Config XHwIcap_ConfigTable[];

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device for which the
*		device configuration pointer is to be returned.
*
* @return
*		- A pointer to the configuration found.
*		- NULL if the specified device ID was not found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XHwIcap_Config *XHwIcap_LookupConfig(u16 DeviceId)
{

	XHwIcap_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPAR_XHWICAP_NUM_INSTANCES; Index++) {
		if (XHwIcap_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XHwIcap_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XHwIcap_Config *XHwIcap_LookupConfig(UINTPTR BaseAddress)
{
	XHwIcap_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; XHwIcap_ConfigTable[Index].Name != NULL; Index++) {
		if ((XHwIcap_ConfigTable[Index].BaseAddress == BaseAddress) || !BaseAddress) {
			CfgPtr = &XHwIcap_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif

/** @} */
