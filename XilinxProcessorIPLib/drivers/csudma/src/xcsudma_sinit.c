/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_sinit.c
* @addtogroup csuma_api CSUDMA APIs
* @{
*
* The xcsudma_sinit.c file contains static initialization methods for Xilinx CSU_DMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   vnsld   22/10/14 First release
* 1.7	sk	08/26/20 Fix MISRA-C violations.
* 1.11	sk	03/03/22 Replace driver version in addtogroup with Overview.
* 1.11	sk	03/03/22 Update Overview section based on review comments.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"
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
* 		device ID/BaseAddress, or NULL if no match is found.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifndef SDT
XCsuDma_Config *XCsuDma_LookupConfig(u16 DeviceId)
{
	XCsuDma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XCSUDMA_NUM_INSTANCES);
								Index++) {
		if (XCsuDma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XCsuDma_ConfigTable[Index];
			break;
		}
	}

	return (XCsuDma_Config *)CfgPtr;
}
#else
XCsuDma_Config *XCsuDma_LookupConfig(UINTPTR BaseAddress)
{
	XCsuDma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XCsuDma_ConfigTable[Index].Name != NULL; Index++) {
		if ((XCsuDma_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			CfgPtr = &XCsuDma_ConfigTable[Index];
			break;
		}
	}

	return (XCsuDma_Config *)CfgPtr;
}
#endif
/** @} */
