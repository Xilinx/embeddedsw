// ==============================================================
// Copyright (c) 2015 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_tpg_sinit.c
 * @addtogroup v_tpg Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xv_tpg.h"

/************************** Constant Definitions *****************************/

/**
 * Default number of TPG instances if not defined by hardware configuration
 */
#ifndef XPAR_XV_TPG_NUM_INSTANCES
#define XPAR_XV_TPG_NUM_INSTANCES   0
#endif

/************************** Variable Definitions *****************************/

extern XV_tpg_Config XV_tpg_ConfigTable[];

/*****************************************************************************/
/**
 * @brief Look up the device configuration based on the base address
 *
 * This function searches the configuration table for the device configuration
 * matching the specified base address.
 *
 * @param  BaseAddress Base address of the TPG device to search for
 *
 * @return Pointer to XV_tpg_Config structure if found, NULL otherwise
 *
 * @note This is the SDT version that uses base address for lookup
 *
 *******************************************************************************/
XV_tpg_Config *XV_tpg_LookupConfig(UINTPTR BaseAddress) {
	XV_tpg_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0U; XV_tpg_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_tpg_ConfigTable[Index].BaseAddress == BaseAddress) ||
			!BaseAddress) {
			ConfigPtr = &XV_tpg_ConfigTable[Index];
		break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the XV_tpg instance
 *
 * This function initializes the XV_tpg instance by looking up the device
 * configuration based on the base address and calling the configuration
 * initialization function.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance to initialize
 * @param  BaseAddress Base address of the TPG device to initialize
 *
 * @return XST_SUCCESS if initialization succeeds
 *         XST_DEVICE_NOT_FOUND if device configuration is not found
 *
 * @note This is the SDT version that uses base address for initialization
 *
 *******************************************************************************/
int XV_tpg_Initialize(XV_tpg *InstancePtr, UINTPTR BaseAddress) {
	XV_tpg_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_tpg_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_tpg_CfgInitialize(InstancePtr,
                                ConfigPtr,
                                ConfigPtr->BaseAddress);
}
#endif
