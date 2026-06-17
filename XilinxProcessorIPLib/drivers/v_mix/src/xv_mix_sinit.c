// ==============================================================
// Copyright (c) 2015 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_mix_sinit.c
* @addtogroup v_mix Overview
*/

#ifndef __linux__

#include "xstatus.h"
#include "xv_mix.h"

extern XV_mix_Config XV_mix_ConfigTable[];

/**
 * Looks up the configuration for a XV_mix device.
 *
 * This function searches the XV_mix_ConfigTable for a configuration entry
 * that matches the specified BaseAddress. If a matching entry is found,
 * a pointer to its configuration structure is returned. If BaseAddress is
 * zero, the function returns the first entry in the table.
 *
 * @param	BaseAddress	The base address of the XV_mix device to look up.
 *
 * @return	A pointer to the configuration structure if found, or NULL if
 *          no matching entry exists.
 */
XV_mix_Config *XV_mix_LookupConfig(UINTPTR BaseAddress) {
	XV_mix_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; XV_mix_ConfigTable[Index].Name != NULL; Index++) {
		if ((XV_mix_ConfigTable[Index].BaseAddress == BaseAddress) ||
				!BaseAddress) {
			ConfigPtr = &XV_mix_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

/**
 * Initialize the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance to be initialized.
 * @param BaseAddress Base address of the device configuration.
 *
 * This function looks up the configuration for the device using the provided
 * base address, and initializes the XV_mix instance with the configuration
 * parameters. If the configuration is not found, the function sets the
 * IsReady flag to 0 and returns XST_DEVICE_NOT_FOUND.
 *
 * @return XST_SUCCESS if initialization is successful,
 *         XST_DEVICE_NOT_FOUND if the configuration is not found.
 */
int XV_mix_Initialize(XV_mix *InstancePtr, UINTPTR BaseAddress) {
	XV_mix_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XV_mix_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XV_mix_CfgInitialize(InstancePtr,
			            ConfigPtr,
			            ConfigPtr->BaseAddress);
}
#endif
