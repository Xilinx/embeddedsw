/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_sinit.c
* @brief Implements the static initialization lookup for the AXI XSPI driver.
*
* This file contains the static initialization lookup function for the
* AXI XSPI driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* 1.0   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup amd_xspi Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "amd_xspi.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @brief Holds the configuration entry for each AXI XSPI instance.
 */
extern AmdXspi_Config AmdXspi_ConfigTable[];

/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the base address.
*
* @param[in]   BaseAddress Base address of the device.
*
* @return      - A pointer to the configuration table entry if found.
*              - NULL if not found.
*
******************************************************************************/
AmdXspi_Config *AmdXspi_LookupConfig(UINTPTR BaseAddress)
{
	AmdXspi_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; AmdXspi_ConfigTable[Index].Name != NULL; Index++) {
		if (AmdXspi_ConfigTable[Index].BaseAddress == BaseAddress) {
			CfgPtr = &AmdXspi_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/** @} */
