/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_sinit.c
* @addtogroup zdma Overview
* @{
*
* This file contains static initialization methods for Xilinx ZDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   vns     2/27/15  First release
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xzdma.h"
#ifndef SDT
#include "xparameters.h"
#endif

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
* A table contains the configuration info for each device in the system.
*
* @if SDT
* @param        BaseAddress contains the base address of the device
* @else
* @param        DeviceId contains the unique ID of the device
* @endif
*
* @return
* 		A pointer to the configuration table entry corresponding to the given
*		device ID/BaseAddress, or NULL if no match is found.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XZDma_Config *XZDma_LookupConfig(u16 DeviceId)
{
	extern XZDma_Config XZDma_ConfigTable[XPAR_XZDMA_NUM_INSTANCES];
	XZDma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XZDMA_NUM_INSTANCES);
	     Index++) {
		if (XZDma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XZDma_ConfigTable[Index];
			break;
		}
	}

	return (XZDma_Config *)CfgPtr;
}
#else
XZDma_Config *XZDma_LookupConfig(UINTPTR BaseAddress)
{
	extern XZDma_Config XZDma_ConfigTable[];
	XZDma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XZDma_ConfigTable[Index].Name != NULL; Index++) {
		if ((XZDma_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XZDma_ConfigTable[Index];
			break;
		}
	}

	return (XZDma_Config *)CfgPtr;
}
#endif
/** @} */
