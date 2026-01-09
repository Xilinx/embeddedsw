/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscuwdt_sinit.c
* @addtogroup Overview
* @{
*
* This file contains method for static initialization (compile-time) of the
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* 2.5   asa 07/18/23 Made updates to support system device tree based
*                    workflow decoupling flow.
* 2.6 	ht  06/03/24 Fix HIS_COMF violations.
* 2.7   vlt 12/29/25 Update Doxygen comments to include SDT flow details.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscuwdt.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID/BaseAddress.
* The XScuWdt_ConfigTable[] contains the configuration info for each device
* in the system.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId contains the unique ID of the device
* @endif
*
* @return    A pointer to the configuration found or NULL if the specified
*            device ID/BaseAddress was not found. See xscuwdt.h for the
*            definition of XScuWdt_Config.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XScuWdt_Config *XScuWdt_LookupConfig(u16 DeviceId)
{
	XScuWdt_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = 0U; Index < (u32)XPAR_XSCUWDT_NUM_INSTANCES; Index++) {
		if (XScuWdt_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XScuWdt_ConfigTable[Index];
			break;
		}
	}

	/* Returns reference to config record if found, else NULL */
	return (XScuWdt_Config *)CfgPtr;
}
#else
XScuWdt_Config *XScuWdt_LookupConfig(UINTPTR BaseAddress)
{
	XScuWdt_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XScuWdt_ConfigTable[Index].Name != NULL; Index++) {
		if ((XScuWdt_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XScuWdt_ConfigTable[Index];
			break;
		}
	}

	return (XScuWdt_Config *)CfgPtr;
}
#endif
/** @} */
