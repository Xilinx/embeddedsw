/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_sinit.c
* @addtogroup xxvethernet Overview
* @{
*
* This file contains static initialization functionality for XXV Ethernet driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   hk   6/16/17  First release
* 1.1   mj   3/30/18  Add XXxvEthernet_LookupConfigBaseAddr(UINTPTR Baseaddr)
*                     API to lookup config by base address.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifndef SDT
#include "xparameters.h"
#endif
#include "xxxvethernet.h"

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
*		A pointer to the configuration table entry corresponding to the given
* 		device ID/BaseAddress, or NULL if no match is found.
*
* @note        In XSCT/classic flow, DeviceId is used to look up the device
*              configuration.
*
******************************************************************************/
#ifdef SDT
XXxvEthernet_Config *XXxvEthernet_LookupConfig(UINTPTR BaseAddress)
{
	extern XXxvEthernet_Config XXxvEthernet_ConfigTable[];
	XXxvEthernet_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0x0; XXxvEthernet_ConfigTable[Index].Name != NULL; Index++) {
		if ((XXxvEthernet_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XXxvEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else

XXxvEthernet_Config *XXxvEthernet_LookupConfig(u16 DeviceId)
{
	extern XXxvEthernet_Config XXxvEthernet_ConfigTable[];
	XXxvEthernet_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0; Index < XPAR_XXXVETHERNET_NUM_INSTANCES; Index++) {
		if (XXxvEthernet_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XXxvEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#endif
/*****************************************************************************/
/**
* XXxvEthernet_LookupConfigBaseAddr returns a reference to an
* XXxvEthernet_Config structure based on base address. The return value
* will refer to an entry in the device configuration table defined in the
* xxxvethernet_g.c file.
*
* @param	 Baseaddr is the base address of the device to lookup for.
*
* @return
*		- Returns a reference to a config record in the
*		  configuration table (in xxxvethernet_g.c) corresponding to
*		  <i>Baseaddr</i>, or NULL
*		- NULL if no match is found.
*
******************************************************************************/
#ifdef SDT
XXxvEthernet_Config *XXxvEthernet_LookupConfigBaseAddr(UINTPTR BaseAddress)
{
	extern XXxvEthernet_Config XXxvEthernet_ConfigTable[];
	XXxvEthernet_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0x0; XXxvEthernet_ConfigTable[Index].Name != NULL; Index++) {
		if ((XXxvEthernet_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XXxvEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#else
XXxvEthernet_Config *XXxvEthernet_LookupConfigBaseAddr(UINTPTR Baseaddr)
{
	extern XXxvEthernet_Config XXxvEthernet_ConfigTable[];
	XXxvEthernet_Config *CfgPtr = NULL;
	int Index;

	for (Index = 0; Index < XPAR_XXXVETHERNET_NUM_INSTANCES; Index++) {
		if (XXxvEthernet_ConfigTable[Index].BaseAddress == Baseaddr) {
			CfgPtr = &XXxvEthernet_ConfigTable[Index];
			break;
		}
	}

	return (CfgPtr);
}
#endif
/** @} */
