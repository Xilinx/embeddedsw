/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscutimer_sinit.c
* @addtogroup scutimer Overview
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
* 1.00a nm  03/10/10 First release
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
* 2.5   dp  07/11/23 Add Support for system device tree flow
* 2.8   bdk 12/08/25 Updated comments to support SDT flow for Doxygen
*                    documentation.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xscutimer.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions ****************************/
#ifndef SDT
extern XScuTimer_Config XScuTimer_ConfigTable[XPAR_XSCUTIMER_NUM_INSTANCES];
#else
extern XScuTimer_Config XScuTimer_ConfigTable[];
#endif

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID/BaseAddress.
* The table contains the configuration info for each device in the system.
*
* @if SDT
* @param	BaseAddr contains the base address of the device
* @else
* @param	DeviceId is the unique device ID of the device being looked up.
* @endif
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID/BaseAddress, or NULL if no match is found.
*
* @note		In XSCT/classic flow, DeviceId is used to look up the device
*		configuration.
*
******************************************************************************/
#ifndef SDT
XScuTimer_Config *XScuTimer_LookupConfig(u16 DeviceId)
{
	XScuTimer_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XSCUTIMER_NUM_INSTANCES; Index++) {
		if (XScuTimer_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XScuTimer_ConfigTable[Index];
			break;
		}
	}

	return (XScuTimer_Config *)CfgPtr;
}
#else
XScuTimer_Config *XScuTimer_LookupConfig(UINTPTR BaseAddr)
{
	XScuTimer_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; XScuTimer_ConfigTable[Index].Name != NULL; Index++) {
		if (XScuTimer_ConfigTable[Index].BaseAddr == BaseAddr) {
			CfgPtr = &XScuTimer_ConfigTable[Index];
			break;
		}
	}

	return (XScuTimer_Config *)CfgPtr;
}
#endif
/** @} */
