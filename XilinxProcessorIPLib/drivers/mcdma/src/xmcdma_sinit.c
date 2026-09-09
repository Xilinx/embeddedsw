/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
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

#ifndef SDT
/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID.
* A table contains the configuration info for each device in the system.
*
* @param[in]	DeviceId contains the unique ID of the device
*
* @return
* Pointer to the matching configuration table entry, or NULL if no match exists.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
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
/*****************************************************************************/
/**
*
* Looks up the device configuration based on the base address.
* A table contains the configuration info for each device in the system.
*
* @param[in]	BaseAddress contains the base address of the device
*
* @return
* Pointer to the matching configuration table entry, or NULL if no match exists.
*
******************************************************************************/
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
* @param[in]	Baseaddr is the base address of the device to lookup for
*
* @return	CfgPtr is a reference to a config record in the configuration
*		table (in xmcdma_g.c) corresponding to Baseaddr, or
*		NULL if no match is found.
*
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
