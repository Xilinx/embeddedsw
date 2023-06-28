/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdevcfg_sinit.c
* @addtogroup devcfg Overview
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
* 1.00a hvm 02/07/11 First release
* 3.5   ms  08/07/17 Fixed compilation warnings.
* 3.8  Nava 06/21/23 Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdevcfg.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID. The table
* contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XDcfg_Config *XDcfg_LookupConfig(u16 DeviceId)
{
	extern XDcfg_Config XDcfg_ConfigTable[];
	XDcfg_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XDCFG_NUM_INSTANCES; Index++) {
		if (XDcfg_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDcfg_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else
XDcfg_Config *XDcfg_LookupConfig(UINTPTR BaseAddress)
{
	extern XDcfg_Config XDcfg_ConfigTable[];
	XDcfg_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; XDcfg_ConfigTable[Index].Name != NULL; Index++) {
		if ((XDcfg_ConfigTable[Index].BaseAddr == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XDcfg_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#endif
/** @} */
