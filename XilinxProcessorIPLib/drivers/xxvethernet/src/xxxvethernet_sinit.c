/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xxxvethernet_sinit.c
* @addtogroup xxvethernet Overview
* @{
*
* @brief Implements static initialization helpers for the XXV Ethernet driver.
* Contains static initialization functionality for XXV Ethernet driver.
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

#ifdef SDT
/*****************************************************************************/
/**
 * @brief Looks up device configuration by base address.
 *
 *****************************************************************************/
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
/*****************************************************************************/
/**
 * @brief Looks up device configuration by device ID.
 *
 *****************************************************************************/
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
 * @brief Looks up device configuration by base address.
 *
 * @if SDT
 * @param[in] BaseAddress is the base address of the device to look up.
 * @else
 * @param[in] Baseaddr is the base address of the device to look up.
 * @endif
 *
 * @return A pointer to the configuration table entry corresponding to the given
 *         base address, or NULL if no match is found.
 *
 *****************************************************************************/
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
