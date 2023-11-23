/******************************************************************************
* Copyright (C) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xdmaps_sinit.c
* @addtogroup dmaps Overview
* @{
*
* The implementation of the XDmaPs driver's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  hbm  08/13/10 First Release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xdmaps.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/
extern XDmaPs_Config XDmaPs_ConfigTable[];

/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param DeviceId contains the ID of the device
*
* @return
*
* A pointer to the configuration structure or NULL if the specified device
* is not in the system.
*
* @note
*
* None.
*
******************************************************************************/
#ifndef SDT
XDmaPs_Config *XDmaPs_LookupConfig(u16 DeviceId)
{
	XDmaPs_Config *CfgPtr = NULL;

	int i;

	for (i = 0; i < XPAR_XDMAPS_NUM_INSTANCES; i++) {
		if (XDmaPs_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XDmaPs_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}
#else
XDmaPs_Config *XDmaPs_LookupConfig(UINTPTR BaseAddress)
{
	XDmaPs_Config *CfgPtr = NULL;
	int i;

	for (i = (u32)0x0; XDmaPs_ConfigTable[i].Name != NULL; i++) {
		if ((XDmaPs_ConfigTable[i].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XDmaPs_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}

u32 XDmaPs_GetDrvIndex(XDmaPs *InstancePtr, UINTPTR BaseAddress)
{
	u32 Index = 0;

	for (Index = (u32)0x0; XDmaPs_ConfigTable[Index].Name != NULL; Index++) {
		if ((XDmaPs_ConfigTable[Index].BaseAddress == BaseAddress)) {
			break;
		}
	}

	return Index;
}
#endif
/** @} */
