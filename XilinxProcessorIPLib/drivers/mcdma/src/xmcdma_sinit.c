/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmcdma_sinit.c
* @addtogroup mcdma Overview
* @{
*
* This file contains static initialization methods for Xilinx MCDMA core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   adk     18/07/17 Initial version.
* 1.2   mj      05/03/18 Implemented XMcdma_LookupConfigBaseAddr() to lookup
*                        configuration based on base address.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xmcdma.h"
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
* XMcdma_LookupConfig returns a reference to an XMcdma_Config structure
* based on the unique device id, <i>DeviceId</i>. The return value will refer
* to an entry in the device configuration table defined in the xmcdma_g.c
* file.
*
* @param	DeviceId is the unique device ID of the device for the lookup
*		operation.
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to <i>DeviceId</i>, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
#ifndef SDT
XMcdma_Config *XMcdma_LookupConfig(u16 DeviceId)
{
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
	     Index++) {
		if (XMcdma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return (XMcdma_Config *)CfgPtr;
}
#else
XMcdma_Config *XMcdma_LookupConfig(UINTPTR BaseAddress)
{
	extern XMcdma_Config XMcdma_ConfigTable[];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; XMcdma_ConfigTable[Index].Name != NULL; Index++) {
		if ((XMcdma_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}
	return (XMcdma_Config *)(CfgPtr);
}
#endif

#ifndef SDT

/*****************************************************************************/
/**
*
* XMcdma_LookupConfigBaseAddr returns a reference to an XMcdma_Config structure
* based on base address. The return value will refer to an entry in the device
* configuration table defined in the xmcdma_g.c file.
*
* @param	Baseaddr is the base address of the device to lookup for
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to Baseaddr, or
*		NULL if no match is found.
*
* @note		None.
******************************************************************************/
XMcdma_Config *XMcdma_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
	extern XMcdma_Config XMcdma_ConfigTable[XPAR_XMCDMA_NUM_INSTANCES];
	XMcdma_Config *CfgPtr = NULL;
	u32 Index;

	/* Checks all the instances */
	for (Index = (u32)0x0; Index < (u32)(XPAR_XMCDMA_NUM_INSTANCES);
	     Index++) {
		if (XMcdma_ConfigTable[Index].BaseAddress == Baseaddr) {
			CfgPtr = &XMcdma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#endif
/** @} */
