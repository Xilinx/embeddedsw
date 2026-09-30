/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xzdma_sinit.c
* @addtogroup zdma Overview
* @{
*
* @brief Implements static initialization helpers for the ZDMA driver.
* Provides static initialization methods for the ZDMA core.
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

#ifndef SDT
/*****************************************************************************/
/**
 * @brief Looks up device configuration by device ID.
 *
 *****************************************************************************/
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
/*****************************************************************************/
/**
 * @brief Looks up device configuration by base address.
 *
 *****************************************************************************/
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
