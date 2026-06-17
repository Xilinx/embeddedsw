// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_scenechange_sinit.c
 * @addtogroup v_scenechange Overview
 */

#ifndef __linux__

/***************************** Include Files *********************************/
#include "xstatus.h"
#include "xv_scenechange.h"



/************************** Variable Definitions *****************************/
extern XV_scenechange_Config XV_scenechange_ConfigTable[];

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Look up the device configuration based on base address
 *
 * This function searches the configuration table for an entry matching the
 * specified base address and returns a pointer to the configuration data.
 * If BaseAddress is 0, returns the first available configuration.
 *
 * @param  BaseAddress is the physical base address of the device
 *
 * @return Pointer to XV_scenechange_Config structure if found
 *         NULL if no matching base address is found
 *
 * @note This function is used in SDT (System Device Tree) builds
 *
 *******************************************************************************/
XV_scenechange_Config *XV_scenechange_LookupConfig(UINTPTR BaseAddress) {
	XV_scenechange_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_scenechange_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_scenechange_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_scenechange_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/*****************************************************************************/
/**
 * @brief Initialize the scene change driver instance using base address
 *
 * This function initializes the scene change driver by looking up the
 * configuration data for the specified base address and performing the
 * core initialization.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  BaseAddress is the physical base address of the device
 *
 * @return XST_SUCCESS if initialization succeeds
 *         XST_DEVICE_NOT_FOUND if the base address is not found
 *
 * @note This function is used in SDT (System Device Tree) builds
 *
 *******************************************************************************/
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, UINTPTR BaseAddress) {
	XV_scenechange_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_scenechange_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	InstancePtr->ScdConfig = ConfigPtr;

	return XV_scenechange_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif
