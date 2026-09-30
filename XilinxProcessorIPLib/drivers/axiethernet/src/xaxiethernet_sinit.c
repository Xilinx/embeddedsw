/*****************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xaxiethernet_sinit.c
* @addtogroup axiethernet Overview
* @{
*
* @brief Implements static initialization and configuration lookup for AXI Ethernet.
* Contains static initialization functionality for Axi Ethernet driver.
*
* @verbatim
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a asa  6/30/10  First release
* 5.6   ms   08/07/17 Fixed compilation warning.
* @endverbatim
*
******************************************************************************/

/**************************** Include Files *********************************/

#include "xaxiethernet.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************* Constant Definitions *****************************/


/*************************** Type Definitions *******************************/


/**************** Macros (Inline Functions) Definitions *********************/


/************************* Function Prototypes ******************************/

#ifndef SDT
/*****************************************************************************/
/**
 * Implementation of XAxiEthernet_LookupConfig(). See xaxiethernet.h for the API
 * description.
 *
 ******************************************************************************/
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

#endif
#if defined(SDT)
/*****************************************************************************/
/**
 * Implementation of XAxiEthernet_LookupConfig(). See xaxiethernet.h for the API
 * description.
 *
 ******************************************************************************/
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
