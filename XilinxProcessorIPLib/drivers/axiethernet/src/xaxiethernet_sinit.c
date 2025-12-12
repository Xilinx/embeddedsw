/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xaxiethernet_sinit.c
* @addtogroup axiethernet Overview
* @{
*
* This file contains static initialization functionality for Axi Ethernet driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10  First release
* 5.6   ms   08/07/17 Fixed compilation warning.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaxiethernet.h"
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
XAxiEthernet_Config *XAxiEthernet_LookupConfig(u16 DeviceId)
{
	extern XAxiEthernet_Config XAxiEthernet_ConfigTable[];
	XAxiEthernet_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < XPAR_XAXIETHERNET_NUM_INSTANCES; Index++) {
		if (XAxiEthernet_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XAxiEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else
XAxiEthernet_Config *XAxiEthernet_LookupConfig(UINTPTR BaseAddress)
{
	extern XAxiEthernet_Config XAxiEthernet_ConfigTable[];
	XAxiEthernet_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0x0; XAxiEthernet_ConfigTable[Index].Name != NULL; Index++) {
		if ((XAxiEthernet_ConfigTable[Index].BaseAddress == BaseAddress) ||
		     !BaseAddress) {
			CfgPtr = &XAxiEthernet_ConfigTable[Index];
			break;
		}
	}


	return (CfgPtr);
}
#endif
/** @} */
